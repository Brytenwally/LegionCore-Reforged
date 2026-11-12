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

#include "OutdoorPvPMgr.h"
#include "ObjectMgr.h"
#include "BattlegroundMap.h"
#include "Player.h"
#include "DisableMgr.h"
#include "ScriptMgr.h"
#include "DatabaseEnv.h"

namespace
{
    // an engine bound to an instance runs in the thread of that map: a player of another map must not reach it
    bool IsReachableBy(OutdoorPvP const* pvp, Player const* player)
    {
        return !pvp->IsBoundToInstance() || pvp->GetMap() == player->GetMap();
    }

    // A game of a battleground has an engine of its own, which the lists below do not hold
    OutdoorPvP* GetGameEngine(Player const* player)
    {
        if (Map* map = player->GetMap())
            if (BattlegroundMap* bgMap = map->ToBgMap())
                return bgMap->GetOutdoorPvP();

        return nullptr;
    }

    // Engines made per game of a battleground, never registered on a zone: only their template is kept
    bool IsGameEngine(OutdoorPvPTypes type)
    {
        return type == OUTDOOR_PVP_ASHRAN;
    }
}

OutdoorPvPMgr::OutdoorPvPMgr()
{
    m_UpdateTimer = 0;
}

void OutdoorPvPMgr::Die()
{
    for (auto v : m_OutdoorPvPSet)
        delete v;

    m_OutdoorPvPSet.clear();

    for (auto v : m_OutdoorPvPDatas)
        delete v.second;

    m_OutdoorPvPDatas.clear();

    m_OutdoorPvPMap.clear();
}

OutdoorPvPMgr* OutdoorPvPMgr::instance()
{
    static OutdoorPvPMgr instance;
    return &instance;
}

void OutdoorPvPMgr::InitOutdoorPvP()
{
    uint32 oldMSTime = getMSTime();

    //                                                 0       1
    QueryResult result = WorldDatabase.Query("SELECT TypeId, ScriptName FROM outdoorpvp_template");

    if (!result)
    {
        TC_LOG_ERROR("server.loading", ">> Loaded 0 outdoor PvP definitions. DB table `outdoorpvp_template` is empty.");
        return;
    }

    uint32 count = 0;
    uint32 typeId = 0;

    do
    {
        Field* fields = result->Fetch();

        typeId = fields[0].GetUInt8();

        if (DisableMgr::IsDisabledFor(DISABLE_TYPE_OUTDOORPVP, typeId))
            continue;

        if (typeId >= MAX_OUTDOORPVP_TYPES)
        {
            TC_LOG_ERROR("sql.sql", "Invalid OutdoorPvPTypes value %u in outdoorpvp_template; skipped.", typeId);
            continue;
        }

        OutdoorPvPData* data = new OutdoorPvPData();
        OutdoorPvPTypes realTypeId = OutdoorPvPTypes(typeId);
        data->TypeId = realTypeId;
        data->ScriptId = sObjectMgr->GetScriptId(fields[1].GetCString());
        m_OutdoorPvPDatas[realTypeId] = data;

        ++count;
    }
    while (result->NextRow());

    for (uint8 i = 1; i < MAX_OUTDOORPVP_TYPES; ++i)
    {
        OutdoorPvPDataMap::iterator iter = m_OutdoorPvPDatas.find(OutdoorPvPTypes(i));
        if (iter == m_OutdoorPvPDatas.end())
        {
            //TC_LOG_ERROR("sql.sql", "Could not initialize OutdoorPvP object for type ID %u; no entry in database.", uint32(i));
            continue;
        }

        if (IsGameEngine(OutdoorPvPTypes(i)))
            continue;

        OutdoorPvP* pvp = sScriptMgr->CreateOutdoorPvP(iter->second);
        if (!pvp)
        {
            TC_LOG_ERROR("outdoorpvp", "Could not initialize OutdoorPvP object for type ID %u; got nullptr pointer from script.", uint32(i));
            continue;
        }

        if (!pvp->SetupOutdoorPvP())
        {
            TC_LOG_ERROR("outdoorpvp", "Could not initialize OutdoorPvP object for type ID %u; SetupOutdoorPvP failed.", uint32(i));
            delete pvp;
            continue;
        }

        m_OutdoorPvPSet.push_back(pvp);
    }

    TC_LOG_INFO("server.loading", ">> Loaded %u outdoor PvP definitions in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

OutdoorPvP* OutdoorPvPMgr::CreateGameEngine(OutdoorPvPTypes type)
{
    auto itr = m_OutdoorPvPDatas.find(type);
    if (itr == m_OutdoorPvPDatas.end() || !IsGameEngine(type))
    {
        TC_LOG_ERROR("outdoorpvp", "OutdoorPvP: no template for the engine of type %u (table `outdoorpvp_template`).", uint32(type));
        return nullptr;
    }

    OutdoorPvP* pvp = sScriptMgr->CreateOutdoorPvP(itr->second);
    if (!pvp)
        TC_LOG_ERROR("outdoorpvp", "OutdoorPvP: could not create the engine of type %u; got nullptr pointer from script.", uint32(type));

    return pvp;
}

void OutdoorPvPMgr::AddZone(uint32 zoneid, OutdoorPvP* handle)
{
    m_OutdoorPvPZone[zoneid] = handle;
    if (AreaTableEntry const* areaEntry = sAreaTableStore.LookupEntry(zoneid))
    {
        m_OutdoorPvPMap[areaEntry->ContinentID].insert(handle);

        MapEntry const* entry = sMapStore.LookupEntry(areaEntry->ContinentID);
        if (!entry)
            return;

        handle->m_zoneSet.insert(areaEntry->ParentAreaID ? areaEntry->ParentAreaID : zoneid);

        if (Map* map = entry->CanCreatedZone() ? sMapMgr->FindMap(areaEntry->ContinentID, zoneid) : sMapMgr->CreateBaseMap(areaEntry->ContinentID))
        {
            if (!map->OutdoorPvPList)
                map->OutdoorPvPList = GetOutdoorPvPMap(areaEntry->ContinentID);
            handle->SetMap(map);

            // nobody stands on the base map of an instanceable map: the engine is initialised when it moves to an instance
            if (map->Instanceable())
                handle->m_initZone = zoneid;
            else
                handle->Initialize(zoneid);
        }
    }
}

std::set<OutdoorPvP*>* OutdoorPvPMgr::GetOutdoorPvPMap(uint32 ContinentID)
{
    return Trinity::Containers::MapGetValuePtr(m_OutdoorPvPMap, ContinentID);
}

void OutdoorPvPMgr::HandlePlayerEnterZone(ObjectGuid guid, uint32 zoneid)
{
    OutdoorPvPZone::iterator itr = m_OutdoorPvPZone.find(zoneid);
    if (itr == m_OutdoorPvPZone.end())
        return;

    itr->second->HandlePlayerEnterZone(guid, zoneid);
}

void OutdoorPvPMgr::HandlePlayerLeaveZone(ObjectGuid guid, uint32 zoneid)
{
    OutdoorPvPZone::iterator itr = m_OutdoorPvPZone.find(zoneid);
    if (itr == m_OutdoorPvPZone.end())
        return;

    itr->second->HandlePlayerLeaveZone(guid, zoneid);
}

void OutdoorPvPMgr::HandlePlayerEnterMap(ObjectGuid guid, uint32 zoneID)
{
    auto itr = m_OutdoorPvPZone.find(zoneID);
    if (itr == m_OutdoorPvPZone.end())
        return;
    
    itr->second->HandlePlayerEnterMap(guid, zoneID);
}

void OutdoorPvPMgr::HandlePlayerLeaveMap(ObjectGuid guid, uint32 zoneID)
{
    // callers pass the zone, as for the enter call (m_OutdoorPvPMap is keyed by map: the lookup never matched)
    auto itr = m_OutdoorPvPZone.find(zoneID);
    if (itr == m_OutdoorPvPZone.end())
        return;

    itr->second->HandlePlayerLeaveMap(guid, zoneID);
}

void OutdoorPvPMgr::HandlePlayerEnterArea(ObjectGuid guid, uint32 areaID)
{
    auto itr = m_OutdoorPvPZone.find(areaID);
    if (itr == m_OutdoorPvPZone.end())
        return;

    itr->second->HandlePlayerEnterArea(guid, areaID);
}

void OutdoorPvPMgr::HandlePlayerLeaveArea(ObjectGuid guid, uint32 areaID)
{
    auto itr = m_OutdoorPvPZone.find(areaID);
    if (itr == m_OutdoorPvPZone.end())
        return;

    itr->second->HandlePlayerLeaveArea(guid, areaID);
}

OutdoorPvP* OutdoorPvPMgr::GetOutdoorPvPToZoneId(uint32 zoneid)
{
    return Trinity::Containers::MapGetValuePtr(m_OutdoorPvPZone, zoneid);
}

void OutdoorPvPMgr::Update(uint32 diff)
{
    m_UpdateTimer += diff;
    if (m_UpdateTimer > OUTDOORPVP_OBJECTIVE_UPDATE_INTERVAL)
    {
        for (OutdoorPvPSet::iterator itr = m_OutdoorPvPSet.begin(); itr != m_OutdoorPvPSet.end(); ++itr)
            (*itr)->Update(m_UpdateTimer);
        m_UpdateTimer = 0;
    }
}

bool OutdoorPvPMgr::HandleCustomSpell(Player* player, uint32 spellId, GameObject* go)
{
    if (OutdoorPvP* game = GetGameEngine(player))
        if (game->HandleCustomSpell(player, spellId, go))
            return true;

    for (OutdoorPvPSet::iterator itr = m_OutdoorPvPSet.begin(); itr != m_OutdoorPvPSet.end(); ++itr)
    {
        if (!IsReachableBy(*itr, player))
            continue;

        if ((*itr)->HandleCustomSpell(player, spellId, go))
            return true;
    }
    return false;
}

ZoneScript* OutdoorPvPMgr::GetZoneScript(uint32 zoneId)
{
    return Trinity::Containers::MapGetValuePtr(m_OutdoorPvPZone, zoneId);
}

bool OutdoorPvPMgr::HandleOpenGo(Player* player, ObjectGuid guid)
{
    if (OutdoorPvP* game = GetGameEngine(player))
        if (game->HandleOpenGo(player, guid))
            return true;

    for (OutdoorPvPSet::iterator itr = m_OutdoorPvPSet.begin(); itr != m_OutdoorPvPSet.end(); ++itr)
    {
        if (!IsReachableBy(*itr, player))
            continue;

        if ((*itr)->HandleOpenGo(player, guid))
            return true;
    }
    return false;
}

void OutdoorPvPMgr::HandleGossipOption(Player* player, ObjectGuid guid, uint32 gossipid)
{
    if (OutdoorPvP* game = GetGameEngine(player))
        if (game->HandleGossipOption(player, guid, gossipid))
            return;

    for (OutdoorPvPSet::iterator itr = m_OutdoorPvPSet.begin(); itr != m_OutdoorPvPSet.end(); ++itr)
    {
        if (!IsReachableBy(*itr, player))
            continue;

        if ((*itr)->HandleGossipOption(player, guid, gossipid))
            return;
    }
}

bool OutdoorPvPMgr::CanTalkTo(Player* player, Creature* creature, GossipMenuItems const& gso)
{
    if (OutdoorPvP* game = GetGameEngine(player))
        if (game->CanTalkTo(player, creature, gso))
            return true;

    for (OutdoorPvPSet::iterator itr = m_OutdoorPvPSet.begin(); itr != m_OutdoorPvPSet.end(); ++itr)
    {
        if (!IsReachableBy(*itr, player))
            continue;

        if ((*itr)->CanTalkTo(player, creature, gso))
            return true;
    }
    return false;
}

void OutdoorPvPMgr::HandleDropFlag(Player* player, uint32 spellId)
{
    if (OutdoorPvP* game = GetGameEngine(player))
        if (game->HandleDropFlag(player, spellId))
            return;

    for (OutdoorPvPSet::iterator itr = m_OutdoorPvPSet.begin(); itr != m_OutdoorPvPSet.end(); ++itr)
    {
        if (!IsReachableBy(*itr, player))
            continue;

        if ((*itr)->HandleDropFlag(player, spellId))
            return;
    }
}

void OutdoorPvPMgr::HandleGameEventStart(uint32 event)
{
    for (OutdoorPvPSet::iterator itr = m_OutdoorPvPSet.begin(); itr != m_OutdoorPvPSet.end(); ++itr)
    {
        (*itr)->HandleGameEventStart(event);
    }
}

void OutdoorPvPMgr::HandlePlayerResurrects(Player* player, uint32 zoneid)
{
    OutdoorPvPZone::iterator itr = m_OutdoorPvPZone.find(zoneid);
    if (itr == m_OutdoorPvPZone.end())
        return;

    if (itr->second->HasPlayer(player))
        itr->second->HandlePlayerResurrects(player, zoneid);
}
