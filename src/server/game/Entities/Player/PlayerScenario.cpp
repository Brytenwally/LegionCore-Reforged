/*
 * Scenario checkpoints across logouts, from LegionCore-7.3.5 (PlayerScenario.cpp, "Persist scenario entrances for safe
 * login recovery"). A character inside a quest scenario keeps an entrance checkpoint (`characters.scenarioResume`):
 * at login it rejoins the same live instance where it stood, or, when that instance is gone (server restart, instance
 * unloaded), a fresh run of the same scenario from its recorded entrance, never from a later coordinate the fresh
 * instance's step zero cannot reach. Completed scenarios, battlegrounds, garrisons, the Proving Grounds hub and
 * challenge modes keep no checkpoint.
 *
 * LegionCore-7.3.5's per-quest exits (Dark Riders, Broken Shore, Sheilun) come with their class phases.
 */

#include "Player.h"
#include "GameTime.h"
#include "LFGMgr.h"
#include "Log.h"
#include "MapManager.h"
#include "ObjectMgr.h"
#include "ScenarioMgr.h"
#include "WorldSession.h"
#include "QuestData.h"
#include "Item.h"
#include "BrokenShoreClassRoute.h"

namespace
{
    bool Resumable(MapEntry const* entry, uint32 scenarioId)
    {
        ScenarioEntry const* scenario = sScenarioStore.LookupEntry(scenarioId);
        return entry && entry->IsScenario() && !entry->IsBattlegroundOrArena() &&
            !entry->IsGarrison() && entry->ID != 1191 && scenario &&
            scenario->Type != SCENARIO_TYPE_CHALLENGE_MODE;
    }

    // the dungeon finder row a scenario of this map is built from for this player (team, class), as at its creation
    lfg::LFGDungeonData const* ResumeDungeon(Player const* player, uint32 scenarioId, uint32 mapId)
    {
        // map 1500: only this class's own scenario, before the direct lookup that would accept another class's
        if (BrokenShoreClassRoute::Applies(mapId, player->getClass()))
        {
            MapDifficultyEntry const* difficulty = sDB2Manager.GetDefaultMapDifficulty(mapId);
            return difficulty ? BrokenShoreClassRoute::Dungeon(player, Difficulty(difficulty->DifficultyID), scenarioId) : nullptr;
        }
        if (lfg::LFGDungeonData const* data = sLFGMgr->GetLFGDungeon(uint16(scenarioId), uint16(mapId)))
            return data;

        // a faction / class scenario can share another scenario's dungeon finder row: resolve the map's route, and
        // accept it only for this player's faction, class and dungeon
        MapDifficultyEntry const* difficulty = sDB2Manager.GetDefaultMapDifficulty(mapId);
        if (!difficulty)
            return nullptr;
        lfg::LFGDungeonData const* data = sLFGMgr->GetLFGDungeon(mapId, Difficulty(difficulty->DifficultyID), player->GetTeam());
        if (!data)
            return nullptr;
        ScenarioData const* route = sObjectMgr->GetScenarioOnMap(mapId, data->difficulty, player->GetTeam(), player->getClass(), data->id);
        if (!route || route->ScenarioID != scenarioId ||
            (route->Team && route->Team != player->GetTeam()) ||
            (route->Class && route->Class != player->getClass()) ||
            (route->DifficultyID && route->DifficultyID != data->difficulty) ||
            (route->LfgDungeonID && route->LfgDungeonID != data->id))
            return nullptr;
        return data;
    }
}

void Player::PrepareScenarioLogin(uint32 mapId, uint32 instanceId)
{
    MapEntry const* entry = sMapStore.LookupEntry(mapId);
    if (!m_scenarioResume.scenario || m_scenarioResume.map != mapId ||
        !Resumable(entry, m_scenarioResume.scenario) ||
        !MapManager::IsValidMapCoord(mapId, m_scenarioResume.x, m_scenarioResume.y, m_scenarioResume.z, m_scenarioResume.o))
    {
        m_scenarioResume = {};
        return;
    }

    // the scenario and its route must still exist after a data update
    if (!ResumeDungeon(this, m_scenarioResume.scenario, mapId) || !sScenarioMgr->GetScenarioSteps(m_scenarioResume.scenario))
    {
        m_scenarioResume = {};
        return;
    }

    m_scenarioLoginId = m_scenarioResume.scenario;
    SetScenarioId(uint16(m_scenarioLoginId));

    // the boot time keeps a recycled instance id of another run from being joined
    Scenario* live = m_scenarioResume.boot == uint64(GameTime::GetStartTime()) && instanceId == m_scenarioResume.instance ?
        sScenarioMgr->GetScenario(instanceId) : nullptr;
    if (live && live->GetMap() && live->GetMap()->GetId() == mapId && live->GetScenarioId() == m_scenarioLoginId)
    {
        // inventory and quest rows load later: keep the saved monk outro tentatively, the login check revalidates it
        if (live->IsCompleted(false) && !(mapId == 1541 && m_scenarioLoginId == 1007 && getClass() == CLASS_MONK))
        {
            m_scenarioResume = {};
            m_scenarioLoginId = 0;
            SetScenarioId(0);
            return;
        }
        m_scenarioLoginInstance = instanceId;
        TC_LOG_INFO("entities.player", "Scenario resume: player %u rejoins live scenario %u instance %u", GetGUIDLow(), m_scenarioLoginId, instanceId);
        return;
    }

    // criteria and actors are not saved: a fresh instance starts at step zero, so the player starts at the entrance
    Relocate(m_scenarioResume.x, m_scenarioResume.y, m_scenarioResume.z, m_scenarioResume.o);
    TC_LOG_INFO("entities.player", "Scenario resume: player %u restarts scenario %u at its recorded entrance", GetGUIDLow(), m_scenarioLoginId);
}

bool Player::CanResumeScenarioLogin(uint32 scenarioId) const
{
    return m_scenarioLoginId && m_scenarioLoginId == scenarioId &&
        m_scenarioResume.map == GetMapId() && GetMap() && Resumable(GetMap()->GetEntry(), scenarioId);
}

void Player::RecordScenarioEntrance()
{
    if (!IsInWorld() || !GetMap())
        return;

    Scenario* progress = GetMap()->IsScenario() ? sScenarioMgr->GetScenario(GetInstanceId()) : nullptr;
    if (!progress || !Resumable(GetMap()->GetEntry(), progress->GetScenarioId()) ||
        (progress->IsCompleted(false) && !(progress->GetScenarioId() == 1007 && HasPendingSheilunReturn())))
    {
        m_scenarioResume = {};
        m_scenarioLoginId = m_scenarioLoginInstance = 0;
        return;
    }

    if (m_scenarioResume.map != GetMapId() || m_scenarioResume.scenario != progress->GetScenarioId())
    {
        Position entrance = GetPosition();
        // a run first recorded in a later part, on a transport or a vehicle starts from its dungeon finder entrance
        if (GetSession()->PlayerLoading() || progress->GetCurrentStep() || GetTransport() || GetVehicle())
        {
            lfg::LFGDungeonData const* data = ResumeDungeon(this, progress->GetScenarioId(), GetMapId());
            if (!data || (!data->x && !data->y && !data->z) ||
                !MapManager::IsValidMapCoord(GetMapId(), data->x, data->y, data->z, data->o))
                return;
            entrance.Relocate(data->x, data->y, data->z, data->o);
        }
        if (!MapManager::IsValidMapCoord(GetMapId(), entrance.GetPositionX(), entrance.GetPositionY(), entrance.GetPositionZ(), entrance.GetOrientation()))
            return;
        m_scenarioResume.map = GetMapId();
        m_scenarioResume.scenario = progress->GetScenarioId();
        m_scenarioResume.x = entrance.GetPositionX();
        m_scenarioResume.y = entrance.GetPositionY();
        m_scenarioResume.z = entrance.GetPositionZ();
        m_scenarioResume.o = entrance.GetOrientation();
    }
    m_scenarioResume.instance = GetInstanceId();
    m_scenarioResume.boot = uint64(GameTime::GetStartTime());
    if (m_scenarioLoginId == progress->GetScenarioId())
        SetScenarioId(0);
}

// Sheilun (monk, quest 41003, scenario 1007 on map 1541): the completed scenario keeps its checkpoint while the owned
// staff still has its return to make (LegionCore-7.3.5 "require manual Sheilun loot and boarded return")
bool Player::HasPendingSheilunReturn() const
{
    if (GetMapId() != 1541 || getClass() != CLASS_MONK || isGameMaster() || IsQuestRewarded(41003) ||
        (GetQuestStatus(41003) != QUEST_STATUS_INCOMPLETE && GetQuestStatus(41003) != QUEST_STATUS_COMPLETE) ||
        GetQuestObjectiveData(41003, 102479) < 1)
        return false;
    Item* item = GetItemByEntry(128937, false);
    if (!item || item->GetOwnerGUID() != GetGUID() || !item->GetCount())
        return false;
    Quest const* quest = sQuestDataStore->GetQuestTemplate(41003);
    if (quest)
        for (auto const& objective : quest->GetObjectives())
            if (objective.ID == 281599 && objective.ObjectID == 101880 && objective.StorageIndex == 2 && objective.Amount == 1)
                return GetQuestObjectiveData(quest, objective.StorageIndex) < objective.Amount;
    return false;
}

// a refused login into the completed Sheilun scenario returns an owner of the staff to the native Mistweaver exit;
// it routes the player and grants nothing
bool Player::ExitCompletedSheilunLogin()
{
    if (GetMapId() != 1541 || getClass() != CLASS_MONK ||
        (m_scenarioLoginId && m_scenarioLoginId != 1007) ||
        (m_scenarioResume.scenario && m_scenarioResume.scenario != 1007) ||
        IsQuestRewarded(41003) ||
        (GetQuestStatus(41003) != QUEST_STATUS_INCOMPLETE && GetQuestStatus(41003) != QUEST_STATUS_COMPLETE) ||
        GetQuestObjectiveData(41003, 102479) < 1)
        return false;

    Scenario* progress = sScenarioMgr->GetScenario(GetInstanceId());
    if (progress && (progress->GetMap() != GetMap() || progress->GetScenarioId() != 1007))
        return false;

    // GetItemByEntry scans the inventory and bag slots even before they are in the world
    Item* staff = GetItemByEntry(128937, false);
    if (!staff || staff->GetOwnerGUID() != GetGUID() || !staff->GetCount())
        return false;

    WorldSafeLocsEntry const* exit = sWorldSafeLocsStore.LookupEntry(5710);
    if (!exit || exit->MapID != 1514)
        return false;
    float facing = exit->Loc.O * float(M_PI / 180.0);
    if (!MapManager::IsValidMapCoord(exit->MapID, exit->Loc.X, exit->Loc.Y, exit->Loc.Z, facing) ||
        !TeleportTo(exit->MapID, exit->Loc.X, exit->Loc.Y, exit->Loc.Z, facing))
        return false;

    m_scenarioResume = {};
    m_scenarioLoginId = m_scenarioLoginInstance = 0;
    SetScenarioId(0);
    TC_LOG_INFO("entities.player", "Sheilun: login returns artifact owner %u to the native Mistweaver exit", GetGUIDLow());
    return true;
}
