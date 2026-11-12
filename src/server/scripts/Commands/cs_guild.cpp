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

/* ScriptData
Name: guild_commandscript
%Complete: 100
Comment: All guild related commands
Category: commandscripts
EndScriptData */

#include "ScriptMgr.h"
#include "Chat.h"
#include "Guild.h"
#include "GuildMgr.h"
#include "ObjectAccessor.h"

class guild_commandscript : public CommandScript
{
public:
    guild_commandscript() : CommandScript("guild_commandscript") { }

    std::vector<ChatCommand> GetCommands() const override
    {
        static std::vector<ChatCommand> guildCommandTable =
        {
            { "create",         SEC_GAMEMASTER,     true,  &HandleGuildCreateCommand,           ""},
            { "delete",         SEC_GAMEMASTER,     true,  &HandleGuildDeleteCommand,           ""},
            { "invite",         SEC_GAMEMASTER,     true,  &HandleGuildInviteCommand,           ""},
            { "uninvite",       SEC_GAMEMASTER,     true,  &HandleGuildUninviteCommand,         ""},
            { "rank",           SEC_GAMEMASTER,     true,  &HandleGuildRankCommand,             ""}
        };
        static std::vector<ChatCommand> commandTable =
        {
            { "guild",          SEC_ADMINISTRATOR,  true, NULL,                                 "", guildCommandTable }
        };
        return commandTable;
    }

    /** \brief GM command level 3 - Create a guild.
     *
     * This command allows a GM (level 3) to create a guild.
     *
     * The "args" parameter contains the name of the guild leader
     * and then the name of the guild.
     *
     */
    // guild id of a target that may stand on another map or be offline
    static ObjectGuid::LowType GetTargetGuildId(Player* target, ObjectGuid guid)
    {
        if (target)
            return target->GetGuildId();

        ObjectGuid::LowType guildId = 0;
        if (ObjectAccessor::WithPlayer(guid, [&guildId](Player* player) { guildId = player->GetGuildId(); }, ObjectAccessor::PlayerScope::InOrOutOfWorld))
            return guildId;

        return Player::GetGuildIdFromDB(guid);
    }

    static bool HandleGuildCreateCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        // if not guild name only (in "") then player name
        Player* target;
        ObjectGuid targetGuid;
        std::string targetName;
        if (!handler->extractOnlinePlayerTarget(*args != '"' ? (char*)args : NULL, &target, &targetGuid, &targetName))
            return false;

        char* tailStr = *args != '"' ? strtok(NULL, "") : (char*)args;
        if (!tailStr)
            return false;

        char* guildStr = handler->extractQuotedArg(tailStr);
        if (!guildStr)
            return false;

        std::string guildName = guildStr;

        if (GetTargetGuildId(target, targetGuid))
        {
            handler->SendSysMessage(LANG_PLAYER_IN_GUILD);
            return true;
        }

        if (!target)
        {
            if (sGuildMgr->GetGuildByName(guildName))
            {
                handler->SendSysMessage(LANG_GUILD_NOT_CREATED);
                handler->SetSentErrorMessage(true);
                return false;
            }

            // the leader's fields are set by his own thread, so the whole creation runs there
            return handler->PostToTarget(targetGuid, [guildName](Player* leader)
            {
                if (leader->GetGuildId())
                    return;

                Guild* guild = new Guild;
                if (!guild->Create(leader, guildName, true))
                {
                    delete guild;
                    return;
                }

                sGuildMgr->AddGuild(guild);
            }, targetName.c_str());
        }

        Guild* guild = new Guild;
        // the target may be another player, possibly on another map: then his guild fields are set in his own thread
        Player* self = handler->GetSession() ? handler->GetSession()->GetPlayer() : nullptr;
        if (!guild->Create(target, guildName, target == self))
        {
            delete guild;
            handler->SendSysMessage(LANG_GUILD_NOT_CREATED);
            handler->SetSentErrorMessage(true);
            return false;
        }

        sGuildMgr->AddGuild(guild);

        return true;
    }

    static bool HandleGuildDeleteCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        char* guildStr = handler->extractQuotedArg((char*)args);
        if (!guildStr)
            return false;

        std::string guildName = guildStr;

        Guild* targetGuild = sGuildMgr->GetGuildByName(guildName);
        if (!targetGuild)
            return false;

        targetGuild->Disband();

        return true;
    }

    static bool HandleGuildInviteCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        // if not guild name only (in "") then player name
        ObjectGuid targetGuid;
        if (!handler->extractPlayerGuidTarget(*args != '"' ? (char*)args : NULL, &targetGuid))
            return false;

        char* tailStr = *args != '"' ? strtok(NULL, "") : (char*)args;
        if (!tailStr)
            return false;

        char* guildStr = handler->extractQuotedArg(tailStr);
        if (!guildStr)
            return false;

        std::string guildName = guildStr;
        Guild* targetGuild = sGuildMgr->GetGuildByName(guildName);
        if (!targetGuild)
            return false;

        // player's guild membership checked in AddMember before add
        return targetGuild->AddMember(targetGuid);
    }

    static bool HandleGuildUninviteCommand(ChatHandler* handler, char const* args)
    {
        Player* target;
        ObjectGuid targetGuid;
        if (!handler->extractPlayerTarget((char*)args, &target, &targetGuid))
            return false;

        ObjectGuid::LowType guildId = GetTargetGuildId(target, targetGuid);
        if (!guildId)
            return false;

        Guild* targetGuild = sGuildMgr->GetGuildById(guildId);
        if (!targetGuild)
            return false;

        targetGuild->DeleteMember(targetGuid, false, true);
        return true;
    }

    static bool HandleGuildRankCommand(ChatHandler* handler, char const* args)
    {
        char* nameStr;
        char* rankStr;
        handler->extractOptFirstArg((char*)args, &nameStr, &rankStr);
        if (!rankStr)
            return false;

        Player* target;
        ObjectGuid targetGuid;
        std::string target_name;
        if (!handler->extractPlayerTarget(nameStr, &target, &targetGuid, &target_name))
            return false;

        ObjectGuid::LowType guildId = GetTargetGuildId(target, targetGuid);
        if (!guildId)
            return false;

        Guild* targetGuild = sGuildMgr->GetGuildById(guildId);
        if (!targetGuild)
            return false;

        uint8 newRank = uint8(atoi(rankStr));
        return targetGuild->ChangeMemberRank(targetGuid, newRank);
    }
};

void AddSC_guild_commandscript()
{
    new guild_commandscript();
}
