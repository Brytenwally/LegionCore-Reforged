////////////////////////////////////////////
///                                      ///
///              DEMONCORE               ///
///    Copyright 2026 Titans-Project     ///
///         All Rights Reserved.         ///
///                                      ///
////////////////////////////////////////////

#include "FlightPathMovementGenerator.h"
#include "MoveSpline.h"
#include "MoveSplineInit.h"
#include "ObjectMgr.h"
#include "PlayerDefines.h"
#include "ScriptsData.h"

#define FLIGHT_TRAVEL_UPDATE 100
#define TIMEDIFF_NEXT_WP 250
#define SKIP_SPLINE_POINT_DISTANCE_SQ (40.f * 40.f)
#define PLAYER_FLIGHT_SPEED 32.0f

uint32 FlightPathMovementGenerator::GetPathAtMapEnd() const
{
    if (_currentNode >= _path.size())
        return _path.size();

    uint32 curMapId = _path[_currentNode]->ContinentID;
    for (uint32 i = _currentNode; i < _path.size(); ++i)
        if (_path[i]->ContinentID != curMapId)
            return i;

    return _path.size();
}

bool IsNodeIncludedInShortenedPath(TaxiPathNodeEntry const* p1, TaxiPathNodeEntry const* p2)
{
    return p1->ContinentID != p2->ContinentID || std::pow(p1->Loc.X - p2->Loc.X, 2) + std::pow(p1->Loc.Y - p2->Loc.Y, 2) > SKIP_SPLINE_POINT_DISTANCE_SQ;
}

void FlightPathMovementGenerator::LoadPath(Player& player, uint32 startNode /*= 0*/)
{
    _path.clear();
    _currentNode = startNode;
    _pointsForPathSwitch.clear();
    std::deque<uint32> const& taxi = player.m_taxi.GetPath();
    for (size_t src = 0, dst = 1; dst < taxi.size(); src = dst++)
    {
        uint32 path, cost;
        sObjectMgr->GetTaxiPath(taxi[src], taxi[dst], path, cost);
        if (path >= sTaxiPathNodesByPath.size())
            return;

        TaxiPathNodeList const& nodes = sTaxiPathNodesByPath[path];
        if (!nodes.empty())
        {
            TaxiPathNodeEntry const* start = nodes[0];
            TaxiPathNodeEntry const* end = nodes[nodes.size() - 1];
            bool passedPreviousSegmentProximityCheck = false;
            for (size_t i = 0; i < nodes.size(); ++i)
            {
                if (passedPreviousSegmentProximityCheck || !src || _path.empty() || IsNodeIncludedInShortenedPath(_path.back(), nodes[i]))
                {
                    if ((!src || (IsNodeIncludedInShortenedPath(start, nodes[i]) && i >= 2)) &&
                        (dst == taxi.size() - 1 || (IsNodeIncludedInShortenedPath(end, nodes[i]) && i < nodes.size() - 1)))
                    {
                        passedPreviousSegmentProximityCheck = true;
                        _path.push_back(nodes[i]);
                    }
                }
                else
                {
                    _path.pop_back();
                    --_pointsForPathSwitch.back().PathIndex;
                }
            }
        }

        _pointsForPathSwitch.push_back({ _path.empty() ? 0u : uint32(_path.size() - 1), int32(cost) });
    }
}

void FlightPathMovementGenerator::DoInitialize(Player& player)
{
    Reset(player);
    InitEndGridInfo();
}

void FlightPathMovementGenerator::DoFinalize(Player& player)
{
    // remove flag to prevent send object build movement packets for flight state and crash (movement generator already not at top of stack)
    player.ClearUnitState(UNIT_STATE_IN_FLIGHT);

    player.Dismount();
    player.RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_REMOVE_CLIENT_CONTROL | UNIT_FLAG_TAXI_FLIGHT);

    if (player.m_taxi.empty())
    {
        player.getHostileRefManager().setOnlineOfflineState(true);
        // update z position to ground and orientation for landing point
        // this prevent cheating with landing  point at lags
        // when client side flight end early in comparison server side
        player.StopMoving();
        player.SetFallInformation(0, player.GetPositionZ());
    }

    player.RemoveFlag(PLAYER_FIELD_PLAYER_FLAGS, PLAYER_FLAGS_TAXI_BENCHMARK);
    player.ResummonPetTemporaryUnSummonedIfAny();
    player.SummonLastSummonedBattlePet();
}

void FlightPathMovementGenerator::DoReset(Player& player)
{
    player.UnsummonPetTemporaryIfAny();
    player.UnsummonCurrentBattlePetIfAny(true);
    player.getHostileRefManager().setOnlineOfflineState(false);
    player.AddUnitState(UNIT_STATE_IN_FLIGHT);
    player.SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_REMOVE_CLIENT_CONTROL | UNIT_FLAG_TAXI_FLIGHT);

    Movement::MoveSplineInit init(player);
    uint32 end = GetPathAtMapEnd();
    for (uint32 i = GetCurrentNode(); i < end; ++i)
    {
        G3D::Vector3 vertice(_path[i]->Loc.X, _path[i]->Loc.Y, _path[i]->Loc.Z);
        init.Path().push_back(vertice);
    }
    init.SetFirstPointId(GetCurrentNode());
    init.SetFly();
    init.SetSmooth();
    init.SetUncompressed();
    init.SetWalk(true);
    init.SetVelocity(PLAYER_FLIGHT_SPEED * player.GetTotalAuraMultiplier(SPELL_AURA_MOD_FLY_PATH_SPEED));
    init.Launch();
}

bool FlightPathMovementGenerator::DoUpdate(Player& player, uint32 /*diff*/)
{
    // A taxi without nodes would otherwise never end (size() - 1 underflows below)
    if (_path.empty())
        return false;

    uint32 pointId = (uint32)player.movespline->currentPathIdx();
    if (pointId > _currentNode)
    {
        bool departureEvent = true;
        do
        {
            if (_currentNode >= _path.size()) // Max point
                return false;

            DoEventIfAny(player, *_path[_currentNode], departureEvent);
            while (!_pointsForPathSwitch.empty() && _pointsForPathSwitch.front().PathIndex <= _currentNode)
            {
                _pointsForPathSwitch.pop_front();
                player.m_taxi.NextTaxiDestination();
                if (!_pointsForPathSwitch.empty())
                {
                    player.UpdateAchievementCriteria(CRITERIA_TYPE_GOLD_SPENT_FOR_TRAVELLING, _pointsForPathSwitch.front().Cost);
                    player.ModifyMoney(-_pointsForPathSwitch.front().Cost);
                }
            }

            if (pointId == _currentNode)
                break;

            if (_currentNode == _preloadTargetNode)
                PreloadEndGrid(player);
            _currentNode += departureEvent ? 1 : 0;
            departureEvent = !departureEvent;
        }
        while (true);
    }

    return _currentNode + 1 < _path.size();
}

void FlightPathMovementGenerator::SetCurrentNodeAfterTeleport()
{
    if (_path.empty() || _currentNode >= _path.size())
        return;

    uint32 map0 = _path[_currentNode]->ContinentID;
    for (size_t i = _currentNode + 1; i < _path.size(); ++i)
    {
        if (_path[i]->ContinentID != map0)
        {
            _currentNode = i;
            return;
        }
    }
}

void FlightPathMovementGenerator::DoEventIfAny(Player& player, TaxiPathNodeEntry const& node, bool departure)
{
    if (uint32 eventid = departure ? node.DepartureEventID : node.ArrivalEventID)
    {
        TC_LOG_DEBUG("maps.script", "Taxi %s event %u of node %u of path %u for player %s", departure ? "departure" : "arrival", eventid, node.NodeIndex, node.PathID, player.GetName());
        player.GetMap()->ScriptsStart(sEventScripts, eventid, &player, &player);
    }
}

bool FlightPathMovementGenerator::GetResetPosition(Unit&, float& x, float& y, float& z)
{
    if (_currentNode >= _path.size())
        return false;

    TaxiPathNodeEntry const* node = _path[_currentNode];
    x = node->Loc.X;
    y = node->Loc.Y;
    z = node->Loc.Z;
    return true;
}

void FlightPathMovementGenerator::InitEndGridInfo()
{
    /*! Storage to preload flightmaster grid at end of flight. For multi-stop flights, this will
       be reinitialized for each flightmaster at the end of each spline (or stop) in the flight. */
    uint32 nodeCount = _path.size();        //! Number of nodes in path.
    if (!nodeCount)
        return;

    _endMapId = _path[nodeCount - 1]->ContinentID; //! MapId of last node
    _preloadTargetNode = nodeCount >= 3 ? nodeCount - 3 : 0;
    _endGridX = _path[nodeCount - 1]->Loc.X;
    _endGridY = _path[nodeCount - 1]->Loc.Y;
    _endGridZ = _path[nodeCount - 1]->Loc.Z;
}

void FlightPathMovementGenerator::PreloadEndGrid(Player& player)
{
    MapEntry const* mapEntry = sMapStore.LookupEntry(_endMapId);
    if (!mapEntry)
        return;

    // used to preload the final grid where the flightmaster is. LoadGrid is only safe from the map's own thread,
    // which is the player's: preload when the flight ends in the map he flies in (zone maps: same zone)
    Map* endMap = player.FindMap();
    if (endMap && (endMap->GetId() != _endMapId || (mapEntry->CanCreatedZone() && endMap->GetInstanceId() != endMap->GetZoneId(_endGridX, _endGridY, _endGridZ))))
        endMap = nullptr;

    // Load the grid
    if (endMap)
    {
        TC_LOG_INFO("misc", "Preloading grid (%f, %f) for map %u at node index %u/%u", _endGridX, _endGridY, _endMapId, _preloadTargetNode, (uint32)(_path.size() - 1));
        endMap->LoadGrid(_endGridX, _endGridY);
    }
    else
        TC_LOG_INFO("misc", "Unable to determine map to preload flightmaster grid");
}
