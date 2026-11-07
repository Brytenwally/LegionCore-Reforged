/*
 * Class hall messengers after "In the Blink of an Eye".
 *
 * On retail, once Dalaran has moved to the Broken Isles (quest 44663, Horde copy 44184), each class's first artifact /
 * class hall quest is brought to the player: a messenger of his class appears beside him and offers it (the mage and
 * death knight quests come as a popup). The database has every messenger with its quest and a SmartAI made for being
 * summoned (follow the summoner, visible to him only, despawn after a while or once the quest is taken), but nothing
 * summoned them: none of them is spawned, so players never got their artifact quests.
 *
 * Here, a few seconds to two minutes after the quest is turned in, or when a character who has it done is on the
 * Broken Isles without having started his class's chain (login, arrival), the messenger is summoned behind him for
 * him alone. If he leaves it, it comes again a few minutes later while he is still on the Broken Isles.
 */

#include "ScriptMgr.h"
#include "GossipDef.h"
#include "ObjectAccessor.h"
#include "Player.h"
#include "QuestData.h"
#include "QuestDef.h"
#include "TemporarySummon.h"
#include "WorldSession.h"
#include <mutex>
#include <unordered_set>

namespace
{
    uint32 const QUEST_IN_THE_BLINK_OF_AN_EYE = 44663;
    uint32 const QUEST_IN_THE_BLINK_OF_AN_EYE_HORDE = 44184;
    uint32 const MAP_BROKEN_ISLES = 1220;

    uint32 const FIRST_DELAY_MIN = 5 * IN_MILLISECONDS;
    uint32 const FIRST_DELAY_MAX = 2 * MINUTE * IN_MILLISECONDS;
    uint32 const RETRY_DELAY = 3 * MINUTE * IN_MILLISECONDS;          // the messenger despawns before this
    uint32 const POPUP_RETRY_DELAY = 5 * MINUTE * IN_MILLISECONDS;
    uint32 const BUSY_RETRY_DELAY = 15 * IN_MILLISECONDS;             // in combat, on a flight...
    uint32 const MESSENGER_DESPAWN = 2 * MINUTE * IN_MILLISECONDS;    // most messengers' own SmartAI despawns sooner

    struct Messenger
    {
        uint32 quest;                                                  // the class's first quest
        uint32 creature;                                               // its giver, summoned; 0 = offered as a popup
    };

    Messenger MessengerFor(Player const* player)
    {
        switch (player->getClass())
        {
            case CLASS_WARRIOR:
                return player->GetTeam() == ALLIANCE ? Messenger{ 42814, 108961 }   // An Important Mission
                                                     : Messenger{ 41052, 93775 };   // A Desperate Plea
            case CLASS_PALADIN:      return { 38710, 92909 };    // An Urgent Gathering, Lord Maxwell Tyrosus
            case CLASS_HUNTER:       return { 40384, 100786 };   // Needs of the Hunters
            case CLASS_ROGUE:        return { 40832, 102018 };   // Call of the Uncrowned, Ravenholdt courier
            case CLASS_PRIEST:       return { 40705, 101344 };   // Priestly Matters
            case CLASS_DEATH_KNIGHT: return { 40714, 0 };        // The Call to War (popup)
            case CLASS_SHAMAN:       return { 39746, 91731 };    // A Ring Unbroken
            case CLASS_MAGE:         return { 41035, 0 };        // Felstorm's Plea (popup)
            case CLASS_WARLOCK:      return { 40716, 103506 };   // The Sixth
            case CLASS_MONK:         return { 12103, 98519 };    // Before the Storm
            case CLASS_DRUID:        return { 40643, 101061 };   // A Summons From Moonglade
            case CLASS_DEMON_HUNTER: return { 39047, 99343 };    // Call of the Illidari
            default:                 return { 0, 0 };
        }
    }

    // players with a check pending (one chain of delayed checks per player)
    std::mutex scheduledLock;
    std::unordered_set<ObjectGuid> scheduled;

    // the messenger has something to bring: Dalaran has moved and the class's chain has not started
    bool NeedsMessenger(Player* player, Quest const*& quest, Messenger& messenger)
    {
        if (!player->GetSession() || player->GetSession()->IsBot())
            return false;
        if (!player->GetQuestRewardStatus(QUEST_IN_THE_BLINK_OF_AN_EYE) &&
            !player->GetQuestRewardStatus(QUEST_IN_THE_BLINK_OF_AN_EYE_HORDE))
            return false;
        messenger = MessengerFor(player);
        if (!messenger.quest)
            return false;
        quest = sQuestDataStore->GetQuestTemplate(messenger.quest);
        if (!quest)
            return false;
        if (player->GetQuestStatus(messenger.quest) != QUEST_STATUS_NONE || player->GetQuestRewardStatus(messenger.quest))
            return false;
        return player->CanTakeQuest(quest, false);
    }

    void Schedule(Player* player, uint32 delay);

    // brings the class quest, or tries again later; ends the chain when nothing is left to bring or he left the isles
    void Check(Player* player)
    {
        {
            std::lock_guard<std::mutex> guard(scheduledLock);
            scheduled.erase(player->GetGUID());
        }

        Quest const* quest = nullptr;
        Messenger messenger;
        if (!player->IsInWorld() || !NeedsMessenger(player, quest, messenger) || player->GetMapId() != MAP_BROKEN_ISLES)
            return;

        if (!player->IsAlive() || player->isInCombat() || player->isInFlight() || player->GetVehicle() ||
            player->IsBeingTeleported())
        {
            Schedule(player, BUSY_RETRY_DELAY);
            return;
        }

        if (!messenger.creature)
        {
            // a quest of no giver (retail: a popup): offered by the player himself, which the accept handler allows
            // for his popup quest only
            player->SetPopupQuestId(quest->GetQuestId());
            PlayerMenu menu(player->GetSession());
            menu.SendQuestGiverQuestDetails(quest, player->GetGUID(), true, false);
            Schedule(player, POPUP_RETRY_DELAY);
            return;
        }

        // behind him, facing him, visible to him alone (its SmartAI then follows him and speaks)
        float x, y, z;
        player->GetClosePoint(x, y, z, player->GetObjectSize(), 3.0f, float(M_PI));
        Position pos;
        pos.Relocate(x, y, z, player->GetAngle(x, y) + float(M_PI));
        if (TempSummon* summon = player->SummonCreature(messenger.creature, pos, TEMPSUMMON_TIMED_DESPAWN,
            MESSENGER_DESPAWN, 0, player->GetGUID()))
        {
            summon->AddPlayerInPersonnalVisibilityList(player->GetGUID());
            summon->SetFacingToObject(player);
            if (summon->GetAIName() != "SmartAI")                     // the others follow by their SmartAI
                summon->GetMotionMaster()->MoveFollow(player, 2.0f, float(M_PI) / 2);
        }
        Schedule(player, RETRY_DELAY);
    }

    void Schedule(Player* player, uint32 delay)
    {
        {
            std::lock_guard<std::mutex> guard(scheduledLock);
            if (!scheduled.insert(player->GetGUID()).second)
                return;                                                // a check is already on its way
        }
        // the event belongs to the player: it goes with him at logout
        player->AddDelayedEvent(delay, [player]() { Check(player); });
    }

    void ScheduleIfNeeded(Player* player, uint32 delay)
    {
        Quest const* quest = nullptr;
        Messenger messenger;
        if (player->GetMapId() == MAP_BROKEN_ISLES && NeedsMessenger(player, quest, messenger))
            Schedule(player, delay);
    }
}

class player_legion_class_hall_messengers : public PlayerScript
{
public:
    player_legion_class_hall_messengers() : PlayerScript("player_legion_class_hall_messengers") { }

    void OnQuestReward(Player* player, Quest const* quest) override
    {
        if (quest->GetQuestId() == QUEST_IN_THE_BLINK_OF_AN_EYE || quest->GetQuestId() == QUEST_IN_THE_BLINK_OF_AN_EYE_HORDE)
            Schedule(player, urand(FIRST_DELAY_MIN, FIRST_DELAY_MAX));
    }

    // characters who turned the quest in before this fix, or who left the isles before the messenger came
    void OnLogin(Player* player) override
    {
        ScheduleIfNeeded(player, urand(FIRST_DELAY_MIN, FIRST_DELAY_MAX));
    }

    void OnMapChanged(Player* player) override
    {
        ScheduleIfNeeded(player, urand(FIRST_DELAY_MIN, FIRST_DELAY_MAX));
    }

    void OnLogout(Player* player) override
    {
        std::lock_guard<std::mutex> guard(scheduledLock);
        scheduled.erase(player->GetGUID());
    }
};

void AddSC_legion_class_hall_messengers()
{
    new player_legion_class_hall_messengers();
}
