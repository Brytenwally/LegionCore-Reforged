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

#ifndef _ACCMGR_H
#define _ACCMGR_H

#include <string>
#include "Define.h"

enum class AccountOpResult : uint8
{
    AOR_OK,
    AOR_NAME_TOO_LONG,
    AOR_PASS_TOO_LONG,
    AOR_NAME_ALREADY_EXIST,
    AOR_NAME_NOT_EXIST,
    AOR_DB_INTERNAL_ERROR,
};

#define MAX_PASS_STR 16
#define MAX_EMAIL_STR 320

namespace AccountMgr
{
    TC_GAME_API AccountOpResult CreateAccount(std::string username, std::string password, bool async = true);
#ifndef _WEB_API
    TC_GAME_API AccountOpResult DeleteAccount(uint32 accountId);
#endif
    TC_GAME_API AccountOpResult ChangePassword(uint32 accountId, std::string newPassword, bool async = true);
    TC_GAME_API bool CheckPassword(uint32 accountId, std::string password);

    TC_GAME_API uint32 GetId(std::string username);
    TC_GAME_API uint32 GetSecurity(uint32 accountId);
    TC_GAME_API uint32 GetSecurity(uint32 accountId, int32 realmId);
    TC_GAME_API bool GetName(uint32 accountId, std::string& name);
    TC_GAME_API uint32 GetCharactersCount(uint32 accountId);

    TC_GAME_API bool IsPlayerAccount(uint32 gmlevel);
    TC_GAME_API bool IsModeratorAccount(uint32 gmlevel);
    TC_GAME_API bool IsGMAccount(uint32 gmlevel);
    TC_GAME_API bool IsAdminAccount(uint32 gmlevel);
    TC_GAME_API bool IsConsoleAccount(uint32 gmlevel);
};

#endif
