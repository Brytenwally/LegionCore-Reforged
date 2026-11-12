////////////////////////////////////////////
///                                      ///
///              DEMONCORE               ///
///    Copyright 2026 Titans-Project     ///
///         All Rights Reserved.         ///
///                                      ///
////////////////////////////////////////////

#include "LoginThrottle.h"
#include "Configuration/Config.h"
#include "Log.h"
#include <algorithm>
#include <boost/asio/ip/address.hpp>
#include <chrono>
#include <sstream>

namespace
{
    // Keeps memory bounded when somebody feeds the service endless account names
    constexpr size_t MAX_ENTRIES = 50000;
    constexpr int64 PURGE_INTERVAL = 60000;

    // Milliseconds, so a block of N seconds lasts N seconds and not between N-1 and N
    int64 Now()
    {
        return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
    }

    // The account name is what the client typed: no control characters in the log, and a bounded length
    std::string ForLog(std::string const& text)
    {
        std::string out = text.substr(0, 96);
        for (char& c : out)
            if (static_cast<unsigned char>(c) < 0x20 || c == 0x7F)
                c = '?';
        return out;
    }
}

LoginThrottle& LoginThrottle::Instance()
{
    static LoginThrottle instance;
    return instance;
}

void LoginThrottle::LoadConfig()
{
    std::vector<uint32> steps;
    std::istringstream stream(sConfigMgr->GetStringDefault("WrongPass.BanSteps", "300 600 900 1800 3600"));
    std::string token;
    while (stream >> token)
    {
        char* end = nullptr;
        unsigned long value = strtoul(token.c_str(), &end, 10);
        if (end && !*end && value > 0 && value <= 0x7FFFFFFF)
            steps.push_back(static_cast<uint32>(value));
        else
            TC_LOG_ERROR("server.rest", "WrongPass.BanSteps: ignoring invalid value \"%s\"", ForLog(token).c_str());
    }

    if (steps.empty())
        steps = { 300, 600, 900, 1800, 3600 };

    std::lock_guard<std::mutex> lock(_mutex);
    _steps = std::move(steps);
    _ipMaxCount = std::max<int32>(0, sConfigMgr->GetIntDefault("WrongPass.IP.MaxCount", 5));
    _accountMaxCount = std::max<int32>(0, sConfigMgr->GetIntDefault("WrongPass.Account.MaxCount", 10));
    _resetTime = int64(std::max<int32>(1, sConfigMgr->GetIntDefault("WrongPass.ResetTime", 86400))) * 1000;
    _exemptLocal = sConfigMgr->GetBoolDefault("WrongPass.ExemptLocal", true);
    _logging = sConfigMgr->GetBoolDefault("WrongPass.Logging", true);
}

bool LoginThrottle::IsExempt(std::string const& ip) const
{
    if (!_exemptLocal)
        return false;

    boost::system::error_code ec;
    boost::asio::ip::address address = boost::asio::ip::make_address(ip, ec);
    return !ec && address.is_loopback();
}

bool LoginThrottle::IsBlockedIn(EntryMap const& map, std::string const& key, int64 now) const
{
    auto itr = map.find(key);
    return itr != map.end() && itr->second.BlockedUntil > now;
}

uint32 LoginThrottle::AddFailure(EntryMap& map, std::string const& key, uint32 maxCount, int64 now)
{
    if (!maxCount)
        return 0;

    auto itr = map.find(key);
    if (itr == map.end())
    {
        if (map.size() >= MAX_ENTRIES)
        {
            PurgeMap(map, now);
            if (map.size() >= MAX_ENTRIES)
                return 0;
        }
        itr = map.emplace(key, Entry()).first;
    }

    Entry& entry = itr->second;

    // Failures that arrive while a block is already running (requests in flight) are not counted again
    if (entry.BlockedUntil > now)
        return 0;

    if (entry.LastActivity && now - std::max(entry.LastActivity, entry.BlockedUntil) >= _resetTime)
    {
        entry.Failures = 0;
        entry.Level = 0;
    }

    entry.LastActivity = now;
    if (++entry.Failures < maxCount)
        return 0;

    uint32 const duration = _steps[std::min<size_t>(entry.Level, _steps.size() - 1)];
    ++entry.Level;
    entry.Failures = 0;
    entry.BlockedUntil = now + int64(duration) * 1000;
    return duration;
}

bool LoginThrottle::IsBlocked(std::string const& ip, std::string const& account)
{
    std::lock_guard<std::mutex> lock(_mutex);
    if (IsExempt(ip))
        return false;

    int64 const now = Now();
    return IsBlockedIn(_ips, ip, now) || IsBlockedIn(_accounts, account, now);
}

void LoginThrottle::RegisterFailure(std::string const& ip, std::string const& account)
{
    std::lock_guard<std::mutex> lock(_mutex);
    if (IsExempt(ip))
        return;

    int64 const now = Now();
    uint32 const ipBlock = AddFailure(_ips, ip, _ipMaxCount, now);
    uint32 const accountBlock = AddFailure(_accounts, account, _accountMaxCount, now);

    if (!_logging)
        return;

    TC_LOG_INFO("server.rest", "[%s] Wrong password for account %s", ip.c_str(), ForLog(account).c_str());
    if (ipBlock)
        TC_LOG_INFO("server.rest", "[%s] IP blocked for %u seconds after %u failed logins (last account %s)", ip.c_str(), ipBlock, _ipMaxCount, ForLog(account).c_str());
    if (accountBlock)
        TC_LOG_INFO("server.rest", "[%s] Account %s blocked for %u seconds after %u failed logins", ip.c_str(), ForLog(account).c_str(), accountBlock, _accountMaxCount);
}

void LoginThrottle::RegisterSuccess(std::string const& ip, std::string const& account)
{
    std::lock_guard<std::mutex> lock(_mutex);
    if (IsExempt(ip))
        return;

    int64 const now = Now();

    // Only the ladder level goes back to zero for an IP: clearing its failure count too would let anyone holding
    // an account of their own reset the counter between guesses at another account
    auto ipItr = _ips.find(ip);
    if (ipItr != _ips.end())
    {
        ipItr->second.Level = 0;
        if (!ipItr->second.Failures && ipItr->second.BlockedUntil <= now)
            _ips.erase(ipItr);
    }

    auto accountItr = _accounts.find(account);
    if (accountItr != _accounts.end() && accountItr->second.BlockedUntil <= now)
        _accounts.erase(accountItr);
}

void LoginThrottle::PurgeMap(EntryMap& map, int64 now)
{
    for (auto itr = map.begin(); itr != map.end();)
    {
        Entry const& entry = itr->second;
        if (entry.BlockedUntil <= now && now - std::max(entry.LastActivity, entry.BlockedUntil) >= _resetTime)
            itr = map.erase(itr);
        else
            ++itr;
    }
}

void LoginThrottle::Purge()
{
    int64 const now = Now();

    std::lock_guard<std::mutex> lock(_mutex);
    if (now - _lastPurge < PURGE_INTERVAL)
        return;

    _lastPurge = now;
    PurgeMap(_ips, now);
    PurgeMap(_accounts, now);
}
