////////////////////////////////////////////
///                                      ///
///              DEMONCORE               ///
///    Copyright 2026 Titans-Project     ///
///         All Rights Reserved.         ///
///                                      ///
////////////////////////////////////////////

#ifndef SCRIPT_UTILS_H
#define SCRIPT_UTILS_H

#include "GridNotifiers.h"
#include "ObjectVisitors.hpp" 
#include "CellImpl.h"

namespace MS
{
    namespace ScriptUtils
    {
        inline std::list<Unit*> SelectNearestUnits(Unit* me, uint32 withEntry, float radius = 0.0f, bool friendly = false)
        {
            std::list<Unit*> targetList;

            if (friendly)
            {
                Trinity::AnyFriendlyUnitInObjectRangeCheck check(me, me, radius);
                Trinity::UnitListSearcher<Trinity::AnyFriendlyUnitInObjectRangeCheck> searcher(me, targetList, check);
                VisitNearbyObject(me, radius, searcher);
            }
            else
            {
                Trinity::AnyUnitInObjectRangeCheck check(me, radius);
                Trinity::UnitListSearcher<Trinity::AnyUnitInObjectRangeCheck> searcher(me, targetList, check);
                VisitNearbyObject(me, radius, searcher);
            }

            targetList.remove_if([withEntry](Unit* unit)
            {
                return withEntry && unit->GetEntry() != withEntry;
            });

            return targetList;
        }

        inline Player* SelectRandomPlayerExcludedTank(Unit* me, float range = 0.0f, bool checkLoS = true)
        {
            auto map = me->GetMap();
            if (!map->IsDungeon())
                return nullptr;

            auto const &playerList = map->GetPlayers();
            if (playerList.isEmpty())
                return nullptr;

            std::list<Player*> targetList;
            for (auto& itr : playerList)
                if (!itr.getSource()->isGameMaster() && (me->IsWithinLOSInMap(itr.getSource()) || !checkLoS) && me->getVictim() != itr.getSource() && (me->IsWithinDistInMap(itr.getSource(), range) || !range) &&
                        itr.getSource()->IsAlive())
                    targetList.push_back(itr.getSource());

            if (targetList.empty())
                return nullptr;

            return Trinity::Containers::SelectRandomContainerElement(targetList);
        }

        inline Player* SelectRandomPlayerIncludedTank(Unit* me, float range = 0.0f, bool checkLoS = false)
        {
            auto map = me->GetMap();
            if (!map->IsDungeon())
                return nullptr;

            auto const &playerList = map->GetPlayers();
            if (playerList.isEmpty())
                return nullptr;

            std::list<Player*> targetList;
            for (auto& itr : playerList)
                if (!itr.getSource()->isGameMaster() && (me->IsWithinLOSInMap(itr.getSource()) || !checkLoS) && (me->IsWithinDistInMap(itr.getSource(), range) || !range) &&
                        itr.getSource()->IsAlive())
                    targetList.push_back(itr.getSource());

            if (targetList.empty())
                return nullptr;

            return Trinity::Containers::SelectRandomContainerElement(targetList);
        }
    }
}

#endif /* !SCRIPT_UTILS */
