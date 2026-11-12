////////////////////////////////////////////
///                                      ///
///              DEMONCORE               ///
///    Copyright 2026 Titans-Project     ///
///         All Rights Reserved.         ///
///                                      ///
///      Based on TrinityCore code       ///
///   Copyright 2008-2012 TrinityCore    ///
///   <http://www.trinitycore.org/>      ///
///                                      ///
////////////////////////////////////////////

#include "Common.h"
#include "Opcodes.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include "Player.h"
#include "World.h"
#include "ObjectMgr.h"
#include "GroupMgr.h"
#include "Group.h"
#include <utility>
#include "Formulas.h"
#include "ObjectAccessor.h"
#include "Battleground.h"
#include "BattlegroundMgr.h"
#include "MapManager.h"
#include "InstanceSaveMgr.h"
#include "Util.h"
#include "LFGMgr.h"
#include "UpdateFieldFlags.h"
#include "GuildMgr.h"
#include "Bracket.h"
#include "LootPackets.h"
#include "PartyPackets.h"
#include "ItemPackets.h"
#include "LootMgr.h"
#include "LFGListMgr.h"
#include "FunctionProcessor.h"
#include "DatabaseEnv.h"
#include "PlayerDefines.h"
#include "GameTime.h"

Roll::Roll(ObjectGuid _guid, LootItem const& li) : itemCount(li.count), totalPlayersRolling(0), totalNeed(0), totalGreed(0), totalPass(0), itemSlot(0), aoeSlot(0), rollVoteMask(ROLL_ALL_TYPE_NO_DISENCHANT),
    canTradeToTapList(li.allowedGUIDs.size() > 1), mapId(0), instanceId(0)
{
    item.itemGUID = _guid;
    item.ItemID = li.item.ItemID;
    item.UpgradeID = li.item.UpgradeID;
    item.RandomPropertiesID = li.item.RandomPropertiesID;
    item.RandomPropertiesSeed = li.item.RandomPropertiesSeed;
    item.ItemBonus.Context = li.item.ItemBonus.Context;
    item.ItemBonus.BonusListIDs = li.item.ItemBonus.BonusListIDs;
}

Roll::~Roll() = default;

Loot* Roll::getLoot()
{
    return sLootMgr->GetLoot(lootedGUID);
}

uint8 Roll::TotalEmited() const
{
    return totalNeed + totalGreed + totalPass;
}

bool Roll::isValid()
{
    return getLoot() != nullptr;
}

void Roll::FillPacket(WorldPackets::Loot::LootItem& lootItem) const
{
    lootItem.Type = LOOT_ITEM_TYPE_ITEM;
    lootItem.UIType = totalPlayersRolling > totalNeed + totalGreed + totalPass ? 1 : 0;
    lootItem.Quantity = itemCount;
    lootItem.LootListID = itemSlot + 1;
    lootItem.LootItemType = LOOT_ITEM_TYPE_ITEM;
    // from the roll's own copy: the loot belongs to another thread than most voters
    lootItem.CanTradeToTapList = canTradeToTapList;
    lootItem.Loot.Initialize(this);
    if (item.UpgradeID)
    {
        lootItem.Loot.Modifications.emplace();
        lootItem.Loot.Modifications->Insert(ITEM_MODIFIER_UPGRADE_ID, item.UpgradeID);
    }
}

InstanceGroupBind::InstanceGroupBind() : save(nullptr), perm(false)
{
}

RaidMarker::RaidMarker(uint32 mapId, float positionX, float positionY, float positionZ, ObjectGuid transportGuid)
{
    Location.WorldRelocate(mapId, positionX, positionY, positionZ);
    TransportGUID = transportGuid;
}

Group::Group(): m_challengeInstanceID(0)
{
    m_leaderName = "";
    m_groupFlags = GROUP_FLAG_NONE;
    m_dungeonDifficulty = DIFFICULTY_NORMAL;
    m_raidDifficulty = DIFFICULTY_10_N;
    m_legacyRaidDifficulty = DIFFICULTY_10_N;
    m_bgGroup = nullptr;
    m_bfGroup = nullptr;
    m_lootMethod = PERSONAL_LOOT;
    m_lootThreshold = ITEM_QUALITY_UNCOMMON;
    m_subGroupsCounts = nullptr;
    m_maxEnchantingLevel = 0;
    m_dbStoreId = 0;
    m_readyCheckStartTime = 0;
    m_readyCheckPartyIndex = 0;
    m_readyCheck = false;
    m_disbanding = false;
    m_disposed = false;
    m_aoe_slots = 0;
    m_activeMarkers = 0;
    m_groupCategory = GROUP_CATEGORY_HOME;
    _team = 0;
    m_challengeEntry = nullptr;
    m_challengeLevel = 0;
    m_affixes.fill(0);
    m_dungeon = nullptr;

    for (auto& itr : m_targetIcons)
        itr.Clear();

    for (uint8 i = 0; i < RAID_MARKERS_COUNT; ++i)
        m_markers[i] = nullptr;
}

Group::~Group()
{
    if (m_bgGroup)
    {
        TC_LOG_DEBUG("bg.battleground", "Group::~Group: battleground group being deleted.");
        if (m_bgGroup->GetBgRaid(ALLIANCE) == this)
            m_bgGroup->SetBgRaid(ALLIANCE, nullptr);
        else if (m_bgGroup->GetBgRaid(HORDE) == this)
            m_bgGroup->SetBgRaid(HORDE, nullptr);
        else
            TC_LOG_ERROR("misc", "Group::~Group: battleground group is not linked to the correct battleground.");
    }

    while (!RollId.empty())
    {
        auto itr = RollId.begin();
        Roll *r = *itr;
        RollId.erase(itr);
        delete r;
    }

    // it is undefined whether objectmgr (which stores the groups) or instancesavemgr
    // will be unloaded first so we must be prepared for both cases
    // this may unload some instance saves
    for (auto& itr : m_boundInstances)
        for (auto& itr2 : itr)
            itr2.second.save->RemoveGroup(this);

    // Sub group counters clean up
    delete[] m_subGroupsCounts;
}

// the leader may stand on another map: only his guild id is read, under the accessor lock
static void UpdateGuildGroupFlagFromLeader(Group* group)
{
    ObjectGuid::LowType leaderGuildId = 0;
    if (!ObjectAccessor::WithPlayer(group->GetLeaderGUID(), [&leaderGuildId](Player* leader) { leaderGuildId = leader->GetGuildId(); }))
        return;

    if (Guild* guild = sGuildMgr->GetGuildById(leaderGuildId))
        group->ChangeFlagGuildGroup(group->IsGuildGroup(guild->GetGUID()));
    else
        group->ChangeFlagGuildGroup(false);
}

// For actions posted to a member's thread: they may run after the group is freed, so nothing reads it
static void HomebindIfInstance(Player* player)
{
    if (!player->isGameMaster() && sMapStore.LookupEntry(player->GetMapId())->IsDungeon())
        player->m_InstanceValid = false;
}

static void SendDestroyedPartyUpdate(Player* player, GroupCategory category, ObjectGuid const& groupGuid)
{
    WorldPackets::Party::PartyUpdate partyUpdate;
    partyUpdate.PartyFlags = GROUP_FLAG_DESTROYED;
    partyUpdate.PartyIndex = category;
    partyUpdate.PartyType = GROUP_TYPE_NONE;
    partyUpdate.PartyGUID = groupGuid;
    partyUpdate.MyIndex = -1;
    partyUpdate.SequenceNum = player->NextGroupUpdateSequenceNumber(category);
    player->SendDirectMessage(partyUpdate.Write());
}

bool Group::Create(Player* leader, uint8 subType /*= 0*/, bool isLfg /*= false*/)
{
    ObjectGuid leaderGuid = leader->GetGUID();

    m_guid = ObjectGuid::Create<HighGuid::Party>(sGroupMgr->GenerateGroupId(), subType);
    m_leaderGuid = leaderGuid;
    m_leaderName = leader->GetName();
    leader->SetFlag(PLAYER_FIELD_PLAYER_FLAGS, PLAYER_FLAGS_GROUP_LEADER);

    m_groupFlags = GROUP_FLAG_NONE;

    if (isBGGroup() || isBFGroup())
    {
        m_groupFlags = GROUP_MASK_BGRAID;
        m_groupCategory = GROUP_CATEGORY_INSTANCE;
    }

    if (isLfg)
    {
        m_groupFlags = GroupFlags(m_groupFlags | GROUP_FLAG_LFG | GROUP_FLAG_LFG_RESTRICTED);
        m_groupCategory = GROUP_CATEGORY_INSTANCE;
    }

    if (m_groupFlags & GROUP_FLAG_RAID)
        _initRaidSubGroupsCounter();

    m_lootMethod = PERSONAL_LOOT;
    m_lootThreshold = ITEM_QUALITY_UNCOMMON;
    m_looterGuid = leaderGuid;

    m_dungeonDifficulty = DIFFICULTY_NORMAL;
    m_raidDifficulty = DIFFICULTY_NORMAL_RAID;
    m_legacyRaidDifficulty = DIFFICULTY_10_N;

    _team = leader->GetTeam();

    if (!isBGGroup() && !isBFGroup())
    {
        m_dungeonDifficulty = leader->GetDungeonDifficultyID();
        m_raidDifficulty = leader->GetRaidDifficultyID();
        m_legacyRaidDifficulty = leader->GetLegacyRaidDifficultyID();

        m_dbStoreId = sGroupMgr->GenerateNewGroupDbStoreId(this);

        // Store group in database
        CharacterDatabasePreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_INS_GROUP);

        uint8 index = 0;

        stmt->setUInt32(index++, m_dbStoreId);
        stmt->setUInt64(index++, GetLeaderGUID().GetCounter());
        stmt->setUInt8(index++, uint8(m_lootMethod));
        stmt->setUInt64(index++, m_looterGuid.GetCounter());
        stmt->setUInt8(index++, uint8(m_lootThreshold));
        stmt->setBinary(index++, m_targetIcons[0].GetRawValue());
        stmt->setBinary(index++, m_targetIcons[1].GetRawValue());
        stmt->setBinary(index++, m_targetIcons[2].GetRawValue());
        stmt->setBinary(index++, m_targetIcons[3].GetRawValue());
        stmt->setBinary(index++, m_targetIcons[4].GetRawValue());
        stmt->setBinary(index++, m_targetIcons[5].GetRawValue());
        stmt->setBinary(index++, m_targetIcons[6].GetRawValue());
        stmt->setBinary(index++, m_targetIcons[7].GetRawValue());
        stmt->setUInt8(index++, uint8(m_groupFlags));
        stmt->setUInt32(index++, uint8(m_dungeonDifficulty != DIFFICULTY_MYTHIC_KEYSTONE ? m_dungeonDifficulty : DIFFICULTY_MYTHIC_DUNGEON));
        stmt->setUInt32(index++, uint8(m_legacyRaidDifficulty));
        stmt->setUInt32(index++, uint8(m_raidDifficulty));

        CharacterDatabase.Execute(stmt);


        ASSERT(AddMember(leader)); // If the leader can't be added to a new group because it appears full, something is clearly wrong.

        if (!isLFGGroup())
        {
            // registered before the conversion is posted, which finds the group by guid (callers add it again: no-op)
            sGroupMgr->AddGroup(this);
            ConvertLeaderInstances(leader->GetGUID(), false);
        }
    }
    else if (!AddMember(leader))
        return false;

    return true;
}

void Group::LoadGroupFromDB(Field* fields)
{
    m_dbStoreId = fields[15].GetUInt32();
    m_guid = ObjectGuid::Create<HighGuid::Party>(sGroupMgr->GenerateGroupId(), 0);
    m_leaderGuid = ObjectGuid::Create<HighGuid::Player>(fields[0].GetUInt64());

    // group leader not exist
    if (!ObjectMgr::GetPlayerNameByGUID(GetLeaderGUID(), m_leaderName))
        return;

    m_lootMethod = LootMethod(fields[1].GetUInt8());
    m_looterGuid = ObjectGuid::Create<HighGuid::Player>(fields[2].GetUInt64());
    m_lootThreshold = ItemQualities(fields[3].GetUInt8());

    for (uint8 i = 0; i < TARGET_ICONS_COUNT; ++i)
        m_targetIcons[i].SetRawValue(fields[4 + i].GetBinary());

    m_groupFlags = GroupFlags(fields[12].GetUInt8());
    if (m_groupFlags & GROUP_FLAG_RAID)
        _initRaidSubGroupsCounter();

    m_dungeonDifficulty = Player::CheckLoadedDungeonDifficultyID(Difficulty(fields[13].GetUInt8()));
    m_raidDifficulty = Player::CheckLoadedRaidDifficultyID(Difficulty(fields[14].GetUInt8()));
    m_legacyRaidDifficulty = Player::CheckLoadedLegacyRaidDifficultyID(Difficulty(fields[18].GetUInt8()));

    _team = sObjectMgr->GetPlayerTeamByGUID(m_leaderGuid);

    if (m_groupFlags & GROUP_FLAG_LFG)
    {
        m_groupCategory = GROUP_CATEGORY_INSTANCE;
        sLFGMgr->_LoadFromDB(fields, GetGUID());
    }
}

void Group::LoadMemberFromDB(ObjectGuid::LowType guidLow, uint8 memberFlags, uint8 subgroup, uint8 roles)
{
    MemberSlot member;
    member.Guid = ObjectGuid::Create<HighGuid::Player>(guidLow);

    // skip non-existed member
    if (!ObjectMgr::GetPlayerNameByGUID(member.Guid, member.Name))
    {
        CharacterDatabasePreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_GROUP_MEMBER);
        stmt->setUInt64(0, guidLow);
        CharacterDatabase.Execute(stmt);
        return;
    }

    if (CharacterInfo const* characterInfo = sWorld->GetCharacterInfo(member.Guid))
        member.Class = characterInfo->Class;

    member.Group = subgroup;
    member.Flags = memberFlags;
    member.Roles = roles;

    {
        std::lock_guard<std::recursive_mutex> guard(m_lock);
        m_memberSlots.push_back(member);
        SubGroupCounterIncrease(subgroup);
    }

    sLFGMgr->SetupGroupMember(member.Guid, GetGUID());
}

void Group::ChangeFlagEveryoneAssistant(bool apply)
{
    if (apply)
        m_groupFlags = GroupFlags(m_groupFlags | GROUP_FLAG_EVERYONE_ASSISTANT);
    else
        m_groupFlags = GroupFlags(m_groupFlags &~GROUP_FLAG_EVERYONE_ASSISTANT);

    SendUpdate();
}

void Group::ChangeFlagGuildGroup(bool apply)
{
    if (!apply && !IsGuildGroup() || apply && IsGuildGroup())
        return;

    if (apply)
        m_groupFlags = GroupFlags(m_groupFlags | GROUP_FLAG_GUILD_GROUP);
    else
        m_groupFlags = GroupFlags(m_groupFlags &~GROUP_FLAG_GUILD_GROUP);

    SendUpdate();
}

bool Group::IsGuildGroup() const
{
    return (m_groupFlags & GROUP_FLAG_GUILD_GROUP) != 0;
}

void Group::ConvertToLFG(lfg::LFGDungeonData const* dungeon, bool update /*= true*/)
{
    if (dungeon->dbc->IsRaidType())
        ConvertToRaid(!update);

    m_dungeon = dungeon;

    if (update)
    {
        m_groupFlags = GroupFlags(m_groupFlags | GROUP_FLAG_LFG | GROUP_FLAG_LFG_RESTRICTED);
        m_groupCategory = GROUP_CATEGORY_INSTANCE;
        m_lootMethod = PERSONAL_LOOT;

        if (!isBGGroup() && !isBFGroup())
        {
            CharacterDatabasePreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_GROUP_TYPE);
        
            stmt->setUInt8(0, uint8(m_groupFlags));
            stmt->setUInt32(1, m_dbStoreId);
        
            CharacterDatabase.Execute(stmt);
        }
        SendUpdate();
    }
}

void Group::ConvertToRaid(bool update /*= true*/)
{
    {
        std::lock_guard<std::recursive_mutex> guard(m_lock);
        m_groupFlags = GroupFlags(m_groupFlags | GROUP_FLAG_RAID);
        m_lootMethod = PERSONAL_LOOT;

        _initRaidSubGroupsCounter();
    }

    if (update && !isBGGroup() && !isBFGroup())
    {
        CharacterDatabasePreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_GROUP_TYPE);

        stmt->setUInt8(0, uint8(m_groupFlags));
        stmt->setUInt32(1, m_dbStoreId);

        CharacterDatabase.Execute(stmt);
    }

    if (update)
        SendUpdate();

    // update quest related GO states (quest activity dependent from raid membership)
    for (ObjectGuid const& memberGuid : GetMemberGuids())
        ObjectAccessor::PostToPlayer(memberGuid, [](Player* player) -> void
        {
            player->UpdateAreaQuestTasks(0, player->GetCurrentAreaID());
            player->UpdateAreaQuestTasks(player->GetCurrentAreaID(), 0);
            player->UpdateForQuestWorldObjects();
        }, 100);
}

void Group::ConvertToGroup()
{
    {
        std::lock_guard<std::recursive_mutex> guard(m_lock);
        if (m_memberSlots.size() > 5)
            return; // What message error should we send?

        // keep the guild/LFG/everyone-assistant flags
        m_groupFlags = GroupFlags(m_groupFlags & ~GROUP_FLAG_RAID);
        m_lootMethod = PERSONAL_LOOT;

        if (m_subGroupsCounts)
        {
            delete[] m_subGroupsCounts;
            m_subGroupsCounts = nullptr;
        }
    }

    if (!isBGGroup() && !isBFGroup())
    {
        CharacterDatabasePreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_GROUP_TYPE);

        stmt->setUInt8(0, uint8(m_groupFlags));
        stmt->setUInt32(1, m_dbStoreId);

        CharacterDatabase.Execute(stmt);
    }

    SendUpdate();

    // update quest related GO states (quest activity dependent from raid membership)
    for (ObjectGuid const& memberGuid : GetMemberGuids())
        ObjectAccessor::PostToPlayer(memberGuid, [](Player* player) -> void
        {
            player->UpdateAreaQuestTasks(0, player->GetCurrentAreaID());
            player->UpdateAreaQuestTasks(player->GetCurrentAreaID(), 0);
            player->UpdateForQuestWorldObjects();
        }, 100);
}

bool Group::AddInvite(Player* player)
{
    if (!player || player->GetGroupInvite())
        return false;
    Group* group = player->GetGroup();
    if (group && (group->isBGGroup() || group->isBFGroup()))
        group = player->GetOriginalGroup();
    if (group)
        return false;

    RemoveInvite(player);

    {
        // done under the lock so that his logout (RemoveInvite) cannot run in between
        std::lock_guard<std::recursive_mutex> guard(m_lock);
        // Disband set the flag under this lock before RemoveAllInvites: an invite added now would outlive the group
        if (m_disbanding)
            return false;
        // another group may have invited him since the test above, from another map
        if (!player->TrySetGroupInvite(this))
            return false;
        m_invitees.insert(player);
    }

    sScriptMgr->OnGroupInviteMember(this, player->GetGUID());

    return true;
}

bool Group::AddLeaderInvite(Player* player)
{
    if (!AddInvite(player))
        return false;

    m_leaderGuid = player->GetGUID();
    m_leaderName = player->GetName();
    return true;
}

void Group::RemoveInvite(Player* player)
{
    if (player)
    {
        std::lock_guard<std::recursive_mutex> guard(m_lock);
        m_invitees.erase(player);
        player->ClearGroupInviteIf(this);
    }
}

void Group::RemoveAllInvites()
{
    std::lock_guard<std::recursive_mutex> guard(m_lock);
    for (InvitesList::iterator itr = m_invitees.begin(); itr != m_invitees.end(); ++itr)
        if (Player* player = *itr)
            player->ClearGroupInviteIf(this);

    m_invitees.clear();
}

Player* Group::GetInvited(ObjectGuid guid)
{
    std::lock_guard<std::recursive_mutex> guard(m_lock);
    for (InvitesList::iterator itr = m_invitees.begin(); itr != m_invitees.end(); ++itr)
    {
        if (Player* player = *itr)
            if (player->GetGUID() == guid)
                return player;
    }
    return nullptr;
}

Player* Group::GetInvited(std::string const& name)
{
    std::lock_guard<std::recursive_mutex> guard(m_lock);
    for (InvitesList::iterator itr = m_invitees.begin(); itr != m_invitees.end(); ++itr)
    {
        if (Player* player = *itr)
            if (player->GetName() == name)
                return player;
    }
    return nullptr;
}

bool Group::AddCreatureMember(Creature* creature)
{
    MemberSlot member;
    member.Guid = creature->GetGUID();
    member.Name = creature->GetName();
    member.Class = creature->getClass();

    {
        std::lock_guard<std::recursive_mutex> guard(m_lock);
        uint8 subGroup = 0;
        if (m_subGroupsCounts)
        {
            bool groupFound = false;
            for (; subGroup < MAX_RAID_SUBGROUPS; ++subGroup)
            {
                if (m_subGroupsCounts[subGroup] < MAX_GROUP_SIZE)
                {
                    groupFound = true;
                    break;
                }
            }
            if (!groupFound)
                return false;
        }

        member.Group = subGroup;
        m_memberSlots.push_back(member);
        SubGroupCounterIncrease(subGroup);
    }

    SendUpdate();
    return true;
}

bool Group::AddMember(Player* player)
{
    if (!player)
        return false;

    MemberSlot member;
    member.Guid = player->GetGUID();
    member.Name = player->GetName();
    member.Class = player->getClass();

    uint8 subGroup = 0;
    {
        // the free sub group is picked and taken in one step: two members may join from two maps at once
        std::lock_guard<std::recursive_mutex> guard(m_lock);
        if (m_disbanding)
            return false;

        // same limit as IsFull, checked with the insertion: an LFG list join may race an invite acceptance
        if (m_memberSlots.size() >= (isRaidGroup() ? MAX_RAID_SIZE : MAX_GROUP_SIZE))
            return false;

        if (m_subGroupsCounts)
        {
            bool groupFound = false;
            for (; subGroup < MAX_RAID_SUBGROUPS; ++subGroup)
            {
                if (m_subGroupsCounts[subGroup] < MAX_GROUP_SIZE)
                {
                    groupFound = true;
                    break;
                }
            }
            // We are raid group and no one slot is free
            if (!groupFound)
                return false;
        }

        member.Group = subGroup;
        m_memberSlots.push_back(member);
        SubGroupCounterIncrease(subGroup);
    }

    if (player)
    {
        // through the inviting group, so that its invitee list does not keep him (it is written on his logout only)
        if (Group* invite = player->GetGroupInvite())
            invite->RemoveInvite(player);

        bool PvPGroup = isBGGroup() || isBFGroup();

        if (player->GetGroup())
        {
            if (PvPGroup) // if player is in group and he is being added to BG raid group, then call SetBattlegroundRaid()
                player->SetBattlegroundOrBattlefieldRaid(this, subGroup);
            else //if player is in bg raid and we are adding him to normal group, then call SetOriginalGroup()
                player->SetOriginalGroup(this, subGroup);
        }
        else //if player is not in group, then call set group
            player->SetGroup(this, subGroup);

        player->SetPartyType(m_groupCategory, PvPGroup ? GROUP_TYPE_BG : GROUP_TYPE_NORMAL);
        player->ResetGroupUpdateSequenceIfNeeded(this);

        // if the same group invites the player back, cancel the homebind timer
        InstanceGroupBind bind = GetBoundInstanceCopy(player);
        if (bind.save && bind.save->GetInstanceId() == player->GetInstanceId())
            player->m_InstanceValid = true;
    }

    if (sLFGListMgr->IsGroupQueued(this))
        sLFGListMgr->PlayerAddedToGroup(player, this);

    for (auto& itr : m_targetIcons)
        itr.Clear();

    // insert into the table if we're not a battleground group
    if (!isBGGroup() && !isBFGroup())
    {
        CharacterDatabasePreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_INS_GROUP_MEMBER);

        stmt->setUInt32(0, m_dbStoreId);
        stmt->setUInt64(1, member.Guid.GetCounter());
        stmt->setUInt8(2, member.Flags);
        stmt->setUInt8(3, member.Group);
        stmt->setUInt8(4, member.Roles);

        CharacterDatabase.Execute(stmt);

    }

    SendUpdate();

    if (player && player->CanContact())
    {
        sScriptMgr->OnGroupAddMember(this, player->GetGUID());

        if (!IsLeader(player->GetGUID()) && !isBGGroup() && !isBFGroup())
        {
            // reset the new member's instances, unless he is currently in one of them
            // including raid/heroic instances that they are not permanently bound to!
            player->ResetInstances(INSTANCE_RESET_GROUP_JOIN, false, false);
            player->ResetInstances(INSTANCE_RESET_GROUP_JOIN, true, false);
            player->ResetInstances(INSTANCE_RESET_GROUP_JOIN, true, true);

            if (player->getLevel() >= LEVELREQUIREMENT_HEROIC)
            {
                if (player->GetDungeonDifficultyID() != GetDungeonDifficultyID())
                {
                    player->SetDungeonDifficultyID(GetDungeonDifficultyID());
                    player->SendDungeonDifficulty();
                }
                if (player->GetRaidDifficultyID() != GetRaidDifficultyID())
                {
                    player->SetRaidDifficultyID(GetRaidDifficultyID());
                    player->SendRaidDifficulty(false);
                }
                if (player->GetLegacyRaidDifficultyID() != GetLegacyRaidDifficultyID())
                {
                    player->SetLegacyRaidDifficultyID(GetLegacyRaidDifficultyID());
                    player->SendRaidDifficulty(true);
                }
            }
        }

        player->SetGroupUpdateFlag(GROUP_UPDATE_FULL);
        if (Pet* pet = player->GetPet())
            pet->SetGroupUpdateFlag(GROUP_UPDATE_PET_FULL);

        UpdatePlayerOutOfRange(player);

        // quest related GO state dependent from raid membership
        if (isRaidGroup())
            player->AddDelayedEvent(100, [player]() -> void
            {
                if (player)
                    player->UpdateForQuestWorldObjects();
            });

        player->SetFieldNotifyFlag(UF_FLAG_PARTY_MEMBER);

        UpdateData groupData(player->GetMapId());
        WorldPacket groupDataPacket;

        // Broadcast group members' fields to player
        for (MemberRef const& ref : GetMemberRefs())
        {
            if (ref.Guid == player->GetGUID())
                continue;

            // only members of the joiner's map can be at his client, and only those belong to this thread
            if (Player* gMember = ObjectAccessor::GetPlayer(*player, ref.Guid))
            {
                if (player->HaveAtClient(gMember))   // must be on the same map, or shit will break
                {
                    gMember->SetFieldNotifyFlag(UF_FLAG_PARTY_MEMBER);
                    gMember->BuildValuesUpdateBlockForPlayer(&groupData, player);
                    gMember->RemoveFieldNotifyFlag(UF_FLAG_PARTY_MEMBER);
                }

                if (gMember->HaveAtClient(player))
                {
                    UpdateData newData(player->GetMapId());
                    WorldPacket newDataPacket;
                    player->BuildValuesUpdateBlockForPlayer(&newData, gMember);
                    if (newData.HasData())
                        if (newData.BuildPacket(&newDataPacket))
                            gMember->SendDirectMessage(&newDataPacket);
                }
            }
        }

        if (groupData.HasData())
            if (groupData.BuildPacket(&groupDataPacket))
                player->SendDirectMessage(&groupDataPacket);

        player->RemoveFieldNotifyFlag(UF_FLAG_PARTY_MEMBER);
    }

    if (m_maxEnchantingLevel < player->GetSkillValue(SKILL_ENCHANTING))
        m_maxEnchantingLevel = player->GetSkillValue(SKILL_ENCHANTING);

    UpdateGuildGroupFlagFromLeader(this);

    return true;
}

bool Group::AddMysteryMember(ObjectGuid::LowType guidLow, std::string Name, uint8 Class, uint8 Roles, uint8 Flags, bool fakeOnline)
{
    MemberSlot member;
    member.Guid = ObjectGuid::Create<HighGuid::Player>(guidLow);

    // skip non-existed member
    if (!ObjectMgr::GetPlayerNameByGUID(member.Guid, member.Name))
    {
        CharacterDatabasePreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_GROUP_MEMBER);
        stmt->setUInt64(0, guidLow);
        CharacterDatabase.Execute(stmt);
        return false;
    }

    std::lock_guard<std::recursive_mutex> guard(m_lock);
    uint8 subGroup = 0;
    if (m_subGroupsCounts)
    {
        bool groupFound = false;
        for (; subGroup < MAX_RAID_SUBGROUPS; ++subGroup)
        {
            if (m_subGroupsCounts[subGroup] < MAX_GROUP_SIZE)
            {
                groupFound = true;
                break;
            }
        }
        if (!groupFound)
            return false;
    }

    member.Name = std::move(Name);
    member.Class = Class;
    member.Roles = Roles;
    member.fakeOnline = fakeOnline;
    member.Group = subGroup;
    member.Flags = Flags;

    m_memberSlots.push_back(member);

    SubGroupCounterIncrease(subGroup);
    //SendUpdate();
    return true;
}

bool Group::RemoveCreatureMember(ObjectGuid const& guid)
{
    if (guid.IsEmpty())
        return false;

    BroadcastGroupUpdate();

    {
        std::lock_guard<std::recursive_mutex> guard(m_lock);
        auto slot = _getMemberWSlot(guid);
        if (slot != m_memberSlots.end())
        {
            SubGroupCounterDecrease(slot->Group);
            m_memberSlots.erase(slot);
        }
    }

    SendUpdate();

    return true;
}

bool Group::RemoveMember(ObjectGuid const& guid, bool /*disbandInfo*/, RemoveMethod const& method /*= GROUP_REMOVEMETHOD_DEFAULT*/, ObjectGuid kicker /*= 0*/, char const* reason /*= NULL*/)
{
    // if (disbandInfo)
        return RemoveMemberQueue(guid, method, kicker, reason);
    // else
    // {
        // m_Functions.AddFunction([this, guid, method, kicker, reason]() -> void
        // {
            // if (!this)
                // return;
            // RemoveMemberQueue(guid, method, kicker, reason);
        // }, m_Functions.CalculateTime(10));
    // }

    //return false;
}

bool Group::RemoveMemberQueue(ObjectGuid const& guid, RemoveMethod const& method /*= GROUP_REMOVEMETHOD_DEFAULT*/, ObjectGuid kicker /*= 0*/, char const* reason /*= NULL*/)
{
    // std::lock_guard<std::recursive_mutex> _lock(m_lock);
    BroadcastGroupUpdate();

    sScriptMgr->OnGroupRemoveMember(this, guid, method, kicker, reason);

    if (ObjectAccessor::IsPlayerOnline(guid))
        if (sLFGListMgr->IsGroupQueued(this))
            sLFGListMgr->PlayerRemoveFromGroup(nullptr, this);

    // LFG group vote kick handled in scripts
    if (isLFGGroup() && method == GROUP_REMOVEMETHOD_KICK)
        return GetMembersCount() != 0;

    // asked before m_lock: LFGListMgr takes Group::m_lock under its own lock
    bool queued = sLFGListMgr->IsGroupQueued(this);

    // Two members may leave from two maps at once: whether the group outlives this removal is decided with the
    // removal itself (strictly more than 2 members before it, BG/BF/LFG allow 1), so both cannot keep or both disband
    bool keepGroup = false;
    {
        std::lock_guard<std::recursive_mutex> guard(m_lock);
        if (m_disbanding)
            return false;

        keepGroup = queued || m_memberSlots.size() > (isBGGroup() || isLFGGroup() || isBFGroup() ? 1u : 2u);
        if (keepGroup)
        {
            // Remove player from loot rolls
            for (Roll* roll : RollId)
            {
                auto itr2 = roll->playerVote.find(guid);
                if (itr2 == roll->playerVote.end())
                    continue;

                if (itr2->second == GREED || itr2->second == DISENCHANT)
                    --roll->totalGreed;
                else if (itr2->second == NEED)
                    --roll->totalNeed;
                else if (itr2->second == PASS)
                    --roll->totalPass;

                if (itr2->second != NOT_VALID)
                    --roll->totalPlayersRolling;

                // Not concluded here even if the leaver was the last one awaited: the leaver may stand on another map,
                // i.e. another thread than the loot and the winner's bags; the roll timer concludes it on the right one
                roll->playerVote.erase(itr2);
            }

            // Update subgroups
            auto slot = _getMemberWSlot(guid);
            if (slot != m_memberSlots.end())
            {
                SubGroupCounterDecrease(slot->Group);
                m_memberSlots.erase(slot);
            }
        }
    }

    if (keepGroup)
    {
        // The leaver may stand on another map (kick, LFG): his side runs in his own thread, from copied group values.
        // Only the group he still points to is left: he may have joined another one by then.
        {
            Group* self = this;
            bool const pvpGroup = isBGGroup() || isBFGroup();
            bool const lfgGroup = isLFGGroup();
            bool const kicked = method == GROUP_REMOVEMETHOD_KICK || method == GROUP_REMOVEMETHOD_KICK_LFG;
            auto const groupFlags = m_groupFlags;
            GroupCategory const category = m_groupCategory;
            uint8 const partyType = IsCreated() ? GROUP_TYPE_NORMAL : GROUP_TYPE_NONE;
            ObjectGuid const groupGuid = GetGUID();
            ObjectAccessor::PostToPlayer(guid, [=](Player* player) -> void
            {
                if (pvpGroup) // Battleground group handling
                {
                    Group* current = player->GetGroup();
                    if (!current || current == self)
                        player->RemoveFromBattlegroundOrBattlefieldRaid();
                }
                else // Regular group
                {
                    if (player->GetOriginalGroup() == self)
                        player->SetOriginalGroup(nullptr);
                    else if (player->GetGroup() == self)
                        player->SetGroup(nullptr);

                    // quest related GO state dependent from raid membership
                    player->AddDelayedEvent(100, [player]() -> void { player->UpdateForQuestWorldObjects(); });
                }

                player->SetPartyType(category, GROUP_TYPE_NONE);

                if (kicked)
                    player->SendDirectMessage(WorldPackets::Party::GroupUninvite().Write());

                WorldPackets::Party::PartyUpdate update;
                update.PartyFlags = groupFlags;
                update.PartyIndex = category;
                update.PartyType = partyType;
                update.MyIndex = -1;
                update.PartyGUID = groupGuid;
                update.SequenceNum = player->NextGroupUpdateSequenceNumber(category);
                player->SendDirectMessage(update.Write());

                if (!lfgGroup)
                    HomebindIfInstance(player);
            });
        }

        // Remove player from group in DB
        if (!isBGGroup() && !isBFGroup())
        {
            CharacterDatabasePreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_GROUP_MEMBER);
            stmt->setUInt64(0, guid.GetCounter());
            CharacterDatabase.Execute(stmt);
            DelinkMember(guid);
        }

        // Reevaluate group enchanter (the leaver's skill is not read: he may stand on another map)
        ResetMaxEnchantingLevel();

        // Pick new leader if necessary
        if (GetLeaderGUID() == guid)
        {
            for (ObjectGuid const& memberGuid : GetMemberGuids())
            {
                if (ObjectAccessor::IsPlayerOnline(memberGuid))
                {
                    ChangeLeader(memberGuid);
                    break;
                }
            }
        }

        UpdateGuildGroupFlagFromLeader(this);

        SendUpdate();

        if (isLFGGroup() && GetMembersCount() == 1)
        {
            uint32 mapId = sLFGMgr->GetDungeonMapId(GetGUID());
            bool leaderOutside = true;
            if (mapId)
                ObjectAccessor::WithPlayer(GetLeaderGUID(), [&leaderOutside, mapId](Player* leader)
                {
                    leaderOutside = leader->IsAlive() && leader->GetMapId() != mapId;
                });
            if (leaderOutside)
            {
                Disband();
                return false;
            }
        }

        // a concurrent leaver may reach the same conclusion: Disband runs once (m_disbanding)
        uint32 linkedMembers = uint32(GetMemberRefs().size());
        if ((!sLFGListMgr->IsGroupQueued(this) || !linkedMembers) && linkedMembers < (isLFGGroup() || isBGGroup() ? 1u : 2u))
            Disband();
        else
        {
            GroupCategory const category = m_groupCategory;
            ObjectGuid const groupGuid = GetGUID();
            ObjectAccessor::PostToPlayer(guid, [category, groupGuid](Player* player) -> void
            {
                SendDestroyedPartyUpdate(player, category, groupGuid);
            });
        }

        return true;
    }

    Disband();
    return false;
}

void Group::ChangeLeader(ObjectGuid const& guid, int8 partyIndex /*= 0*/)
{
    std::string newLeaderName;
    {
        std::lock_guard<std::recursive_mutex> guard(m_lock);
        auto slot = _getMemberWSlot(guid);
        if (slot == m_memberSlots.end())
            return;

        newLeaderName = slot->Name;
    }

    // the new and old leaders may stand on other maps: their flags change in their own threads
    std::string newLeaderPlayerName;
    if (!ObjectAccessor::WithPlayer(guid, [&newLeaderPlayerName](Player* player) { newLeaderPlayerName = player->GetName(); }))
        return;

    sScriptMgr->OnGroupChangeLeader(this, GetLeaderGUID(), guid);

    if (!isBGGroup() && !isBFGroup())
    {
        // Remove the groups permanent instance bindings; the saves are told once m_bound_lock is released
        std::vector<InstanceSave*> unboundSaves;
        {
            std::lock_guard<std::recursive_mutex> _lock(m_bound_lock);
            for (auto& itr : m_boundInstances)
            {
                for (auto itr2 = itr.begin(); itr2 != itr.end();)
                {
                    if (itr2->second.perm)
                    {
                        unboundSaves.push_back(itr2->second.save);
                        itr.erase(itr2++);
                    }
                    else
                        ++itr2;
                }
            }
        }

        for (InstanceSave* save : unboundSaves)
            save->RemoveGroup(this);

        // Same in the database

        CharacterDatabasePreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_GROUP_INSTANCE_PERM_BINDING);

        stmt->setUInt32(0, m_dbStoreId);
        stmt->setUInt64(1, guid.GetCounter());

        CharacterDatabase.Execute(stmt);

        // Update the group leader
        stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_GROUP_LEADER);

        stmt->setUInt64(0, guid.GetCounter());
        stmt->setUInt32(1, m_dbStoreId);

        CharacterDatabase.Execute(stmt);
    }

    ObjectGuid const groupGuid = GetGUID();
    ObjectGuid const oldLeaderGuid = GetLeaderGUID();
    if (oldLeaderGuid != guid)
        ObjectAccessor::PostToPlayer(oldLeaderGuid, [groupGuid, oldLeaderGuid](Player* oldLeader) -> void
        {
            // leader again meanwhile
            if (Group* group = sGroupMgr->GetGroupByGUID(groupGuid))
                if (group->IsLeader(oldLeaderGuid))
                    return;
            oldLeader->RemoveFlag(PLAYER_FIELD_PLAYER_FLAGS, PLAYER_FLAGS_GROUP_LEADER);
        });

    {
        std::lock_guard<std::recursive_mutex> guard(m_lock);
        auto slot = _getMemberWSlot(guid);
        if (slot == m_memberSlots.end())
            return;

        ToggleGroupMemberFlag(slot, MEMBER_FLAG_ASSISTANT, false);
    }

    m_leaderGuid = guid;
    m_leaderName = newLeaderPlayerName;

    ObjectAccessor::PostToPlayer(guid, [groupGuid, guid](Player* player) -> void
    {
        if (Group* group = sGroupMgr->GetGroupByGUID(groupGuid))
            if (group->IsLeader(guid) && !group->IsDisbanding())
                player->SetFlag(PLAYER_FIELD_PLAYER_FLAGS, PLAYER_FLAGS_GROUP_LEADER);
    });

    // Copy the permanent binds from the new leader to the group, once he is the leader the posted call checks for
    if (!isBGGroup() && !isBFGroup() && !isLFGGroup())
        ConvertLeaderInstances(guid, true);

    WorldPackets::Party::GroupNewLeader groupNewLeader;
    groupNewLeader.Name = newLeaderName;
    groupNewLeader.PartyIndex = partyIndex;
    BroadcastPacket(groupNewLeader.Write(), true);
}

void Group::Disband(bool hideDestroy /* = false */)
{
    // two members leaving from two maps may both get here: only the first call disbands
    {
        std::lock_guard<std::recursive_mutex> guard(m_lock);
        if (m_disbanding)
            return;
        m_disbanding = true;
    }

    sScriptMgr->OnGroupDisband(this);
    sLFGListMgr->Remove(GetGUIDLow(), nullptr, false);

    // Members stand on any map: their side runs in their own threads, from copied values, since the group is freed
    // later from the world thread. The references are all unlinked below, before any of these runs.
    {
        Group* self = this;
        bool const pvpGroup = isBGGroup() || isBFGroup();
        bool const raidGroup = isRaidGroup();
        bool const lfgGroup = isLFGGroup();
        GroupCategory const category = m_groupCategory;
        ObjectGuid const groupGuid = GetGUID();
        for (ObjectGuid const& memberGuid : GetMemberGuids())
        {
            ObjectAccessor::PostToPlayer(memberGuid, [=](Player* player) -> void
            {
                if (!player->CanContact())
                    return;

                //we cannot call _removeMember because it would invalidate member iterator
                //if we are removing player from battleground raid
                if (pvpGroup)
                {
                    // back to his original group, unless he joined another raid meanwhile
                    Group* current = player->GetGroup();
                    if (!current || current == self)
                        player->RemoveFromBattlegroundOrBattlefieldRaid();
                }
                else
                {
                    //we can remove player who is in battleground from his original group
                    if (player->GetOriginalGroup() == self)
                        player->SetOriginalGroup(nullptr);
                    else if (player->GetGroup() == self)
                        player->SetGroup(nullptr);
                }

                player->SetPartyType(category, GROUP_TYPE_NONE);

                // quest related GO state dependent from raid membership
                if (raidGroup)
                    player->AddDelayedEvent(100, [player]() -> void { player->UpdateForQuestWorldObjects(); });

                if (!hideDestroy)
                    player->SendDirectMessage(WorldPackets::Party::GroupDestroyed().Write());

                SendDestroyedPartyUpdate(player, category, groupGuid);

                if (!lfgGroup)
                    HomebindIfInstance(player);
            }, 0, ObjectAccessor::PlayerScope::InWorld);
        }
    }
    {
        std::lock_guard<std::recursive_mutex> guard(m_lock);
        for (Roll* roll : RollId)
            delete roll;
        RollId.clear();
        m_memberSlots.clear();

        // players not reached above (offline, loading) must not keep a reference to a group about to be freed
        while (GroupReference* ref = m_memberMgr.getFirst())
            ref->unlink();
    }

    RemoveAllInvites();

    if (!isBGGroup() && !isBFGroup())
    {
        CharacterDatabaseTransaction trans = CharacterDatabase.BeginTransaction();

        CharacterDatabasePreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_GROUP);
        stmt->setUInt32(0, m_dbStoreId);
        trans->Append(stmt);

        stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_GROUP_MEMBER_ALL);
        stmt->setUInt32(0, m_dbStoreId);
        trans->Append(stmt);

        CharacterDatabase.CommitTransaction(trans);

        ResetInstances(INSTANCE_RESET_GROUP_DISBAND, false, false, nullptr);
        ResetInstances(INSTANCE_RESET_GROUP_DISBAND, true, false, nullptr);
        ResetInstances(INSTANCE_RESET_GROUP_DISBAND, true, true, nullptr);

        // the saves forget the group now rather than in its destructor: the db store id is reused once freed, and
        // InstanceSave::SaveBindsToDB writes group rows from these lists
        std::vector<InstanceSave*> boundSaves;
        {
            std::lock_guard<std::recursive_mutex> _lock(m_bound_lock);
            m_bindsClosed = true;
            for (auto& binds : m_boundInstances)
            {
                for (auto const& bind : binds)
                    boundSaves.push_back(bind.second.save);
                binds.clear();
            }
        }

        for (InstanceSave* save : boundSaves)
            save->RemoveGroup(this);

        stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_LFG_DATA);
        stmt->setUInt32(0, m_dbStoreId);
        CharacterDatabase.Execute(stmt);

        // the db store id is freed by GroupMgr when the group is really deleted: until then pointers still held by
        // other threads write rows with it, which must not land on a new group's id
    }

    // the battleground may be gone before GroupMgr frees the group
    if (Battleground* bg = m_bgGroup)
    {
        if (bg->GetBgRaid(ALLIANCE) == this)
            bg->SetBgRaid(ALLIANCE, nullptr);
        else if (bg->GetBgRaid(HORDE) == this)
            bg->SetBgRaid(HORDE, nullptr);
        m_bgGroup = nullptr;
    }
    m_bfGroup = nullptr;

    sGroupMgr->RemoveGroup(this);
    // other threads may still hold the pointer they looked up a moment ago
    sGroupMgr->QueueForDelete(this);
}

bool Group::IsDisbanding() const
{
    std::lock_guard<std::recursive_mutex> guard(m_lock);
    return m_disbanding;
}

// The leader's binds belong to his thread: the copy into the group runs there, the group found again by guid
void Group::ConvertLeaderInstances(ObjectGuid const& leaderGuid, bool switchLeader)
{
    ObjectGuid groupGuid = GetGUID();
    ObjectAccessor::PostToPlayer(leaderGuid, [groupGuid, leaderGuid, switchLeader](Player* leader) -> void
    {
        if (Group* group = sGroupMgr->GetGroupByGUID(groupGuid))
            if (group->IsLeader(leaderGuid) && !group->IsDisbanding())
                Player::ConvertInstancesToGroup(leader, group, switchLeader);
    });
}

/*********************************************************/
/***                   LOOT SYSTEM                     ***/
/*********************************************************/

// Need is reserved to items the player can use (as Need Before Greed in TrinityCore).
static bool CanRollNeedOnItem(Player const* player, Roll const& roll)
{
    if (!(roll.rollVoteMask & ROLL_FLAG_TYPE_NEED))
        return false;

    ItemTemplate const* proto = sObjectMgr->GetItemTemplate(roll.item.ItemID);
    return proto && player->CanUseItem(proto) == EQUIP_ERR_OK;
}

void Group::SendLootStartRoll(uint32 mapID, Roll const& roll)
{
    for (const auto& itr : roll.playerVote)
    {
        if (itr.second != NOT_EMITED_YET)
            continue;

        // short reads and a send: the voter may have left for another map since he was picked
        ObjectAccessor::WithPlayer(itr.first, [this, mapID, &roll](Player* player)
        {
            SendLootStartRollToPlayer(mapID, player, CanRollNeedOnItem(player, roll), roll);
        });
    }
}

// a member may stand on another map: tested and sent under the accessor lock
static void SendToContactableMember(ObjectGuid const& guid, WorldPacket const* packet)
{
    ObjectAccessor::WithPlayer(guid, [packet](Player* player)
    {
        if (player->CanContact())
            player->SendDirectMessage(packet);
    });
}

void Group::SendLootStartRollToPlayer(uint32 mapID, Player* player, bool canNeed, Roll const& roll)
{
    if (!player || !player->CanContact())
        return;

    WorldPackets::Loot::StartLootRoll lootRoll;
    roll.FillPacket(lootRoll.Item);
    lootRoll.LootObj = roll.lootedGUID;
    lootRoll.MapID = mapID;
    lootRoll.Item.UIType = LOOT_ITEM_UI_ROLL;
    lootRoll.RollTime = isRaidGroup() ? RAID_ROLL_TIMER : NORMAL_ROLL_TIMER;
    lootRoll.ValidRolls = roll.rollVoteMask;
    if (!canNeed)
        lootRoll.ValidRolls &= ~ROLL_FLAG_TYPE_NEED;
    lootRoll.Method = GetLootMethod();
    player->SendDirectMessage(lootRoll.Write());
}

void Group::SendLootRoll(ObjectGuid targetGuid, uint8 rollNumber, uint8 rollType, Roll const& roll)
{
    WorldPackets::Loot::LootRollResponse response;
    roll.FillPacket(response.LootItems);
    response.LootItems.UIType = 2;
    response.LootObj = roll.lootedGUID;
    response.Player = targetGuid;
    response.Roll = rollNumber;
    response.RollType = rollType;
    response.Autopassed = false;

    WorldPacket const* pdata = response.Write();
    for (const auto& itr : roll.playerVote)
        if (itr.second != NOT_VALID)
            SendToContactableMember(itr.first, pdata);
}

void Group::SendLootRollWon(ObjectGuid targetGuid, uint8 rollNumber, uint8 rollType, Roll const& roll)
{
    WorldPackets::Loot::LootRollWon won;
    won.LootObj = roll.lootedGUID;
    won.Player = targetGuid;
    won.Roll = rollNumber;
    won.RollType = rollType;
    won.LootItems.UIType = 1;
    roll.FillPacket(won.LootItems);

    // roll.TotalEmited()

    WorldPackets::Loot::LootRollsComplete rollsComplete;
    rollsComplete.LootListID = roll.itemSlot;
    rollsComplete.LootObj = roll.lootedGUID;

    WorldPacket const* pwon = won.Write();
    WorldPacket const* prollsComplete = rollsComplete.Write();

    for (const auto& itr : roll.playerVote)
    {
        if (itr.second != NOT_VALID)
        {
            SendToContactableMember(itr.first, pwon);
            SendToContactableMember(itr.first, prollsComplete);
        }
    }
}

void Group::SendLootAllPassed(Roll const& roll)
{
    WorldPackets::Loot::LootAllPassed  passed;
    roll.FillPacket(passed.LootItems);
    passed.LootObj = roll.lootedGUID;
    passed.LootItems.UIType = 1;
    // roll.TotalEmited() ???

    WorldPacket const* ppassed = passed.Write();

    for (const auto& itr : roll.playerVote)
        if (itr.second != NOT_VALID)
            SendToContactableMember(itr.first, ppassed);
}

void Group::SendLooter(Creature* creature, Player* groupLooter)
{
    ASSERT(creature);

    WorldPackets::Loot::LootList lootList;
    lootList.Owner = creature->GetGUID();
    lootList.LootObj = creature->loot.GetGUID();

    if (GetLootMethod() == MASTER_LOOT && creature->loot.hasOverThresholdItem())
        lootList.Master = m_looterGuid;

    if (groupLooter)
        lootList.RoundRobinWinner = groupLooter->GetGUID();

    BroadcastPacket(lootList.Write(), false);
}

// The loot and the winner's bags belong to the map the roll was started on
static bool IsOnRollMap(Player const* player, Roll const& roll)
{
    return player->IsInWorld() && player->GetMapId() == roll.mapId && player->GetInstanceId() == roll.instanceId;
}

// Called on the loot's map thread: a player found on that map under the accessor lock can only be removed by this
// thread, so the pointer stays valid for the caller
static Player* FindOnRollMap(ObjectGuid const& guid, Roll const& roll)
{
    Player* found = nullptr;
    ObjectAccessor::WithPlayer(guid, [&found, &roll](Player* player)
    {
        if (IsOnRollMap(player, roll))
            found = player;
    });
    return found;
}

static bool IsContactableMember(ObjectGuid const& guid)
{
    bool contactable = false;
    ObjectAccessor::WithPlayer(guid, [&contactable](Player* player) { contactable = player->CanContact(); });
    return contactable;
}

void Group::GroupLoot(Loot* loot, WorldObject* pLootedObject)
{
    std::vector<LootItem>::iterator i;
    ItemTemplate const* item;
    uint8 itemSlot = 0;

    PurgeStaleRolls();

    // called from the loot's map; the roll stays private to this thread until it is pushed into RollId
    std::vector<MemberRef> members = GetMemberRefs();

    for (i = loot->items.begin(); i != loot->items.end(); ++i, ++itemSlot)
    {
        if (i->freeforall || i->currency)
            continue;

        item = sObjectMgr->GetItemTemplate(i->item.ItemID);
        if (!item)
        {
            //TC_LOG_DEBUG("misc", "Group::GroupLoot: missing item prototype for item with id: %d", i->itemid);
            continue;
        }

        //roll for over-threshold item if it's one-player loot
        if (item->GetQuality() >= uint32(m_lootThreshold))
        {
            ObjectGuid newitemGUID = ObjectGuid::Create<HighGuid::Item>(sObjectMgr->GetGenerator<HighGuid::Item>()->Generate());
            auto r = new Roll(newitemGUID, *i);
            r->lootedGUID = loot->GetGUID();
            r->mapId = pLootedObject->GetMapId();
            r->instanceId = pLootedObject->GetInstanceId();

            //a vector is filled with only near party members (the loot's map: this thread)
            for (MemberRef const& ref : members)
            {
                Player* member = ObjectAccessor::GetPlayer(*pLootedObject, ref.Guid);
                if (!member || !member->CanContact())
                    continue;
                if (loot->AllowedForPlayer(member, i->item.ItemID, i->item.CurrencyID, i->type, i->needs_quest, &*i))
                {
                    if (member->IsWithinDistInMap(pLootedObject, sWorld->getFloatConfig(CONFIG_GROUP_XP_DISTANCE), false))
                    {
                        r->totalPlayersRolling++;

                        if (member->GetPassOnGroupLoot())
                        {
                            r->playerVote[member->GetGUID()] = PASS;
                            r->totalPass++;
                            // can't broadcast the pass now. need to wait until all rolling players are known.
                        }
                        else
                            r->playerVote[member->GetGUID()] = NOT_EMITED_YET;
                    }
                }
            }

            if (r->totalPlayersRolling > 0)
            {
                r->itemSlot = itemSlot;
                // if (item->DisenchantID && m_maxEnchantingLevel >= item->RequiredDisenchantSkill)
                    // r->rollVoteMask |= ROLL_FLAG_TYPE_DISENCHANT;

                if (item->GetFlags2() & ITEM_FLAG2_CAN_ONLY_ROLL_GREED)
                    r->rollVoteMask &= ~ROLL_FLAG_TYPE_NEED;

                loot->items[itemSlot].is_blocked = true;

                // If there is any "auto pass", broadcast the pass now.
                if (r->totalPass)
                {
                    for (Roll::PlayerVote::const_iterator itr = r->playerVote.begin(); itr != r->playerVote.end(); ++itr)
                        if (itr->second == PASS && ObjectAccessor::GetPlayer(*pLootedObject, itr->first))
                            SendLootRoll(itr->first, 128, ROLL_PASS, *r);
                }

                // listed before it is announced, so that an immediate vote finds it; once listed, r may be freed by
                // a Disband from another thread: the announce reads a copy
                std::unique_ptr<Roll> snapshot;
                {
                    std::lock_guard<std::recursive_mutex> guard(m_lock);
                    r->aoeSlot = ++m_aoe_slots;
                    RollId.push_back(r);
                    snapshot.reset(new Roll(*r));
                }

                ArmRollTimer(pLootedObject);
                SendLootStartRoll(pLootedObject->GetMapId(), *snapshot);
            }
            else
                delete r;
        }
        else
            i->is_underthreshold = true;
    }

    for (i = loot->quest_items.begin(); i != loot->quest_items.end(); ++i, ++itemSlot)
    {
        if (!i->follow_loot_rules)
            continue;

        item = sObjectMgr->GetItemTemplate(i->item.ItemID);
        if (!item)
            continue;

        ObjectGuid newitemGUID = ObjectGuid::Create<HighGuid::Item>(sObjectMgr->GetGenerator<HighGuid::Item>()->Generate());
        auto r = new Roll(newitemGUID, *i);
        r->lootedGUID = loot->GetGUID();
        r->mapId = pLootedObject->GetMapId();
        r->instanceId = pLootedObject->GetInstanceId();

        //a vector is filled with only near party members (the loot's map: this thread)
        for (MemberRef const& ref : members)
        {
            Player* member = ObjectAccessor::GetPlayer(*pLootedObject, ref.Guid);
            if (!member || !member->CanContact())
                continue;

            if (loot->AllowedForPlayer(member, i->item.ItemID, i->item.CurrencyID, i->type, i->needs_quest, &*i))
            {
                if (member->IsWithinDistInMap(pLootedObject, sWorld->getFloatConfig(CONFIG_GROUP_XP_DISTANCE), false))
                {
                    r->totalPlayersRolling++;
                    r->playerVote[member->GetGUID()] = NOT_EMITED_YET;
                }
            }
        }

        if (r->totalPlayersRolling > 0)
        {
            r->itemSlot = itemSlot;

            loot->quest_items[itemSlot - loot->items.size()].is_blocked = true;

            std::unique_ptr<Roll> snapshot;
            {
                std::lock_guard<std::recursive_mutex> guard(m_lock);
                r->aoeSlot = ++m_aoe_slots;
                RollId.push_back(r);
                snapshot.reset(new Roll(*r));
            }

            ArmRollTimer(pLootedObject);
            SendLootStartRoll(pLootedObject->GetMapId(), *snapshot);
        }
        else
            delete r;
    }
}

void Group::MasterLoot(Loot* loot, WorldObject* lootObj)
{
    WorldPackets::Loot::MasterLootCandidateList list;
    list.LootObj = loot->GetGUID();
    for (MemberRef const& ref : GetMemberRefs())
    {
        // candidates are on the loot's map, i.e. this thread
        Player* looter = ObjectAccessor::GetPlayer(*lootObj, ref.Guid);
        if (!looter)
            continue;

        if (looter->IsWithinDistInMap(lootObj, sWorld->getFloatConfig(CONFIG_GROUP_XP_DISTANCE), false))
            list.Players.push_back(ObjectGuid::Create<HighGuid::Player>(looter->GetGUIDLow())); //HardHack! Plr should have off-like hiGuid
    }

    ObjectAccessor::SendToPlayer(GetLooterGuid(), list.Write());
}

void Group::DoRollForAllMembers(ObjectGuid guid, uint8 slot, uint32 mapid, Loot* loot, LootItem& item, Player* player)
{
    PurgeStaleRolls();

    // Already rolled? (a roll naming this live loot is valid)
    {
        std::lock_guard<std::recursive_mutex> guard(m_lock);
        for (Roll* roll : RollId)
            if (roll->itemSlot == slot && !roll->lootedGUID.IsEmpty() && roll->lootedGUID == loot->GetGUID())
                return;
    }

    WorldObject* pLootedObject = nullptr;

    if (guid.IsLoot())
        guid = loot->objGuid;

    if (guid.IsCreatureOrVehicle())
        pLootedObject = player->GetMap()->GetCreature(guid);
    else if (guid.IsGameObject())
        pLootedObject = player->GetMap()->GetGameObject(guid);

    if (!pLootedObject)
        return;

    ObjectGuid newitemGUID = ObjectGuid::Create<HighGuid::Item>(sObjectMgr->GetGenerator<HighGuid::Item>()->Generate());
    auto r = new Roll(newitemGUID, item);
    r->lootedGUID = loot->GetGUID();
    r->mapId = pLootedObject->GetMapId();
    r->instanceId = pLootedObject->GetInstanceId();

    //a vector is filled with only near party members (the loot's map: this thread)
    for (MemberRef const& ref : GetMemberRefs())
    {
        Player* member = ObjectAccessor::GetPlayer(*pLootedObject, ref.Guid);
        if (!member || !member->CanContact())
            continue;

        if (loot->AllowedForPlayer(member, item.item.ItemID, item.item.CurrencyID, item.type, item.needs_quest, &item))
        {
            if (member->IsWithinDistInMap(pLootedObject, sWorld->getFloatConfig(CONFIG_GROUP_XP_DISTANCE), false))
            {
                r->totalPlayersRolling++;
                r->playerVote[member->GetGUID()] = NOT_EMITED_YET;
            }
        }
    }

    if (!r->totalPlayersRolling)
    {
        delete r;
        return;
    }

    r->itemSlot = slot;

    // as GroupLoot: listed before it is announced, announced from a copy
    std::unique_ptr<Roll> snapshot;
    {
        std::lock_guard<std::recursive_mutex> guard(m_lock);
        r->aoeSlot = ++m_aoe_slots;     //restart at next loot. it's normall
        RollId.push_back(r);
        snapshot.reset(new Roll(*r));
    }

    // without it a roll whose last voter stands on another map would never end (RollIsActive stays true)
    ArmRollTimer(pLootedObject);
    SendLootStartRoll(mapid, *snapshot);
}

bool Group::CountRollVote(Player* voter, uint8 AoeSlot, uint8 Choice)
{
    if (!voter)
        return false;

    ObjectGuid playerGUID = voter->GetGUID();
    std::set<ObjectGuid> validLoots = GetValidRollLoots();

    // Need is checked on the voter (this thread's player) without m_lock; the roll is then found again by its item GUID
    ObjectGuid rollItemGuid;
    uint32 rollItemId = 0;
    bool needAllowed = false;
    {
        std::lock_guard<std::recursive_mutex> guard(m_lock);
        auto rollI = GetRoll(AoeSlot, validLoots);
        if (rollI == RollId.end())
            return false;

        rollItemGuid = (*rollI)->item.itemGUID;
        rollItemId = (*rollI)->item.ItemID;
        needAllowed = ((*rollI)->rollVoteMask & ROLL_FLAG_TYPE_NEED) != 0;
    }

    bool canNeed = false;
    if (Choice == ROLL_NEED && needAllowed)
        if (ItemTemplate const* proto = sObjectMgr->GetItemTemplate(rollItemId))
            canNeed = voter->CanUseItem(proto) == EQUIP_ERR_OK;

    std::unique_ptr<Roll> snapshot;
    Roll* concluded = nullptr;
    bool postConclusion = false;
    {
        std::lock_guard<std::recursive_mutex> guard(m_lock);
        auto rollI = GetRoll(AoeSlot, validLoots);
        if (rollI == RollId.end() || (*rollI)->item.itemGUID != rollItemGuid)
            return false;

        Roll* roll = *rollI;
        auto itr = roll->playerVote.find(playerGUID);
        // this condition means that player joins to the party after roll begins
        if (itr == roll->playerVote.end())
            return false;

        // one vote per player: a repeated Need would otherwise close the roll early
        if (itr->second != NOT_EMITED_YET)
            return false;

        switch (Choice)
        {
            case ROLL_PASS:                                     // Player choose pass
                ++roll->totalPass;
                itr->second = PASS;
                break;
            case ROLL_NEED:                                     // player choose Need
                if (!canNeed)
                    return false;

                ++roll->totalNeed;
                itr->second = NEED;
                break;
            case ROLL_GREED:                                    // player choose Greed
                if (!(roll->rollVoteMask & ROLL_FLAG_TYPE_GREED))
                    return false;

                ++roll->totalGreed;
                itr->second = GREED;
                break;
            case ROLL_DISENCHANT:                               // player choose Disenchant
                if (!(roll->rollVoteMask & ROLL_FLAG_TYPE_DISENCHANT))
                    return false;

                ++roll->totalGreed;
                itr->second = DISENCHANT;
                break;
            default:
                return false;
        }

        snapshot.reset(new Roll(*roll));

        // a last vote cast from another map is handed to a member on the loot's map (the loot's roll timer otherwise)
        if (roll->TotalEmited() >= roll->totalPlayersRolling)
        {
            if (IsOnRollMap(voter, *roll))
                concluded = DetachRoll(rollI);
            else
                postConclusion = true;
        }
    }

    SendLootRoll(playerGUID, 0, Choice, *snapshot);

    if (concluded)
        CountTheRoll(concluded);
    else if (postConclusion)
        PostRollConclusion(*snapshot);

    return true;
}

// The last vote came from another map: a member standing on the loot's map concludes the roll in his own thread
void Group::PostRollConclusion(Roll const& roll)
{
    ObjectGuid groupGuid = GetGUID();
    ObjectGuid rollItemGuid = roll.item.itemGUID;
    for (ObjectGuid const& memberGuid : GetMemberGuids())
    {
        // read from another thread, checked again in the member's own
        bool onRollMap = false;
        ObjectAccessor::WithPlayer(memberGuid, [&onRollMap, &roll](Player* member) { onRollMap = IsOnRollMap(member, roll); });
        if (!onRollMap)
            continue;

        if (ObjectAccessor::PostToPlayer(memberGuid, [groupGuid, rollItemGuid](Player* member) -> void
        {
            if (Group* group = sGroupMgr->GetGroupByGUID(groupGuid))
                group->ConcludeRollOnMap(member, rollItemGuid);
        }, 0, ObjectAccessor::PlayerScope::InWorld))
            return;
    }
}

// In onRollMap's thread; if he left the loot's map meanwhile, the loot's roll timer concludes the roll
void Group::ConcludeRollOnMap(Player* onRollMap, ObjectGuid const& rollItemGuid)
{
    Roll* concluded = nullptr;
    {
        std::lock_guard<std::recursive_mutex> guard(m_lock);
        for (auto itr = RollId.begin(); itr != RollId.end(); ++itr)
        {
            if ((*itr)->item.itemGUID != rollItemGuid)
                continue;

            if ((*itr)->TotalEmited() >= (*itr)->totalPlayersRolling && IsOnRollMap(onRollMap, **itr))
                concluded = DetachRoll(itr);
            break;
        }
    }

    if (concluded)
        CountTheRoll(concluded);
}

// The loot's owner ends its rolls from its own thread when this expires (Creature/GameObject::Update -> EndRoll)
void Group::ArmRollTimer(WorldObject* lootedObject)
{
    uint32 rollTimer = isRaidGroup() ? RAID_ROLL_TIMER : NORMAL_ROLL_TIMER;
    if (Creature* creature = lootedObject->ToCreature())
    {
        creature->m_groupLootTimer = rollTimer;
        creature->lootingGroupLowGUID = GetGUID();
    }
    else if (GameObject* go = lootedObject->ToGameObject())
    {
        go->m_groupLootTimer = rollTimer;
        go->lootingGroupLowGUID = GetGUID();
    }
}

//called when roll timer expires, by the loot's owner: this is the loot's map thread
void Group::EndRoll(Loot* pLoot)
{
    if (!pLoot || pLoot->GetGUID().IsEmpty())
        return;

    std::vector<Roll*> ended;
    {
        std::lock_guard<std::recursive_mutex> guard(m_lock);
        for (auto itr = RollId.begin(); itr != RollId.end();)
        {
            if ((*itr)->lootedGUID == pLoot->GetGUID())
            {
                ended.push_back(*itr);
                itr = RollId.erase(itr);
            }
            else
                ++itr;
        }
    }

    for (Roll* roll : ended)
        CountTheRoll(roll);           //i don't have to edit player votes, who didn't vote ... he will pass
}

void Group::ClearAoeSlots()
{
    std::lock_guard<std::recursive_mutex> guard(m_lock);
    m_aoe_slots = 0;
}

bool Group::isRolledSlot(uint8 _slot)
{
    std::set<ObjectGuid> validLoots = GetValidRollLoots();

    std::lock_guard<std::recursive_mutex> guard(m_lock);
    for (auto& iter : RollId)
        if (iter->aoeSlot == _slot && validLoots.count(iter->lootedGUID))
            return true;
    return false;
}

bool Group::RollIsActive() const
{
    std::set<ObjectGuid> vanished = GetVanishedRollLoots();

    std::lock_guard<std::recursive_mutex> guard(m_lock);
    for (Roll const* roll : RollId)
        if (!vanished.count(roll->lootedGUID))
            return true;
    return false;
}

// Only loots known to be gone: a roll listed meanwhile names a new, live loot and is kept
void Group::PurgeStaleRolls()
{
    std::set<ObjectGuid> vanished = GetVanishedRollLoots();
    if (vanished.empty())
        return;

    std::vector<Roll*> stale;
    {
        std::lock_guard<std::recursive_mutex> guard(m_lock);
        for (auto itr = RollId.begin(); itr != RollId.end();)
        {
            if (vanished.count((*itr)->lootedGUID))
            {
                stale.push_back(*itr);
                itr = RollId.erase(itr);
            }
            else
                ++itr;
        }
    }

    for (Roll* roll : stale)
        delete roll;
}

// LootMgr is asked without m_lock held
std::set<ObjectGuid> Group::GetVanishedRollLoots() const
{
    std::vector<ObjectGuid> lootGuids;
    {
        std::lock_guard<std::recursive_mutex> guard(m_lock);
        for (Roll const* roll : RollId)
            lootGuids.push_back(roll->lootedGUID);
    }

    std::set<ObjectGuid> vanished;
    for (ObjectGuid const& lootGuid : lootGuids)
        if (lootGuid.IsEmpty() || !sLootMgr->GetLoot(lootGuid))
            vanished.insert(lootGuid);

    return vanished;
}

// LootMgr is asked without m_lock held
std::set<ObjectGuid> Group::GetValidRollLoots() const
{
    std::vector<ObjectGuid> lootGuids;
    {
        std::lock_guard<std::recursive_mutex> guard(m_lock);
        for (Roll const* roll : RollId)
            lootGuids.push_back(roll->lootedGUID);
    }

    std::set<ObjectGuid> validLoots;
    for (ObjectGuid const& lootGuid : lootGuids)
        if (!lootGuid.IsEmpty() && sLootMgr->GetLoot(lootGuid))
            validLoots.insert(lootGuid);

    return validLoots;
}

std::vector<Group::PendingRoll> Group::GetPendingRolls(ObjectGuid const& member) const
{
    std::set<ObjectGuid> const validLoots = GetValidRollLoots();

    std::vector<PendingRoll> pending;
    std::lock_guard<std::recursive_mutex> guard(m_lock);
    for (Roll const* roll : RollId)
    {
        if (!validLoots.count(roll->lootedGUID))
            continue;
        auto itr = roll->playerVote.find(member);
        if (itr == roll->playerVote.end() || itr->second != NOT_EMITED_YET)
            continue;
        PendingRoll entry;
        entry.aoeSlot = roll->aoeSlot;
        entry.itemId = roll->item.ItemID;
        entry.bonusListIds = roll->item.ItemBonus.BonusListIDs;
        entry.rollVoteMask = roll->rollVoteMask;
        pending.push_back(entry);
    }
    return pending;
}

// m_lock held
Roll* Group::DetachRoll(Rolls::iterator rollI)
{
    Roll* roll = *rollI;
    RollId.erase(rollI);
    return roll;
}

// The roll has been taken out of RollId (it belongs to the caller), on the loot's map thread
void Group::CountTheRoll(Roll* roll)
{
    std::unique_ptr<Roll> owner(roll);

    Loot* loot = roll->getLoot();
    if (!loot)                                              // is loot already deleted ?
        return;

    if (roll->itemSlot >= loot->items.size() + loot->quest_items.size())
        return;

    //end of the roll
    LootItem* item = &(roll->itemSlot >= loot->items.size() ? loot->quest_items[roll->itemSlot - loot->items.size()] : loot->items[roll->itemSlot]);

    // as TrinityCore: voters who went offline are skipped, Need falls back to Greed, then to all passed
    if (roll->totalNeed > 0)
    {
        uint8 maxresul = 0;
        ObjectGuid maxguid;

        for (Roll::PlayerVote::const_iterator itr = roll->playerVote.begin(); itr != roll->playerVote.end(); ++itr)
        {
            if (itr->second != NEED)
                continue;

            if (!IsContactableMember(itr->first))
            {
                --roll->totalNeed;
                continue;
            }

            uint8 randomN = urand(1, 100);
            SendLootRoll(itr->first, randomN, ROLL_NEED, *roll);
            if (maxresul < randomN)
            {
                maxguid = itr->first;
                maxresul = randomN;
            }
        }

        if (!maxguid.IsEmpty())
        {
            SendLootRollWon(maxguid, maxresul, ROLL_NEED, *roll);

            Player* player = FindOnRollMap(maxguid, *roll);
            // a winner who left the loot's map since is another thread's: the item is freed for anyone to loot
            if (player)
            {
                player->UpdateAchievementCriteria(CRITERIA_TYPE_ROLL_NEED_ON_LOOT, roll->item.ItemID, maxresul);

                ItemPosCountVec dest;
                InventoryResult msg = player->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, roll->item.ItemID, item->count);
                if (msg == EQUIP_ERR_OK)
                {
                    item->is_looted = true;
                    loot->NotifyItemRemoved(roll->itemSlot);
                    loot->unlootedCount--;
                    player->StoreNewItem(dest, roll->item.ItemID, true, item->item.RandomPropertiesID, item->GetAllowedLooters(), item->item.ItemBonus.BonusListIDs, item->item.ItemBonus.Context);
                }
                else
                {
                    item->is_blocked = false;
                    player->SendEquipError(msg, nullptr, nullptr, roll->item.ItemID);
                }
            }
            else
                item->is_blocked = false;
        }
        else
            roll->totalNeed = 0;
    }

    if (roll->totalNeed == 0 && roll->totalGreed > 0)
    {
        uint8 maxresul = 0;
        ObjectGuid maxguid;
        RollVote rollvote = NOT_VALID;

        for (auto itr = roll->playerVote.begin(); itr != roll->playerVote.end(); ++itr)
        {
            if (itr->second != GREED && itr->second != DISENCHANT)
                continue;

            if (!IsContactableMember(itr->first))
            {
                --roll->totalGreed;
                continue;
            }

            uint8 randomN = urand(1, 100);
            SendLootRoll(itr->first, randomN, itr->second, *roll);
            if (maxresul < randomN)
            {
                maxguid = itr->first;
                maxresul = randomN;
                rollvote = itr->second;
            }
        }

        if (!maxguid.IsEmpty())
        {
            SendLootRollWon(maxguid, maxresul, rollvote, *roll);

            Player* player = FindOnRollMap(maxguid, *roll);
            if (player)
            {
                player->UpdateAchievementCriteria(CRITERIA_TYPE_ROLL_GREED_ON_LOOT, roll->item.ItemID, maxresul);

                if (rollvote == GREED)
                {
                    ItemPosCountVec dest;
                    InventoryResult msg = player->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, roll->item.ItemID, item->count);
                    if (msg == EQUIP_ERR_OK)
                    {
                        item->is_looted = true;
                        loot->NotifyItemRemoved(roll->itemSlot);
                        loot->unlootedCount--;
                        player->StoreNewItem(dest, roll->item.ItemID, true, item->item.RandomPropertiesID, item->GetAllowedLooters(), item->item.ItemBonus.BonusListIDs, item->item.ItemBonus.Context);
                    }
                    else
                    {
                        item->is_blocked = false;
                        player->SendEquipError(msg, nullptr, nullptr, roll->item.ItemID);
                    }
                }
                else if (rollvote == DISENCHANT)
                {
                    item->is_looted = true;
                    loot->NotifyItemRemoved(roll->itemSlot);
                    loot->unlootedCount--;
                    ItemTemplate const* pProto = sObjectMgr->GetItemTemplate(roll->item.ItemID);
                    player->AutoStoreLoot(pProto->GetId(), LootTemplates_Disenchant, false, true);
                    player->UpdateAchievementCriteria(CRITERIA_TYPE_CAST_SPELL, 13262); // Disenchant
                }
            }
            else
                item->is_blocked = false;
        }
        else
            roll->totalGreed = 0;
    }

    if (roll->totalNeed == 0 && roll->totalGreed == 0)
    {
        SendLootAllPassed(*roll);

        // remove is_blocked so that the item is lootable by all players
        item->is_blocked = false;
    }
}

void Group::SetTargetIcon(uint8 symbol, ObjectGuid target, ObjectGuid changedBy, uint8 partyIndex)
{
    if (symbol >= TARGET_ICONS_COUNT)
        return;

    if (!target.IsEmpty())
        for (uint8 i = 0; i < TARGET_ICONS_COUNT; ++i)
            if (m_targetIcons[i] == target)
                SetTargetIcon(i, ObjectGuid::Empty, changedBy, partyIndex);

    m_targetIcons[symbol] = target;

    WorldPackets::Party::SendRaidTargetUpdateSingle updateSingle;
    updateSingle.PartyIndex = partyIndex;
    updateSingle.Target = target;
    updateSingle.ChangedBy = changedBy;
    updateSingle.Symbol = symbol;
    BroadcastPacket(updateSingle.Write(), true);
}

void Group::SendTargetIconList(int8 partyIndex)
{
    WorldPackets::Party::SendRaidTargetUpdateAll updateAll;
    updateAll.PartyIndex = partyIndex;
    for (uint8 i = 0; i < TARGET_ICONS_COUNT; i++)
        updateAll.TargetIcons.insert(std::make_pair(i, m_targetIcons[i]));

    BroadcastPacket(updateAll.Write(), true);
}

void Group::SendUpdate()
{
    for (ObjectGuid const& memberGuid : GetMemberGuids())
        SendUpdateToPlayer(memberGuid);
}

// Built from a copy of the member list: the slot argument is only kept for the interface, the receiver is playerGUID
void Group::SendUpdateToPlayer(ObjectGuid playerGUID, MemberSlot* /*slot = nullptr*/)
{
    if (!playerGUID.IsPlayer())
        return;

    // the receiver and the members may stand on other maps: only short reads under the accessor lock
    bool receiverOk = false;
    uint32 receiverTeam = 0;
    ObjectAccessor::WithPlayer(playerGUID, [this, &receiverOk, &receiverTeam](Player* player)
    {
        receiverOk = player->GetSession() && player->GetGroup() == this;
        receiverTeam = player->GetTeam();
    });
    if (!receiverOk)
        return;

    MemberSlotList memberSlots = GetMemberSlots();
    if (std::none_of(memberSlots.begin(), memberSlots.end(), [&playerGUID](MemberSlot const& member) { return member.Guid == playerGUID; }))
        return;

    WorldPackets::Party::PartyUpdate partyUpdate;
    bool PvPGroup = isBGGroup() || isBFGroup();
    partyUpdate.PartyType = PvPGroup ? GROUP_TYPE_BG : GROUP_TYPE_NORMAL;
    partyUpdate.PartyFlags = m_groupFlags;
    partyUpdate.PartyGUID = m_guid;
    partyUpdate.LeaderGUID = GetLeaderGUID();
    partyUpdate.PartyIndex = m_groupCategory;
    partyUpdate.MyIndex = -1;

    uint8 index = 0;
    for (member_citerator citr = memberSlots.begin(); citr != memberSlots.end(); ++citr, ++index)
    {
        if (playerGUID == citr->Guid)
            partyUpdate.MyIndex = index;

        WorldPackets::Party::GroupPlayerInfos playerInfos;

        playerInfos.GUID = citr->Guid;
        playerInfos.Name = citr->Name;
        playerInfos.Class = citr->Class;

        playerInfos.Status = citr->fakeOnline ? MEMBER_STATUS_ONLINE : MEMBER_STATUS_OFFLINE;
        if (IsContactableMember(citr->Guid))
            playerInfos.Status = MEMBER_STATUS_ONLINE | (isBGGroup() || isBFGroup() ? MEMBER_STATUS_PVP : 0);

        playerInfos.Subgroup = citr->Group;
        playerInfos.Flags = citr->Flags;
        playerInfos.RolesAssigned = citr->Roles;
        playerInfos.FromSocialQueue = false;

        partyUpdate.PlayerList.push_back(playerInfos);
    }

    if (!memberSlots.empty())
    {
        partyUpdate.LootSettings.emplace();
        partyUpdate.LootSettings->Method = m_lootMethod;
        partyUpdate.LootSettings->Threshold = m_lootThreshold;
        partyUpdate.LootSettings->LootMaster = m_lootMethod == MASTER_LOOT ? m_looterGuid : ObjectGuid::Empty;

        partyUpdate.DifficultySettings.emplace();
        partyUpdate.DifficultySettings->DungeonDifficultyID = m_dungeonDifficulty;
        partyUpdate.DifficultySettings->RaidDifficultyID = m_raidDifficulty;
        partyUpdate.DifficultySettings->LegacyRaidDifficultyID = m_legacyRaidDifficulty;
    }

    // LfgInfos
    if (isLFGGroup())
    {
        uint32 QueueId = sLFGMgr->GetQueueId(m_guid);
        auto dungeon = sLFGMgr->GetLFGDungeon(sLFGMgr->GetDungeon(m_guid, true), receiverTeam);
        auto lfgState = sLFGMgr->GetState(m_guid, QueueId);
        uint8 flags = 0;
        if (lfgState == lfg::LFG_STATE_FINISHED_DUNGEON || dungeon && dungeon->dbc->Flags & LFG_FLAG_NON_BACKFILLABLE)
            flags |= 2;

        partyUpdate.LfgInfos.emplace();
        partyUpdate.LfgInfos->Slot = sLFGMgr->GetLFGDungeonEntry(sLFGMgr->GetDungeon(m_guid));
        partyUpdate.LfgInfos->MyFlags = flags;
        partyUpdate.LfgInfos->MyPartialClear = sLFGMgr->GetState(m_guid, QueueId) == lfg::LFG_STATE_FINISHED_DUNGEON ? 2 : 0;
        if (dungeon)
            partyUpdate.LfgInfos->MyGearDiff = 1.0f;
        partyUpdate.LfgInfos->MyFirstReward = lfgState != lfg::LFG_STATE_FINISHED_DUNGEON;

        partyUpdate.LfgInfos->MyRandomSlot = [playerGUID, QueueId]() -> uint32
        {
            auto const& selectedDungeons = sLFGMgr->GetSelectedDungeons(playerGUID, QueueId);
            if (selectedDungeons.size() == 1)
                if (auto dungeon = sLfgDungeonsStore.LookupEntry(*selectedDungeons.begin()))
                    if (dungeon->TypeID == LFG_TYPE_RANDOM)
                        return dungeon->ID;

            return 0;
        }();

        partyUpdate.LfgInfos->BootCount = 0;
        partyUpdate.LfgInfos->Aborted = false;
        partyUpdate.LfgInfos->MyStrangerCount = 0;
        partyUpdate.LfgInfos->MyKickVoteCount = 0;
    }

    ObjectAccessor::WithPlayer(playerGUID, [this, &partyUpdate](Player* player)
    {
        if (player->GetGroup() != this)
            return;

        partyUpdate.SequenceNum = player->NextGroupUpdateSequenceNumber(m_groupCategory);
        player->SendDirectMessage(partyUpdate.Write());
    });
}

void Group::SendUpdateDestroyGroupToPlayer(Player* player) const
{
    SendDestroyedPartyUpdate(player, m_groupCategory, m_guid);
}

void Group::UpdatePlayerOutOfRange(Player* player)
{
    if (!player || !player->IsInWorld() || !player->CanContact())
        return;

    WorldPackets::Party::PartyMemberStatseUpdate packet;
    packet.Initialize(player);

    auto p = packet.Write();
    for (MemberRef const& ref : GetMemberRefs())
    {
        if (ref.Guid == player->GetGUID())
            continue;

        // a member of the player's map belongs to this thread; one of another map is out of range anyway
        if (Player* member = ObjectAccessor::GetPlayer(*player, ref.Guid))
        {
            if (!member->IsWithinDist(player, member->GetSightRange(), false))
                member->SendDirectMessage(p);
        }
        else
            ObjectAccessor::SendToPlayer(ref.Guid, p);
    }
}

void Group::BroadcastAddonMessagePacket(WorldPacket const* packet, std::string const& prefix, bool ignorePlayersInBGRaid, int group /*= -1*/, ObjectGuid ignore /*= ObjectGuid::Empty*/)
{
    for (MemberRef const& ref : GetMemberRefs())
    {
        if (ignore && ref.Guid == ignore || group != -1 && ref.SubGroup != group)
            continue;

        ObjectAccessor::WithPlayer(ref.Guid, [this, packet, &prefix, ignorePlayersInBGRaid](Player* player)
        {
            if (!player->CanContact() || ignorePlayersInBGRaid && player->GetGroup() != this)
                return;

            if (WorldSession* session = player->GetSession())
                if (session->IsAddonRegistered(prefix))
                    player->SendDirectMessage(packet);
        });
    }
}

// Members are copied under m_lock and looked up again without it (see Group.h)
void Group::BroadcastPacket(const WorldPacket* packet, bool ignorePlayersInBGRaid, int group, ObjectGuid ignore)
{
    for (MemberRef const& ref : GetMemberRefs())
    {
        if ((ignore && ref.Guid == ignore) || (group != -1 && ref.SubGroup != group))
            continue;

        ObjectAccessor::WithPlayer(ref.Guid, [this, packet, ignorePlayersInBGRaid](Player* player)
        {
            if (player->CanContact() && (!ignorePlayersInBGRaid || player->GetGroup() == this))
                player->SendDirectMessage(packet);
        });
    }
}

void Group::BroadcastReadyCheck(WorldPacket const* packet)
{
    for (MemberRef const& ref : GetMemberRefs())
        if (IsLeader(ref.Guid) || IsAssistant(ref.Guid) || m_groupFlags & GROUP_FLAG_EVERYONE_ASSISTANT)
            SendToContactableMember(ref.Guid, packet);
}

void Group::OfflineReadyCheck()
{
    std::vector<ObjectGuid> pending;
    {
        std::lock_guard<std::recursive_mutex> guard(m_lock);
        for (auto const& member : m_memberSlots)
            if (member.Guid.IsPlayer() && !member.ReadyChecked)
                pending.push_back(member.Guid);
    }

    bool ready = false;
    for (ObjectGuid const& memberGuid : pending)
    {
        bool connected = false;
        ObjectAccessor::WithPlayer(memberGuid, [&connected](Player* player) { connected = player->GetSession() != nullptr; });
        if (connected)
            continue;

        if (!SetMemberReadyChecked(memberGuid))
            continue;

        WorldPackets::Party::ReadyCheckResponse response;
        response.PartyGUID = GetGUID();
        response.Player = memberGuid;
        response.IsReady = ready;
        BroadcastReadyCheck(response.Write());
    }
}

bool Group::StartReadyCheck(ObjectGuid starterGuid, int8 partyIndex)
{
    std::lock_guard<std::recursive_mutex> guard(m_lock);
    if (IsReadyCheckActive())
        return false;

    auto starterSlot = _getMemberWSlot(starterGuid);
    if (starterSlot == m_memberSlots.end())
        return false;

    for (auto& member : m_memberSlots)
        member.ReadyChecked = false;

    // the initiator counts as having answered
    starterSlot->ReadyChecked = true;

    m_readyCheck = true;
    m_readyCheckStartTime = GameTime::GetGameTimeMS();
    m_readyCheckPartyIndex = partyIndex;
    return true;
}

bool Group::IsReadyCheckActive() const
{
    std::lock_guard<std::recursive_mutex> guard(m_lock);
    // a check whose timeout event was lost (initiator logged out) lapses on its own
    return m_readyCheck && getMSTimeDiff(m_readyCheckStartTime, GameTime::GetGameTimeMS()) < READY_CHECK_DURATION;
}

bool Group::SetMemberReadyChecked(ObjectGuid guid)
{
    std::lock_guard<std::recursive_mutex> guard(m_lock);
    if (!IsReadyCheckActive())
        return false;

    auto slot = _getMemberWSlot(guid);
    if (slot == m_memberSlots.end() || slot->ReadyChecked)
        return false;

    slot->ReadyChecked = true;
    return true;
}

bool Group::IsReadyCheckCompleted() const
{
    std::lock_guard<std::recursive_mutex> guard(m_lock);
    for (auto const& member : m_memberSlots)
        if (member.Guid.IsPlayer() && !member.ReadyChecked)
            return false;

    return true;
}

void Group::EndReadyCheck()
{
    {
        // two last answers from two maps: only one of them ends the check
        std::lock_guard<std::recursive_mutex> guard(m_lock);
        if (!m_readyCheck)
            return;

        m_readyCheck = false;
    }

    WorldPackets::Party::ReadyCheckCompleted readyCheckCompleted;
    readyCheckCompleted.PartyIndex = m_readyCheckPartyIndex;
    readyCheckCompleted.PartyGUID = GetGUID();
    BroadcastPacket(readyCheckCompleted.Write(), true);
}

void Group::ReadyCheckTimeout(uint32 startTime)
{
    {
        std::lock_guard<std::recursive_mutex> guard(m_lock);
        if (!m_readyCheck || m_readyCheckStartTime != startTime)
            return;
    }

    EndReadyCheck();
}

bool Group::_setMembersGroup(ObjectGuid guid, uint8 group)
{
    {
        std::lock_guard<std::recursive_mutex> guard(m_lock);
        auto slot = _getMemberWSlot(guid);
        if (slot == m_memberSlots.end())
            return false;

        slot->Group = group;

        SubGroupCounterIncrease(group);
    }

    if (!isBGGroup() && !isBFGroup())
    {
        CharacterDatabasePreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_GROUP_MEMBER_SUBGROUP);

        stmt->setUInt8(0, group);
        stmt->setUInt64(1, guid.GetCounter());

        CharacterDatabase.Execute(stmt);
    }

    return true;
}

bool Group::SameSubGroup(Player const* member1, Player const* member2) const
{
    if (!member1 || !member2)
        return false;

    if (member1->GetGroup() != this || member2->GetGroup() != this)
        return false;

    return member1->GetSubGroup() == member2->GetSubGroup();
}

// Allows setting sub groups both for online or offline members
void Group::ChangeMembersGroup(ObjectGuid guid, uint8 group)
{
    // Only raid groups have sub groups
    if (!isRaidGroup())
        return;

    {
        std::lock_guard<std::recursive_mutex> guard(m_lock);
        // Check if player is really in the raid
        auto slot = _getMemberWSlot(guid);
        if (slot == m_memberSlots.end())
            return;

        // Abort if the player is already in the target sub group
        uint8 prevSubGroup = slot->Group;
        if (prevSubGroup == group)
            return;

        // Update the player slot with the new sub group setting
        slot->Group = group;

        // Increase the counter of the new sub group..
        SubGroupCounterIncrease(group);

        // ..and decrease the counter of the previous one
        SubGroupCounterDecrease(prevSubGroup);
    }

    // Preserve new sub group in database for non-raid groups
    if (!isBGGroup() && !isBFGroup())
    {
        CharacterDatabasePreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_GROUP_MEMBER_SUBGROUP);

        stmt->setUInt8(0, group);
        stmt->setUInt64(1, guid.GetCounter());

        CharacterDatabase.Execute(stmt);
    }

    // In case the moved player is online, update the player object with the new sub group references (his thread)
    PostSubGroupChange(guid, group);

    // Broadcast the changes to the group
    SendUpdate();
}

void Group::SwapMembersGroups(ObjectGuid firstGuid, ObjectGuid secondGuid)
{
    if (!isRaidGroup())
        return;

    std::pair<ObjectGuid, uint8> swapped[2];
    {
        std::lock_guard<std::recursive_mutex> guard(m_lock);
        member_witerator slots[2];
        slots[0] = _getMemberWSlot(firstGuid);
        slots[1] = _getMemberWSlot(secondGuid);
        if (slots[0] == m_memberSlots.end() || slots[1] == m_memberSlots.end())
            return;

        if (slots[0]->Group == slots[1]->Group)
            return;

        std::swap(slots[0]->Group, slots[1]->Group);
        for (uint8 i = 0; i < 2; ++i)
            swapped[i] = { slots[i]->Guid, slots[i]->Group };
    }

    for (uint8 i = 0; i < 2; ++i)
    {
        if (!isBGGroup() && !isBFGroup())
        {
            CharacterDatabasePreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_GROUP_MEMBER_SUBGROUP);
            stmt->setUInt8(0, swapped[i].second);
            stmt->setUInt64(1, swapped[i].first.GetCounter());
            CharacterDatabase.Execute(stmt);
        }

        PostSubGroupChange(swapped[i].first, swapped[i].second);
    }

    SendUpdate();
}

void Group::PostSubGroupChange(ObjectGuid const& guid, uint8 subGroup)
{
    Group* self = this;
    ObjectAccessor::PostToPlayer(guid, [self, subGroup](Player* player) -> void
    {
        if (player->GetGroup() == self)
            player->GetGroupRef().setSubGroup(subGroup);
        else if (player->GetOriginalGroup() == self)
            player->GetOriginalGroupRef().setSubGroup(subGroup);
    });
}

// Retrieve the next Round-Roubin player for the group
//
// No update done if loot method is Master or FFA.
//
// If the RR player is not yet set for the group, the first group member becomes the round-robin player.
// If the RR player is set, the next player in group becomes the round-robin player.
//
// If ifneed is true,
//      the current RR player is checked to be near the looted object.
//      if yes, no update done.
//      if not, he loses his turn.
void Group::UpdateLooterGuid(WorldObject* pLootedObject, bool ifneed)
{
    switch (GetLootMethod())
    {
        case MASTER_LOOT:
        case FREE_FOR_ALL:
        case PERSONAL_LOOT:
            return;
        default:
            // round robin style looting applies for all low
            // quality items in each loot method except free for all and master loot
            break;
    }

    ObjectGuid oldLooterGUID = GetLooterGuid();
    std::vector<ObjectGuid> memberGuids = GetMemberGuids();
    auto guid_itr = std::find(memberGuids.begin(), memberGuids.end(), oldLooterGUID);
    if (guid_itr != memberGuids.end())
    {
        if (ifneed)
        {
            // not update if only update if need and ok (candidates are on the loot's map: this thread)
            Player* looter = ObjectAccessor::GetPlayer(*pLootedObject, *guid_itr);
            if (looter && looter->IsWithinDistInMap(pLootedObject, sWorld->getFloatConfig(CONFIG_GROUP_XP_DISTANCE), false))
                return;
        }
        ++guid_itr;
    }

    // search next after current
    Player* pNewLooter = nullptr;
    for (auto itr = guid_itr; itr != memberGuids.end(); ++itr)
    {
        if (!itr->IsPlayer())
            continue;

        if (Player* player = ObjectAccessor::GetPlayer(*pLootedObject, *itr))
            if (player->IsWithinDistInMap(pLootedObject, sWorld->getFloatConfig(CONFIG_GROUP_XP_DISTANCE), false))
            {
                pNewLooter = player;
                break;
            }
    }

    if (!pNewLooter)
    {
        // search from start
        for (auto itr = memberGuids.begin(); itr != guid_itr; ++itr)
        {
            if (!itr->IsPlayer())
                continue;
            if (Player* player = ObjectAccessor::GetPlayer(*pLootedObject, *itr))
                if (player->IsWithinDistInMap(pLootedObject, sWorld->getFloatConfig(CONFIG_GROUP_XP_DISTANCE), false))
                {
                    pNewLooter = player;
                    break;
                }
        }
    }

    if (pNewLooter)
    {
        if (oldLooterGUID != pNewLooter->GetGUID())
        {
            SetLooterGuid(pNewLooter->GetGUID());
            SendUpdate();
        }
    }
    else
    {
        SetLooterGuid(ObjectGuid::Empty);
        SendUpdate();
    }
}

uint8 Group::CanJoinBattlegroundQueue(Battleground const* bgOrTemplate, uint8 bgQueueTypeId, uint32 minPlayerCount, bool isRated, uint32 bracketType, ObjectGuid& errorGuid)
{
    if (isLFGGroup())
        return MS::Battlegrounds::GroupJoinBattlegroundResult::ERR_LFG_CANT_USE_BATTLEGROUND;

    auto bgEntry = sBattlemasterListStore.LookupEntry(bgOrTemplate->GetTypeID());
    if (!bgEntry)
        return MS::Battlegrounds::GroupJoinBattlegroundResult::ERR_BATTLEGROUND_JOIN_FAILED;

    auto memberscount = GetMembersCount();
    if (memberscount > bgEntry->MaxPlayers)
        return MS::Battlegrounds::GroupJoinBattlegroundResult::ERR_BATTLEGROUND_NONE;

    std::vector<MemberRef> refs = GetMemberRefs();

    auto bgQueueTypeIdRandom = MS::Battlegrounds::GetBgQueueTypeIdByBgTypeID(MS::Battlegrounds::BattlegroundTypeId::BattlegroundRandom);
    uint8 const pvpType = bgOrTemplate->IsArena() || bgOrTemplate->IsSkirmish() ? MS::Battlegrounds::IternalPvpTypes::Arena : MS::Battlegrounds::IternalPvpTypes::Battleground;

    // members stand on other maps: their state is copied under the accessor lock, then checked
    struct MemberState
    {
        uint32 Team = 0;
        uint8 Level = 0;
        bool HasBracket = false;
        bool InThisQueue = false;
        bool InRandomQueue = false;
        bool InAnyQueue = false;
        bool CanJoin = false;
        bool HasFreeQueue = false;
    };
    auto readMember = [&](ObjectGuid const& guid, MemberState& state) -> bool
    {
        return ObjectAccessor::WithPlayer(guid, [&](Player* member)
        {
            state.Team = member->GetTeam();
            state.Level = member->getLevel();
            state.HasBracket = bracketType >= MS::Battlegrounds::BracketType::Max || member->getBracket(bracketType);
            state.InThisQueue = member->InBattlegroundQueueForBattlegroundQueueType(bgQueueTypeId);
            state.InRandomQueue = member->InBattlegroundQueueForBattlegroundQueueType(bgQueueTypeIdRandom);
            state.InAnyQueue = member->InBattlegroundQueue();
            state.CanJoin = member->CanJoinToBattleground(pvpType);
            state.HasFreeQueue = member->HasFreeBattlegroundQueueId();
        }, ObjectAccessor::PlayerScope::InOrOutOfWorld);
    };

    MemberState reference;
    if (refs.empty() || !readMember(refs.front().Guid, reference))
        return MS::Battlegrounds::GroupJoinBattlegroundResult::ERR_BATTLEGROUND_JOIN_FAILED;

    auto bracketEntry = sDB2Manager.GetBattlegroundBracketByLevel(bgOrTemplate->GetMapId(), reference.Level);
    if (!bracketEntry)
        return MS::Battlegrounds::GroupJoinBattlegroundResult::ERR_BATTLEGROUND_JOIN_FAILED;

    if (!reference.HasBracket)
        return MS::Battlegrounds::GroupJoinBattlegroundResult::ERR_BATTLEGROUND_JOIN_FAILED;

    if (bgOrTemplate->GetMaxGroupSize() && GetMembersCount() > bgOrTemplate->GetMaxGroupSize())
        return MS::Battlegrounds::GroupJoinBattlegroundResult::ERR_BATTLEFIELD_TEAM_PARTY_SIZE;

    auto team = reference.Team;

    // the same for every member: asked once, outside the accessor lock (they take m_lock)
    bool const arenaAll = bgOrTemplate->GetTypeID() == MS::Battlegrounds::BattlegroundTypeId::ArenaAll;
    bool const tooManyHealers = arenaAll && !GetMaxCountOfRolesForArenaQueue(ROLES_HEALER);
    bool const tooManyTanks = arenaAll && !GetMaxCountOfRolesForArenaQueue(ROLES_TANK);

    memberscount = 0;
    for (MemberRef const& ref : refs)
    {
        ++memberscount;
        MemberState member;
        if (!readMember(ref.Guid, member))
            return MS::Battlegrounds::GroupJoinBattlegroundResult::ERR_BATTLEGROUND_JOIN_FAILED;

        if (member.Team != team)
        {
            errorGuid = ref.Guid;
            return MS::Battlegrounds::GroupJoinBattlegroundResult::ERR_BATTLEGROUND_JOIN_TIMED_OUT;
        }

        if (sDB2Manager.GetBattlegroundBracketByLevel(bracketEntry->MapID, member.Level) != bracketEntry)
            return MS::Battlegrounds::GroupJoinBattlegroundResult::ERR_BATTLEGROUND_JOIN_RANGE_INDEX;

        if (member.InThisQueue)
            return MS::Battlegrounds::GroupJoinBattlegroundResult::ERR_BATTLEGROUND_JOIN_FAILED;            // not blizz-like

        if (member.InRandomQueue)
            return MS::Battlegrounds::GroupJoinBattlegroundResult::ERR_IN_RANDOM_BG;

        if (bgOrTemplate->GetTypeID() == MS::Battlegrounds::BattlegroundTypeId::BattlegroundRandom && member.InAnyQueue)
            return MS::Battlegrounds::GroupJoinBattlegroundResult::ERR_IN_NON_RANDOM_BG;

        if (arenaAll && !member.CanJoin)
            return MS::Battlegrounds::GroupJoinBattlegroundResult::ERR_GROUP_JOIN_BATTLEGROUND_DESERTERS;

        if (tooManyHealers)
            return MS::Battlegrounds::GroupJoinBattlegroundResult::ERR_BATTLEGROUND_JOIN_TOO_MANY_HEALERS;

        if (tooManyTanks)
            return MS::Battlegrounds::GroupJoinBattlegroundResult::ERR_BATTLEGROUND_JOIN_TOO_MANY_TANKS;

        if (!member.HasFreeQueue)
            return MS::Battlegrounds::GroupJoinBattlegroundResult::ERR_BATTLEGROUND_TOO_MANY_QUEUES;        // not blizz-like

        if (sLFGMgr->HasQueue(ref.Guid))
            return MS::Battlegrounds::GroupJoinBattlegroundResult::ERR_LFG_CANT_USE_BATTLEGROUND;
    }

    if ((bgOrTemplate->IsArena() || isRated) && memberscount != minPlayerCount && !sBattlegroundMgr->isTesting())
        return MS::Battlegrounds::GroupJoinBattlegroundResult::ERR_ARENA_TEAM_PARTY_SIZE;

    return MS::Battlegrounds::GroupJoinBattlegroundResult::ERR_BATTLEGROUND_NONE;
}

//===================================================
//============== Roll ===============================
//===================================================

void Group::SetDungeonDifficultyID(Difficulty difficulty)
{
    {
        std::lock_guard<std::recursive_mutex> guard(m_lock);
        m_dungeonDifficulty = difficulty;
    }
    if (!isBGGroup() && !isBFGroup() && difficulty != DIFFICULTY_MYTHIC_KEYSTONE)
    {
        CharacterDatabasePreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_GROUP_DIFFICULTY);

        stmt->setUInt8(0, uint8(m_dungeonDifficulty != DIFFICULTY_MYTHIC_KEYSTONE ? m_dungeonDifficulty : DIFFICULTY_MYTHIC_DUNGEON));
        stmt->setUInt32(1, m_dbStoreId);

        CharacterDatabase.Execute(stmt);
    }

    // members of other maps change in their own threads
    for (MemberRef const& ref : GetMemberRefs())
        ObjectAccessor::PostToPlayer(ref.Guid, [difficulty](Player* player) -> void
        {
            if (!player->GetSession())
                return;

            player->SetDungeonDifficultyID(difficulty);
            player->SendDungeonDifficulty();
        });
}

void Group::SetRaidDifficultyID(Difficulty difficulty)
{
    m_raidDifficulty = difficulty;
    if (!isBGGroup() && !isBFGroup())
    {
        CharacterDatabasePreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_GROUP_RAID_DIFFICULTY);

        stmt->setUInt8(0, uint8(m_raidDifficulty));
        stmt->setUInt32(1, m_dbStoreId);

        CharacterDatabase.Execute(stmt);
    }

    // members of other maps change in their own threads
    for (MemberRef const& ref : GetMemberRefs())
        ObjectAccessor::PostToPlayer(ref.Guid, [difficulty](Player* player) -> void
        {
            if (!player->GetSession())
                return;

            player->SetRaidDifficultyID(difficulty);
            player->SendRaidDifficulty(false);
        });
}

void Group::SetLegacyRaidDifficultyID(Difficulty difficulty)
{
    m_legacyRaidDifficulty = difficulty;
    if (!isBGGroup() && !isBFGroup())
    {
        CharacterDatabasePreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_GROUP_LEGACY_RAID_DIFFICULTY);

        stmt->setUInt8(0, uint8(m_legacyRaidDifficulty));
        stmt->setUInt32(1, m_dbStoreId);

        CharacterDatabase.Execute(stmt);
    }

    // members of other maps change in their own threads
    for (MemberRef const& ref : GetMemberRefs())
        ObjectAccessor::PostToPlayer(ref.Guid, [difficulty](Player* player) -> void
        {
            if (!player->GetSession())
                return;

            player->SetLegacyRaidDifficultyID(difficulty);
            player->SendRaidDifficulty(true);
        });
}

Difficulty Group::GetDifficultyID(MapEntry const* mapEntry) const
{
    if (!mapEntry->IsRaid())
        return m_dungeonDifficulty;

    MapDifficultyEntry const* defaultDifficulty = sDB2Manager.GetDefaultMapDifficulty(mapEntry->ID);
    if (!defaultDifficulty)
        return m_legacyRaidDifficulty;

    DifficultyEntry const* difficulty = sDifficultyStore.LookupEntry(defaultDifficulty->DifficultyID);
    if (!difficulty || difficulty->Flags & DIFFICULTY_FLAG_LEGACY)
        return m_legacyRaidDifficulty;

    return m_raidDifficulty;
}

Difficulty Group::GetDungeonDifficultyID() const
{
    std::lock_guard<std::recursive_mutex> guard(m_lock);
    return m_dungeonDifficulty;
}

Difficulty Group::GetRaidDifficultyID() const
{
    return m_raidDifficulty;
}

Difficulty Group::GetLegacyRaidDifficultyID() const
{
    return m_legacyRaidDifficulty;
}

void Group::ResetInstances(uint8 method, bool isRaid, bool isLegacy, Player* SendMsgTo)
{
    if (isBGGroup() || isBFGroup())
        return;

    // method can be INSTANCE_RESET_ALL, INSTANCE_RESET_CHANGE_DIFFICULTY, INSTANCE_RESET_GROUP_DISBAND

    // we assume that when the difficulty changes, all instances that can be reset will be
    Difficulty diff = GetDungeonDifficultyID();
    if (isRaid)
    {
        if (!isLegacy)
            diff = GetRaidDifficultyID();
        else
            diff = GetLegacyRaidDifficultyID();
    }
    // after the raid switch, as in Player::ResetInstances
    uint8 boundType = sObjectMgr->GetboundTypeFromDifficulty(diff);

    // the binds are copied under m_bound_lock; maps, saves and the database are dealt with once it is released
    std::vector<std::pair<uint32, InstanceSave*>> candidates;
    {
        std::lock_guard<std::recursive_mutex> _lock(m_bound_lock);
        for (auto const& itr : m_boundInstances[boundType])
        {
            InstanceSave* instanceSave = itr.second.save;
            const MapEntry* entry = sMapStore.LookupEntry(itr.first);
            if (!entry || entry->IsRaid() != isRaid || !instanceSave->CanReset() && method != INSTANCE_RESET_GROUP_DISBAND)
                continue;

            // the "reset all instances" method can only reset normal maps
            if (method == INSTANCE_RESET_ALL && (entry->IsRaid() || diff == DIFFICULTY_MYTHIC_DUNGEON || diff == DIFFICULTY_MYTHIC_KEYSTONE))
                continue;

            candidates.emplace_back(itr.first, instanceSave);
        }
    }

    for (auto const& candidate : candidates)
    {
        InstanceSave* instanceSave = candidate.second;

        bool isEmpty = true;
        // if the map is loaded, reset it: it runs in its own thread, so the reset is only requested there and the
        // answer given here is whether players are inside right now (what InstanceMap::Reset returned). The map is
        // only reached under its parent's lock, no Map* is kept here.
        // (a save that cannot be reset only gets here on a disband, which then just unbinds)
        if (instanceSave->CanReset())
        {
            bool hadPlayers = false;
            if (sMapMgr->RequestInstanceReset(instanceSave->GetMapId(), instanceSave->GetInstanceId(), method, &hadPlayers))
                isEmpty = !hadPlayers;
        }

        if (SendMsgTo)
        {
            if (isEmpty)
                SendMsgTo->SendResetInstanceSuccess(instanceSave->GetMapId());
            else
                SendMsgTo->SendResetInstanceFailed(ResetFailedReason::FAILED, instanceSave->GetMapId());
        }

        if (!isEmpty && method != INSTANCE_RESET_GROUP_DISBAND && method != INSTANCE_RESET_CHANGE_DIFFICULTY)
            continue;

        // unbound only if no other thread changed the bind meanwhile
        bool unbound = false;
        {
            std::lock_guard<std::recursive_mutex> _lock(m_bound_lock);
            auto itr = m_boundInstances[boundType].find(candidate.first);
            if (itr != m_boundInstances[boundType].end() && itr->second.save == instanceSave)
            {
                m_boundInstances[boundType].erase(itr);
                unbound = true;
            }
        }

        if (!unbound)
            continue;

        // do not reset the instance, just unbind if others are permanently bound to it
        if (instanceSave->CanReset())
            instanceSave->DeleteFromDB();
        else
        {
            CharacterDatabasePreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_GROUP_INSTANCE_BY_INSTANCE);

            stmt->setUInt32(0, instanceSave->GetInstanceId());

            CharacterDatabase.Execute(stmt);
        }

        // this unloads the instance save unless online players are bound to it
        // (eg. permanent binds or GM solo binds)
        instanceSave->RemoveGroup(this);
    }
}

InstanceGroupBind Group::GetBoundInstanceCopy(Player* player)
{
    return GetBoundInstanceCopy(sMapStore.LookupEntry(player->GetMapId()));
}

InstanceGroupBind Group::GetBoundInstanceCopy(Map* aMap)
{
    return GetBoundInstanceCopy(aMap->GetEntry());
}

InstanceGroupBind Group::GetBoundInstanceCopy(MapEntry const* mapEntry)
{
    if (!mapEntry || !mapEntry->IsDungeon())
        return InstanceGroupBind();

    return GetBoundInstanceCopy(GetDifficultyID(mapEntry), mapEntry->ID);
}

InstanceGroupBind Group::GetBoundInstanceCopy(Difficulty difficulty, uint32 mapId)
{
    // some instances only have one difficulty
    sDB2Manager.GetDownscaledMapDifficultyData(mapId, difficulty);

    uint8 boundType = sObjectMgr->GetboundTypeFromDifficulty(difficulty);

    std::lock_guard<std::recursive_mutex> _lock(m_bound_lock);
    auto itr = m_boundInstances[boundType].find(mapId);
    if (itr != m_boundInstances[boundType].end())
        return itr->second;
    return InstanceGroupBind();
}

Group::BoundInstancesMap Group::GetBoundInstancesCopy(Difficulty difficulty)
{
    std::lock_guard<std::recursive_mutex> _lock(m_bound_lock);
    return m_boundInstances[sObjectMgr->GetboundTypeFromDifficulty(difficulty)];
}

InstanceGroupBind* Group::GetBoundInstance(Player* player)
{
    return GetBoundInstance(sMapStore.LookupEntry(player->GetMapId()));
}

InstanceGroupBind* Group::GetBoundInstance(Map* aMap)
{
    return GetBoundInstance(aMap->GetEntry());
}

InstanceGroupBind* Group::GetBoundInstance(MapEntry const* mapEntry)
{
    if (!mapEntry || !mapEntry->IsDungeon())
        return nullptr;

    return GetBoundInstance(GetDifficultyID(mapEntry), mapEntry->ID);
}

InstanceGroupBind* Group::GetBoundInstance(Difficulty difficulty, uint32 mapId)
{
    // some instances only have one difficulty
    sDB2Manager.GetDownscaledMapDifficultyData(mapId, difficulty);

    uint8 boundType = sObjectMgr->GetboundTypeFromDifficulty(difficulty);

    std::lock_guard<std::recursive_mutex> _lock(m_bound_lock);
    auto itr = m_boundInstances[boundType].find(mapId);
    if (itr != m_boundInstances[boundType].end())
        return &itr->second;
    return nullptr;
}

InstanceGroupBind Group::BindToInstance(InstanceSave* save, bool permanent, bool load)
{
    if (!save || isBGGroup() || isBFGroup())
        return InstanceGroupBind();

    if (save->CanBeSave())
        save->SetPerm(permanent);
    else
        permanent = false;

    uint8 boundType = sObjectMgr->GetboundTypeFromDifficulty(save->GetDifficultyID());

    // The save learns about the group before the bind is visible, so that it is never unloaded under a bind; its own
    // list lock is not taken under m_bound_lock.
    bool alreadyBound = false;
    {
        std::lock_guard<std::recursive_mutex> _lock(m_bound_lock);
        auto itr = m_boundInstances[boundType].find(save->GetMapId());
        alreadyBound = itr != m_boundInstances[boundType].end() && itr->second.save == save;
    }

    if (!alreadyBound)
        save->AddGroup(this);

    InstanceSave* previousSave = nullptr;
    bool previousPerm = false;
    InstanceGroupBind result;
    bool closed = false;
    {
        std::lock_guard<std::recursive_mutex> _lock(m_bound_lock);
        // a disbanding group takes no new bind (its rows would be written under a db store id about to be freed)
        if (m_bindsClosed)
            closed = true;
        else
        {
            InstanceGroupBind& bind = m_boundInstances[boundType][save->GetMapId()];
            previousSave = bind.save;
            previousPerm = bind.perm;
            bind.save = save;
            bind.perm = permanent;
            result = bind;
        }
    }

    if (closed)
    {
        if (!alreadyBound)
            save->RemoveGroup(this);
        return InstanceGroupBind();
    }

    if (save->CanBeSave() && !load && (!previousSave || permanent != previousPerm || save != previousSave))
        UpdateInstance(save);

    if (previousSave && previousSave != save)
        previousSave->RemoveGroup(this);
    // rebound to the same save by another thread in between: the save holds the group twice, std::list::remove drops both at once

    if (!load)
        TC_LOG_DEBUG("maps", "Group::BindToInstance: Group (guid: %u, storage id: %u) is now bound to map %d, instance %d, difficulty %d",
        GetGUIDLow(), m_dbStoreId, save->GetMapId(), save->GetInstanceId(), save->GetDifficultyID());

    return result;
}

void Group::UnbindInstance(uint32 mapid, uint8 difficulty, bool unload)
{
    uint8 boundType = sObjectMgr->GetboundTypeFromDifficulty(difficulty);
    InstanceSave* save = nullptr;
    {
        std::lock_guard<std::recursive_mutex> _lock(m_bound_lock);
        auto itr = m_boundInstances[boundType].find(mapid);
        if (itr == m_boundInstances[boundType].end())
            return;

        save = itr->second.save;
        m_boundInstances[boundType].erase(itr);
    }

    if (!unload)
    {
        CharacterDatabasePreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_GROUP_INSTANCE_BY_GUID);
        stmt->setUInt32(0, m_dbStoreId);
        stmt->setUInt32(1, save->GetInstanceId());
        CharacterDatabase.Execute(stmt);
    }

    save->RemoveGroup(this);                // save can become invalid
}

// For InstanceSaveManager (world thread, no save lock held): the bind is dropped only if it still names this save;
// the save forgets the group in any case. RemoveGroup may unload the save unless the manager holds lock_instLists.
void Group::UnbindInstance(InstanceSave* save, bool unload)
{
    if (!save)
        return;

    uint8 boundType = sObjectMgr->GetboundTypeFromDifficulty(save->GetDifficultyID());
    bool unbound = false;
    {
        std::lock_guard<std::recursive_mutex> _lock(m_bound_lock);
        auto itr = m_boundInstances[boundType].find(save->GetMapId());
        if (itr != m_boundInstances[boundType].end() && itr->second.save == save)
        {
            m_boundInstances[boundType].erase(itr);
            unbound = true;
        }
    }

    if (unbound && !unload)
    {
        CharacterDatabasePreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_GROUP_INSTANCE_BY_GUID);
        stmt->setUInt32(0, m_dbStoreId);
        stmt->setUInt32(1, save->GetInstanceId());
        CharacterDatabase.Execute(stmt);
    }

    save->RemoveGroup(this);
}

void Group::UpdateInstance(InstanceSave* save)
{
    if (!save || !save->GetMapEntry() || save->GetMapEntry()->IsGarrison() || save->GetMapEntry()->CanCreatedZone())
        return;

    CharacterDatabasePreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_REP_GROUP_INSTANCE);

    stmt->setUInt64(0, m_dbStoreId);
    stmt->setUInt32(1, save->GetInstanceId());
    stmt->setUInt16(2, save->GetMapId());
    stmt->setUInt8(3, save->GetDifficultyID());
    stmt->setBool(4, save->GetPerm());
    stmt->setUInt32(5, save->GetCompletedEncounterMask());
    stmt->setString(6, save->GetData());
    stmt->setUInt32(7, save->GetResetTime());

    CharacterDatabase.Execute(stmt);
}

void Group::_homebindIfInstance(Player* player)
{
    if (player)
        HomebindIfInstance(player);
}

void Group::BroadcastGroupUpdate()
{
    // FG: HACK: force flags update on group leave - for values update hack
    // -- not very efficient but safe
    for (ObjectGuid const& memberGuid : GetMemberGuids())
    {
        if (!memberGuid.IsPlayer())
            continue;
        // the update queue belongs to the member's map
        ObjectAccessor::PostToPlayer(memberGuid, [](Player* pp) -> void
        {
            pp->ForceValuesUpdateAtIndex(UNIT_FIELD_BYTES_2);
            pp->ForceValuesUpdateAtIndex(UNIT_FIELD_FACTION_TEMPLATE);
            TC_LOG_DEBUG("misc", "-- Forced group value update for '%s'", pp->GetName());
        }, 0, ObjectAccessor::PlayerScope::InWorld);
    }
}

uint8 Group::GetGroupFlags() const
{
    return m_groupFlags;
}

GroupCategory Group::GetGroupCategory() const
{
    return m_groupCategory;
}

void Group::ResetMaxEnchantingLevel()
{
    uint32 maxEnchantingLevel = 0;
    for (ObjectGuid const& memberGuid : GetMemberGuids())
    {
        if (!memberGuid.IsPlayer())
            continue;
        ObjectAccessor::WithPlayer(memberGuid, [&maxEnchantingLevel](Player* pMember)
        {
            if (maxEnchantingLevel < pMember->GetSkillValue(SKILL_ENCHANTING))
                maxEnchantingLevel = pMember->GetSkillValue(SKILL_ENCHANTING);
        });
    }
    m_maxEnchantingLevel = maxEnchantingLevel;
}

void Group::SetLootMethod(LootMethod method)
{
    if (method > PERSONAL_LOOT)
        return;

    m_lootMethod = method;
    SaveLootSettings();
}

void Group::SetLooterGuid(ObjectGuid const& guid)
{
    m_looterGuid = guid;
    SaveLootSettings();
}

void Group::SetLootThreshold(ItemQualities threshold)
{
    m_lootThreshold = threshold;
    SaveLootSettings();
}

void Group::SaveLootSettings()
{
    if (isBGGroup() || isBFGroup() || !m_dbStoreId)
        return;

    CharacterDatabasePreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_GROUP_LOOT);
    stmt->setUInt8(0, uint8(m_lootMethod));
    stmt->setUInt64(1, m_looterGuid.GetCounter());
    stmt->setUInt8(2, uint8(m_lootThreshold));
    stmt->setUInt32(3, m_dbStoreId);
    CharacterDatabase.Execute(stmt);
}

void Group::SetLfgRoles(ObjectGuid guid, const uint8 roles)
{
    {
        std::lock_guard<std::recursive_mutex> guard(m_lock);
        auto slot = _getMemberWSlot(guid);
        if (slot == m_memberSlots.end())
            return;

        slot->Roles = roles;
    }

    if (!isBGGroup() && !isBFGroup())
    {
        CharacterDatabasePreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_GROUP_MEMBER_ROLE);
        stmt->setUInt8(0, roles);
        stmt->setUInt64(1, guid.GetCounter());
        CharacterDatabase.Execute(stmt);
    }

    // nothing is sent when the group is not listed
    sLFGListMgr->SendSocialQueueUpdateNotify(GetGUIDLow());
}

uint8 Group::GetLfgRoles(ObjectGuid guid)
{
    std::lock_guard<std::recursive_mutex> guard(m_lock);
    auto slot = _getMemberWSlot(guid);
    if (slot == m_memberSlots.end())
        return 0;

    return slot->Roles;
}

bool Group::IsFull() const
{
    std::lock_guard<std::recursive_mutex> guard(m_lock);
    return isRaidGroup() ? m_memberSlots.size() >= MAX_RAID_SIZE : m_memberSlots.size() >= MAX_GROUP_SIZE;
}

bool Group::isLFGGroup() const
{
    return (m_groupFlags & GROUP_FLAG_LFG) != 0;
}

bool Group::isRaidGroup() const
{
    return (m_groupFlags & GROUP_FLAG_RAID) != 0;
}

bool Group::isBGGroup() const
{
    return m_bgGroup != nullptr;
}

bool Group::isArenaGroup() const
{
    return m_bgGroup && m_bgGroup->IsArena();
}

bool Group::isBFGroup() const
{
    return m_bfGroup != nullptr;
}

bool Group::IsCreated() const
{
    return GetMembersCount() > 0;
}

ObjectGuid Group::GetLeaderGUID() const
{
    return m_leaderGuid;
}

ObjectGuid Group::GetGUID() const
{
    return m_guid;
}

uint32 Group::GetGUIDLow() const
{
    return m_guid.GetGUIDLow();
}

const char * Group::GetLeaderName() const
{
    return m_leaderName.c_str();
}

LootMethod Group::GetLootMethod() const
{
    return m_lootMethod;
}

ObjectGuid Group::GetLooterGuid() const
{
    return m_looterGuid;
}

ItemQualities Group::GetLootThreshold() const
{
    return m_lootThreshold;
}

uint32 Group::GetDbStoreId() const
{
    return m_dbStoreId;
}

bool Group::IsMember(ObjectGuid guid) const
{
    std::lock_guard<std::recursive_mutex> guard(m_lock);
    return _getMemberCSlot(guid) != m_memberSlots.end();
}

bool Group::IsLeader(ObjectGuid guid) const
{
    return GetLeaderGUID() == guid;
}

bool Group::IsAssistant(ObjectGuid guid) const
{
    std::lock_guard<std::recursive_mutex> guard(m_lock);
    auto mslot = _getMemberCSlot(guid);
    if (mslot == m_memberSlots.end())
        return false;

    return mslot->Flags & MEMBER_FLAG_ASSISTANT;
}

bool Group::SameSubGroup(ObjectGuid guid1, ObjectGuid guid2) const
{
    std::lock_guard<std::recursive_mutex> guard(m_lock);
    auto mslot2 = _getMemberCSlot(guid2);
    if (mslot2 == m_memberSlots.end())
        return false;
    return SameSubGroup(guid1, &*mslot2);
}

bool Group::SameSubGroup(ObjectGuid guid1, MemberSlot const* slot2) const
{
    std::lock_guard<std::recursive_mutex> guard(m_lock);
    auto mslot1 = _getMemberCSlot(guid1);
    if (mslot1 == m_memberSlots.end() || !slot2)
        return false;

    return mslot1->Group == slot2->Group;
}

bool Group::HasFreeSlotSubGroup(uint8 subgroup) const
{
    std::lock_guard<std::recursive_mutex> guard(m_lock);
    return m_subGroupsCounts && m_subGroupsCounts[subgroup] < MAX_GROUP_SIZE;
}

// a copy: the list changes from other map threads
Group::MemberSlotList Group::GetMemberSlots() const
{
    std::lock_guard<std::recursive_mutex> guard(m_lock);
    return m_memberSlots;
}

std::vector<ObjectGuid> Group::GetMemberGuids() const
{
    std::lock_guard<std::recursive_mutex> guard(m_lock);
    std::vector<ObjectGuid> guids;
    guids.reserve(m_memberSlots.size());
    for (MemberSlot const& member : m_memberSlots)
        guids.push_back(member.Guid);
    return guids;
}

// The linked (connected) members. A player cannot be freed while it is read here: its GroupReference unlinks under m_lock.
std::vector<Group::MemberRef> Group::GetMemberRefs() const
{
    std::lock_guard<std::recursive_mutex> guard(m_lock);
    std::vector<MemberRef> refs;
    for (GroupReference const* itr = m_memberMgr.getFirst(); itr != nullptr; itr = itr->next())
        if (Player const* player = itr->getSource())
            refs.push_back({ player->GetGUID(), itr->getSubGroup() });
    return refs;
}

void Group::ForEachLinkedMember(std::function<void(Player*)> const& fn) const
{
    for (MemberRef const& ref : GetMemberRefs())
        ObjectAccessor::WithPlayer(ref.Guid, fn);
}

void Group::ForEachMemberInMap(Map* map, std::function<void(Player*)> const& fn) const
{
    for (MemberRef const& ref : GetMemberRefs())
        if (Player* player = ObjectAccessor::FindPlayer(map, ref.Guid))
            fn(player);
}

GroupReference* Group::GetFirstMember()
{
    return m_memberMgr.getFirst();
}

GroupReference const* Group::GetFirstMember() const
{
    return m_memberMgr.getFirst();
}

uint32 Group::GetMembersCount() const
{
    std::lock_guard<std::recursive_mutex> guard(m_lock);
    return m_memberSlots.size();
}

uint8 Group::GetMemberGroup(ObjectGuid guid) const
{
    std::lock_guard<std::recursive_mutex> guard(m_lock);
    auto mslot = _getMemberCSlot(guid);
    if (mslot == m_memberSlots.end())
        return MAX_RAID_SUBGROUPS + 1;

    return mslot->Group;
}

void Group::SetBattlegroundGroup(Battleground* bg)
{
    m_bgGroup = bg;
}

void Group::SetBattlefieldGroup(Battlefield *bg)
{
    m_bfGroup = bg;
}

void Group::setGroupMemberRole(ObjectGuid guid, uint32 role)
{
    {
        std::lock_guard<std::recursive_mutex> guard(m_lock);
        for (auto& itr : m_memberSlots)
        {
            if (itr.Guid == guid)
            {
                itr.Roles = role;
                break;
            }
        }
    }

    // group_member is keyed by memberGuid only: a BG raid must not overwrite the home group row
    if (isBGGroup() || isBFGroup())
        return;

    CharacterDatabasePreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_GROUP_MEMBER_ROLE);
    if (stmt != nullptr)
    {
        stmt->setUInt8(0, role);
        stmt->setUInt64(1, guid.GetCounter());
        CharacterDatabase.Execute(stmt);
    }
}

void Group::SetGroupMemberFlag(ObjectGuid guid, bool apply, GroupMemberFlags flag)
{
    // Assistants, main assistants and main tanks are only available in raid groups
    if (!isRaidGroup())
        return;

    switch (flag)
    {
        case MEMBER_FLAG_MAINASSIST:
        case MEMBER_FLAG_MAINTANK:
        case MEMBER_FLAG_ASSISTANT:
            break;
        default:
            return;                                                      // This should never happen
    }

    // the flags changed under m_lock, written to the database once it is released
    std::vector<std::pair<ObjectGuid, uint8>> changedFlags;
    {
        std::lock_guard<std::recursive_mutex> guard(m_lock);
        // Check if player is really in the raid
        auto slot = _getMemberWSlot(guid);
        if (slot == m_memberSlots.end())
            return;

        // Unique flags: only taken from the current holder when given to someone else.
        if (apply && flag != MEMBER_FLAG_ASSISTANT)
        {
            for (auto& itr : m_memberSlots)
            {
                if (!(itr.Flags & flag))
                    continue;

                itr.Flags &= ~flag;
                changedFlags.emplace_back(itr.Guid, itr.Flags);
            }
        }

        // Switch the actual flag
        ToggleGroupMemberFlag(slot, flag, apply);
        changedFlags.emplace_back(guid, slot->Flags);
    }

    // Preserve the new setting in the db
    if (!isBGGroup() && !isBFGroup())
    {
        for (auto const& changed : changedFlags)
        {
            CharacterDatabasePreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_GROUP_MEMBER_FLAG);

            stmt->setUInt8(0, changed.second);
            stmt->setUInt64(1, changed.first.GetCounter());

            CharacterDatabase.Execute(stmt);
        }
    }

    // Broadcast the changes to the group
    SendUpdate();
}


void Group::ErraseRollbyRealSlot(uint8 slot, Loot* loot)
{
    // a roll naming this live loot is valid
    std::lock_guard<std::recursive_mutex> guard(m_lock);
    for (auto iter = RollId.begin(); iter != RollId.end(); ++iter)
    {
        if ((*iter)->itemSlot == slot && !(*iter)->lootedGUID.IsEmpty() && (*iter)->lootedGUID == loot->GetGUID())
        {
            delete *iter;
            RollId.erase(iter);
            break;
        }
    }
}

// m_lock held; validLoots from GetValidRollLoots
Group::Rolls::iterator Group::GetRoll(uint8 _aoeSlot, std::set<ObjectGuid> const& validLoots)
{
    for (auto iter = RollId.begin(); iter != RollId.end(); ++iter)
        if ((*iter)->aoeSlot == _aoeSlot && validLoots.count((*iter)->lootedGUID))
            return iter;
    return RollId.end();
}

void Group::LinkMember(GroupReference* pRef)
{
    std::lock_guard<std::recursive_mutex> guard(m_lock);
    m_memberMgr.insertFirst(pRef);
}

void Group::DelinkMember(ObjectGuid guid)
{
    std::lock_guard<std::recursive_mutex> guard(m_lock);
    GroupReference* ref = m_memberMgr.getFirst();
    while (ref)
    {
        GroupReference* nextRef = ref->next();
        if (ref->getSource()->GetGUID() == guid)
        {
            ref->unlink();
            break;
        }
        ref = nextRef;
    }
}

// unlocked reference, for the GM command only: GetBoundInstancesCopy elsewhere
Group::BoundInstancesMap& Group::GetBoundInstances(Difficulty difficulty)
{
    return m_boundInstances[sObjectMgr->GetboundTypeFromDifficulty(difficulty)];
}

void Group::_initRaidSubGroupsCounter()
{
    std::lock_guard<std::recursive_mutex> guard(m_lock);
    // Sub group counters initialization
    if (!m_subGroupsCounts)
        m_subGroupsCounts = new uint8[MAX_RAID_SUBGROUPS];

    memset(static_cast<void*>(m_subGroupsCounts), 0, MAX_RAID_SUBGROUPS * sizeof(uint8));

    for (member_citerator itr = m_memberSlots.begin(); itr != m_memberSlots.end(); ++itr)
        ++m_subGroupsCounts[itr->Group];
}

// _getMemberCSlot, _getMemberWSlot, SubGroupCounter*, ToggleGroupMemberFlag: the caller holds m_lock while using the result
Group::member_citerator Group::_getMemberCSlot(ObjectGuid Guid) const
{
    std::lock_guard<std::recursive_mutex> _lock(const_cast<Group*>(this)->m_lock);
    for (auto itr = m_memberSlots.begin(); itr != m_memberSlots.end(); ++itr)
        if (itr->Guid == Guid)
            return itr;

    return m_memberSlots.end();
}

Group::member_witerator Group::_getMemberWSlot(ObjectGuid Guid)
{
    for (auto itr = m_memberSlots.begin(); itr != m_memberSlots.end(); ++itr)
        if (itr->Guid == Guid)
            return itr;

    return m_memberSlots.end();
}

void Group::SubGroupCounterIncrease(uint8 subgroup)
{
    if (m_subGroupsCounts)
        ++m_subGroupsCounts[subgroup];
}

void Group::SubGroupCounterDecrease(uint8 subgroup)
{
    if (m_subGroupsCounts)
        --m_subGroupsCounts[subgroup];
}

void Group::RemoveUniqueGroupMemberFlag(GroupMemberFlags flag)
{
    std::vector<std::pair<ObjectGuid, uint8>> changedFlags;
    {
        std::lock_guard<std::recursive_mutex> guard(m_lock);
        for (auto& itr : m_memberSlots)
        {
            if (!(itr.Flags & flag))
                continue;

            itr.Flags &= ~flag;
            changedFlags.emplace_back(itr.Guid, itr.Flags);
        }
    }

    if (isBGGroup() || isBFGroup())
        return;

    for (auto const& changed : changedFlags)
    {
        CharacterDatabasePreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_GROUP_MEMBER_FLAG);
        stmt->setUInt8(0, changed.second);
        stmt->setUInt64(1, changed.first.GetCounter());
        CharacterDatabase.Execute(stmt);
    }
}

void Group::ToggleGroupMemberFlag(member_witerator slot, uint8 flag, bool apply)
{
    if (apply)
        slot->Flags |= flag;
    else
        slot->Flags &= ~flag;
}

bool Group::IsGuildGroup(ObjectGuid const& guildId, bool AllInSameMap, bool AllInSameInstanceId)
{
    uint32 mapId = 0;
    uint32 InstanceId = 0;

    // members stand on other maps: their state is copied under the accessor lock
    struct GuildMemberState
    {
        uint32 MapId;
        uint32 InstanceId;
        Map* CurrentMap;
        uint32 BattlegroundId;
        uint16 BattlegroundTypeId;
    };
    std::vector<GuildMemberState> members;
    for (MemberRef const& ref : GetMemberRefs()) // Loop trought all members
        ObjectAccessor::WithPlayer(ref.Guid, [&members, &guildId](Player* player)
        {
            if (player->GetGuildId() == guildId.GetCounter() && player->FindMap()) // Check if it has a guild
                members.push_back({ player->GetMapId(), player->GetInstanceId(), player->FindMap(), player->GetBattlegroundId(), player->GetBattlegroundTypeId() });
        });

    bool ret = false;

    auto count = members.size();
    uint32 membersCount = GetMembersCount();
    for (GuildMemberState const& member : members) // Iterate through players
    {
        if (mapId == 0)
            mapId = member.MapId;

        if (InstanceId == 0)
            InstanceId = member.InstanceId;

        Map* map = member.CurrentMap;
        if (count >= map->GetMapMaxPlayers() * 0.8f)
            ret = true;

        if (map->IsNonRaidDungeon() && !ret)
            if (count >= 3)
                ret = true;

        if (map->IsBattleArena() && !ret)
            if (count == membersCount)
                ret = true;

        if (map->IsBattleground() && !ret && member.BattlegroundId)
            if (Battleground* bg = sBattlegroundMgr->GetBattleground(member.BattlegroundId, member.BattlegroundTypeId))
                if (count >= uint32(bg->GetMaxPlayers() * 0.8f))
                    ret = true;

        if (AllInSameMap && mapId != member.MapId)
            return false;

        if (AllInSameInstanceId && InstanceId != member.InstanceId)
            return false;
    }

    return ret;
}

void Group::UpdateGuildAchievementCriteria(CriteriaTypes type, uint32 miscValue1, uint32 miscValue2, uint32 miscValue3, Unit* pUnit, WorldObject* pRewardSource)
{
    // members are only credited on the reward source's map (every caller passes one): they belong to this thread
    if (!pRewardSource)
        return;

    // We will update criteria for each guild in grouplist but only once
    std::list<ObjectGuid::LowType> guildList;
    for (MemberRef const& ref : GetMemberRefs())
    {
        if (Player *pPlayer = ObjectAccessor::GetPlayer(*pRewardSource, ref.Guid))
        {
            // Check for reward
            if (pRewardSource)
            {
                if (!pPlayer->IsAtGroupRewardDistance(pRewardSource))
                    continue;

                if (!pPlayer->IsAlive())
                    continue;
            }

            ObjectGuid::LowType guildId = pPlayer->GetGuildId();
            if (!guildId)
                continue;

            if (!guildList.empty())
            {
                // if we already have any guild in list
                // then check new guild
                bool bUnique = true;
                for (std::list<ObjectGuid::LowType>::const_iterator itr2 = guildList.begin(); itr2 != guildList.end(); ++itr2)
                {
                    if (*itr2 == guildId)
                    {
                        bUnique = false;
                        break;
                    }
                }
                // If we have already rewarded current guild then continue
                // else update criteria 
                if (bUnique && guildId)
                {
                    guildList.push_back(guildId);
                    if (Guild* pGuild = sGuildMgr->GetGuildById(guildId))
                        pGuild->UpdateAchievementCriteria(type, miscValue1, miscValue2, miscValue3, pUnit, pPlayer);
                }
            }
            else
            {
                // If that's first guild in list
                // then add to the list and update criteria
                guildList.push_back(guildId);
                if (Guild* pGuild = sGuildMgr->GetGuildById(guildId))
                    pGuild->UpdateAchievementCriteria(type, miscValue1, miscValue2, miscValue3, pUnit, pPlayer);
            }
        }
    }
}

uint32 Group::GetAverageMMR(uint8 bracket) const
{
    uint32 matchMakerRating = 0;
    uint32 playerDivider = 0;

    for (ObjectGuid const& memberGuid : GetMemberGuids())
    {
        ObjectAccessor::WithPlayer(memberGuid, [&matchMakerRating, &playerDivider, bracket](Player* member)
        {
            matchMakerRating += member->getBracket(bracket)->getMMV();
            ++playerDivider;
        });
    }

    // x/0 = crash
    if (playerDivider == 0)
        playerDivider = 1;

    matchMakerRating /= playerDivider;

    return matchMakerRating;
}

ItemQualities Group::GetThreshold() const
{
    return m_lootThreshold;
}

uint32 Group::GetTeam() const
{
    return _team;
}

void Group::AddRaidMarker(uint8 markerId, uint32 mapId, float positionX, float positionY, float positionZ, ObjectGuid transportGuid)
{
    if (markerId >= RAID_MARKERS_COUNT || m_markers[markerId])
        return;

    m_activeMarkers |= 1 << markerId;
    m_markers[markerId] = Trinity::make_unique<RaidMarker>(mapId, positionX, positionY, positionZ, transportGuid);
    SendRaidMarkersChanged();
}

void Group::DeleteRaidMarker(uint8 markerId)
{
    if (markerId > RAID_MARKERS_COUNT)
        return;

    for (uint8 i = 0; i < RAID_MARKERS_COUNT; i++)
        if (m_markers[i] && (markerId == i || markerId == RAID_MARKERS_COUNT))
        {
            m_markers[i] = nullptr;
            m_activeMarkers &= ~(1 << i);
        }

    SendRaidMarkersChanged();
}

void Group::SendRaidMarkersChanged(WorldSession* session, int8 partyIndex)
{
    WorldPackets::Party::RaidMarkersChanged packet;

    packet.PartyIndex = partyIndex;
    packet.ActiveMarkers = m_activeMarkers;

    for (uint8 i = 0; i < RAID_MARKERS_COUNT; i++)
        if (m_markers[i])
            packet.RaidMarkers.push_back(m_markers[i].get());

    if (session)
        session->SendPacket(packet.Write());
    else
        BroadcastPacket(packet.Write(), false);
}

bool Group::InChallenge()
{
    std::lock_guard<std::recursive_mutex> guard(m_lock);
    return m_dungeonDifficulty == DIFFICULTY_MYTHIC_KEYSTONE;
}

Group::ChallengeInfo Group::GetChallengeInfo() const
{
    std::lock_guard<std::recursive_mutex> guard(m_lock);
    ChallengeInfo info;
    info.Entry = m_challengeEntry;
    info.Owner = m_challengeOwner;
    info.Item = m_challengeItem;
    info.Level = m_challengeLevel;
    info.InstanceID = m_challengeInstanceID;
    info.Affixes = m_affixes;
    return info;
}

Group::ChallengeInfo Group::StartChallenge(uint32 instanceId)
{
    std::lock_guard<std::recursive_mutex> guard(m_lock);
    m_challengeInstanceID = instanceId;
    return GetChallengeInfo();
}

void Group::SetChallenge(MapChallengeModeEntry const* entry, ObjectGuid owner, ObjectGuid item, uint32 level, std::array<uint32, 3> const& affixes)
{
    std::lock_guard<std::recursive_mutex> guard(m_lock);
    m_challengeEntry = entry;
    m_challengeOwner = owner;
    m_challengeItem = item;
    m_challengeLevel = level;
    m_affixes = affixes;
}

void Group::ResetChallenge()
{
    {
        std::lock_guard<std::recursive_mutex> guard(m_lock);
        m_challengeEntry = nullptr;
        m_challengeLevel = 0;
        m_affixes.fill(0);
        m_challengeInstanceID = 0;
    }
    SetDungeonDifficultyID(DIFFICULTY_MYTHIC_DUNGEON);
}

bool Group::ResetChallengeIf(uint32 instanceId, ObjectGuid item)
{
    {
        std::lock_guard<std::recursive_mutex> guard(m_lock);
        if (m_challengeInstanceID != instanceId || m_challengeItem != item)
            return false;

        m_challengeEntry = nullptr;
        m_challengeLevel = 0;
        m_affixes.fill(0);
        m_challengeInstanceID = 0;
    }
    SetDungeonDifficultyID(DIFFICULTY_MYTHIC_DUNGEON);
    return true;
}

bool Group::GetMaxCountOfRolesForArenaQueue(uint8 role)
{
    uint8 count = 0;
   
    for (MemberRef const& ref : GetMemberRefs())
    {
        bool hasRole = false;
        ObjectAccessor::WithPlayer(ref.Guid, [&hasRole, role](Player* plr) { hasRole = plr->GetSpecializationRole() == role; },
            ObjectAccessor::PlayerScope::InOrOutOfWorld);
        if (hasRole && ++count > 1)
            return false;
    }
    return true;
}
