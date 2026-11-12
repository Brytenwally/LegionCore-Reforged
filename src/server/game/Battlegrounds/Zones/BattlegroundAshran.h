////////////////////////////////////////////
///                                      ///
///              DEMONCORE               ///
///    Copyright 2026 Titans-Project     ///
///         All Rights Reserved.         ///
///                                      ///
////////////////////////////////////////////

#ifndef __BATTLEGROUNDASHRAN_H
#define __BATTLEGROUNDASHRAN_H

#include "Battleground.h"
#include "BattlegroundScore.h"

class OutdoorPvP;

// Ashran as an epic battleground, 1191 being its map. The frame lives here (queue, invitation, preparation countdown,
// entry and exit, end of the game); the battle itself belongs to the OutdoorPvPAshran engine, one per game: it is made
// with the map of the game, which owns it, and driven from here.

struct BattlegroundAshranScore final : BattlegroundScore
{
    friend class BattlegroundAshran;

protected:
    BattlegroundAshranScore(ObjectGuid playerGuid, TeamId team) : BattlegroundScore(playerGuid, team) { }

    // no objective column until the battle has objectives to count
    void BuildObjectivesBlock(std::vector<int32>& /*stats*/) override { }
};

class BattlegroundAshran : public Battleground
{
public:
    BattlegroundAshran();

    void OnMapCreated() override;
    bool SetupBattleground() override;
    void AddPlayer(Player* player) override;
    void StartingEventCloseDoors() override;
    void StartingEventOpenDoors() override;
    void FillInitialWorldStates(WorldPackets::WorldState::InitWorldStates& packet) override;
    WorldSafeLocsEntry const* GetClosestGraveYard(Player* player) override;

protected:
    void RemovePlayer(Player* player, ObjectGuid guid, uint32 team) override;

private:
    void PostUpdateImpl(uint32 diff) override;

    // null once the map of the game is gone
    OutdoorPvP* GetEngine() const;

    uint32 m_engineTimer;
};

#endif
