////////////////////////////////////////////
///                                      ///
///              DEMONCORE               ///
///    Copyright 2026 Titans-Project     ///
///         All Rights Reserved.         ///
///                                      ///
////////////////////////////////////////////

#pragma once

#include "Common.h"
#include "Map.h"

class Battleground;
class OutdoorPvP;
enum Difficulty : uint8;

class BattlegroundMap : public Map
{
    Battleground* m_bg;
    OutdoorPvP* m_outdoorPvP;
public:
    BattlegroundMap(uint32 id, time_t, uint32 instanceId, Map* parent, Difficulty difficulty);
    ~BattlegroundMap() override;

    bool AddPlayerToMap(Player*, bool initPlayer = true) override;
    void RemovePlayerFromMap(Player*, bool) override;
    bool CanEnter(Player* player) override;
    void SetUnload();
    void RemoveAllPlayers() override;
    void InitVisibilityDistance() override;
    Battleground* GetBG();
    void SetBG(Battleground* bg);

    // The world PvP engine of a game (Ashran) lives as long as the map: its creatures and objects are the map's own, and
    // they look it up as long as the map stands, whichever way the game itself ends.
    OutdoorPvP* GetOutdoorPvP() const { return m_outdoorPvP; }
    void SetOutdoorPvP(OutdoorPvP* pvp);
};
