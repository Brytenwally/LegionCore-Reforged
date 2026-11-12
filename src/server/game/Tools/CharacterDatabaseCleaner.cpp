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

#include "Common.h"
#include "CharacterDatabaseCleaner.h"
#include "World.h"
#include "Database/DatabaseEnv.h"
#include "SpellMgr.h"
#include<string>
#include<vector>

void CharacterDatabaseCleaner::ResetCharacterDB()
{
    if (!sWorld->getBoolConfig(CONFIG_RESET_CHARACTER_DB))
        return;

    TC_LOG_WARN("server.loading", "ResetCharacterDB is enabled! Wiping ALL character data...");

    uint32 oldMSTime = getMSTime();

    static const char* tables[] =
    {
        // Account-level progression
        "account_achievement", "account_achievement_progress", "account_battlepet",
        "account_data", "account_flagged", "account_heirlooms",
        "account_item_favorite_appearances", "account_mounts", "account_progress",
        "account_toys", "account_transmogs", "account_tutorial",
        // Characters
        "characters", "character_account_data", "character_achievement",
        "character_achievement_progress", "character_action", "character_adventure_quest",
        "character_archaeology", "character_archaeology_finds", "character_army_training_info",
        "account_ban_attempts", "account_bot_detected", "character_bag_slot_flags",
        "character_aura", "character_aura_effect", "character_banned",
        "character_battleground_data", "character_battleground_random",
        "character_brackets_info", "character_brackets_info_season", "character_cuf_profiles", "character_currency",
        "character_custom_event_reapeter", "character_declinedname",
        "character_demon_invasion_progress", "character_equipmentsets",
        "character_garrison", "character_garrison_blueprints", "character_garrison_buildings",
        "character_garrison_follower_abilities", "character_garrison_followers",
        "character_garrison_missions", "character_garrison_shipment", "character_garrison_talents",
        "character_gifts", "character_glyphs", "character_homebind", "character_honor",
        "character_instance", "character_inventory", "character_kill",
        "character_lfg_cooldown", "character_loot_cooldown",
        "character_pet", "character_pet_declinedname", "character_pvp_talent",
        "character_queststatus", "character_queststatus_daily",
        "character_queststatus_objectives", "character_queststatus_rewarded",
        "character_queststatus_seasonal", "character_queststatus_weekly",
        "character_queststatus_world", "character_rates", "character_reputation",
        "character_reward", "character_skills", "character_social", "character_spell",
        "character_spell_cooldown", "character_stat_kill_creature", "character_talent",
        "character_transmog_outfits", "character_visuals", "character_void_storage",
        // Items
        "item_instance", "item_instance_artifact", "item_instance_artifact_powers",
        "item_instance_gems", "item_instance_modifiers", "item_instance_relics",
        "item_instance_transmog", "item_refund_instance", "item_soulbound_trade_data",
        // Guilds
        "guild", "guild_achievement", "guild_achievement_progress",
        "guild_bank_eventlog", "guild_bank_item", "guild_bank_right", "guild_bank_tab",
        "guild_challenges", "guild_eventlog", "guild_finder_applicant",
        "guild_finder_guild_settings", "guild_member", "guild_newslog", "guild_rank",
        // Mail & Auction
        "mail", "mail_items", "mailbox_queue",
        "auctionhouse", "blackmarket_auctions",
        // Pets
        "pet_aura", "pet_aura_effect", "pet_spell", "pet_spell_cooldown",
        // Social / Tickets / Calendar
        "calendar_events", "calendar_invites", "channels", "corpse",
        "gm_subsurveys", "gm_surveys", "gm_tickets",
        "petition", "petition_sign", "report_complaints",
        "log_faction_change", "log_rename",
        // Groups / LFG
        "`groups`", "group_instance", "group_member", "lfg_data",
        // Challenge
        "challenge", "challenge_key", "challenge_member", "challenge_oplote_loot",
        // World respawns
        "world_quest", "creature_respawn", "gameobject_respawn", "pool_quest_save",
        // Addons
        "addons"
    };

    for (auto table : tables)
        CharacterDatabase.DirectPExecute("TRUNCATE %s", table);

    // Reset character count in auth
    LoginDatabase.DirectExecute("UPDATE realmcharacters SET numchars = 0");

    TC_LOG_WARN("server.loading", ">> Character database wiped in %u ms. Set ResetCharacterDB = 0 to prevent next reset!", GetMSTimeDiffToNow(oldMSTime));
}

void CharacterDatabaseCleaner::CleanDatabase()
{
    // config to disable
    if (!sWorld->getBoolConfig(CONFIG_CLEAN_CHARACTER_DB))
        return;

    TC_LOG_INFO("misc", "Cleaning character database...");

    uint32 oldMSTime = getMSTime();

    // check flags which clean ups are necessary
    QueryResult result = CharacterDatabase.Query("SELECT value FROM worldstates WHERE entry = 20004");
    if (!result)
        return;

    uint32 flags = (*result)[0].GetUInt32();

    // clean up
    if (flags & CLEANING_FLAG_ACHIEVEMENT_PROGRESS)
        CleanAllAchievementProgress();

    if (flags & CLEANING_FLAG_SKILLS)
        CleanCharacterSkills();

    if (flags & CLEANING_FLAG_SPELLS)
        CleanCharacterSpell();

    if (flags & CLEANING_FLAG_QUESTSTATUS)
        CleanCharacterQuestStatus();

    // NOTE: In order to have persistentFlags be set in worldstates for the next cleanup,
    // you need to define them at least once in worldstates.
    flags &= sWorld->getIntConfig(CONFIG_PERSISTENT_CHARACTER_CLEAN_FLAGS);
    CharacterDatabase.DirectPExecute("UPDATE worldstates SET value = %u WHERE entry = 20004", flags);

    sWorld->SetCleaningFlags(flags);

    TC_LOG_INFO("server.loading", ">> Cleaned character database in %u ms", GetMSTimeDiffToNow(oldMSTime));

}

void CharacterDatabaseCleaner::CheckUnique(const char* column, const char* table, bool (*check)(uint32))
{
    QueryResult result = CharacterDatabase.PQuery("SELECT DISTINCT %s FROM %s", column, table);
    if (!result)
    {
        TC_LOG_INFO("misc", "Table %s is empty.", table);
        return;
    }

    bool found = false;
    std::ostringstream ss;
    do
    {
        Field* fields = result->Fetch();

        uint32 id = fields[0].GetUInt32();

        if (!check(id))
        {
            if (!found)
            {
                ss << "DELETE FROM " << table << " WHERE " << column << " IN (";
                found = true;
            }
            else
                ss << ',';

            ss << id;
        }
    }
    while (result->NextRow());

    if (found)
    {
        ss << ')';
        CharacterDatabase.Execute(ss.str().c_str());
    }
}

bool CharacterDatabaseCleaner::AchievementProgressCheck(uint32 criteria)
{
    return sCriteriaTreeStore.LookupEntry(criteria) != nullptr;
}

void CharacterDatabaseCleaner::CleanCharacterAchievementProgress()
{
    CheckUnique("criteria", "character_achievement_progress", &AchievementProgressCheck);
}

void CharacterDatabaseCleaner::CleanGuildAchievementProgress()
{
    CheckUnique("criteria", "guild_achievement_progress", &AchievementProgressCheck);
}

void CharacterDatabaseCleaner::CleanAccountAchievementProgress()
{
    CheckUnique("criteria", "account_achievement_progress", &AchievementProgressCheck);
}

void CharacterDatabaseCleaner::CleanAllAchievementProgress()
{
    CleanCharacterAchievementProgress();
    CleanAccountAchievementProgress();
    CleanGuildAchievementProgress();
}

bool CharacterDatabaseCleaner::SkillCheck(uint32 skill)
{
    return sSkillLineStore.LookupEntry(skill) != nullptr;
}

void CharacterDatabaseCleaner::CleanCharacterSkills()
{
    CheckUnique("skill", "character_skills", &SkillCheck);
}

bool CharacterDatabaseCleaner::SpellCheck(uint32 spell_id)
{
    return sSpellMgr->GetSpellInfo(spell_id);
}

void CharacterDatabaseCleaner::CleanCharacterSpell()
{
    CheckUnique("spell", "character_spell", &SpellCheck);
}

void CharacterDatabaseCleaner::CleanCharacterQuestStatus()
{
    CharacterDatabase.DirectExecute("DELETE FROM character_queststatus WHERE status = 0");
}
