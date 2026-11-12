////////////////////////////////////////////
///                                      ///
///              DEMONCORE               ///
///    Copyright 2026 Titans-Project     ///
///         All Rights Reserved.         ///
///                                      ///
///      Based on TrinityCore code       ///
///   Copyright 2008-2012 TrinityCore    ///
///   <http://www.trinitycore.org/>      ///
///                                      ///
////////////////////////////////////////////

#include "AccountMgr.h"
#include "DatabaseEnv.h"
#ifndef _WEB_API
#include "ObjectAccessor.h"
#include "Player.h"
#endif
#include "Util.h"
#include "Argon2.h"
#include "PasswordHash.h"

namespace AccountMgr
{

AccountOpResult CreateAccount(std::string username, std::string password, bool async)
{
    if (utf8length(username) > MAX_EMAIL_STR)
        return AccountOpResult::AOR_NAME_TOO_LONG;          // username's too long

    if (utf8length(password) > MAX_PASS_STR)
        return AccountOpResult::AOR_PASS_TOO_LONG;

    Utf8ToUpperOnlyLatin(username);
    Utf8ToUpperOnlyLatin(password);

    if (GetId(username))
        return AccountOpResult::AOR_NAME_ALREADY_EXIST;     // username does already exist

    std::string const passwordHash = Argon2::Hash(password);
    if (passwordHash.empty())
        return AccountOpResult::AOR_DB_INTERNAL_ERROR;

    LoginDatabasePreparedStatement* stmt = LoginDatabase.GetPreparedStatement(LOGIN_INS_ACCOUNT);
    stmt->setString(0, username);
    stmt->setString(1, passwordHash);

    if (async)
        LoginDatabase.Execute(stmt);
    else
        LoginDatabase.DirectExecute(stmt);

    return AccountOpResult::AOR_OK;                         // everything's fine
}

#ifndef _WEB_API
AccountOpResult DeleteAccount(uint32 accountId)
{
    // Check if accounts exists
    LoginDatabasePreparedStatement* stmt = LoginDatabase.GetPreparedStatement(LOGIN_SEL_ACCOUNT_BY_ID);
    stmt->setUInt32(0, accountId);
    PreparedQueryResult result = LoginDatabase.Query(stmt);

    if (!result)
        return AccountOpResult::AOR_NAME_NOT_EXIST;

    // Obtain accounts characters
    CharacterDatabasePreparedStatement* stmt2 = CharacterDatabase.GetPreparedStatement(CHAR_SEL_CHARS_BY_ACCOUNT_ID);

    stmt2->setUInt32(0, accountId);

    result = CharacterDatabase.Query(stmt2);

    if (result)
    {
        do
        {
            ObjectGuid guid = ObjectGuid::Create<HighGuid::Player>((*result)[0].GetUInt32());

            // Kick if player is online. The logout runs in his own thread at the next session update, without saving
            ObjectAccessor::WithPlayer(guid, [](Player* p)
            {
                WorldSession* s = p->GetSession();
                s->DisableLogoutSave();
                s->KickPlayer();
            });

            Player::DeleteFromDB(guid, accountId, false);       // no need to update realm characters
        } while (result->NextRow());
    }

    // table realm specific but common for all characters of account for realm
    stmt2 = CharacterDatabase.GetPreparedStatement(CHAR_DEL_TUTORIALS);
    stmt2->setUInt32(0, accountId);
    CharacterDatabase.Execute(stmt2);

    stmt2 = CharacterDatabase.GetPreparedStatement(CHAR_DEL_ACCOUNT_DATA);
    stmt2->setUInt32(0, accountId);
    CharacterDatabase.Execute(stmt2);

    stmt2 = CharacterDatabase.GetPreparedStatement(CHAR_DEL_CHARACTER_BAN);
    stmt2->setUInt32(0, accountId);
    CharacterDatabase.Execute(stmt2);

    LoginDatabaseTransaction trans = LoginDatabase.BeginTransaction();

    stmt = LoginDatabase.GetPreparedStatement(LOGIN_DEL_ACCOUNT);
    stmt->setUInt32(0, accountId);
    trans->Append(stmt);

    stmt = LoginDatabase.GetPreparedStatement(LOGIN_DEL_ACCOUNT_ACCESS);
    stmt->setUInt32(0, accountId);
    trans->Append(stmt);

    stmt = LoginDatabase.GetPreparedStatement(LOGIN_DEL_REALM_CHARACTERS);
    stmt->setUInt32(0, accountId);
    trans->Append(stmt);

    stmt = LoginDatabase.GetPreparedStatement(LOGIN_DEL_ACCOUNT_BANNED);
    stmt->setUInt32(0, accountId);
    trans->Append(stmt);

    LoginDatabase.CommitTransaction(trans);

    return AccountOpResult::AOR_OK;
}
#endif

AccountOpResult ChangePassword(uint32 accountId, std::string newPassword, bool async)
{
    std::string username;

    if (!GetName(accountId, username))
        return AccountOpResult::AOR_NAME_NOT_EXIST;     // account doesn't exist

    if (utf8length(newPassword) > MAX_PASS_STR)
        return AccountOpResult::AOR_PASS_TOO_LONG;

    Utf8ToUpperOnlyLatin(newPassword);

    std::string const passwordHash = Argon2::Hash(newPassword);
    if (passwordHash.empty())
        return AccountOpResult::AOR_DB_INTERNAL_ERROR;

    LoginDatabasePreparedStatement* stmt = LoginDatabase.GetPreparedStatement(LOGIN_UPD_PASSWORD);

    stmt->setString(0, passwordHash);
    stmt->setUInt32(1, accountId);

    if (async)
        LoginDatabase.Execute(stmt);
    else
        LoginDatabase.DirectExecute(stmt);

    return AccountOpResult::AOR_OK;
}

uint32 GetId(std::string username)
{
    LoginDatabasePreparedStatement* stmt = LoginDatabase.GetPreparedStatement(LOGIN_GET_ACCOUNT_ID_BY_USERNAME);
    stmt->setString(0, username);
    PreparedQueryResult result = LoginDatabase.Query(stmt);

    return (result) ? (*result)[0].GetUInt32() : 0;
}

uint32 GetSecurity(uint32 accountId)
{
    LoginDatabasePreparedStatement* stmt = LoginDatabase.GetPreparedStatement(LOGIN_GET_ACCOUNT_ACCESS_GMLEVEL);
    stmt->setUInt32(0, accountId);
    PreparedQueryResult result = LoginDatabase.Query(stmt);

    return result ? (*result)[0].GetUInt8() : uint32(SEC_PLAYER);
}

uint32 GetSecurity(uint32 accountId, int32 realmId)
{
    LoginDatabasePreparedStatement* stmt = LoginDatabase.GetPreparedStatement(LOGIN_GET_GMLEVEL_BY_REALMID);
    stmt->setUInt32(0, accountId);
    stmt->setInt32(1, realmId);
    PreparedQueryResult result = LoginDatabase.Query(stmt);

    return result ? (*result)[0].GetUInt8() : uint32(SEC_PLAYER);
}

bool GetName(uint32 accountId, std::string& name)
{
    LoginDatabasePreparedStatement* stmt = LoginDatabase.GetPreparedStatement(LOGIN_GET_USERNAME_BY_ID);
    stmt->setUInt32(0, accountId);
    PreparedQueryResult result = LoginDatabase.Query(stmt);

    if (result)
    {
        name = (*result)[0].GetString();
        return true;
    }

    return false;
}

bool CheckPassword(uint32 accountId, std::string password)
{
    std::string username;

    if (!GetName(accountId, username))
        return false;

    Utf8ToUpperOnlyLatin(username);
    Utf8ToUpperOnlyLatin(password);

    LoginDatabasePreparedStatement* stmt = LoginDatabase.GetPreparedStatement(LOGIN_SEL_PASS_HASH_BY_ID);
    stmt->setUInt32(0, accountId);
    PreparedQueryResult result = LoginDatabase.Query(stmt);
    if (!result)
        return false;

    std::string const storedHash = (*result)[0].GetString();

    bool needsUpgrade = false;
    if (!PasswordHash::Verify(storedHash, username, password, &needsUpgrade))
        return false;

    if (needsUpgrade)
    {
        // Matching on the old hash keeps a password changed in the meantime from being overwritten
        std::string const upgraded = Argon2::Hash(password);
        if (!upgraded.empty())
        {
            stmt = LoginDatabase.GetPreparedStatement(LOGIN_UPD_PASS_HASH_UPGRADE);
            stmt->setString(0, upgraded);
            stmt->setUInt32(1, accountId);
            stmt->setString(2, storedHash);
            LoginDatabase.Execute(stmt);
        }
    }

    return true;
}

uint32 GetCharactersCount(uint32 accountId)
{
    // check character count
    CharacterDatabasePreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_SUM_CHARS);
    stmt->setUInt32(0, accountId);
    PreparedQueryResult result = CharacterDatabase.Query(stmt);

    return result ? (*result)[0].GetUInt64() : 0;
}

bool IsPlayerAccount(uint32 gmlevel)
{
    return gmlevel == SEC_PLAYER;
}

bool IsModeratorAccount(uint32 gmlevel)
{
    return gmlevel >= SEC_MODERATOR && gmlevel <= SEC_CONSOLE;
}

bool IsGMAccount(uint32 gmlevel)
{
    return gmlevel >= SEC_GAMEMASTER && gmlevel <= SEC_CONSOLE;
}

bool IsAdminAccount(uint32 gmlevel)
{
    return gmlevel >= SEC_ADMINISTRATOR && gmlevel <= SEC_CONSOLE;
}

bool IsConsoleAccount(uint32 gmlevel)
{
    return gmlevel == SEC_CONSOLE;
}

} // Namespace AccountMgr
