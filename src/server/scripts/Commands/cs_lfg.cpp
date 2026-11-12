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

#include "ScriptMgr.h"
#include "Chat.h"
#include "LFGMgr.h"
#include "Group.h"
#include "GroupMgr.h"
#include "ObjectAccessor.h"
#include "Player.h"

void GetPlayerInfo(ChatHandler* handler, ObjectGuid guid, char const* name, ObjectGuid gguid)
{
    uint32 queueId = sLFGMgr->GetQueueId(gguid);

    lfg::LfgDungeonSet dungeons = sLFGMgr->GetSelectedDungeons(guid, queueId);

    std::string const& state = lfg::GetStateString(sLFGMgr->GetState(guid, queueId));
    handler->PSendSysMessage(LANG_LFG_PLAYER_INFO, name,
        state.c_str(), uint8(dungeons.size()), lfg::ConcatenateDungeons(dungeons).c_str(),
        lfg::GetRolesString(sLFGMgr->GetRoles(guid, queueId)).c_str(), "");
}

void GetPlayerInfo(ChatHandler* handler, Player* target, ObjectGuid guid, char const* name)
{
    Group* grp = ChatHandler::FindTargetGroup(target, guid);
    if (!grp)
        return;

    GetPlayerInfo(handler, guid, name, grp->GetGUID());
}

class lfg_commandscript : public CommandScript
{
public:
    lfg_commandscript() : CommandScript("lfg_commandscript") { }

    std::vector<ChatCommand> GetCommands() const override
    {
        static std::vector<ChatCommand> lfgCommandTable =
        {
            {   "clean",  SEC_ADMINISTRATOR, false,      &HandleLfgCleanCommand, ""},
            {   "group",     SEC_GAMEMASTER, false,  &HandleLfgGroupInfoCommand, ""},
            {    "join",  SEC_ADMINISTRATOR, false,       &HandleLfgJoinCommand, ""},
            { "options",  SEC_ADMINISTRATOR, false,    &HandleLfgOptionsCommand, ""},
            {  "player",     SEC_GAMEMASTER, false, &HandleLfgPlayerInfoCommand, ""},
            {   "queue",     SEC_GAMEMASTER, false,  &HandleLfgQueueInfoCommand, ""}
        };

        static std::vector<ChatCommand> commandTable =
        {
            {       "lfg",   SEC_GAMEMASTER, false,                        NULL, "", lfgCommandTable }
        };
        return commandTable;
    }

    static bool HandleLfgJoinCommand(ChatHandler* handler, char const* args)
    {
        if (!sLFGMgr->isOptionEnabled(lfg::LFG_OPTION_ENABLE_DUNGEON_FINDER | lfg::LFG_OPTION_ENABLE_RAID_BROWSER))
            return false;

        Player* player = handler->getSelectedPlayer();
        if (!player)
            return false;

        Tokenizer tokens(args, ' ');
        if (tokens.size() < 2)
            return false;

        uint32 roles = 1;
        lfg::LfgDungeonSet dungeons;
        for (uint32 i = 0; i < tokens.size(); ++i)
        {
            if (i == 0)
                roles = std::atoi(tokens[i]);
            else
                dungeons.insert(std::atoi(tokens[i]));
        }

        sLFGMgr->JoinLfg(player, uint8(roles), dungeons);
        return true;
    }

    static bool HandleLfgPlayerInfoCommand(ChatHandler* handler, char const* args)
    {
        Player* target = NULL;
        ObjectGuid targetGuid;
        std::string playerName;
        if (!handler->extractOnlinePlayerTarget((char*)args, &target, &targetGuid, &playerName))
            return false;

        GetPlayerInfo(handler, target, targetGuid, playerName.c_str());
        return true;
    }

    static bool HandleLfgGroupInfoCommand(ChatHandler* handler, char const* args)
    {
        Player* target = NULL;
        ObjectGuid targetGuid;
        std::string playerName;
        if (!handler->extractOnlinePlayerTarget((char*)args, &target, &targetGuid, &playerName))
            return false;

        Group* grp = ChatHandler::FindTargetGroup(target, targetGuid);
        if (!grp)
        {
            handler->PSendSysMessage(LANG_LFG_NOT_IN_GROUP, playerName.c_str());
            return true;
        }

        ObjectGuid guid = grp->GetGUID();
        uint32 queueId = sLFGMgr->GetQueueId(guid);
        std::string const& state = lfg::GetStateString(sLFGMgr->GetState(guid, queueId));
        handler->PSendSysMessage(LANG_LFG_GROUP_INFO, grp->isLFGGroup(),
            state.c_str(), sLFGMgr->GetDungeon(guid));

        // members may stand on other maps: the name is read under the accessor lock, the LFG state outside it
        for (Group::MemberSlot const& slot : grp->GetMemberSlots())
        {
            ObjectGuid const memberGuid = slot.Guid;
            std::string memberName;
            if (ObjectAccessor::WithPlayer(memberGuid, [&memberName](Player* member) { memberName = member->GetName(); }))
                GetPlayerInfo(handler, memberGuid, memberName.c_str(), grp->GetGUID());
        }

        return true;
    }

    static bool HandleLfgOptionsCommand(ChatHandler* handler, char const* args)
    {
        int32 options = -1;
        if (char* str = strtok((char*)args, " "))
        {
            int32 tmp = atoi(str);
            if (tmp > -1)
                options = tmp;
        }

        if (options != -1)
        {
            sLFGMgr->SetOptions(options);
            handler->PSendSysMessage(LANG_LFG_OPTIONS_CHANGED);
        }
        handler->PSendSysMessage(LANG_LFG_OPTIONS, sLFGMgr->GetOptions());
        return true;
    }

    static bool HandleLfgQueueInfoCommand(ChatHandler* handler, char const* args)
    {
        handler->SendSysMessage(sLFGMgr->DumpQueueInfo(atoi(args) != 0).c_str());
        return true;
    }

    static bool HandleLfgCleanCommand(ChatHandler* handler, char const* /*args*/)
    {
        handler->PSendSysMessage(LANG_LFG_CLEAN);
        sLFGMgr->Clean();
        return true;
    }
};

void AddSC_lfg_commandscript()
{
    new lfg_commandscript();
}
