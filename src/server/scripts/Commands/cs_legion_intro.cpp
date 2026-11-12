/*
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

/* ScriptData
Name: legionintro_commandscript
%Complete: 100
Comment: .legionintro [$player]: skip the Legion introduction (Broken Shore up to "In the Blink of an Eye") and go to Dalaran
Category: commandscripts
EndScriptData */

/*
 * A test helper for the artifact / class hall quests. For a character of level 98 or more it:
 *  - marks his faction's introduction chain rewarded (no rewards, no quest scripts: those would start scenes, phases and
 *    teleports of their own), as the database chains it:
 *      Alliance: The Legion Returns 40519, To Be Prepared 42782, The Battle for Broken Shore 42740, The Fallen Lion 40517,
 *                Demons Among Us 40593, Illidari Allies 44120;
 *      Horde:    The Legion Returns 43926, To Be Prepared 44281, The Battle for Broken Shore 40518, Fate of the Horde 40522,
 *                Emissary 40760, Demons Among Us 40607, Keep Your Friends Close 40605;
 *    demon hunters also their own route out of the Vault (Alliance 44471, 44463; Horde 40976, 40982, 40983, 41002);
 *  - turns in "In the Blink of an Eye" (44663) for real (accepted if needed, completed, rewarded), so its reward and the
 *    OnQuestReward scripts run: the class hall messenger comes with the class's first artifact quest;
 *  - teleports him to Dalaran on the Broken Isles.
 * A character who already has 44663 or its other copy 44184 done is only teleported.
 */

#include "ScriptMgr.h"
#include "Chat.h"
#include "ObjectAccessor.h"
#include "Player.h"
#include "QuestData.h"
#include "QuestDef.h"
#include "SpellInfo.h"
#include "SpellMgr.h"
#include <vector>

namespace
{
    uint32 const QUEST_IN_THE_BLINK_OF_AN_EYE = 44663;
    uint32 const QUEST_IN_THE_BLINK_OF_AN_EYE_OTHER = 44184;
    uint8 const MIN_LEVEL = 98;

    std::vector<uint32> const AllianceChain = { 40519, 42782, 42740, 40517, 40593, 44120 };
    std::vector<uint32> const HordeChain = { 43926, 44281, 40518, 40522, 40760, 40607, 40605 };
    std::vector<uint32> const AllianceDemonHunter = { 44471, 44463 };
    std::vector<uint32> const HordeDemonHunter = { 40976, 40982, 40983, 41002 };

    // Dalaran (Broken Isles), the "NewDalaran" game_tele point
    uint32 const DALARAN_MAP = 1220;
    float const DALARAN_X = -864.9f;
    float const DALARAN_Y = 4404.84f;
    float const DALARAN_Z = 737.43f;
    float const DALARAN_O = 0.0f;

    // the reward spell's learned spells; its other effects (scenes, teleports) are left out
    void LearnRewardSpell(Player* player, int32 spellId)
    {
        SpellInfo const* spellInfo = spellId > 0 ? sSpellMgr->GetSpellInfo(uint32(spellId)) : nullptr;
        if (!spellInfo)
            return;
        for (SpellEffectInfo const* effect : spellInfo->Effects)
            if (effect && effect->Effect == SPELL_EFFECT_LEARN_SPELL && effect->TriggerSpell && !player->HasSpell(effect->TriggerSpell))
                player->learnSpell(effect->TriggerSpell, false);
    }

    uint32 MarkRewarded(Player* player, std::vector<uint32> const& questIds)
    {
        uint32 done = 0;
        for (uint32 questId : questIds)
        {
            Quest const* quest = sQuestDataStore->GetQuestTemplate(questId);
            if (!quest || player->IsQuestRewarded(questId))
                continue;
            if (player->FindQuestSlot(questId) < MAX_QUEST_LOG_SIZE)
                player->RemoveActiveQuest(questId);
            player->SetRewardedQuest(questId);
            LearnRewardSpell(player, quest->RewardSpell);
            for (uint32 displaySpell : quest->RewardDisplaySpell)
                LearnRewardSpell(player, int32(displaySpell));
            ++done;
        }
        return done;
    }

    // runs in the player's own thread; returns what was done, for the GM
    std::string SkipLegionIntro(Player* player)
    {
        bool const alliance = player->GetTeam() == ALLIANCE;
        uint32 marked = MarkRewarded(player, alliance ? AllianceChain : HordeChain);
        if (player->getClass() == CLASS_DEMON_HUNTER)
            marked += MarkRewarded(player, alliance ? AllianceDemonHunter : HordeDemonHunter);

        bool turnedIn = false;
        if (!player->IsQuestRewarded(QUEST_IN_THE_BLINK_OF_AN_EYE) && !player->IsQuestRewarded(QUEST_IN_THE_BLINK_OF_AN_EYE_OTHER))
        {
            if (Quest const* blink = sQuestDataStore->GetQuestTemplate(QUEST_IN_THE_BLINK_OF_AN_EYE))
            {
                if (player->GetQuestStatus(QUEST_IN_THE_BLINK_OF_AN_EYE) == QUEST_STATUS_NONE)
                    player->AddQuest(blink, nullptr);
                player->CompleteQuest(QUEST_IN_THE_BLINK_OF_AN_EYE);
                player->RewardQuest(blink, 0, player);
                turnedIn = player->IsQuestRewarded(QUEST_IN_THE_BLINK_OF_AN_EYE);
            }
        }

        // phases follow quest state
        player->UpdateAreaDependentAuras(player->GetAreaId());
        player->SaveToDB();
        player->TeleportTo(DALARAN_MAP, DALARAN_X, DALARAN_Y, DALARAN_Z, DALARAN_O);

        std::string result = std::to_string(marked) + " introduction quest(s) marked done, ";
        result += turnedIn ? "\"In the Blink of an Eye\" turned in" : "\"In the Blink of an Eye\" already done";
        return result + ", teleported to Dalaran.";
    }
}

class legionintro_commandscript : public CommandScript
{
public:
    legionintro_commandscript() : CommandScript("legionintro_commandscript") { }

    std::vector<ChatCommand> GetCommands() const override
    {
        static std::vector<ChatCommand> commandTable =
        {
            { "legionintro",    SEC_GAMEMASTER,     true,  &HandleLegionIntroCommand,          "" }
        };
        return commandTable;
    }

    // .legionintro [$player]
    static bool HandleLegionIntroCommand(ChatHandler* handler, char const* args)
    {
        Player* target;
        ObjectGuid targetGuid;
        std::string targetName;
        if (!handler->extractOnlinePlayerTarget((char*)args, &target, &targetGuid, &targetName))
            return false;

        uint8 level = 0;
        if (target)
            level = target->getLevel();
        else
            ObjectAccessor::WithPlayer(targetGuid, [&level](Player* player) { level = player->getLevel(); }, ObjectAccessor::PlayerScope::InOrOutOfWorld);

        if (level < MIN_LEVEL)
        {
            handler->PSendSysMessage("%s is level %u: the Legion introduction needs level %u or more.", targetName.c_str(), uint32(level), uint32(MIN_LEVEL));
            handler->SetSentErrorMessage(true);
            return false;
        }

        if (target)
        {
            handler->PSendSysMessage("%s: %s", targetName.c_str(), SkipLegionIntro(target).c_str());
            return true;
        }

        return handler->PostToTarget(targetGuid, [](Player* player)
        {
            ChatHandler(player->GetSession()).PSendSysMessage("Legion introduction skipped: %s", SkipLegionIntro(player).c_str());
        }, targetName.c_str());
    }
};

void AddSC_legionintro_commandscript()
{
    new legionintro_commandscript();
}
