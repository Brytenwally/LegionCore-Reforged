////////////////////////////////////////////
///                                      ///
///              DEMONCORE               ///
///    Copyright 2026 Titans-Project     ///
///         All Rights Reserved.         ///
///                                      ///
////////////////////////////////////////////

#include "BattlegroundAshran.h"
#include "BattlegroundMap.h"
#include "OutdoorPvP.h"
#include "OutdoorPvPMgr.h"
#include "Player.h"
#include "WorldStatePackets.h"

namespace
{
    uint32 const EngineUpdateInterval = 1000;    // same pace as OutdoorPvPMgr gave the engines of the open world
    uint32 const BattleActiveWorldState = 19191; // custom, read by the client's conditions on Ashran's bonus objectives

    enum AshranGates
    {
        GATE_ALLIANCE,
        GATE_HORDE,
        GATE_MAX
    };

    // the garrison gates of Draenor shut the way out of each base until the game starts
    uint32 const GateEntry[GATE_MAX] = { 229498, 234092 };
    Position const GatePos[GATE_MAX] =
    {
        { 4088.4f, -4121.0f, 48.3f, 2.55f },
        { 4945.9f, -4170.4f, 38.3f, 0.45f },
    };
}

BattlegroundAshran::BattlegroundAshran() : m_engineTimer(0)
{
    BgObjects.resize(GATE_MAX);
}

bool BattlegroundAshran::SetupBattleground()
{
    for (uint8 i = 0; i < GATE_MAX; ++i)
    {
        Position const& pos = GatePos[i];
        float const o = pos.GetOrientation();
        if (!AddObject(i, GateEntry[i], pos.GetPositionX(), pos.GetPositionY(), pos.GetPositionZ(), o, 0.0f, 0.0f, std::sin(o / 2.0f), std::cos(o / 2.0f), RESPAWN_IMMEDIATELY))
            return false;
    }

    return true;
}

void BattlegroundAshran::StartingEventCloseDoors()
{
    for (uint8 i = 0; i < GATE_MAX; ++i)
        DoorClose(i);
}

OutdoorPvP* BattlegroundAshran::GetEngine() const
{
    BattlegroundMap* map = GetBgMap();
    return map ? map->GetOutdoorPvP() : nullptr;
}

void BattlegroundAshran::OnMapCreated()
{
    // made before the first grid is loaded: the creatures of the map look for the engine as they enter the world
    OutdoorPvP* engine = sOutdoorPvPMgr->CreateGameEngine(OUTDOOR_PVP_ASHRAN);
    if (!engine)
        return;

    BattlegroundMap* map = GetBgMap();
    map->SetOutdoorPvP(engine);
    engine->BindToGame(this, map);
}

void BattlegroundAshran::AddPlayer(Player* player)
{
    Battleground::AddPlayer(player);

    // the scoreboard and the kill counters read this entry
    if (PlayerScores.find(player->GetGUID()) == PlayerScores.end())
        PlayerScores[player->GetGUID()] = new BattlegroundAshranScore(player->GetGUID(), player->GetBGTeamId());

    if (OutdoorPvP* engine = GetEngine())
        engine->AddGamePlayer(player);
}

void BattlegroundAshran::RemovePlayer(Player* /*player*/, ObjectGuid guid, uint32 /*team*/)
{
    if (OutdoorPvP* engine = GetEngine())
        engine->RemoveGamePlayer(guid);
}

void BattlegroundAshran::StartingEventOpenDoors()
{
    for (uint8 i = 0; i < GATE_MAX; ++i)
        DoorOpen(i);

    if (OutdoorPvP* engine = GetEngine())
        engine->HandleGameStart();
}

void BattlegroundAshran::PostUpdateImpl(uint32 diff)
{
    // the engine stops with the game: nothing of the battle goes on once a side has won
    if (GetStatus() == STATUS_WAIT_LEAVE)
        return;

    OutdoorPvP* engine = GetEngine();
    if (!engine)
        return;

    // Runs in the thread of the base map, once the updates of its instances are done: nothing of the game moves
    // meanwhile, and the map does not update the engine (a map of a game holds no list of engines).
    m_engineTimer += diff;
    if (m_engineTimer < EngineUpdateInterval)
        return;

    engine->Update(m_engineTimer);
    m_engineTimer = 0;
}

void BattlegroundAshran::FillInitialWorldStates(WorldPackets::WorldState::InitWorldStates& packet)
{
    // the client shows Ashran's bonus objectives only while this is set (hotfixed WorldStateExpression 24580)
    packet.Worldstates.emplace_back(static_cast<WorldStates>(BattleActiveWorldState), 1);

    if (OutdoorPvP* engine = GetEngine())
        engine->FillInitialWorldStates(packet);
}

WorldSafeLocsEntry const* BattlegroundAshran::GetClosestGraveYard(Player* player)
{
    if (OutdoorPvP* engine = GetEngine())
        if (WorldSafeLocsEntry const* graveyard = engine->GetClosestGraveyard(player))
            return graveyard;

    return Battleground::GetClosestGraveYard(player);
}
