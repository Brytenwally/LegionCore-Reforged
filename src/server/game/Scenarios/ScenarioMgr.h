////////////////////////////////////////////
///                                      ///
///              DEMONCORE               ///
///    Copyright 2026 Titans-Project     ///
///         All Rights Reserved.         ///
///                                      ///
///      Based on TrinityCore code       ///
///   Copyright 2008-2014 TrinityCore    ///
///   <http://www.trinitycore.org/>      ///
///                                      ///
////////////////////////////////////////////

#ifndef TRINITY_SCENARIOMGR_H
#define TRINITY_SCENARIOMGR_H

#include "Common.h"
#include "Scenario.h"
#include <shared_mutex>

class Scenario;
struct ScenarioStepEntry;

typedef std::vector<ScenarioStepEntry const*> ScenarioSteps;
typedef std::map<uint32 /*instance_id*/, Scenario*> ScenarioMap;
typedef std::map<uint32, ScenarioSteps> ScenarioStepsByScenarioMap;
typedef std::vector<ScenarioSteps*> ScenarioStepsByScenarioVector;

class TC_GAME_API ScenarioMgr
{
public:
    ScenarioMgr();
    ~ScenarioMgr();

    void UnloadAll();
    static ScenarioMgr* instance();

    Scenario* AddScenario(Map* map, lfg::LFGDungeonData const* _dungeonData, Player* player, bool find = false);
    // a scenario a script runs on a map that has none of its own (garrison invasions); only from the map's thread
    Scenario* AddScenario(Map* map, uint32 scenarioId);
    void RemoveScenario(Map* map);
    void RemoveScenario(uint32 instanceId);
    // The scenario is deleted by RemoveScenario when its map is destroyed: the pointer is only valid for code running in
    // the thread of that map. From another thread (a group member elsewhere, a world update) use WithScenario.
    Scenario* GetScenario(uint32 instanceId);

    // Runs fn(Scenario*) under the store lock, so that RemoveScenario waits for it. Short reads only: fn must not call
    // ScenarioMgr nor take a domain lock (the store lock is one). False when the instance has no scenario.
    template<class Fn>
    bool WithScenario(uint32 instanceId, Fn&& fn)
    {
        std::shared_lock<std::shared_mutex> lock(_scenarioLock);
        ScenarioMap::iterator itr = _scenarioStore.find(instanceId);
        if (itr == _scenarioStore.end())
            return false;

        fn(itr->second);
        return true;
    }

    ScenarioSteps const* GetScenarioSteps(uint32 scenarioId, bool Teeming = false);
    bool HasScenarioStep(lfg::LFGDungeonData const* _dungeonData, Player* player);
    bool HasAffixesTeeming(uint16 CriteriaTreeID);

private:
    ScenarioMap _scenarioStore;
    // each map thread adds, removes and looks up its own scenario in this shared store
    std::shared_mutex _scenarioLock;

    ScenarioStepsByScenarioMap m_stepMap;
    ScenarioStepsByScenarioMap m_stepTeemingMap;
    ScenarioStepsByScenarioVector m_stepVector;
};

#define sScenarioMgr ScenarioMgr::instance()

#endif
