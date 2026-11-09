/*
 * This file is part of the LegionCore Project.
 *
 * Artifact acquisition: the pieces of Brytenwally/LegionCore-7.3.5's src/server/scripts/Legion/ClassHalls/
 * artifact_acquisition.cpp ported so far (2026-11-09), unchanged apart from the cuts noted:
 *  - ArtifactAcquisition: the artifact chests (go_artifact_acquisition) advance their scenario only through the real
 *    completed-loot event; a player who already owns the weapon (a shop or bank copy) replays that event instead;
 *  - Dark Riders completion (warlock Ulthalesh, shared scenario 988 / map 1533 with the Death Knight's Apocalypse);
 *  - Blood (Maw of the Damned) and Frost (Blades of the Fallen Prince) scenes: the scene's completion equips the
 *    owned weapon through its native equip spell (spell_scene_event), refused outside the right scenario step.
 * The other classes' pieces come with their phases (docs/legioncore-735-port.md).
 */

#include "ScriptMgr.h"
#include "Player.h"
#include "Item.h"
#include "GameObject.h"
#include "Scenario.h"
#include "ScenarioMgr.h"
#include "SpellMgr.h"
#include "Log.h"

namespace ArtifactAcquisition
{
struct Pickup {uint32 object,item,playerClass,quest,alternate;};
Pickup const Pickups[]={
    {249812,128861,CLASS_HUNTER,42185,0}, // Never Hunt Alone, after the workshop
    {248834,128808,CLASS_HUNTER,40385,0},
    {248419,128826,CLASS_HUNTER,40419,0},
    {248572,128868,CLASS_PRIEST,41631,0},
    {246699,128403,CLASS_DEATH_KNIGHT,40934,40986},
    {249814,127857,CLASS_MAGE,42011,0},
    {251049,128862,CLASS_MAGE,42479,0},
    {246021,128942,CLASS_WARLOCK,40623,41155},
    {249821,128943,CLASS_WARLOCK,42125,0},
    {246437,128858,CLASS_DRUID,40838,0}
};
bool Active(Player* p,uint32 quest)
{
    return quest && !p->GetQuestRewardStatus(quest) &&
        (p->GetQuestStatus(quest)==QUEST_STATUS_INCOMPLETE || p->GetQuestStatus(quest)==QUEST_STATUS_COMPLETE);
}
bool Use(Player* p,GameObject* go)
{
    for (Pickup const& row:Pickups)
        if (go->GetEntry()==row.object)
        {
            if (!p->IsAlive() || p->IsBeingTeleported() || p->getClass()!=row.playerClass ||
                (!Active(p,row.quest) && !Active(p,row.alternate)) || !go->isSpawned() ||
                go->HasFlag(GAMEOBJECT_FIELD_FLAGS,GO_FLAG_IN_USE|GO_FLAG_NOT_SELECTABLE) ||
                p->GetMap()!=go->GetMap() || !p->InSamePhase(go) || !p->canSeeOrDetect(go) || !p->IsWithinLOSInMap(go) ||
                p->GetExactDist(go)>5.0f || go->getLootState()==GO_JUST_DEACTIVATED) return true;
            if (row.object == 249812)
            {
                Scenario* scenario = sScenarioMgr->GetScenario(p->GetInstanceId());
                if (go->GetMapId() != 1609 || !scenario || scenario->GetScenarioId() != 1099 ||
                    scenario->GetCurrentStep() < 4 || scenario->GetCurrentStep() > 5)
                    return true;
            }
            if (row.object == 248834)
            {
                Scenario* scenario = sScenarioMgr->GetScenario(p->GetInstanceId());
                if (go->GetMapId() != 1511 || !scenario || scenario->GetScenarioId() != 973 ||
                    scenario->GetCurrentStep() < 12 || scenario->GetCurrentStep() > 13)
                    return true;
            }
            if (row.object == 248419)
            {
                Scenario* scenario = sScenarioMgr->GetScenario(p->GetInstanceId());
                if (go->GetMapId() != 1489 || !scenario || scenario->GetScenarioId() != 972 ||
                    scenario->GetCurrentStep() < 7 || scenario->GetCurrentStep() > 8)
                    return true;
                return false;
            }
            if (row.object == 248572)
            {
                Scenario* scenario = sScenarioMgr->GetScenario(p->GetInstanceId());
                if (go->GetMapId() != 1583 || !scenario || scenario->GetScenarioId() != 1065 ||
                    scenario->GetCurrentStep() < 6 || scenario->GetCurrentStep() > 7)
                    return true;
            }
            if (!p->HasItemCount(row.item,1,true)) return false;
            // A shop/bank copy satisfies ownership. Replay the same native
            // loot-completed AI event, without trying to loot another unique
            // weapon. Ordinary players still use real loot, including bag errors.
            go->SetLootState(GO_JUST_DEACTIVATED,p);
            // StoreNewItem normally emits this event. A previously bought
            // unique artifact emits no new item event at its quest chest.
            p->UpdateAchievementCriteria(CRITERIA_TYPE_OWN_ITEM,row.item,1);
            TC_LOG_INFO("entities.player","Artifact acquisition: player %u used owned artifact %u at object %u",p->GetGUIDLow(),row.item,row.object);
            return true;
        }
    return false;
}
}
namespace DarkRidersCompletion
{
void Recover(Player* player)
{
    // Shop ownership alone is not a quest completion. The persisted claim
    // objective is awarded by the final chest's native 204819 spell, after
    // Ariden has been defeated. Also repairs a claim saved before this fix.
    if (!player || !player->IsInWorld() || player->IsBeingTeleported() ||
        player->getClass() != CLASS_WARLOCK || player->GetQuestRewardStatus(40623) ||
        player->GetQuestStatus(40623) != QUEST_STATUS_INCOMPLETE ||
        player->GetQuestObjectiveData(40623, 103473) < 1 ||
        player->GetQuestObjectiveData(40623, 100813) >= 1 ||
        !player->HasItemCount(128942, 1, true))
        return;

    player->KilledMonsterCredit(100813);
    TC_LOG_INFO("entities.player", "Dark Riders: restored scenario completion after claimed Ulthalesh for player %u", player->GetGUIDLow());
}
}
class player_dark_riders_completion : public PlayerScript
{
public:
    player_dark_riders_completion() : PlayerScript("player_dark_riders_completion") { }
    void OnLogin(Player* player) override { DarkRidersCompletion::Recover(player); }
    void OnUpdate(Player* player, uint32) override { DarkRidersCompletion::Recover(player); }
};
class go_artifact_acquisition : public GameObjectScript
{
public:
    go_artifact_acquisition():GameObjectScript("go_artifact_acquisition"){}
    bool OnGossipHello(Player* player,GameObject* go) override {return ArtifactAcquisition::Use(player,go);}
    // LegionCore-7.3.5 also retires the hunter world pickups here (its Talonclaw / Titanstrike world pickup
    // helpers): ported with the hunter artifacts
};

namespace FrostArtifactScene
{
struct Pickup
{
    uint32 scene, package, map, scenario, firstStep, quest, item;
};

Pickup const Pickups[] = {
    {1071, 1466, 1480, 901, 6, 38990, 128292}
};

// Scene-specific admission is independent of ordinary native equip-spell use.
// In particular, a late completion after map change must not fall through to
// the source17 branch that preserves those spells outside their quest maps.
bool Denied(Player* player, SpellScene const* scene, std::string const& trigger)
{
    if (!scene)
        return false;
    Pickup const* pickup = nullptr;
    for (Pickup const& row : Pickups)
        if (uint32(scene->MiscValue) == row.scene)
        {
            pickup = &row;
            break;
        }
    if (!pickup)
        return false;
    if (uint32(scene->SceneScriptPackageID) != pickup->package || trigger != "complete" || !player ||
        !player->HasSceneStatus(pickup->scene, SCENE_COMPLETE) || !player->IsInWorld() ||
        !player->CanContact() || !player->IsAlive() || player->IsBeingTeleported() ||
        player->getClass() != CLASS_DEATH_KNIGHT || player->GetMapId() != pickup->map ||
        !ArtifactAcquisition::Active(player, pickup->quest) || !player->HasItemCount(pickup->item, 1, false))
        return true;
    Scenario* scenario = sScenarioMgr->GetScenario(player->GetInstanceId());
    return !scenario || scenario->GetScenarioId() != pickup->scenario ||
        scenario->GetCurrentStep() < pickup->firstStep || scenario->GetCurrentStep() > pickup->firstStep + 1;
}
}

class scene_frost_artifact_equipment : public SceneTriggerScript
{
public:
    scene_frost_artifact_equipment() : SceneTriggerScript("scene_frost_artifact_equipment") { }
    bool OnTrigger(Player* player, SpellScene const* scene, std::string trigger) override
    {
        return FrostArtifactScene::Denied(player, scene, trigger);
    }
};

namespace BloodArtifactScene
{
struct Pickup
{
    uint32 scene, package, map, scenario, firstStep, quest, item;
};

Pickup const Pickups[] = {
    {1168, 1532, 1489, 940, 9, 40740, 128402}
};

// Scene-specific admission is independent of ordinary native equip-spell use.
// In particular, a late completion after map change must not fall through to
// the source17 branch that preserves those spells outside their quest maps.
bool Denied(Player* player, SpellScene const* scene, std::string const& trigger)
{
    if (!scene)
        return false;
    Pickup const* pickup = nullptr;
    for (Pickup const& row : Pickups)
        if (uint32(scene->MiscValue) == row.scene)
        {
            pickup = &row;
            break;
        }
    if (!pickup)
        return false;
    if (uint32(scene->SceneScriptPackageID) != pickup->package || trigger != "complete" || !player ||
        !player->HasSceneStatus(pickup->scene, SCENE_COMPLETE) || !player->IsInWorld() ||
        !player->CanContact() || !player->IsAlive() || player->IsBeingTeleported() ||
        player->getClass() != CLASS_DEATH_KNIGHT || player->GetMapId() != pickup->map ||
        !ArtifactAcquisition::Active(player, pickup->quest) || !player->HasItemCount(pickup->item, 1, false))
        return true;
    Scenario* scenario = sScenarioMgr->GetScenario(player->GetInstanceId());
    return !scenario || scenario->GetScenarioId() != pickup->scenario ||
        scenario->GetCurrentStep() < pickup->firstStep || scenario->GetCurrentStep() > pickup->firstStep + 1;
}
}

class scene_blood_artifact_equipment : public SceneTriggerScript
{
public:
    scene_blood_artifact_equipment() : SceneTriggerScript("scene_blood_artifact_equipment") { }
    bool OnTrigger(Player* player, SpellScene const* scene, std::string trigger) override
    {
        return BloodArtifactScene::Denied(player, scene, trigger);
    }
};


void AddSC_artifact_acquisition()
{
    new go_artifact_acquisition();
    new player_dark_riders_completion();
    new scene_frost_artifact_equipment();
    new scene_blood_artifact_equipment();
}
