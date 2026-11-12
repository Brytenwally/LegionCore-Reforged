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

/*
playerdump.cpp if you get pdump
unknown errors try using DTT_CHAR_TABLE.
*/

#include "Common.h"
#include "PlayerDump.h"
#include "DatabaseEnv.h"
#include "UpdateFields.h"
#include "ObjectMgr.h"
#include "AccountMgr.h"
#include "GarrisonMgr.h"
#include "CharacterData.h"
#include "GlobalFunctional.h"

#define DUMP_TABLE_COUNT 31
struct DumpTable
{
    char const* name;
    DumpTableType type;
    char const* select;
};

//REMEMBER TO UPDATE DUMP_TABLE_COUNT!!!
static DumpTable dumpTables[DUMP_TABLE_COUNT] =
{
    { "characters",                       DTT_CHARACTER, "`guid`, `account`, `name`, `slot`, `race`, `class`, `gender`, `level`, `xp`, `money`, `skin`, `face`, `hairStyle`, `hairColor`, `tattoo`, `horn`, `inventorySlots`, `blindfold`, `facialStyle`, `bankSlots`, `drunk`, `playerFlags`, `playerFlagsEx`, `position_x`, `position_y`, `position_z`, `map`, `instance_id`, `dungeonDifficulty`, `raidDifficulty`, `legacyRaidDifficulty`, `orientation`, `taximask`, `online`, `cinematic`, `totaltime`, `leveltime`, `created_time`, `logout_time`, `is_logout_resting`, `rest_bonus`, `trans_x`, `trans_y`, `trans_z`, `trans_o`, `transguid`, `extra_flags`, `summonedPetNumber`, `at_login`, `zone`, `death_expire_time`, `taxi_path`, `totalKills`, `todayKills`, `yesterdayKills`, `killPoints`, `chosenTitle`, `watchedFaction`, `lfgBonusFaction`, `health`, `mana`, `latency`, `activespec`, `specialization`, `lootspecialization`, `exploredZones`, `equipmentCache`, `knownTitles`, `actionBars`, `grantableLevels`, `deleteInfos_Account`, `deleteInfos_Name`, `deleteDate`, `LastCharacterUndelete`"},
    { "character_achievement",            DTT_CHAR_TABLE, "`guid`, `achievement`, `date`"},
    { "character_achievement_progress",   DTT_CHAR_TABLE, "`guid`, `criteria`, `counter`, `date`"},
    { "character_action",                 DTT_CHAR_TABLE, "`guid`, `spec`, `button`, `action`, `type`"},
    { "character_currency",               DTT_CHAR_TABLE, "`guid`, `currency`, `total_count`, `week_count`, `season_total`, `flags`, `curentcap`"},
    { "character_declinedname",           DTT_CHAR_TABLE, "`guid`, `genitive`, `dative`, `accusative`, `instrumental`, `prepositional`"},
    { "character_equipmentsets",          DTT_EQSET_TABLE,"`guid`, `setguid`, `setindex`, `name`, `iconname`, `ignore_mask`, `item0`, `item1`, `item2`, `item3`, `item4`, `item5`, `item6`, `item7`, `item8`, `item9`, `item10`, `item11`, `item12`, `item13`, `item14`, `item15`, `item16`, `item17`, `item18`"},
    { "character_homebind",               DTT_CHAR_TABLE, "`guid`, `mapId`, `zoneId`, `posX`, `posY`, `posZ`"},
    { "character_inventory",              DTT_INVENTORY,  "`guid`, `bag`, `slot`, `item`"},
    { "character_pet",                    DTT_PET,        "`id`, `entry`, `owner`, `modelid`, `CreatedBySpell`, `PetType`, `level`, `exp`, `Reactstate`, `name`, `renamed`, `curhealth`, `curmana`, `savetime`, `abdata`, `specialization`"},
    { "character_pet_declinedname",       DTT_PET,        "`id`, `owner`, `genitive`, `dative`, `accusative`, `instrumental`, `prepositional`"},
    { "character_queststatus",            DTT_CHAR_TABLE, "`guid`, `account`, `quest`, `status`, `timer`"},
    { "character_queststatus_rewarded",   DTT_CHAR_TABLE, "`guid`, `account`, `quest`"},
    { "character_queststatus_objectives", DTT_CHAR_TABLE, "`guid`, `account`, `quest`, `objective`, `data`"},
    { "character_reputation",             DTT_CHAR_TABLE, "`guid`, `faction`, `standing`, `flags`"},
    { "character_skills",                 DTT_CHAR_TABLE, "`guid`, `skill`, `value`, `max`"},
    { "character_spell",                  DTT_CHAR_TABLE, "`guid`, `spell`, `active`, `disabled`"},
    { "character_spell_cooldown",         DTT_CHAR_TABLE, "`guid`, `spell`, `item`, `time`"},
    { "character_talent",                 DTT_CHAR_TABLE, "`guid`, `talent`, `spec`"},
    { "character_void_storage",           DTT_VS_TABLE,   "`itemId`, `playerGuid`, `itemEntry`, `slot`, `creatorGuid`, `randomProperty`, `suffixFactor`, `randomPropertyType`, `itemGuid`"},
    { "mail",                             DTT_MAIL,       "`id`, `messageType`, `stationery`, `mailTemplateId`, `sender`, `receiver`, `subject`, `body`, `has_items`, `expire_time`, `deliver_time`, `money`, `cod`, `checked`"},
    { "mail_items",                       DTT_MAIL_ITEM,  "`mail_id`, `item_guid`, `receiver`"},
    { "item_instance",                    DTT_ITEM,       "`guid`, `itemEntry`, `owner_guid`, `creatorGuid`, `giftCreatorGuid`, `count`, `duration`, `charges`, `flags`, `enchantments`, `randomPropertyType`, `randomPropertyId`, `durability`, `playedTime`, `text`, `upgradeId`, `battlePetSpeciesId`, `battlePetBreedData`, `battlePetLevel`, `battlePetDisplayId`, `bonusListIDs`, `itemLevel`, `dungeonEncounterID`, `contextID`, `createdTime`"},
    { "pet_spell",                        DTT_PET_TABLE,  "`guid`, `spell`, `active`"},
    { "pet_spell_cooldown",               DTT_PET_TABLE,  "`guid`, `spell`, `time`"},
    { "character_archaeology",            DTT_CHAR_TABLE, "`guid`, `sites`, `counts`, `projects`"},
    { "character_archaeology_finds",      DTT_CHAR_TABLE, "`guid`, `id`, `count`, `date`"},
    //{ "character_garrison",               DTT_CHAR_TABLE, "`guid`, `siteLevelId`, `followerActivationsRemainingToday`, `lastResTaken`, `_MissionGen`"},
    //{ "character_garrison_blueprints",    DTT_CHAR_TABLE, "`guid`, `buildingId`"},
    //{ "character_garrison_buildings",     DTT_CHAR_TABLE, "`guid`, `plotInstanceId`, `buildingId`, `timeBuilt`, `data`, `active`"},
    //{ "character_garrison_followers",     DTT_CHAR_VS_FOLLOW, "`dbId`, `guid`, `followerId`, `quality`, `level`, `itemLevelWeapon`, `itemLevelArmor`, `xp`, `currentBuilding`, `currentMission`, `status`"},
    //{ "character_garrison_shipment",      DTT_CHAR_VS_SHIP, "`dbId`, `guid`, `shipmentID`, `orderTime`"},
    { "character_transmog_outfits",       DTT_CHAR_TRANSMOG, "`guid`, `setguid`, `setindex`, `name`, `iconname`, `ignore_mask`, `appearance0`, `appearance1`, `appearance2`, `appearance3`, `appearance4`, `appearance5`, `appearance6`, `appearance7`, `appearance8`, `appearance9`, `appearance10`, `appearance11`, `appearance12`, `appearance13`, `appearance14`, `appearance15`, `appearance16`, `appearance17`, `appearance18`, `mainHandEnchant`, `offHandEnchant`"},
    //{ "item_instance_artifact",           DTT_ARTI_TABLE, "`itemGuid`, `xp`, `artifactAppearanceId`, `itemEntry`, `tier`, `char_guid`, `totalrank`"},
    //{ "item_instance_artifact_powers",    DTT_ARTI_TABLE, "`itemGuid`, `char_guid`, `artifactPowerId`, `purchasedRank`, `itemEntry`"},
    { "item_instance_gems",               DTT_ITEM_TABLE, "`itemGuid`, `gemItemId1`, `gemBonuses1`, `gemContext1`, `gemScalingLevel1`, `gemItemId2`, `gemBonuses2`, `gemScalingLevel2`, `gemContext2`, `gemItemId3`, `gemBonuses3`, `gemContext3`, `gemScalingLevel3`"},
    { "item_instance_modifiers",          DTT_ITEM_TABLE, "`itemGuid`, `fixedScalingLevel`, `artifactKnowledgeLevel`"},
    { "item_instance_transmog",           DTT_ITEM_TABLE, "`itemGuid`, `itemModifiedAppearanceAllSpecs`, `itemModifiedAppearanceSpec1`, `itemModifiedAppearanceSpec2`, `itemModifiedAppearanceSpec3`, `itemModifiedAppearanceSpec4`, `spellItemEnchantmentAllSpecs`, `spellItemEnchantmentSpec1`, `spellItemEnchantmentSpec2`, `spellItemEnchantmentSpec3`, `spellItemEnchantmentSpec4`"}
};

// Low level functions
bool FindNth(std::string &str, int n, std::string::size_type &s, std::string::size_type &e)
{
    s = str.find("VALUES ('");
    if (s == std::string::npos) return false;
    s += 9;

    do
    {
        e = str.find('\'', s);
        if (e == std::string::npos) return false;
    } while (str[e-1] == '\\');

    for (int i = 1; i < n; ++i)
    {
        do
        {
            s = e+4;
            e = str.find('\'', s);
            if (e == std::string::npos) return false;
        } while (str[e-1] == '\\');
    }
    return true;
}

std::string GetTableName(std::string &str)
{
    std::string::size_type const s = 13;
    if (str.compare(0, s, "INSERT INTO `") != 0)
        return "";

    std::string::size_type e = str.find('`', s);
    if (e == std::string::npos)
        return "";

    return str.substr(s, e - s);
}

bool ChangeNth(std::string &str, int n, const char *with, bool insert = false, bool nonzero = false)
{
    std::string::size_type s, e;
    if (!FindNth(str, n, s, e))
        return false;

    if (nonzero && str.substr(s, e-s) == "0")
        return true;                                        // not an error
    if (!insert)
        str.replace(s, e-s, with);
    else
        str.insert(s, with);

    return true;
}

std::string GetNth(std::string &str, int n)
{
    std::string::size_type s, e;
    if (!FindNth(str, n, s, e))
        return "";

    return str.substr(s, e-s);
}

// lines are executed as raw SQL: only the single quoted-values INSERT written by CreateDumpString is accepted
static bool IsPlainInsertLine(std::string const& line)
{
    if (line.compare(0, 13, "INSERT INTO `") != 0)
        return false;

    std::string::size_type pos = line.find(") VALUES (");
    if (pos == std::string::npos)
        return false;

    for (std::string::size_type i = 13; i < pos; ++i)
    {
        unsigned char c = line[i];
        if (!isalnum(c) && c != '_' && c != '`' && c != ',' && c != ' ' && c != '(')
            return false;
    }

    pos += 10;
    for (;;)
    {
        if (pos >= line.size() || line[pos] != '\'')
            return false;

        for (++pos; pos < line.size() && line[pos] != '\''; ++pos)
            if (line[pos] == '\\')
                ++pos;

        if (pos >= line.size())
            return false;

        ++pos;
        if (line.compare(pos, 2, ", ") == 0)
            pos += 2;
        else
            break;
    }

    if (line.compare(pos, 2, ");") != 0)
        return false;

    return line.find_first_not_of(" \t\r\n", pos + 2) == std::string::npos;
}

uint64 RegisterNewGuid(uint64 oldGuid, std::map<uint64, uint64> &guidMap, uint64 hiGuid)
{
    std::map<uint64, uint64>::const_iterator itr = guidMap.find(oldGuid);
    if (itr != guidMap.end())
        return itr->second;

    uint64 newguid = hiGuid + guidMap.size();
    guidMap[oldGuid] = newguid;
    return newguid;
}

bool ChangeGuid(std::string &str, int n, std::map<uint64, uint64> &guidMap, uint32 hiGuid, bool nonzero = false)
{
    char chritem[21];
    ObjectGuid::LowType oldGuid = strtoull(GetNth(str, n).c_str(), nullptr, 10);
    if (nonzero && !oldGuid)
        return true;                                        // not an error

    uint64 newGuid = RegisterNewGuid(oldGuid, guidMap, hiGuid);
    snprintf(chritem, 21, UI64FMTD, newGuid);

    return ChangeNth(str, n, chritem, false, nonzero);
}

std::string CreateDumpString(char const* tableName, QueryResult result, char const* tableSelect)
{
    if (!tableName || !result) return "";
    std::ostringstream ss;
    ss << "INSERT INTO `" << tableName << "` (" << tableSelect << ") VALUES (";
    Field* fields = result->Fetch();
    for (uint32 i = 0; i < result->GetFieldCount(); ++i)
    {
        if (i == 0) ss << '\'';
        else ss << ", '";

        std::string s = fields[i].GetString();
        CharacterDatabase.EscapeString(s);
        ss << s;

        ss << '\'';
    }
    ss << ");\n";
    return ss.str();
}

std::string PlayerDumpWriter::GenerateWhereStr(char const* field, ObjectGuid::LowType guid)
{
    std::ostringstream wherestr;
    wherestr << field << " = '" << guid << '\'';
    return wherestr.str();
}

std::string PlayerDumpWriter::GenerateWhereStr(char const* field, GUIDs const& guids, GUIDs::const_iterator& itr, size_t maxLen)
{
    std::ostringstream wherestr;
    wherestr << field << " IN ('";
    for (; itr != guids.end(); ++itr)
    {
        wherestr << *itr;

        if (size_t(wherestr.tellp()) > maxLen)               // near to max query
        {
            ++itr;
            break;
        }

        GUIDs::const_iterator itr2 = itr;
        if (++itr2 != guids.end())
            wherestr << "', '";
    }
    wherestr << "')";
    return wherestr.str();
}

void StoreGUID(QueryResult result, uint32 field, std::set<uint32>& guids)
{
    Field* fields = result->Fetch();
    ObjectGuid::LowType guid = fields[field].GetUInt64();
    if (guid)
        guids.insert(guid);
}

// Writing - High-level functions
bool PlayerDumpWriter::DumpTable(std::string& dump, ObjectGuid::LowType guid, char const*tableFrom, char const*tableTo, DumpTableType type, char const*tableSelect)
{
    GUIDs const* guids = nullptr;
    char const* fieldname = nullptr;

    switch (type)
    {
        case DTT_ITEM:          fieldname = "guid";      guids = &items; break;
        case DTT_ITEM_GIFT:     fieldname = "item_guid"; guids = &items; break;
        case DTT_ITEM_TABLE:    fieldname = "itemGuid";  guids = &items; break;
        case DTT_PET:           fieldname = "owner";                     break;
        case DTT_PET_TABLE:     fieldname = "guid";      guids = &pets;  break;
        case DTT_MAIL:          fieldname = "receiver";                  break;
        case DTT_MAIL_ITEM:     fieldname = "mail_id";   guids = &mails; break;
        case DTT_VS_TABLE:      fieldname = "playerGuid";                break;
        case DTT_CHAR_VS_FOLLOW:fieldname = "dbId";                      break;
        case DTT_FOLLOW:        fieldname = "dbId";  guids = &follower;  break;
        default:                fieldname = "guid";                      break;
    }

    // for guid set stop if set is empty
    if (guids && guids->empty())
        return true;                                        // nothing to do

    // PQuery truncates at MAX_QUERY_LEN: what the SELECT itself takes is not available to the IN list
    size_t const maxWhereLen = MAX_QUERY_LEN - 50 - strlen(tableSelect) - strlen(tableFrom) - 32;

    // setup for guids case start position
    GUIDs::const_iterator guids_itr;
    if (guids)
        guids_itr = guids->begin();

    do
    {
        std::string wherestr;

        if (guids)                                           // set case, get next guids string
            wherestr = GenerateWhereStr(fieldname, *guids, guids_itr, maxWhereLen);
        else                                                // not set case, get single guid string
            wherestr = GenerateWhereStr(fieldname, guid);

        QueryResult result = CharacterDatabase.PQuery("SELECT %s FROM %s WHERE %s", tableSelect, tableFrom, wherestr.c_str());
        if (!result)
        {
            if (type == DTT_CHARACTER)
                return false;                               // no such character

            continue;                                       // an empty chunk must not drop the following ones
        }

        do
        {
            // collect guids
            switch (type)
            {
                case DTT_INVENTORY:
                    StoreGUID(result, 3, items);                // item guid collection (character_inventory.item)
                    break;
                case DTT_PET:
                    StoreGUID(result, 0, pets);                 // pet petnumber collection (character_pet.id)
                    break;
                case DTT_MAIL:
                    StoreGUID(result, 0, mails);                // mail id collection (mail.id)
                    break;
                case DTT_MAIL_ITEM:
                    StoreGUID(result, 1, items);                // item guid collection (mail_items.item_guid)
                    break;
                case DTT_FOLLOW:
                    StoreGUID(result, 0, follower);                // item guid collection (mail_items.item_guid)
                    break;
                default:
                    break;
            }

            dump += CreateDumpString(tableTo, result, tableSelect);
        }
        while (result->NextRow());
    }
    while (guids && guids_itr != guids->end());              // not set case iterate single time, set case iterate for all guids
    return true;
}

bool PlayerDumpWriter::GetDump(ObjectGuid::LowType guid, std::string &dump)
{
    dump = "";

    //dump += "IMPORTANT NOTE: THIS DUMPFILE IS MADE FOR USE WITH THE 'PDUMP' COMMAND ONLY - EITHER THROUGH INGAME CHAT OR ON CONSOLE!;\n";
    //dump += "IMPORTANT NOTE: DO NOT apply it directly - it will irreversibly DAMAGE and CORRUPT your database! You have been warned!;\n\n";

    for (int i = 0; i < DUMP_TABLE_COUNT; ++i)
        if (!DumpTable(dump, guid, dumpTables[i].name, dumpTables[i].name, dumpTables[i].type, dumpTables[i].select))
            return false;

    // TODO: Add instance/group..
    // TODO: Add a dump level option to skip some non-important tables

    return true;
}

DumpReturn PlayerDumpWriter::WriteDump(std::string const& file, ObjectGuid::LowType guid)
{
    FILE *fout = fopen(file.c_str(), "w");
    if (!fout)
        return DUMP_FILE_OPEN_ERROR;

    DumpReturn ret = DUMP_SUCCESS;
    std::string dump;
    if (!GetDump(guid, dump))
        ret = DUMP_CHARACTER_DELETED;

    fprintf(fout, "%s\n", dump.c_str());
    fclose(fout);
    return ret;
}

DumpReturn PlayerDumpWriter::WriteDump(ObjectGuid::LowType guid, std::string& dump)
{
    if (!GetDump(guid, dump))
        return DUMP_CHARACTER_DELETED;
    return DUMP_SUCCESS;
}

// Reading - High-level functions
#define ROLLBACK(DR) {fclose(fin); return (DR);}

void fixNULLfields(std::string &line)
{
    std::string nullString("'NULL'");
    size_t pos = line.find(nullString);
    while (pos != std::string::npos)
    {
        line.replace(pos, nullString.length(), "NULL");
        pos = line.find(nullString);
    }
}

DumpReturn PlayerDumpReader::LoadDump(std::string const& file, uint32 account, std::string name, ObjectGuid::LowType guid)
{
    uint32 charcount = AccountMgr::GetCharactersCount(account);
    if (charcount >= sWorld->getIntConfig(CONFIG_CHARACTERS_PER_REALM))
        return DUMP_TOO_MANY_CHARS;

    FILE* fin = fopen(file.c_str(), "r");
    if (!fin)
        return DUMP_FILE_OPEN_ERROR;

    char newguid[20], chraccount[20], newpetid[20], currpetid[20], lastfollowerid[20];

    // make sure the same guid doesn't already exist and is safe to use
    bool incHighest = true;
    if (guid != 0 && guid < sObjectMgr->GetGenerator<HighGuid::Player>()->GetNextAfterMaxUsed())
    {
        CharacterDatabasePreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_CHECK_GUID);
        stmt->setUInt32(0, guid);
        PreparedQueryResult result = CharacterDatabase.Query(stmt);
        if (result)
            guid = sObjectMgr->GetGenerator<HighGuid::Player>()->GetNextAfterMaxUsed();                     // use first free if exists
        else incHighest = false;
    }
    else
        guid = sObjectMgr->GetGenerator<HighGuid::Player>()->GetNextAfterMaxUsed();

    // normalize the name if specified and check if it exists
    if (!normalizePlayerName(name))
        name = "";

    if (sCharacterDataStore->CheckPlayerName(name, sWorld->GetDefaultDbcLocale(), true) == CHAR_NAME_SUCCESS)
    {
        CharacterDatabasePreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_CHECK_NAME);
        stmt->setString(0, name);
        PreparedQueryResult result = CharacterDatabase.Query(stmt);

        if (result)
            name = "";                                      // use the one from the dump
    }
    else
        name = "";

    // name encoded or empty

    snprintf(newguid, 20, "%llu", guid);
    snprintf(chraccount, 20, "%u", account);
    snprintf(newpetid, 20, "%u", sObjectMgr->GeneratePetNumber());
    snprintf(lastfollowerid, 20, "%llu", sGarrisonMgr.GenerateFollowerDbId());

    std::map<uint64, uint64> items;
    std::map<uint64, uint64> mails;
    char buf[32000] = "";

    typedef std::map<uint32, uint32> PetIds;                // old->new petid relation
    typedef PetIds::value_type PetIdsPair;
    PetIds petids;

    uint8 gender = GENDER_NONE;
    uint8 race = RACE_NONE;
    uint8 playerClass = 0;
    uint8 level = 0;

    CharacterDatabaseTransaction trans = CharacterDatabase.BeginTransaction();
    while (!feof(fin))
    {
        if (!fgets(buf, 32000, fin))
        {
            if (feof(fin))
                break;
            ROLLBACK(DUMP_FILE_BROKEN);
        }

        std::string line; line.assign(buf);

        // skip empty strings
        size_t nw_pos = line.find_first_not_of(" \t\n\r\7");
        if (nw_pos == std::string::npos)
            continue;

        // skip logfile-side dump start notice, the important notes and dump end notices
        if ((line.substr(nw_pos, 16) == "== START DUMP ==") ||
            (line.substr(nw_pos, 15) == "IMPORTANT NOTE:") ||
            (line.substr(nw_pos, 14) == "== END DUMP =="))
            continue;

        if (!IsPlainInsertLine(line))
        {
            TC_LOG_ERROR("network", "LoadPlayerDump: line is not a plain INSERT: '%s'!", line.c_str());
            ROLLBACK(DUMP_FILE_BROKEN);
        }

        // determine table name and load type
        std::string tn = GetTableName(line);
        if (tn.empty())
        {
            TC_LOG_ERROR("network", "LoadPlayerDump: Can't extract table name from line: '%s'!", line.c_str());
            ROLLBACK(DUMP_FILE_BROKEN);
        }

        DumpTableType type = DumpTableType(0);
        uint8 i;
        for (i = 0; i < DUMP_TABLE_COUNT; ++i)
        {
            if (tn == dumpTables[i].name)
            {
                type = dumpTables[i].type;
                break;
            }
        }

        if (i == DUMP_TABLE_COUNT)
        {
            TC_LOG_ERROR("network", "LoadPlayerDump: Unknown table: '%s'!", tn.c_str());
            ROLLBACK(DUMP_FILE_BROKEN);
        }

        // change the data to server values
        switch (type)
        {
            case DTT_CHARACTER:
            {
                if (!ChangeNth(line, 1, newguid))           // characters.guid update
                    ROLLBACK(DUMP_FILE_BROKEN);

                if (!ChangeNth(line, 2, chraccount))        // characters.account update
                    ROLLBACK(DUMP_FILE_BROKEN);

                race = uint8(atol(GetNth(line, 5).c_str()));
                playerClass = uint8(atol(GetNth(line, 6).c_str()));
                gender = uint8(atol(GetNth(line, 7).c_str()));
                level = uint8(atol(GetNth(line, 8).c_str()));
                if (name == "")
                {
                    // check if the original name already exists
                    name = GetNth(line, 3);

                    CharacterDatabasePreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_CHECK_NAME);
                    stmt->setString(0, name);
                    PreparedQueryResult result = CharacterDatabase.Query(stmt);

                    if (result)
                        if (!ChangeNth(line, 49, "1"))       // characters.at_login set to "rename on login"
                            ROLLBACK(DUMP_FILE_BROKEN);
                }
                else if (!ChangeNth(line, 3, name.c_str())) // characters.name
                    ROLLBACK(DUMP_FILE_BROKEN);
                break;
            }
            case DTT_CHAR_TABLE:
            {
                if (!ChangeNth(line, 1, newguid))           // character_*.guid update
                    ROLLBACK(DUMP_FILE_BROKEN);

                if (tn == "character_queststatus" || tn == "character_queststatus_rewarded" || tn == "character_queststatus_objectives")
                    if (!ChangeNth(line, 2, chraccount))    // account column follows the new owner
                        ROLLBACK(DUMP_FILE_BROKEN);
                break;
            }
            case DTT_EQSET_TABLE:
            {
                if (!ChangeNth(line, 1, newguid))
                    ROLLBACK(DUMP_FILE_BROKEN);             // character_equipmentsets.guid

                char newSetGuid[24];
                snprintf(newSetGuid, 24, UI64FMTD, sObjectMgr->GenerateEquipmentSetGuid());
                if (!ChangeNth(line, 2, newSetGuid))
                    ROLLBACK(DUMP_FILE_BROKEN);             // character_equipmentsets.setguid

                for (int itemColumn = 7; itemColumn <= 25; ++itemColumn)    // item0..item18 follow the remapped item guids
                    if (!ChangeGuid(line, itemColumn, items, sObjectMgr->GetGenerator<HighGuid::Item>()->GetNextAfterMaxUsed(), true))
                        ROLLBACK(DUMP_FILE_BROKEN);
                break;
            }
            case DTT_INVENTORY:
            {
                if (!ChangeNth(line, 1, newguid))           // character_inventory.guid update
                    ROLLBACK(DUMP_FILE_BROKEN);

                if (!ChangeGuid(line, 2, items, sObjectMgr->GetGenerator<HighGuid::Item>()->GetNextAfterMaxUsed(), true))
                    ROLLBACK(DUMP_FILE_BROKEN);             // character_inventory.bag update
                if (!ChangeGuid(line, 4, items, sObjectMgr->GetGenerator<HighGuid::Item>()->GetNextAfterMaxUsed()))
                    ROLLBACK(DUMP_FILE_BROKEN);             // character_inventory.item update
                break;
            }
            case DTT_VS_TABLE:
            {
                uint64 newItemIdNum = sObjectMgr->GenerateVoidStorageItemId();
                char newItemId[20];
                snprintf(newItemId, 20, "%llu", newItemIdNum);

                if (!ChangeNth(line, 1, newItemId))           // character_void_storage.itemId update
                    ROLLBACK(DUMP_FILE_BROKEN);
                if (!ChangeNth(line, 2, newguid))           // character_void_storage.playerGuid update
                    ROLLBACK(DUMP_FILE_BROKEN);            // character_void_storage.itemGuid update
                if (!ChangeGuid(line, 9, items, sObjectMgr->GetGenerator<HighGuid::Item>()->GetNextAfterMaxUsed()))
                    ROLLBACK(DUMP_FILE_BROKEN);  
                break;
            }
            case DTT_MAIL:                                  // mail
            {
                if (!ChangeGuid(line, 1, mails, sObjectMgr->_mailId))
                    ROLLBACK(DUMP_FILE_BROKEN);             // mail.id update
                if (!ChangeNth(line, 6, newguid))           // mail.receiver update
                    ROLLBACK(DUMP_FILE_BROKEN);
                break;
            }
            case DTT_MAIL_ITEM:                             // mail_items
            {
                if (!ChangeGuid(line, 1, mails, sObjectMgr->_mailId))
                    ROLLBACK(DUMP_FILE_BROKEN);             // mail_items.id
                if (!ChangeGuid(line, 2, items, sObjectMgr->GetGenerator<HighGuid::Item>()->GetNextAfterMaxUsed()))
                    ROLLBACK(DUMP_FILE_BROKEN);             // mail_items.item_guid
                if (!ChangeNth(line, 3, newguid))           // mail_items.receiver
                    ROLLBACK(DUMP_FILE_BROKEN);
                break;
            }
            case DTT_ITEM:
            {
                // item, owner, data field:item, owner guid
                if (!ChangeGuid(line, 1, items, sObjectMgr->GetGenerator<HighGuid::Item>()->GetNextAfterMaxUsed()))
                   ROLLBACK(DUMP_FILE_BROKEN);              // item_instance.guid update
                if (!ChangeNth(line, 3, newguid))           // item_instance.owner_guid update
                    ROLLBACK(DUMP_FILE_BROKEN);
                break;
            }
            case DTT_ITEM_GIFT:
            {
                if (!ChangeNth(line, 1, newguid))           // character_gifts.guid update
                    ROLLBACK(DUMP_FILE_BROKEN);
                if (!ChangeGuid(line, 2, items, sObjectMgr->GetGenerator<HighGuid::Item>()->GetNextAfterMaxUsed()))
                    ROLLBACK(DUMP_FILE_BROKEN);             // character_gifts.item_guid update
                break;
            }
            case DTT_PET:
            {
                //store a map of old pet id to new inserted pet id for use by type 5 tables
                snprintf(currpetid, 20, "%s", GetNth(line, 1).c_str());

                // rows of a pet are not always contiguous (character_pet_declinedname comes after every character_pet)
                std::map<uint32, uint32> :: const_iterator petids_iter = petids.find(atoi(currpetid));
                if (petids_iter != petids.end())
                    snprintf(newpetid, 20, "%u", petids_iter->second);
                else
                {
                    if (!petids.empty())                    // the first pet takes the number reserved above
                        snprintf(newpetid, 20, "%u", sObjectMgr->GeneratePetNumber());
                    petids.insert(PetIdsPair(atoi(currpetid), atoi(newpetid)));
                }

                if (!ChangeNth(line, 1, newpetid))          // character_pet.id update
                    ROLLBACK(DUMP_FILE_BROKEN);
                if (!ChangeNth(line, tn == "character_pet_declinedname" ? 2 : 3, newguid))  // owner update
                    ROLLBACK(DUMP_FILE_BROKEN);

                break;
            }
            case DTT_PET_TABLE:                             // pet_aura, pet_spell, pet_spell_cooldown
            {
                snprintf(currpetid, 20, "%s", GetNth(line, 1).c_str());

                // lookup currpetid and match to new inserted pet id
                std::map<uint32, uint32> :: const_iterator petids_iter = petids.find(atoi(currpetid));
                if (petids_iter == petids.end())             // couldn't find new inserted id
                    ROLLBACK(DUMP_FILE_BROKEN);

                snprintf(newpetid, 20, "%d", petids_iter->second);

                if (!ChangeNth(line, 1, newpetid))
                    ROLLBACK(DUMP_FILE_BROKEN);

                break;
            }
            case DTT_CHAR_VS_FOLLOW:
            {
                if (!ChangeNth(line, 2, newguid))           // character_*.guid update
                {
                    TC_LOG_ERROR("network", "LoadPlayerDump: character_garrison_followers.guid line: '%s'!", line.c_str());
                    ROLLBACK(DUMP_FILE_BROKEN);
                }
                if (!ChangeNth(line, 1, lastfollowerid))
                {
                    TC_LOG_ERROR("network", "LoadPlayerDump: character_garrison_followers.dbId update line: '%s'!", line.c_str());
                    ROLLBACK(DUMP_FILE_BROKEN);
                }
                break;
            }
            case DTT_ITEM_TABLE:
            {
                if (!ChangeGuid(line, 1, items, sObjectMgr->GetGenerator<HighGuid::Item>()->GetNextAfterMaxUsed()))
                    ROLLBACK(DUMP_FILE_BROKEN);             // item_instance_*.itemGuid update
                break;
            }
            case DTT_CHAR_TRANSMOG:
            {
                if (!ChangeNth(line, 1, newguid))           // character_transmog_outfits.guid update
                    ROLLBACK(DUMP_FILE_BROKEN);

                char newSetGuid[24];
                snprintf(newSetGuid, 24, UI64FMTD, sObjectMgr->GenerateEquipmentSetGuid());
                if (!ChangeNth(line, 2, newSetGuid))
                    ROLLBACK(DUMP_FILE_BROKEN);             // character_transmog_outfits.setguid
                break;
            }
            default:
                TC_LOG_ERROR("network", "Unknown dump table type: %u", type);
                ROLLBACK(DUMP_FILE_BROKEN);                 // an unremapped row would keep the original guids
        }

        fixNULLfields(line);

        trans->Append(line.c_str());
    }

    fclose(fin);

    // the transaction runs on a database thread: wait for it, a failed insert must not be reported as success
    if (!CharacterDatabase.AsyncCommitTransaction(trans).m_future.get())
    {
        TC_LOG_ERROR("network", "LoadPlayerDump: the transaction failed, nothing was imported.");
        return DUMP_DATABASE_ERROR;
    }

    // in case of name conflict player has to rename at login anyway
    sWorld->AddCharacterInfo(ObjectGuid::Create<HighGuid::Player>(guid), account, name, gender, race, playerClass, level);

    sObjectMgr->GetGenerator<HighGuid::Item>()->Set(sObjectMgr->GetGenerator<HighGuid::Item>()->GetNextAfterMaxUsed() + items.size());
    sObjectMgr->_mailId     += mails.size();

    if (incHighest)
        sObjectMgr->GetGenerator<HighGuid::Player>()->Generate();

    return DUMP_SUCCESS;
}
