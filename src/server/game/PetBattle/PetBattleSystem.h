////////////////////////////////////////////
///                                      ///
///              DEMONCORE               ///
///    Copyright 2026 Titans-Project     ///
///         All Rights Reserved.         ///
///                                      ///
////////////////////////////////////////////

#pragma once
#ifndef _PetBattleSystem
#define _PetBattleSystem

#include "Common.h"
#include <atomic>
#include <memory>
#include <mutex>
#include <vector>

class PetBattle;
class Player;
struct PetBattleRequest;

enum eBattlePetRequests
{
    PETBATTLE_REQUEST_CREATE_FAILED          = 0,
    PETBATTLE_REQUEST_NOT_HERE               = 1,
    PETBATTLE_REQUEST_NOT_HERE_ON_TRANSPORT  = 2,
    PETBATTLE_REQUEST_NOT_HERE_UNEVEN_GROUND = 3,
    PETBATTLE_REQUEST_NOT_HERE_OBSTRUCTED    = 4,
    PETBATTLE_REQUEST_NOT_WHILE_IN_COMBAT    = 5,
    PETBATTLE_REQUEST_NOT_WHILE_DEAD         = 6,
    PETBATTLE_REQUEST_NOT_WHILE_FLYING       = 7,
    PETBATTLE_REQUEST_TARGET_INVALID         = 8,
    PETBATTLE_REQUEST_TARGET_OUT_OF_RANGE    = 9,
    PETBATTLE_REQUEST_TARGET_NOT_CAPTURABLE  = 10,
    PETBATTLE_REQUEST_NOT_A_TRAINER          = 11,
    PETBATTLE_REQUEST_DECLINED               = 12,
    PETBATTLE_REQUEST_IN_BATTLE              = 13,
    PETBATTLE_REQUEST_INVALID_LOADOUT        = 14,
    PETBATTLE_REQUEST_ALL_PETS_DEAD          = 15,
    PETBATTLE_REQUEST_NO_PETS_IN_SLOT        = 16,
    PETBATTLE_REQUEST_NO_ACCOUNT_LOCK        = 17,
    PETBATTLE_REQUEST_WILD_PET_TAPPED        = 18,

    /// Custom value
    PETBATTLE_REQUEST_OK                     = 0xFF
};

enum LFBAnswer : int32
{
    LFB_ANSWER_PENDING = -1,
    LFB_ANSWER_DENY = 0,
    LFB_ANSWER_AGREE = 1
};

enum LFBState : uint32
{
    LFB_STATE_NONE      = 0,
    LFB_STATE_QUEUED    = 1,
    LFB_STATE_PROPOSAL  = 2,
    LFB_STATE_IN_COMBAT = 3,
    LFB_STATE_FINISHED  = 4,
};

struct LFBTicket
{
    LFBTicket* MatchingOpponent;
    ObjectGuid RequesterGUID;
    LFBState State;
    LFBAnswer ProposalAnswer;
    uint32 JoinTime;
    uint32 TicketID;
    uint32 Weight;
    uint32 TeamID;
    uint32 ProposalTime;
};

// Threading: map threads (packet handlers) and the world thread (Update) share this manager. _lock guards the
// battle and request maps and the deletion queues, _LFBRequestsMutex the matchmaking tickets. Neither is held
// while a Player is touched or a battle is locked: entries are copied out first. Lock order: PetBattle::BattleLock
// may be held when taking _lock; the two manager locks never nest.
class TC_GAME_API PetBattleSystem
{
    PetBattleSystem();
    ~PetBattleSystem();

public:
    static PetBattleSystem* instance();

    std::shared_ptr<PetBattle> NewBattle();
    std::shared_ptr<PetBattle> AcquireBattle(ObjectGuid battleID);

    // A request can only be taken by the opponent it names, and while it has not expired.
    void AddRequest(std::shared_ptr<PetBattleRequest> const& request);
    std::shared_ptr<PetBattleRequest> TakeRequest(ObjectGuid requesterGuid, ObjectGuid opponentGuid);

    void RemoveRequest(ObjectGuid requesterGuid);

    void JoinQueue(Player* player);
    void ProposalResponse(Player* player, bool accept);
    void LeaveQueue(Player* player);

    void Update(uint32 diff);

    // Called from WorldSession::LogoutPlayer, in the leaving player's own thread, before he is freed:
    // withdraws his challenge and abandons his battle so the wild creature or the opponent is released.
    void OnPlayerLogout(Player* player);

    // Undoes what a battle that never began left on the player, in his own update; another battle he joined since is left alone.
    static void ReleaseFromUnstartedBattle(ObjectGuid const& playerGuid, ObjectGuid const& battleID, bool matchmaking);

    eBattlePetRequests CanPlayerEnterInPetBattle(Player* player, PetBattleRequest* petBattleRequest);

private:
    void UpdateQueue();
    void RecordQueueWait(uint32 queuedTime);

    std::map<ObjectGuid, std::shared_ptr<PetBattle>> _petBattles;           ///< All running battles
    std::map<ObjectGuid, std::shared_ptr<PetBattleRequest>> _battleRequests; ///< All pending battles request
    std::map<ObjectGuid, LFBTicket*> _LFBRequests;
    std::vector<ObjectGuid> _deleteQueuePending;   ///< Battles finished during the current interval
    std::vector<ObjectGuid> _deleteQueueReady;     ///< Battles erased at the next interval
    std::mutex _lock;
    std::mutex _LFBRequestsMutex;
    IntervalTimer _LFBRequestsUpdateTimer;
    IntervalTimer _deleteUpdateTimer;        ///< Deletion queue update timer
    uint32 _LFBAvgWaitTime;
    uint32 _LFBNumWaitTimeAvg;
    std::atomic<uint32> _maxPetBattleID;       ///< Global battle unique id
};

#define sPetBattleSystem PetBattleSystem::instance()

#endif
