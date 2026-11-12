////////////////////////////////////////////
///                                      ///
///              DEMONCORE               ///
///    Copyright 2026 Titans-Project     ///
///         All Rights Reserved.         ///
///                                      ///
////////////////////////////////////////////

#ifndef LoginThrottle_h__
#define LoginThrottle_h__

#include "Define.h"
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

// In-memory progressive blocking of repeated wrong passwords, per IP and per account
class LoginThrottle
{
public:
    static LoginThrottle& Instance();

    void LoadConfig();

    bool IsBlocked(std::string const& ip, std::string const& account);
    void RegisterFailure(std::string const& ip, std::string const& account);
    void RegisterSuccess(std::string const& ip, std::string const& account);
    void Purge();

private:
    struct Entry
    {
        uint32 Failures = 0;
        uint32 Level = 0;
        int64 BlockedUntil = 0;
        int64 LastActivity = 0;
    };

    using EntryMap = std::unordered_map<std::string, Entry>;

    bool IsExempt(std::string const& ip) const;
    bool IsBlockedIn(EntryMap const& map, std::string const& key, int64 now) const;
    uint32 AddFailure(EntryMap& map, std::string const& key, uint32 maxCount, int64 now);
    void PurgeMap(EntryMap& map, int64 now);

    std::mutex _mutex;
    EntryMap _ips;
    EntryMap _accounts;
    uint32 _ipMaxCount = 5;
    uint32 _accountMaxCount = 10;
    std::vector<uint32> _steps;
    int64 _resetTime = 86400;
    bool _exemptLocal = true;
    bool _logging = true;
    int64 _lastPurge = 0;
};

#define sLoginThrottle LoginThrottle::Instance()

#endif // LoginThrottle_h__
