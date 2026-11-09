#ifndef LEGION_BROKEN_SHORE_CLASS_ROUTE_H
#define LEGION_BROKEN_SHORE_CLASS_ROUTE_H

// From LegionCore-7.3.5. Map 1500 holds three class scenarios: warrior 933 (dungeon finder entry 1176, whose own
// ScenarioID is 0), paladin 775 (906) and demon hunter 961 (1166), all on difficulty 12. The first scenario row or
// dungeon of the map belongs to another class, so the scenario and its dungeon are resolved together, by class.
// Needs the class rows of scenario_data (2026_11_09_26_lc735_broken_shore_class_routes.sql).

#include "LFGMgr.h"
#include "ObjectMgr.h"
#include "Player.h"
namespace BrokenShoreClassRoute
{
    inline bool Applies(uint32 map, uint32 playerClass)
    {
        return map == 1500 && (playerClass == 1 || playerClass == 2 || playerClass == 12);
    }

    inline uint32 ScenarioId(uint32 playerClass)
    {
        return playerClass == 1 ? 933 : playerClass == 2 ? 775 : playerClass == 12 ? 961 : 0;
    }

    inline lfg::LFGDungeonData const* Dungeon(Player const* player, Difficulty difficulty, uint32 expectedScenario = 0)
    {
        if (!player || !Applies(1500, player->getClass())) return nullptr;
        uint32 scenario = ScenarioId(player->getClass());
        uint32 dungeon = player->getClass() == 1 ? 1176 : player->getClass() == 2 ? 906 : 1166;
        if (expectedScenario && expectedScenario != scenario) return nullptr;
        auto route = sObjectMgr->GetScenarioOnMap(1500, difficulty, player->GetTeam(), player->getClass(), 0);
        if (!route || route->ScenarioID != scenario || route->MapID != 1500 ||
            route->Class != player->getClass() || route->LfgDungeonID != dungeon ||
            (route->Team && route->Team != player->GetTeam()) ||
            (route->DifficultyID && route->DifficultyID != difficulty)) return nullptr;
        auto data = sLFGMgr->GetLFGDungeon(dungeon, player->GetTeam());
        if (!data || !data->dbc || data->map != 1500 || data->difficulty != difficulty ||
            (data->dbc->ScenarioID && data->dbc->ScenarioID != scenario)) return nullptr;
        return data;
    }
}
#endif
