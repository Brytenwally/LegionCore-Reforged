////////////////////////////////////////////
///                                      ///
///              DEMONCORE               ///
///    Copyright 2026 Titans-Project     ///
///         All Rights Reserved.         ///
///                                      ///
////////////////////////////////////////////

#ifndef DEFENSE_OF_KARABOR_H
#define DEFENSE_OF_KARABOR_H

#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "ScriptedGossip.h"
#include "InstanceScript.h"
#include "SpellScript.h"
#include "Player.h"
#include "Scenario.h"
#include "ScenarioMgr.h"
#include "TemporarySummon.h"
#include "LFGMgr.h"
#include "DatabaseEnv.h"

namespace DefenseOfKarabor
{
    enum Data
    {
        MAP_DEFENSE_OF_KARABOR      = 1277,
        SCENARIO_DEFENSE            = 449,
        LFG_DUNGEON_DEFENSE         = 818,
        QUEST_DEFENSE_OF_KARABOR    = 33256,
        NPC_SCENARIO_CREDIT         = 73772,

        NPC_AKAMA                   = 72413,
        NPC_YREL                    = 73395,
        NPC_KARA                    = 79664,
        NPC_VORKA                   = 74715,
        NPC_ARNOKK                  = 75358,
        NPC_IRON_GRUNT              = 79731,
        NPC_IRON_SHIELDBEARER       = 79732,
        NPC_IRON_BRUTE              = 79719,
        NPC_IRON_BOLTBLASTER        = 79733,

        SPELL_LAUNCH_DEFENSE        = 148425,
        SPELL_CELEBRATION_SCENE     = 173694,
        SPELL_KARAS_BENEVOLENCE     = 149699,

        EVENT_MEET_YREL             = 40077,
        EVENT_KARA_EMPOWERED        = 40209,
        EVENT_VORKA_DEFEATED        = 40049,

        STEP_ARNOKK                 = 0,
        STEP_HARBOR                 = 1,
        STEP_KARA                   = 2,
        STEP_VORKA                  = 3,

        TALK_AKAMA_LAST_FORCES      = 0,
        TALK_AKAMA_TO_THE_HARBOR    = 1,
        TALK_YREL_DEFEND_KARA       = 3,
        TALK_YREL_KARA_EMPOWERED    = 4,
        TALK_YREL_THANKS            = 6,
        TALK_YREL_LIGHT_PREVAILS    = 7,
        TALK_VORKA_LANDS            = 3,
        TALK_VORKA_WAVE             = 4,

        WAVE_COUNT                  = 6,
        WAVE_INTERVAL               = 20 * IN_MILLISECONDS,
        DEFENSE_MAX_DURATION        = 3 * MINUTE * IN_MILLISECONDS,
        EXIT_SCENE_DELAY            = 8 * IN_MILLISECONDS,
        EXIT_TELEPORT_DELAY         = 25 * IN_MILLISECONDS,
    };

    Position const EnterPosition   = { 980.3f, -2921.1f, 99.4f, 2.64f };      // beside Exarch Akama, at the temple gates
    Position const ExitPosition    = { 936.907f, -833.35f, -28.8338f, 3.95473f }; // Embaari Village, where Yrel waits
    Position const WaveSpawns[2]   = { { 492.8f, -3067.9f, 58.4f, 6.05f }, { 666.4f, -3066.2f, 58.4f, 3.14f } };
    uint32 const WaveUnits[WAVE_COUNT][3] =
    {
        { NPC_IRON_GRUNT,       NPC_IRON_GRUNT,        NPC_IRON_BOLTBLASTER },
        { NPC_IRON_GRUNT,       NPC_IRON_SHIELDBEARER, NPC_IRON_BOLTBLASTER },
        { NPC_IRON_SHIELDBEARER, NPC_IRON_GRUNT,       NPC_IRON_GRUNT       },
        { NPC_IRON_BRUTE,       NPC_IRON_GRUNT,        NPC_IRON_BOLTBLASTER },
        { NPC_IRON_SHIELDBEARER, NPC_IRON_BOLTBLASTER, NPC_IRON_BOLTBLASTER },
        { NPC_IRON_BRUTE,       NPC_IRON_SHIELDBEARER, NPC_IRON_GRUNT       },
    };

    // the player is marked for the right scenario of the map, then the map builds it with its step tracker
    inline void EnterScenario(Player* player)
    {
        if (!player || player->GetMapId() == MAP_DEFENSE_OF_KARABOR || player->GetQuestStatus(QUEST_DEFENSE_OF_KARABOR) != QUEST_STATUS_INCOMPLETE)
            return;

        player->SetScenarioId(SCENARIO_DEFENSE);
        if (!player->TeleportTo(MAP_DEFENSE_OF_KARABOR, EnterPosition.GetPositionX(), EnterPosition.GetPositionY(), EnterPosition.GetPositionZ(), EnterPosition.GetOrientation()))
        {
            player->SetScenarioId(0);
            TC_LOG_ERROR("scripts", "Defense of Karabor: %s could not be sent into the scenario (map %u refused)", player->GetName(), uint32(MAP_DEFENSE_OF_KARABOR));
        }
    }
}

#endif // DEFENSE_OF_KARABOR_H
