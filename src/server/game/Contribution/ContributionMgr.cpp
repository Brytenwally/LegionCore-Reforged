/*
    Created by d7561985@gmail.com
*/
#include "ContributionMgr.h"
#include "ConditionMgr.h"
#include "DatabaseEnv.h"
#include "DB2Stores.h"
#include "GameEventMgr.h"
#include "GameTime.h"
#include "Log.h"
#include "MiscPackets.h"
#include "Player.h"
#include "QuestData.h"
#include "World.h"
#include "WorldStateMgr.h"

using namespace ContributionData;

namespace
{
    // characters.worldstates rows keeping the time of the last stage change, one per building
    uint32 const LastChangeWorldStateBase = 20120;

    // 1 while Legionfall is open: the client shows the construction on the Broken Isles map only then
    uint32 const LegionfallOpenWorldState = 13317;

    // The Broken Shore rares of the map (AreaPOI 5284 to 5308): the client shows one while its world state is 1 and
    // its tracking quest is not done. Three of the 25 have no spawn in the database. A pool keeps a few of them up at a
    // time; the icon follows the ones the pool has out.
    uint32 const BrokenIslesMapId = 1220;
    uint32 const RareLifetime = 30 * MINUTE;       // a rare left alive leaves after this, as on retail
    struct BrokenShoreRare
    {
        uint32 WorldState;
        uint32 Entry;
    };
    BrokenShoreRare const BrokenShoreRares[] =
    {
        { 12867, 117141 },  // Malgrazoth
        { 12868, 117140 },  // Salethan the Broodwalker
        { 12869, 117094 },  // Malorus the Soulkeeper
        { 12870, 117086 },  // Emberfire
        { 12871, 117096 },  // Potionmaster Gloop
        { 12872, 117091 },  // Felmaw Emberfiend
        { 12873, 117089 },  // Inquisitor Chillbane
        { 12875, 117095 },  // Dreadblade Annihilator
        { 12877, 117090 },  // Xorogun the Flamecarver
        { 12878, 116953 },  // Corrupted Bonebreaker
        { 12879, 117103 },  // Felcaller Zelthae
        { 12880, 118993 },  // Dreadeye
        { 12882, 119718 },  // Imp Mother Bruva
        { 12883, 120998 },  // Flllurlokkr
        { 12884, 121016 },  // Aqueux
        { 12885, 121029 },  // Brood Mother Nix
        { 12886, 121037 },  // Grossir
        { 12887, 121046 },  // Brother Badatin
        { 12888, 121107 },  // Lady Eldrathe
        { 12889, 121112 },  // Somber Dawn
        { 12890, 121134 },  // Duke Sithizi
        { 12891, 116166 },  // Eye of Gurgh
    };

    // The Sentinax of the map (AreaPOI 5252 to 5261) reads its place from this world state, 1 to 9; the game events
    // 117 to 125 move the ship from place to place
    uint32 const SentinaxPlaceWorldState = 13321;
    std::pair<uint16, uint32> const SentinaxPlaces[] =
    {
        { 117, 1 }, { 121, 2 }, { 125, 3 }, { 124, 4 }, { 119, 5 }, { 123, 6 }, { 122, 7 }, { 120, 8 }, { 118, 9 }
    };

    uint32 GetPersonalTracker(uint32 contributionID)
    {
        switch (contributionID)
        {
            case CONTRIBUTION_MAGE_TOWER:       return CURRENCY_TYPE_LEGIONFALL_PERSONAL_TRACKER_MAGE_TOWER;
            case CONTRIBUTION_COMMAND_CENTER:   return CURRENCY_TYPE_LEGIONFALL_PERSONAL_TRACKER_COMMAND_TOWER;
            case CONTRIBUTION_NETHER_DISRUPTOR: return CURRENCY_TYPE_LEGIONFALL_PERSONAL_TRACKER_NETHER_TOWER;
            default:                            return 0;
        }
    }
}

ContributionMgr& ContributionMgr::Instance()
{
    static ContributionMgr instance;
    return instance;
}

bool ContributionMgr::IsOpen() const
{
    return true;                                            // the full 7.3.5 game: the Broken Shore is always open
}

bool ContributionMgr::IsAlwaysBuilt(ContributionLifeData const& data) const
{
    return data.Contribution->ID == CONTRIBUTION_MAGE_TOWER && sWorld->getBoolConfig(CONFIG_LEGIONFALL_MAGE_TOWER_ALWAYS_BUILT);
}

uint32 ContributionMgr::GetValue(int32 worldStateID) const
{
    return sWorldStateMgr.GetWorldStateValue(uint32(worldStateID));
}

void ContributionMgr::SetValue(int32 worldStateID, uint32 value)
{
    if (GetValue(worldStateID) != value)
        sWorldStateMgr.SetWorldState(uint32(worldStateID), 0, value);
}

void ContributionMgr::Initialize()
{
    std::lock_guard<std::recursive_mutex> guard(_lock);
    _contributions.clear();

    for (ContributionEntry const* contribution : sContributionStore)
    {
        ManagedWorldStateInputEntry const* input = sManagedWorldStateInputStore.LookupEntry(contribution->ManagedWorldStateInputID);
        ManagedWorldStateEntry const* worldState = input ? sManagedWorldStateStore.LookupEntry(input->ManagedWorldStateID) : nullptr;
        if (!worldState)
            continue;

        ContributionLifeData& data = _contributions[contribution->ID];
        data.Contribution = contribution;
        data.Input = input;
        data.WorldState = worldState;

        uint32 const flags = 1 << WorldStatesData::Flags::InitialState;
        sWorldStateMgr.AddTemplate(worldState->CurrentStageWorldStateID, WorldStatesData::Types::World, 0, flags, CONTRIBUTION_STATE_BUILDING);
        sWorldStateMgr.AddTemplate(worldState->ProgressWorldStateID, WorldStatesData::Types::World, 0, flags, 0);
        sWorldStateMgr.AddTemplate(worldState->OccurrencesWorldStateID, WorldStatesData::Types::World, 0, flags, 0);
    }

    sWorldStateMgr.AddTemplate(LegionfallOpenWorldState, WorldStatesData::Types::World, 0, 1 << WorldStatesData::Flags::InitialState, 0);
    sWorldStateMgr.SetWorldState(LegionfallOpenWorldState, 0, IsOpen() ? 1 : 0);

    // a rare is up while out of its pool and not waiting for its respawn; respawns shorter than 15 minutes are not
    // saved, so they read as over. The pools spawn after this, and tell which rares they bring out.
    _rares.clear();
    std::string entries;
    for (BrokenShoreRare const& rare : BrokenShoreRares)
    {
        sWorldStateMgr.AddTemplate(rare.WorldState, WorldStatesData::Types::World, 0, 1 << WorldStatesData::Flags::InitialState, 0);
        _rares[rare.Entry] = RareState();
        entries += (entries.empty() ? "" : ",") + std::to_string(rare.Entry);
    }
    std::map<uint64, uint32> guidToEntry;
    if (QueryResult result = WorldDatabase.PQuery("SELECT guid, id FROM creature WHERE map = %u AND id IN (%s)", BrokenIslesMapId, entries.c_str()))
    {
        do
        {
            Field* fields = result->Fetch();
            guidToEntry[fields[0].GetUInt64()] = fields[1].GetUInt32();
        }
        while (result->NextRow());
    }
    if (QueryResult result = CharacterDatabase.PQuery("SELECT guid, respawnTime FROM creature_respawn WHERE mapId = %u", BrokenIslesMapId))
    {
        do
        {
            Field* fields = result->Fetch();
            auto itr = guidToEntry.find(fields[0].GetUInt64());
            if (itr != guidToEntry.end())
                _rares[itr->second].RespawnTime = fields[1].GetUInt32();
        }
        while (result->NextRow());
    }
    if (QueryResult result = WorldDatabase.Query("SELECT guid FROM pool_creature"))
    {
        do
        {
            auto itr = guidToEntry.find(result->Fetch()[0].GetUInt64());
            if (itr != guidToEntry.end())
                _rares[itr->second].Out = false;
        }
        while (result->NextRow());
    }

    // the world states were loaded before these templates existed, so their saved values were skipped: read them back
    std::map<uint32, uint32> saved;
    if (QueryResult result = CharacterDatabase.Query("SELECT VariableID, Value FROM worldstate_data"))
    {
        do
        {
            Field* fields = result->Fetch();
            saved[fields[0].GetUInt32()] = fields[1].GetUInt32();
        }
        while (result->NextRow());
    }
    if (QueryResult result = CharacterDatabase.PQuery("SELECT entry, value FROM worldstates WHERE entry BETWEEN %u AND %u", LastChangeWorldStateBase, LastChangeWorldStateBase + 255))
    {
        do
        {
            Field* fields = result->Fetch();
            auto itr = _contributions.find(fields[0].GetUInt32() - LastChangeWorldStateBase);
            if (itr != _contributions.end())
                itr->second.LastChange = fields[1].GetUInt32();
        }
        while (result->NextRow());
    }

    uint32 const now = uint32(GameTime::GetGameTime());
    for (auto& itr : _contributions)
    {
        ContributionLifeData& data = itr.second;
        ManagedWorldStateEntry const* worldState = data.WorldState;
        for (int32 id : { worldState->CurrentStageWorldStateID, worldState->ProgressWorldStateID, worldState->OccurrencesWorldStateID })
        {
            auto value = saved.find(uint32(id));
            sWorldStateMgr.SetWorldState(uint32(id), 0, value != saved.end() ? value->second : 0);
        }

        uint32 const stage = GetValue(worldState->CurrentStageWorldStateID);
        if (stage < CONTRIBUTION_STATE_BUILDING || stage > CONTRIBUTION_STATE_DESTROYED || !data.LastChange)
            ChangeState(data, CONTRIBUTION_STATE_BUILDING, now);
        if (IsOpen() && IsAlwaysBuilt(data) && GetValue(worldState->CurrentStageWorldStateID) != CONTRIBUTION_STATE_ACTIVE)
            ChangeState(data, CONTRIBUTION_STATE_ACTIVE, now);

        TC_LOG_INFO("server.loading", ">> Legionfall building %u: stage %u, progress %u, built %u times", itr.first,
            GetValue(worldState->CurrentStageWorldStateID), GetValue(worldState->ProgressWorldStateID), GetValue(worldState->OccurrencesWorldStateID));
    }
}

void ContributionMgr::ChangeState(ContributionLifeData& data, ContributionState state, uint32 now)
{
    ManagedWorldStateEntry const* worldState = data.WorldState;
    switch (state)
    {
        case CONTRIBUTION_STATE_ACTIVE:
            SetValue(worldState->ProgressWorldStateID, uint32(worldState->AccumulationStateTargetValue));
            if (GetValue(worldState->CurrentStageWorldStateID) != CONTRIBUTION_STATE_ACTIVE)
                SetValue(worldState->OccurrencesWorldStateID, GetValue(worldState->OccurrencesWorldStateID) + 1);
            break;
        case CONTRIBUTION_STATE_UNDERATTACK:
            SetValue(worldState->ProgressWorldStateID, uint32(worldState->DepletionStateTargetValue));
            break;
        default:
            SetValue(worldState->ProgressWorldStateID, 0);
            break;
    }
    SetValue(worldState->CurrentStageWorldStateID, state);

    data.LastChange = now;
    CharacterDatabase.PExecute("REPLACE INTO worldstates (entry, value) VALUES (%u, %u)", LastChangeWorldStateBase + data.Contribution->ID, now);

    TC_LOG_INFO("misc", "Legionfall building %u enters stage %u", data.Contribution->ID, uint32(state));
}

void ContributionMgr::Update(uint32 diff)
{
    _updateTimer += diff;
    if (_updateTimer < 10 * IN_MILLISECONDS)
        return;
    _updateTimer = 0;

    // Legionfall does not exist before tier 7.2: the buildings keep their stage until it opens
    if (!IsOpen())
        return;

    std::lock_guard<std::recursive_mutex> guard(_lock);
    uint32 const now = uint32(GameTime::GetGameTime());
    for (BrokenShoreRare const& rare : BrokenShoreRares)
    {
        RareState const& state = _rares[rare.Entry];
        SetValue(rare.WorldState, state.Out && state.RespawnTime <= now ? 1 : 0);
    }

    uint32 sentinaxPlace = 0;
    for (auto const& place : SentinaxPlaces)
        if (sGameEventMgr->IsActiveEvent(place.first))
            sentinaxPlace = place.second;
    SetValue(SentinaxPlaceWorldState, sentinaxPlace);

    for (auto& itr : _contributions)
    {
        ContributionLifeData& data = itr.second;
        ManagedWorldStateEntry const* worldState = data.WorldState;
        switch (GetValue(worldState->CurrentStageWorldStateID))
        {
            case CONTRIBUTION_STATE_ACTIVE:
                if (!IsAlwaysBuilt(data) && now >= data.LastChange + uint32(worldState->UpTimeSecs))
                    ChangeState(data, CONTRIBUTION_STATE_UNDERATTACK, now);
                break;
            case CONTRIBUTION_STATE_UNDERATTACK:
            {
                if (IsAlwaysBuilt(data))
                {
                    ChangeState(data, CONTRIBUTION_STATE_ACTIVE, now);
                    break;
                }

                uint32 const elapsed = now > data.LastChange ? now - data.LastChange : 0;
                uint32 const lost = elapsed / MINUTE * uint32(worldState->DepletionAmountPerMinute);
                uint32 const left = lost < uint32(worldState->DepletionStateTargetValue) ? uint32(worldState->DepletionStateTargetValue) - lost : 0;
                if (left)
                    SetValue(worldState->ProgressWorldStateID, left);
                else
                    ChangeState(data, CONTRIBUTION_STATE_DESTROYED, now);
                break;
            }
            case CONTRIBUTION_STATE_DESTROYED:
                if (now >= data.LastChange + uint32(worldState->DownTimeSecs))
                    ChangeState(data, CONTRIBUTION_STATE_BUILDING, now);
                break;
            default:    // building: contributions move it
                break;
        }
    }
}

void ContributionMgr::OnCreatureDeath(uint32 entry, uint32 respawnTime)
{
    // called from the map threads: the world state follows on the next update of the world thread
    std::lock_guard<std::recursive_mutex> guard(_lock);
    auto itr = _rares.find(entry);
    if (itr != _rares.end())
        itr->second.RespawnTime = respawnTime;
}

void ContributionMgr::OnPoolSpawn(uint32 entry, bool out)
{
    std::lock_guard<std::recursive_mutex> guard(_lock);
    auto itr = _rares.find(entry);
    if (itr == _rares.end())
        return;

    itr->second.Out = out;
    if (out)
        itr->second.OutSince = uint32(GameTime::GetGameTime());
}

uint32 ContributionMgr::GetRareTimeLeft(uint32 entry) const
{
    std::lock_guard<std::recursive_mutex> guard(_lock);
    auto itr = _rares.find(entry);
    if (itr == _rares.end() || !itr->second.Out || !itr->second.OutSince)
        return 0;

    // counted from its coming out, so that a reload of its grid does not start it again
    uint32 const now = uint32(GameTime::GetGameTime());
    uint32 const end = itr->second.OutSince + RareLifetime;
    return end > now ? (end - now) * IN_MILLISECONDS : IN_MILLISECONDS;
}

void ContributionMgr::SendResult(Player* player, uint32 contributionID, ContributionResult result) const
{
    WorldPackets::Misc::ContributionResponse response;
    response.Data = result;
    response.ContributionID = contributionID;
    player->SendDirectMessage(response.Write());
}

void ContributionMgr::Contribute(Player* player, uint32 orderIndex)
{
    std::lock_guard<std::recursive_mutex> guard(_lock);

    // the packet names the order index of the table; the contribution ID is accepted when no order matches
    ContributionLifeData* data = nullptr;
    for (auto& itr : _contributions)
        if (uint32(itr.second.Contribution->OrderIndex) == orderIndex)
            data = &itr.second;
    if (!data)
    {
        auto itr = _contributions.find(orderIndex);
        data = itr != _contributions.end() ? &itr->second : nullptr;
    }

    if (!data || !IsOpen())
    {
        SendResult(player, orderIndex, CONTRIBUTUIN_RESULT_INVALID_ID);
        return;
    }

    uint32 const contributionID = uint32(data->Contribution->ID);
    TC_LOG_INFO("server", "Legionfall: %s contributes, client sent %u, building %u", player->GetName(), orderIndex, contributionID);
    ManagedWorldStateEntry const* worldState = data->WorldState;
    if (GetValue(worldState->CurrentStageWorldStateID) != CONTRIBUTION_STATE_BUILDING)
    {
        SendResult(player, contributionID, CONTRIBUTUIN_RESULT_INCORRECT_STATE);
        return;
    }

    if (data->Input->ValidInputConditionID && !sConditionMgr->IsPlayerMeetingCondition(player, data->Input->ValidInputConditionID))
    {
        SendResult(player, contributionID, CONTRIBUTUIN_RESULT_FAILED_CONDITION_CHECK);
        return;
    }

    uint32 const questID = uint32(data->Input->QuestID);
    Quest const* quest = sQuestDataStore->GetQuestTemplate(questID);
    if (!quest)
    {
        SendResult(player, contributionID, CONTRIBUTUIN_RESULT_QUEST_DATA_MISSING);
        return;
    }

    // the quest asks for the War Supplies and takes them when rewarded, with the reputation and its item
    for (QuestObjective const& objective : quest->GetObjectives())
    {
        if (objective.Type == QUEST_OBJECTIVE_CURRENCY && !player->HasCurrency(objective.ObjectID, objective.Amount))
        {
            SendResult(player, contributionID, CONTRIBUTUIN_RESULT_FAILED_CONDITION_CHECK);
            return;
        }
    }

    if (player->GetQuestStatus(questID) == QUEST_STATUS_NONE || player->GetQuestStatus(questID) == QUEST_STATUS_REWARDED)
    {
        if (!player->CanTakeQuest(quest, false) || !player->CanAddQuest(quest, false))
        {
            SendResult(player, contributionID, CONTRIBUTUIN_RESULT_UNABLE_TO_COMPLETE_TURN_IN);
            return;
        }
        player->AddQuest(quest, nullptr);
    }

    if (player->CanCompleteQuest(questID))
        player->CompleteQuest(questID);

    if (player->GetQuestStatus(questID) != QUEST_STATUS_COMPLETE || !player->CanRewardQuest(quest, 0, false))
    {
        uint16 const slot = player->FindQuestSlot(questID);
        if (slot < MAX_QUEST_LOG_SIZE)
            player->SetQuestSlot(slot, 0);
        player->RemoveActiveQuest(questID);
        SendResult(player, contributionID, CONTRIBUTUIN_RESULT_UNABLE_TO_COMPLETE_TURN_IN);
        return;
    }

    player->RewardQuest(quest, 0, player, true);
    if (uint32 tracker = GetPersonalTracker(contributionID))
        player->ModifyCurrency(tracker, 1, false, true);

    uint32 const target = uint32(worldState->AccumulationStateTargetValue);
    uint32 const needed = std::max<uint32>(1, sWorld->getIntConfig(CONFIG_LEGIONFALL_CONTRIBUTIONS_REQUIRED));
    uint32 const progress = uint32(std::min<uint64>(target, uint64(GetValue(worldState->ProgressWorldStateID)) + (target + needed - 1) / needed));
    if (progress >= target)
        ChangeState(*data, CONTRIBUTION_STATE_ACTIVE, uint32(GameTime::GetGameTime()));
    else
        SetValue(worldState->ProgressWorldStateID, progress);

    SendResult(player, contributionID, CONTRIBUTUIN_RESULT_SUCCESS);
}

void ContributionMgr::SendLastChange(Player* player, uint32 contributionID, uint32 requestGuid)
{
    std::lock_guard<std::recursive_mutex> guard(_lock);
    auto itr = _contributions.find(contributionID);

    WorldPackets::Misc::ContributionResponse response;
    response.Data = itr != _contributions.end() ? itr->second.LastChange : 0;
    response.ContributionID = contributionID;
    response.ContributionGUID = requestGuid;
    player->SendDirectMessage(response.Write());
}

ContributionState ContributionMgr::GetState(uint32 contributionID) const
{
    std::lock_guard<std::recursive_mutex> guard(_lock);
    auto itr = _contributions.find(contributionID);
    return itr != _contributions.end() ? ContributionState(GetValue(itr->second.WorldState->CurrentStageWorldStateID)) : CONTRIBUTION_STATE_NONE;
}

uint32 ContributionMgr::GetProgress(uint32 contributionID) const
{
    std::lock_guard<std::recursive_mutex> guard(_lock);
    auto itr = _contributions.find(contributionID);
    return itr != _contributions.end() ? GetValue(itr->second.WorldState->ProgressWorldStateID) : 0;
}

uint32 ContributionMgr::GetOccurrences(uint32 contributionID) const
{
    std::lock_guard<std::recursive_mutex> guard(_lock);
    auto itr = _contributions.find(contributionID);
    return itr != _contributions.end() ? GetValue(itr->second.WorldState->OccurrencesWorldStateID) : 0;
}

uint32 ContributionMgr::GetNextChange(uint32 contributionID) const
{
    std::lock_guard<std::recursive_mutex> guard(_lock);
    auto itr = _contributions.find(contributionID);
    if (itr == _contributions.end())
        return 0;

    ContributionLifeData const& data = itr->second;
    ManagedWorldStateEntry const* worldState = data.WorldState;
    switch (GetValue(worldState->CurrentStageWorldStateID))
    {
        case CONTRIBUTION_STATE_ACTIVE:
            return IsAlwaysBuilt(data) ? 0 : data.LastChange + uint32(worldState->UpTimeSecs);
        case CONTRIBUTION_STATE_UNDERATTACK:
            return worldState->DepletionAmountPerMinute ? data.LastChange + uint32(worldState->DepletionStateTargetValue / worldState->DepletionAmountPerMinute) * MINUTE : 0;
        case CONTRIBUTION_STATE_DESTROYED:
            return data.LastChange + uint32(worldState->DownTimeSecs);
        default:
            return 0;
    }
}

bool ContributionMgr::SetState(uint32 contributionID, ContributionState state)
{
    std::lock_guard<std::recursive_mutex> guard(_lock);
    auto itr = _contributions.find(contributionID);
    if (itr == _contributions.end() || state < CONTRIBUTION_STATE_BUILDING || state > CONTRIBUTION_STATE_DESTROYED)
        return false;

    ChangeState(itr->second, state, uint32(GameTime::GetGameTime()));
    return true;
}

bool ContributionMgr::SetProgress(uint32 contributionID, uint32 percent)
{
    std::lock_guard<std::recursive_mutex> guard(_lock);
    auto itr = _contributions.find(contributionID);
    if (itr == _contributions.end() || percent > 100 || GetValue(itr->second.WorldState->CurrentStageWorldStateID) != CONTRIBUTION_STATE_BUILDING)
        return false;

    ManagedWorldStateEntry const* worldState = itr->second.WorldState;
    if (percent == 100)
        ChangeState(itr->second, CONTRIBUTION_STATE_ACTIVE, uint32(GameTime::GetGameTime()));
    else
        SetValue(worldState->ProgressWorldStateID, uint32(uint64(worldState->AccumulationStateTargetValue) * percent / 100));
    return true;
}
