////////////////////////////////////////////
///                                      ///
///         LEGIONCORE REFORGED          ///
///    Copyright 2026 Titans-Project     ///
///         All Rights Reserved.         ///
///                                      ///
///      Based on TrinityCore code       ///
///   Copyright 2008-2012 TrinityCore    ///
///   <http://www.trinitycore.org/>      ///
///                                      ///
////////////////////////////////////////////

#ifndef GROUP_H
#define GROUP_H

#include "DBCEnums.h"
#include "GroupRefManager.h"
#include "LootMgr.h"
#include "SharedDefines.h"
#include <functional>

class Battlefield;
class Battleground;
class Creature;
class GroupReference;
class InstanceSave;
class Map;
class Player;
class Unit;
class WorldObject;
class WorldPacket;
class WorldSession;
class FunctionProcessor;

namespace WorldPackets
{
    namespace Loot
    {
        struct LootItem;
    }
}

namespace lfg
{
    struct LFGDungeonData;
}

struct MapEntry;

static uint8 const MAX_GROUP_SIZE = 5;
static uint8 const MAX_RAID_SIZE = 40;
static uint8 constexpr MAX_RAID_SUBGROUPS = MAX_RAID_SIZE / MAX_GROUP_SIZE;
#define TARGET_ICONS_COUNT 8
#define RAID_MARKERS_COUNT 8
#define RAID_ROLL_TIMER 180000
#define NORMAL_ROLL_TIMER 60000
#define READY_CHECK_DURATION 35000

enum class GroupScalingRange : int32
{
    Min = 10,
    Max = 30
};

enum RollVote
{
    PASS              = 0,
    NEED              = 1,
    GREED             = 2,
    DISENCHANT        = 3,
    NOT_EMITED_YET    = 4,
    NOT_VALID         = 5
};

enum GroupMemberOnlineStatus
{
    MEMBER_STATUS_OFFLINE       = 0x0000,
    MEMBER_STATUS_ONLINE        = 0x0001, // Lua_UnitIsConnected
    MEMBER_STATUS_PVP           = 0x0002, // Lua_UnitIsPVP
    MEMBER_STATUS_DEAD          = 0x0004, // Lua_UnitIsDead
    MEMBER_STATUS_GHOST         = 0x0008, // Lua_UnitIsGhost
    MEMBER_STATUS_PVP_FFA       = 0x0010, // Lua_UnitIsPVPFreeForAll
    MEMBER_STATUS_PVP_INACTIVE  = 0x0020, // used in calls from Lua_GetPlayerMapPosition/Lua_GetBattlefieldFlagPosition
    MEMBER_STATUS_AFK           = 0x0040, // Lua_UnitIsAFK
    MEMBER_STATUS_DND           = 0x0080, // Lua_UnitIsDND
    MEMBER_STATUS_RAF           = 0x0100,
    MEMBER_STATUS_USING_VEHICLE = 0x0200,
};

enum GroupMemberFlags
{
    MEMBER_FLAG_ASSISTANT   = 0x01,
    MEMBER_FLAG_MAINTANK    = 0x02,
    MEMBER_FLAG_MAINASSIST  = 0x04,
};

enum GroupMemberAssignment
{
    GROUP_ASSIGN_MAINTANK   = 0,
    GROUP_ASSIGN_MAINASSIST = 1,
};

enum GroupType
{
    GROUP_TYPE_NONE         = 0,
    GROUP_TYPE_NORMAL       = 1,
    GROUP_TYPE_BG           = 2,   // TODO: temp name
    GROUP_TYPE_WORLD_PVP    = 4
};

enum GroupFlags
{
    GROUP_FLAG_NONE                 = 0x000,
    GROUP_FLAG_FAKE_RAID            = 0x001,
    GROUP_FLAG_RAID                 = 0x002,
    GROUP_FLAG_LFG_RESTRICTED       = 0x004, // Script_HasLFGRestrictions()
    GROUP_FLAG_LFG                  = 0x008,
    GROUP_FLAG_DESTROYED            = 0x010,
    GROUP_FLAG_ONE_PERSON_PARTY     = 0x020, // Script_IsOnePersonParty()
    GROUP_FLAG_EVERYONE_ASSISTANT   = 0x040, // Script_IsEveryoneAssistant()
    GROUP_FLAG_GUILD_GROUP          = 0x100,

    GROUP_MASK_BGRAID                = GROUP_FLAG_FAKE_RAID | GROUP_FLAG_RAID,
};

enum GroupCategory : uint8
{
    GROUP_CATEGORY_HOME     = 0,
    GROUP_CATEGORY_INSTANCE = 1,

    MAX_GROUP_CATEGORY
};

enum GroupUpdateFlags
{
    GROUP_UPDATE_FLAG_NONE              = 0x00000000,       // nothing
    GROUP_UPDATE_FLAG_OTHER_PARTY       = 0x00000001,       // for (2) uint8 unk
    GROUP_UPDATE_FLAG_STATUS            = 0x00000002,       // uint16
    GROUP_UPDATE_FLAG_POWER_TYPE        = 0x00000004,       // uint8 (PowerType)
    GROUP_UPDATE_FLAG_POWER_DISPLAY_ID  = 0x00000008,       // uint16 (unk)
    GROUP_UPDATE_FLAG_CUR_HP            = 0x00000010,       // uint32 (HP)
    GROUP_UPDATE_FLAG_MAX_HP            = 0x00000020,       // uint32 (max HP)
    GROUP_UPDATE_FLAG_CUR_POWER         = 0x00000040,       // int16 (power value)
    GROUP_UPDATE_FLAG_MAX_POWER         = 0x00000080,       // int16 (max power value)
    GROUP_UPDATE_FLAG_LEVEL             = 0x00000100,       // uint16 (level value)
    GROUP_UPDATE_FLAG_WMO_GROUP_ID      = 0x00000200,       // int16 (unk)
    GROUP_UPDATE_FLAG_ZONE              = 0x00000400,       // uint16 (zone id)
    GROUP_UPDATE_FLAG_SPECIALIZATION_ID = 0x00000800,       // int16 (specID)
    GROUP_UPDATE_FLAG_WMO_DOODAD_PLACEMENT_ID = 0x00001000, // int32 (unk)
    GROUP_UPDATE_FLAG_POSITION          = 0x00002000,       // uint16 (x), uint16 (y), uint16 (z)
    GROUP_UPDATE_FLAG_VEHICLE_SEAT      = 0x00104000,       // int32 (vehicle seat id)
    GROUP_UPDATE_FLAG_AURAS             = 0x00008000,       // uint8 (unk), uint64 (mask), uint32 (count), for each bit set: uint32 (spell id) + uint16 (AuraFlags)  (if has flags Scalable -> 3x int32 (bps))
    GROUP_UPDATE_FLAG_PET               = 0x00010000,       // complex (pet)
    GROUP_UPDATE_FLAG_PHASE             = 0x00020000,       // int32 (unk), uint32 (phase count), for (count) uint16(phaseId)

    GROUP_UPDATE_FULL = GROUP_UPDATE_FLAG_OTHER_PARTY   | GROUP_UPDATE_FLAG_STATUS      | GROUP_UPDATE_FLAG_POWER_TYPE |
                        GROUP_UPDATE_FLAG_POWER_DISPLAY_ID        | GROUP_UPDATE_FLAG_CUR_HP      | GROUP_UPDATE_FLAG_MAX_HP |
                        GROUP_UPDATE_FLAG_CUR_POWER     | GROUP_UPDATE_FLAG_MAX_POWER   | GROUP_UPDATE_FLAG_LEVEL |
                        GROUP_UPDATE_FLAG_WMO_GROUP_ID     | GROUP_UPDATE_FLAG_ZONE        | GROUP_UPDATE_FLAG_SPECIALIZATION_ID |
                        GROUP_UPDATE_FLAG_WMO_DOODAD_PLACEMENT_ID    | GROUP_UPDATE_FLAG_POSITION    | GROUP_UPDATE_FLAG_VEHICLE_SEAT |
                        GROUP_UPDATE_FLAG_AURAS         | GROUP_UPDATE_FLAG_PET         | GROUP_UPDATE_FLAG_PHASE
};

enum GroupUpdatePetFlags
{
    GROUP_UPDATE_FLAG_PET_NONE      = 0x00000000,       // nothing
    GROUP_UPDATE_FLAG_PET_GUID      = 0x00000001,       // ObjectGuid (pet guid)
    GROUP_UPDATE_FLAG_PET_NAME      = 0x00000002,       // cstring (name, NULL terminated string)
    GROUP_UPDATE_FLAG_PET_MODEL_ID  = 0x00000004,       // uint16 (model id)
    GROUP_UPDATE_FLAG_PET_CUR_HP    = 0x00000008,       // uint32 (HP)
    GROUP_UPDATE_FLAG_PET_MAX_HP    = 0x00000010,       // uint32 (max HP)
    GROUP_UPDATE_FLAG_PET_AURAS     = 0x00000020,       // [see GROUP_UPDATE_FLAG_AURAS]

    GROUP_UPDATE_PET_FULL = GROUP_UPDATE_FLAG_PET_GUID      | GROUP_UPDATE_FLAG_PET_NAME    | GROUP_UPDATE_FLAG_PET_MODEL_ID |
                            GROUP_UPDATE_FLAG_PET_CUR_HP    | GROUP_UPDATE_FLAG_PET_MAX_HP  | GROUP_UPDATE_FLAG_PET_AURAS // all pet flags
};

class Roll
{
    public:
        Roll(ObjectGuid _guid, LootItem const& li);
        ~Roll();
        Loot* getLoot();
        uint8 TotalEmited() const;
        bool isValid();
        void FillPacket(WorldPackets::Loot::LootItem& lootItem) const;
        //ItemDisenchantLootEntry const* GetItemDisenchantLoot(Player const* player) const;
        
        struct
        {
            ObjectGuid itemGUID;
            uint32 ItemID = 0;
            uint32 RandomPropertiesSeed = 0;
            uint32 UpgradeID = 0;
            ItemRandomEnchantmentId RandomPropertiesID;

            struct
            {
                uint8 Context = 0;
                std::vector<uint32> BonusListIDs;
            } ItemBonus;

        } item;

        uint8 itemCount;
        ObjectGuid lootedGUID;
        typedef std::map<ObjectGuid, RollVote> PlayerVote;
        PlayerVote playerVote;                              //vote position correspond with player position (in group)
        uint8 totalPlayersRolling;
        uint8 totalNeed;
        uint8 totalGreed;
        uint8 totalPass;
        uint8 itemSlot;
        uint8 aoeSlot;
        uint8 rollVoteMask;
        bool canTradeToTapList;
        // the loot and the winner's bags belong to this map: the roll is concluded only from its thread
        uint32 mapId;
        uint32 instanceId;
};

struct InstanceGroupBind
{
    InstanceGroupBind();

    InstanceSave* save;
    bool perm;
    /* permanent InstanceGroupBinds exist if the leader has a permanent
       PlayerInstanceBind for the same instance. */
};

struct RaidMarker
{
    RaidMarker(uint32 mapId, float positionX, float positionY, float positionZ, ObjectGuid transportGuid = ObjectGuid::Empty);

    WorldLocation Location;
    ObjectGuid TransportGUID;
};

class TC_GAME_API Group
{
    public:
        struct MemberSlot
        {
            ObjectGuid Guid;
            std::string Name;
            uint8 Class = 0;
            uint8 Group = 0;
            uint8 Flags = 0;
            uint8 Roles = 0;
            bool fakeOnline = false;
            bool ReadyChecked = false;
        };
        typedef std::list<MemberSlot> MemberSlotList;
        typedef MemberSlotList::const_iterator member_citerator;

        typedef std::unordered_map< uint32 /*mapId*/, InstanceGroupBind> BoundInstancesMap;
    protected:
        typedef MemberSlotList::iterator member_witerator;
        typedef std::set<Player*> InvitesList;

        typedef std::vector<Roll*> Rolls;

    public:
        Group();
        ~Group();

        bool   Create(Player* leader, uint8 subType = 0, bool isLfg = false);
        void   LoadGroupFromDB(Field* field);
        void   LoadMemberFromDB(ObjectGuid::LowType guidLow, uint8 memberFlags, uint8 subgroup, uint8 roles);
        bool   AddInvite(Player* player);
        void   RemoveInvite(Player* player);
        void   RemoveAllInvites();
        bool   AddLeaderInvite(Player* player);
        bool   AddMember(Player* player);
        bool   AddMysteryMember(ObjectGuid::LowType guidLow, std::string Name, uint8 Class, uint8 Roles, uint8 Flags, bool fakeOnline);
        bool   RemoveMember(ObjectGuid const& guid, bool disbandInfo = false, const RemoveMethod &method = GROUP_REMOVEMETHOD_DEFAULT, ObjectGuid kicker = ObjectGuid::Empty, const char* reason = nullptr);
        bool   RemoveMemberQueue(ObjectGuid const& guid, const RemoveMethod &method = GROUP_REMOVEMETHOD_DEFAULT, ObjectGuid kicker = ObjectGuid::Empty, const char* reason = nullptr);
        bool   AddCreatureMember(Creature* creature);
        void   ChangeLeader(ObjectGuid const& guid, int8 partyIndex = 0);
        bool   RemoveCreatureMember(ObjectGuid const& guid);
        void   SetLootMethod(LootMethod method);
        void   SetLooterGuid(ObjectGuid const& guid);
        void   UpdateLooterGuid(WorldObject* pLootedObject, bool ifneed = false);
        void   SetLootThreshold(ItemQualities threshold);
        // never deletes the group: GroupMgr frees it after a grace delay (QueueForDelete)
        void   Disband(bool hideDestroy=false);
        bool   IsDisbanding() const;
        // pending invite (group not created yet), both under m_inviteLock: a dropped group is already queued for deletion
        bool   IsDisposed() const { return m_disposed; }
        void   SetDisposed() { m_disposed = true; }
        void   SetLfgRoles(ObjectGuid guid, const uint8 roles);
        uint8  GetLfgRoles(ObjectGuid guid);

        bool IsFull() const;
        bool isLFGGroup() const;
        bool isRaidGroup() const;
        bool isBGGroup() const;
        bool isArenaGroup()const;
        bool isBFGroup() const;
        bool IsCreated() const;

        ObjectGuid GetLeaderGUID() const;
        ObjectGuid GetGUID() const;
        uint32 GetGUIDLow() const;
        const char * GetLeaderName() const;
        LootMethod GetLootMethod() const;
        ObjectGuid GetLooterGuid() const;
        ItemQualities GetLootThreshold() const;

        uint32 GetDbStoreId() const;

        // members stand on any map: the worker runs under the accessor lock and may only build and send packets
        template<class Worker>
        void BroadcastWorker(Worker& worker)
        {
            ForEachLinkedMember([&worker](Player* player) { worker(player); });
        }

        template<class Worker>
        void BroadcastWorker(Worker const& worker) const
        {
            ForEachLinkedMember([&worker](Player* player) { worker(player); });
        }

        void ForEachLinkedMember(std::function<void(Player*)> const& fn) const;
        // only the members standing on `map`: for code running in that map's thread, which may use their Player freely
        void ForEachMemberInMap(Map* map, std::function<void(Player*)> const& fn) const;

        bool IsMember(ObjectGuid guid) const;
        bool IsLeader(ObjectGuid guid) const;
        bool IsAssistant(ObjectGuid guid) const;
        bool IsGuildGroup(ObjectGuid const& guildId, bool AllInSameMap = false, bool AllInSameInstanceId = false);
        void UpdateGuildAchievementCriteria(CriteriaTypes type, uint32 miscValue1, uint32 miscValue2, uint32 miscValue3, Unit* unit, WorldObject* rewardSource);

        Player* GetInvited(ObjectGuid guid);
        Player* GetInvited(std::string const& name);

        bool SameSubGroup(ObjectGuid guid1, ObjectGuid guid2) const;
        bool SameSubGroup(ObjectGuid guid1, MemberSlot const* slot2) const;
        bool SameSubGroup(Player const* member1, Player const* member2) const;
        bool HasFreeSlotSubGroup(uint8 subgroup) const;

        MemberSlotList GetMemberSlots() const;
        GroupReference* GetFirstMember();
        GroupReference const* GetFirstMember() const;
        uint32 GetMembersCount() const;
        uint8 GetMemberGroup(ObjectGuid guid) const;

        void ChangeFlagEveryoneAssistant(bool apply);
        void ChangeFlagGuildGroup(bool apply);
        bool IsGuildGroup() const;
        void ConvertToLFG(lfg::LFGDungeonData const* dungeon, bool update = true);
        void ConvertToRaid(bool update = true);
        void ConvertToGroup();

        void SetBattlegroundGroup(Battleground* bg);
        void SetBattlefieldGroup(Battlefield* bf);
        uint8 CanJoinBattlegroundQueue(Battleground const* bgOrTemplate, uint8 bgQueueTypeId, uint32 MinPlayerCount, bool isRated, uint32 arenaSlot, ObjectGuid& errorGuid);

        void ChangeMembersGroup(ObjectGuid guid, uint8 group);
        void SwapMembersGroups(ObjectGuid firstGuid, ObjectGuid secondGuid);
        void SetTargetIcon(uint8 id, ObjectGuid whoGuid, ObjectGuid targetGuid, uint8 partyIndex);
        void SetGroupMemberFlag(ObjectGuid guid, bool apply, GroupMemberFlags flag);
        void setGroupMemberRole(ObjectGuid guid, uint32 role);
        void RemoveUniqueGroupMemberFlag(GroupMemberFlags flag);

        void SetDungeonDifficultyID(Difficulty difficulty);
        void SetRaidDifficultyID(Difficulty difficulty);
        void SetLegacyRaidDifficultyID(Difficulty difficulty);
        Difficulty GetDifficultyID(MapEntry const* mapEntry) const;
        Difficulty GetDungeonDifficultyID() const;
        Difficulty GetRaidDifficultyID() const;
        Difficulty GetLegacyRaidDifficultyID() const;
        void ResetInstances(uint8 method, bool isRaid, bool isLegacy, Player* SendMsgTo);

        void SendTargetIconList(int8 partyIndex);
        void SendUpdate();
        void SendUpdateToPlayer(ObjectGuid playerGUID, MemberSlot* slot = nullptr);
        void SendUpdateDestroyGroupToPlayer(Player* player) const;

        void UpdatePlayerOutOfRange(Player* player);

        void BroadcastPacket(WorldPacket const* packet, bool ignorePlayersInBGRaid, int group = -1, ObjectGuid ignore = ObjectGuid::Empty);
        void BroadcastAddonMessagePacket(WorldPacket const* packet, std::string const& prefix, bool ignorePlayersInBGRaid, int group = -1, ObjectGuid ignore = ObjectGuid::Empty);
        void BroadcastReadyCheck(WorldPacket const* packet);
        void OfflineReadyCheck();

        void AddRaidMarker(uint8 markerID, uint32 mapID, float positionX, float positionY, float positionZ, ObjectGuid transportGuid = ObjectGuid::Empty);
        void DeleteRaidMarker(uint8 markerID);
        void SendRaidMarkersChanged(WorldSession* session = nullptr, int8 partyIndex = 0);

        bool isRollLootActive() const { return RollIsActive(); }
        void SendLootStartRoll(uint32 mapid, const Roll &r);
        void SendLootStartRollToPlayer(uint32 mapId, Player* p, bool canNeed, Roll const& r);
        void SendLootRoll(ObjectGuid TargetGuid, uint8 RollNumber, uint8 RollType, const Roll &r);
        void SendLootRollWon(ObjectGuid TargetGuid, uint8 RollNumber, uint8 RollType, const Roll &r);
        void SendLootAllPassed(Roll const& roll);
        void SendLooter(Creature* creature, Player* pLooter);
        void GroupLoot(Loot* loot, WorldObject* pLootedObject);
        void MasterLoot(Loot* loot, WorldObject* pLootedObject);
        Rolls::iterator GetRoll(uint8 slot, std::set<ObjectGuid> const& validLoots);
        void ErraseRollbyRealSlot(uint8 slot, Loot* loot);
        void CountTheRoll(Roll* roll);
        bool CountRollVote(Player* voter, uint8 slot, uint8 Choise);
        // For modules (playerbots vote as a client would, through CountRollVote): the rolls of live loot still
        // waiting for this member's vote
        struct PendingRoll
        {
            uint8 aoeSlot = 0;
            uint32 itemId = 0;
            std::vector<uint32> bonusListIds;
            uint8 rollVoteMask = 0;
        };
        std::vector<PendingRoll> GetPendingRolls(ObjectGuid const& member) const;
        void DoRollForAllMembers(ObjectGuid guid, uint8 slot, uint32 mapid, Loot*, LootItem&, Player*);
        void EndRoll(Loot* loot);
        void ConcludeRollOnMap(Player* onRollMap, ObjectGuid const& rollItemGuid);
        void ClearAoeSlots();
        bool isRolledSlot(uint8 _slot);
        // only rolls whose loot still exists count: a roll whose loot vanished is never concluded
        bool RollIsActive() const;
        void PurgeStaleRolls();

        void ResetMaxEnchantingLevel();

        void LinkMember(GroupReference* pRef);
        void DelinkMember(ObjectGuid guid);

        InstanceGroupBind BindToInstance(InstanceSave* save, bool permanent, bool load = false);
        void UnbindInstance(uint32 mapid, uint8 difficulty, bool unload = false);
        void UnbindInstance(InstanceSave* save, bool unload = true);
        // Copies taken under m_bound_lock (save == nullptr: not bound). Use these from map threads.
        InstanceGroupBind GetBoundInstanceCopy(Player* player);
        InstanceGroupBind GetBoundInstanceCopy(Map* aMap);
        InstanceGroupBind GetBoundInstanceCopy(MapEntry const* mapEntry);
        InstanceGroupBind GetBoundInstanceCopy(Difficulty difficulty, uint32 mapId);
        BoundInstancesMap GetBoundInstancesCopy(Difficulty difficulty);
        // Pointer into m_boundInstances: erased by an unbind from another map thread. Kept until the callers move to the copies.
        InstanceGroupBind* GetBoundInstance(Player* player);
        InstanceGroupBind* GetBoundInstance(Map* aMap);
        InstanceGroupBind* GetBoundInstance(MapEntry const* mapEntry);
        InstanceGroupBind* GetBoundInstance(Difficulty difficulty, uint32 mapId);
        BoundInstancesMap& GetBoundInstances(Difficulty difficulty);
        void UpdateInstance(InstanceSave* save);

        void BroadcastGroupUpdate();
        bool StartReadyCheck(ObjectGuid starterGuid, int8 partyIndex);
        bool IsReadyCheckActive() const;
        bool SetMemberReadyChecked(ObjectGuid guid);
        bool IsReadyCheckCompleted() const;
        void EndReadyCheck();
        void ReadyCheckTimeout(uint32 startTime);
        uint32 GetReadyCheckStartTime() const { return m_readyCheckStartTime; }
        uint8 GetGroupFlags() const;
        GroupCategory GetGroupCategory() const;

        uint32 GetAverageMMR(uint8 bracket) const;
        ItemQualities GetThreshold() const;
        uint32 GetTeam() const;

        // Members of one group act from several map threads at once.
        // m_lock: m_memberSlots, m_subGroupsCounts, m_memberMgr links, m_invitees, RollId and the rolls.
        // m_bound_lock: m_boundInstances.
        // Both are leaves: only plain accessors are called under them (no other manager, no Player method that may lock,
        // no database, no packet), and they are never nested. LFGListMgr may take m_lock under its own lock, not the reverse.
        // m_inviteLock serialises the acceptances of one pending invite (group creation) and is taken first.
        // A Group* looked up (GroupMgr, a player's invite) stays valid for the current call: Disband only queues the
        // group, GroupMgr frees it from the world thread after a grace delay.
        mutable std::recursive_mutex m_lock;
        mutable std::recursive_mutex m_bound_lock;
        std::mutex m_inviteLock;

        bool InChallenge();
        bool GetMaxCountOfRolesForArenaQueue(uint8 role);

        // Mythic+ key state. The fields below are guarded by m_lock: from a map thread use these accessors, not the fields.
        struct ChallengeInfo
        {
            MapChallengeModeEntry const* Entry = nullptr;
            ObjectGuid Owner;
            ObjectGuid Item;
            uint32 Level = 0;
            uint32 InstanceID = 0;
            std::array<uint32, 3> Affixes{};
        };
        ChallengeInfo GetChallengeInfo() const;
        // records the instance of the run and returns the key state it starts with
        ChallengeInfo StartChallenge(uint32 instanceId);
        void SetChallenge(MapChallengeModeEntry const* entry, ObjectGuid owner, ObjectGuid item, uint32 level, std::array<uint32, 3> const& affixes);
        void ResetChallenge();
        // only if the group still runs this instance with this key (not a key started since)
        bool ResetChallengeIf(uint32 instanceId, ObjectGuid item);

        ObjectGuid m_challengeOwner;
        ObjectGuid m_challengeItem;
        MapChallengeModeEntry const* m_challengeEntry;
        uint32 m_challengeLevel;
        uint32 m_challengeInstanceID;
        std::array<uint32, 3> m_affixes{};

        void AddDelayedEvent(uint64 timeOffset, std::function<void()>&& function)
        {
            m_Functions.AddDelayedEvent(timeOffset, std::move(function));
        }

        lfg::LFGDungeonData const* m_dungeon;

    protected:
        bool _setMembersGroup(ObjectGuid guid, uint8 group);
        void SaveLootSettings();
        void _homebindIfInstance(Player* player);

        void _initRaidSubGroupsCounter();
        member_citerator _getMemberCSlot(ObjectGuid Guid) const;
        member_witerator _getMemberWSlot(ObjectGuid Guid);
        void SubGroupCounterIncrease(uint8 subgroup);
        void SubGroupCounterDecrease(uint8 subgroup);
        static void ToggleGroupMemberFlag(member_witerator slot, uint8 flag, bool apply);

        struct MemberRef
        {
            ObjectGuid Guid;
            uint8 SubGroup;
        };
        std::vector<MemberRef> GetMemberRefs() const;
        std::vector<ObjectGuid> GetMemberGuids() const;
        Roll* DetachRoll(Rolls::iterator rollI);
        std::set<ObjectGuid> GetValidRollLoots() const;
        std::set<ObjectGuid> GetVanishedRollLoots() const;
        void ArmRollTimer(WorldObject* lootedObject);
        void PostRollConclusion(Roll const& roll);
        void ConvertLeaderInstances(ObjectGuid const& leaderGuid, bool switchLeader);
        void PostSubGroupChange(ObjectGuid const& guid, uint8 subGroup);

        MemberSlotList      m_memberSlots;
        GroupRefManager     m_memberMgr;
        InvitesList         m_invitees;
        ObjectGuid          m_leaderGuid;
        std::string         m_leaderName;
        GroupFlags          m_groupFlags;
        GroupCategory       m_groupCategory;
        Difficulty          m_dungeonDifficulty;
        Difficulty          m_raidDifficulty;
        Difficulty          m_legacyRaidDifficulty;
        Battleground*       m_bgGroup;
        Battlefield*        m_bfGroup;
        ObjectGuid          m_targetIcons[TARGET_ICONS_COUNT];
        LootMethod          m_lootMethod;
        ItemQualities       m_lootThreshold;
        ObjectGuid          m_looterGuid;
        Rolls               RollId;
        BoundInstancesMap   m_boundInstances[MAX_BOUND];
        uint8*              m_subGroupsCounts;
        ObjectGuid          m_guid;
        uint32              m_maxEnchantingLevel;
        uint32              m_dbStoreId;                    // Represents the ID used in database (Can be reused by other groups if group was disbanded)
        uint32              m_readyCheckStartTime;
        int8                m_readyCheckPartyIndex;
        uint8               m_aoe_slots;                    // centrilize aoe loot method
        bool                m_readyCheck;
        bool                m_disbanding;                   // m_lock: set once by the Disband call that owns the group's end
        bool                m_disposed;                     // m_inviteLock
        bool                m_bindsClosed = false;          // m_bound_lock: Disband has emptied the binds, no new one may appear
        uint32              _team;

        std::array<std::unique_ptr<RaidMarker>, RAID_MARKERS_COUNT> m_markers;
        uint32              m_activeMarkers;
        FunctionProcessor m_Functions;
};
#endif
