////////////////////////////////////////////
///                                      ///
///              DEMONCORE               ///
///    Copyright 2026 Titans-Project     ///
///         All Rights Reserved.         ///
///                                      ///
///      Based on TrinityCore code       ///
///   Copyright 2008-2014 TrinityCore    ///
///   <http://www.trinitycore.org/>      ///
///                                      ///
////////////////////////////////////////////

#include <memory>
#include <boost/asio/write.hpp>
#include <boost/asio/read_until.hpp>
#include <boost/array.hpp>
#include "RASession.h"
#include "AccountMgr.h"
#include "Log.h"
#include "DatabaseEnv.h"
#include "World.h"
#include "Config.h"

using boost::asio::ip::tcp;

void RASession::Start()
{
    boost::asio::socket_base::bytes_readable command(true);
    _socket.io_control(command);
    std::size_t bytes_readable = command.get();

    // Check if there are bytes available, if they are, then the client is requesting the negotiation
    if (bytes_readable > 0)
    {
        // Handle subnegotiation
        boost::array<char, 1024> buf;
        _socket.read_some(boost::asio::buffer(buf));

        // Send the end-of-negotiation packet
        uint8 const reply[2] = { 0xFF, 0xF0 };
        _socket.write_some(boost::asio::buffer(reply));
    }

    Send("Authentication Required\r\n");
    Send("Username: ");

    std::string username = ReadString();

    if (username.empty())
        return;

    TC_LOG_INFO("commands.ra", "Accepting RA connection from user %s (IP: %s)", username.c_str(), GetRemoteIpAddress().c_str());

    Send("Password: ");

    std::string password = ReadString();
    if (password.empty())
        return;

    if (!CheckAccessLevel(username) || !CheckPassword(username, password))
    {
        Send("Authentication failed\r\n");
        _socket.close();
        return;
    }

    TC_LOG_INFO("commands.ra", "User %s (IP: %s) authenticated correctly to RA", username.c_str(), GetRemoteIpAddress().c_str());

    // Authentication successful, send the motd
    for (std::string const& line : sWorld->GetMotd())
        Send(line.c_str());
    Send("\r\n");

    // Read commands
    for (;;)
    {
        Send("TC>");
        std::string command2 = ReadString();
        if (ProcessCommand(command2))
            break;
    }

    _socket.close();
}

int RASession::Send(const char* data)
{
    std::ostream os(&_writeBuffer);
    os << data;
    size_t written = _socket.send(_writeBuffer.data());
    _writeBuffer.consume(written);
    return written;
}

std::string RASession::ReadString()
{
    boost::system::error_code error;
    size_t read = boost::asio::read_until(_socket, _readBuffer, "\r\n", error);
    if (!read)
    {
        _socket.close();
        return "";
    }

    std::string line;
    std::istream is(&_readBuffer);
    std::getline(is, line);

    if (*line.rbegin() == '\r')
        line.erase(line.length() - 1);

    return line;
}

bool RASession::CheckAccessLevel(const std::string& user)
{
    std::string safeUser = user;

    Utf8ToUpperOnlyLatin(safeUser);

    LoginDatabasePreparedStatement* stmt = LoginDatabase.GetPreparedStatement(LOGIN_SEL_ACCOUNT_ACCESS);
    stmt->setString(0, safeUser);
    PreparedQueryResult result = LoginDatabase.Query(stmt);

    if (!result)
    {
        TC_LOG_INFO("commands.ra", "User %s does not exist in database", user.c_str());
        return false;
    }

    Field* fields = result->Fetch();

    if (fields[1].GetUInt8() < sConfigMgr->GetIntDefault("RA.MinLevel", 3))
    {
        TC_LOG_INFO("commands.ra", "User %s has no privilege to login", user.c_str());
        return false;
    }
    else if (fields[2].GetInt32() != -1)
    {
        TC_LOG_INFO("commands.ra", "User %s has to be assigned on all realms (with RealmID = '-1')", user.c_str());
        return false;
    }

    return true;
}

bool RASession::CheckPassword(const std::string& user, const std::string& pass)
{
    std::string safe_user = user;
    Utf8ToUpperOnlyLatin(safe_user);

    uint32 const accountId = AccountMgr::GetId(safe_user);
    if (!accountId || !AccountMgr::CheckPassword(accountId, pass))
    {
        TC_LOG_INFO("commands.ra", "Wrong password for user: %s", user.c_str());
        return false;
    }

    return true;
}

bool RASession::ProcessCommand(std::string& command)
{
    if (command.length() == 0)
        return true;

    // passwords stay out of the log, same rule as the in-game command log (case-insensitive here)
    std::string logged = command;
    {
        std::string lowered = command;
        std::transform(lowered.begin(), lowered.end(), lowered.begin(), ::tolower);
        if (lowered.find("password") != std::string::npos || lowered.find("account create") != std::string::npos)
        {
            size_t const firstSpace = command.find(' ');
            size_t const secondSpace = firstSpace == std::string::npos ? std::string::npos : command.find(' ', firstSpace + 1);
            logged = command.substr(0, secondSpace == std::string::npos ? firstSpace : secondSpace) + " ***";
        }
    }

    TC_LOG_INFO("commands.ra", "Received command: %s", logged.c_str());

    // handle quit, exit and logout commands to terminate connection
    if (command == "quit" || command == "exit" || command == "logout")
    {
        Send("Bye\r\n");
        return true;
    }

    // Obtain a new promise per command
    delete _commandExecuting;

    _commandExecuting = new std::promise<void>();

    CliCommandHolder* cmd = new CliCommandHolder(this, command.c_str(), &RASession::CommandPrint, &RASession::CommandFinished);
    sWorld->QueueCliCommand(cmd);

    // Wait for the command to finish
    _commandExecuting->get_future().wait();

    return false;
}

void RASession::CommandPrint(void* callbackArg, const char* text)
{
    if (!text || !*text)
        return;

    RASession* session = static_cast<RASession*>(callbackArg);
    session->Send(text);
}

void RASession::CommandFinished(void* callbackArg, bool /*success*/)
{
    RASession* session = static_cast<RASession*>(callbackArg);
    session->_commandExecuting->set_value();
}
