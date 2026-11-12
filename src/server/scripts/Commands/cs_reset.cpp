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
Name: reset_commandscript
%Complete: 100
Comment: All reset related commands
Category: commandscripts
EndScriptData */

#include "ScriptMgr.h"
#include "Chat.h"

class reset_commandscript : public CommandScript
{
public:
    reset_commandscript() : CommandScript("reset_commandscript") { }

    std::vector<ChatCommand> GetCommands() const override
    {
        static std::vector<ChatCommand> resetCommandTable =
        {
            { "achievements",   SEC_ADMINISTRATOR,  true,  &HandleResetAchievementsCommand,     ""},
            { "honor",          SEC_ADMINISTRATOR,  true,  &HandleResetHonorCommand,            ""},
            { "spells",         SEC_ADMINISTRATOR,  true,  &HandleResetSpellsCommand,           ""},
            { "stats",          SEC_ADMINISTRATOR,  true,  &HandleResetStatsCommand,            ""},
            { "talents",        SEC_ADMINISTRATOR,  true,  &HandleResetTalentsCommand,          ""},
            { "all",            SEC_ADMINISTRATOR,  true,  &HandleResetAllCommand,              ""}
        };
        static std::vector<ChatCommand> commandTable =
        {
            { "reset",          SEC_ADMINISTRATOR,  true, NULL,                                 "", resetCommandTable }
        };
        return commandTable;
    }

    static bool HandleResetAchievementsCommand(ChatHandler* handler, char const* args)
    {
        Player* target;
        ObjectGuid targetGuid;
        std::string targetName;
        if (!handler->extractPlayerTarget((char*)args, &target, &targetGuid, &targetName))
            return false;

        if (target)
            target->GetAchievementMgr()->Reset();
        else if (ChatHandler::IsOnlineElsewhere(target, targetGuid))
        {
            if (!handler->PostToTarget(targetGuid, [](Player* player) { player->GetAchievementMgr()->Reset(); }, targetName.c_str()))
                return false;
        }
        else
            AchievementMgr<Player>::DeleteFromDB(targetGuid);

        return true;
    }

    static bool HandleResetHonorCommand(ChatHandler* handler, char const* args)
    {
        Player* target;
        ObjectGuid targetGuid;
        std::string targetName;
        if (!handler->extractOnlinePlayerTarget((char*)args, &target, &targetGuid, &targetName))
            return false;

        auto resetHonor = [](Player* player)
        {
            player->SetUInt32Value(PLAYER_FIELD_YESTERDAY_HONORABLE_KILLS, 0);
            player->SetUInt32Value(PLAYER_FIELD_LIFETIME_HONORABLE_KILLS, 0);
            player->UpdateAchievementCriteria(CRITERIA_TYPE_EARN_HONORABLE_KILL);
        };

        if (target)
            resetHonor(target);
        else if (!handler->PostToTarget(targetGuid, resetHonor, targetName.c_str()))
            return false;

        return true;
    }

    static bool HandleResetStatsOrLevelHelper(Player* player)
    {
        ChrClassesEntry const* classEntry = sChrClassesStore.LookupEntry(player->getClass());
        if (!classEntry)
        {
            TC_LOG_ERROR("misc", "Class %u not found in DBC (Wrong DBC files?)", player->getClass());
            return false;
        }

        uint8 powerType = classEntry->DisplayPower;

        // reset m_form if no aura
        if (!player->HasAuraType(SPELL_AURA_MOD_SHAPESHIFT))
            player->SetShapeshiftForm(FORM_NONE);

        player->SetFloatValue(UNIT_FIELD_BOUNDING_RADIUS, DEFAULT_WORLD_OBJECT_SIZE);
        player->SetFloatValue(UNIT_FIELD_COMBAT_REACH, DEFAULT_COMBAT_REACH);

        player->setFactionForRace(player->getRace());

        player->SetUInt32Value(UNIT_FIELD_BYTES_0, ((player->getRace()) | (player->getClass() << 8) | (player->getGender() << 16) | (powerType << 24)));

        // reset only if player not in some form;
        if (player->GetShapeshiftForm() == FORM_NONE)
            player->InitDisplayIds();

        player->SetByteValue(UNIT_FIELD_BYTES_2, 1, UNIT_BYTE2_FLAG_PVP);

        player->SetUInt32Value(UNIT_FIELD_FLAGS, UNIT_FLAG_PVP_ATTACKABLE);

        //-1 is default value
        player->SetInt32Value(PLAYER_FIELD_WATCHED_FACTION_INDEX, -1);

        //player->SetUInt32Value(PLAYER_FIELD_BYTES_1, 0xEEE00000);
        return true;
    }

    static bool HandleResetSpellsCommand(ChatHandler* handler, char const* args)
    {
        Player* target;
        ObjectGuid targetGuid;
        std::string targetName;
        if (!handler->extractPlayerTarget((char*)args, &target, &targetGuid, &targetName))
            return false;

        if (target)
        {
            target->resetSpells();

            ChatHandler(target).SendSysMessage(LANG_RESET_SPELLS);
            if (!handler->GetSession() || handler->GetSession()->GetPlayer() != target)
                handler->PSendSysMessage(LANG_RESET_SPELLS_ONLINE, handler->GetNameLink(target).c_str());
        }
        else if (ChatHandler::IsOnlineElsewhere(target, targetGuid))
        {
            handler->PSendSysMessage(LANG_RESET_SPELLS_ONLINE, handler->playerLink(targetName).c_str());
            return handler->PostToTarget(targetGuid, [](Player* player)
            {
                player->resetSpells();
                ChatHandler(player).SendSysMessage(LANG_RESET_SPELLS);
            });
        }
        else
        {
            CharacterDatabasePreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_ADD_AT_LOGIN_FLAG);
            stmt->setUInt16(0, uint16(AT_LOGIN_RESET_SPELLS));
            stmt->setUInt64(1, targetGuid.GetGUIDLow());
            CharacterDatabase.Execute(stmt);

            handler->PSendSysMessage(LANG_RESET_SPELLS_OFFLINE, targetName.c_str());
        }

        return true;
    }

    static bool HandleResetStatsCommand(ChatHandler* handler, char const* args)
    {
        Player* target;
        ObjectGuid targetGuid;
        std::string targetName;
        if (!handler->extractOnlinePlayerTarget((char*)args, &target, &targetGuid, &targetName))
            return false;

        auto resetStats = [](Player* player)
        {
            if (!HandleResetStatsOrLevelHelper(player))
                return false;

            player->InitRunes();
            player->InitStatsForLevel(true);
            player->InitTaxiNodesForLevel();
            player->InitTalentForLevel();
            return true;
        };

        if (target)
            return resetStats(target);

        return handler->PostToTarget(targetGuid, [resetStats](Player* player) { resetStats(player); }, targetName.c_str());
    }

    static bool HandleResetTalentsCommand(ChatHandler* handler, char const* args)
    {
        Player* target;
        ObjectGuid targetGuid;
        std::string targetName;
        if (!handler->extractPlayerTarget((char*)args, &target, &targetGuid, &targetName))
        {
            handler->SendSysMessage(LANG_NO_CHAR_SELECTED);
            handler->SetSentErrorMessage(true);
            return false;
        }

        if (target)
        {
            target->ResetTalentSpecialization();
            target->ResetTalents(true);
            target->SendTalentsInfoData(false);
            ChatHandler(target).SendSysMessage(LANG_RESET_TALENTS);
            if (!handler->GetSession() || handler->GetSession()->GetPlayer() != target)
                handler->PSendSysMessage(LANG_RESET_TALENTS_ONLINE, handler->GetNameLink(target).c_str());

            Pet* pet = target->GetPet();
            if (pet)
                target->SendTalentsInfoData(true);
            return true;
        }
        if (ChatHandler::IsOnlineElsewhere(target, targetGuid))
        {
            handler->PSendSysMessage(LANG_RESET_TALENTS_ONLINE, handler->playerLink(targetName).c_str());
            return handler->PostToTarget(targetGuid, [](Player* player)
            {
                player->ResetTalentSpecialization();
                player->ResetTalents(true);
                player->SendTalentsInfoData(false);
                ChatHandler(player).SendSysMessage(LANG_RESET_TALENTS);

                if (player->GetPet())
                    player->SendTalentsInfoData(true);
            });
        }
        if (targetGuid)
        {
            CharacterDatabasePreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_ADD_AT_LOGIN_FLAG);
            stmt->setUInt16(0, uint16(AT_LOGIN_RESET_TALENTS));
            stmt->setUInt64(1, targetGuid.GetGUIDLow());
            CharacterDatabase.Execute(stmt);

            std::string nameLink = handler->playerLink(targetName);
            handler->PSendSysMessage(LANG_RESET_TALENTS_OFFLINE, nameLink.c_str());
            return true;
        }
        handler->SendSysMessage(LANG_NO_CHAR_SELECTED);
        handler->SetSentErrorMessage(true);
        return false;
    }

    static bool HandleResetAllCommand(ChatHandler* handler, char const* args)
    {
        Player* target;
        ObjectGuid targetGuid;
        std::string targetName;
        if (!handler->extractPlayerTarget((char*) args, &target, &targetGuid, &targetName))
        {
            handler->SendSysMessage(LANG_NO_CHAR_SELECTED);
            handler->SetSentErrorMessage(true);
            return false;
        }

        if (target)
        {
            target->resetSpells();
            target->ResetTalentSpecialization();
            target->ResetTalents(true);
            target->SendTalentsInfoData(false);

            ChatHandler(target).SendSysMessage(LANG_RESET_TALENTS);
            if (!handler->GetSession() || handler->GetSession()->GetPlayer() != target)
                handler->PSendSysMessage(LANG_RESET_TALENTS_ONLINE, handler->GetNameLink(target).c_str());

            return true;
        }
        if (ChatHandler::IsOnlineElsewhere(target, targetGuid))
        {
            handler->PSendSysMessage(LANG_RESET_TALENTS_ONLINE, handler->playerLink(targetName).c_str());
            return handler->PostToTarget(targetGuid, [](Player* player)
            {
                player->resetSpells();
                player->ResetTalentSpecialization();
                player->ResetTalents(true);
                player->SendTalentsInfoData(false);
                ChatHandler(player).SendSysMessage(LANG_RESET_TALENTS);
            });
        }
        if (targetGuid)
        {
            CharacterDatabasePreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_ADD_AT_LOGIN_FLAG);
            stmt->setUInt16(0, uint16(AT_LOGIN_NONE | AT_LOGIN_RESET_SPELLS | AT_LOGIN_RESET_TALENTS));
            stmt->setUInt64(1, targetGuid.GetGUIDLow());
            CharacterDatabase.Execute(stmt);

            std::string nameLink = handler->playerLink(targetName);
            handler->PSendSysMessage(LANG_RESET_TALENTS_OFFLINE, nameLink.c_str());
            return true;
        }
        handler->SendSysMessage(LANG_NO_CHAR_SELECTED);
        handler->SetSentErrorMessage(true);
        return false;
    }
};

void AddSC_reset_commandscript()
{
    new reset_commandscript();
}
