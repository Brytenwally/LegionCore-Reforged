////////////////////////////////////////////
///                                      ///
///              DEMONCORE               ///
///    Copyright 2026 Titans-Project     ///
///         All Rights Reserved.         ///
///                                      ///
////////////////////////////////////////////

#ifndef BATTLE_FOR_SHATTRATH_H
#define BATTLE_FOR_SHATTRATH_H

#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "ScriptedGossip.h"
#include "InstanceScript.h"
#include "ObjectAccessor.h"
#include "Player.h"
#include "Scenario.h"
#include "ScenarioMgr.h"
#include "TemporarySummon.h"
#include "Vehicle.h"
#include "MoveSpline.h"
#include "MoveSplineInit.h"
#include "DatabaseEnv.h"

namespace BattleForShattrath
{
    enum Data
    {
        MAP_TALADOR                 = 1116,
        MAP_SHATTRATH_SCENARIO      = 1207,
        SCENARIO_SHATTRATH          = 331,
        QUEST_BATTLE_ALLIANCE       = 34099,
        QUEST_BATTLE_HORDE          = 33731,
        MOVIE_BATTLE_FOR_SHATTRATH  = 188,

        NPC_KHADGAR_TALADOR         = 75805,
        NPC_GAZLOWE                 = 77191,
        NPC_THAELIN                 = 77192,
        NPC_DUROTAN                 = 77193,
        NPC_DRAKA                   = 77194,
        NPC_KHADGAR                 = 77195,
        NPC_MARAAD                  = 77196,
        NPC_YREL                    = 77197,
        NPC_SHATTRATH_DEFENDER      = 77199,
        NPC_GROMKAR_VANGUARD        = 77202,
        NPC_GROMKAR_GRIMSHOT        = 77203,
        NPC_GROMKAR_SHADOWBLADE     = 77204,
        NPC_BURNING_BLADEMASTER     = 77205,
        NPC_GROMKAR_DEADEYE         = 77207,
        NPC_BLACKHAND               = 77256,
        NPC_ORGRIM                  = 77257,
        NPC_MACHINIST_BRANDT        = 80962,
        NPC_BLACKROCK_BOMB          = 84206,
        NPC_GROMKAR_GRENADIER       = 84630,
        NPC_IRON_STAR_RIDE          = 86671,
        NPC_IRON_STAR_SHELTER       = 86712,
        NPC_IRON_BRUTE              = 83492,
        NPC_DOCKS_BLADEMASTER       = 83636,
        NPC_DOCKS_VANGUARD          = 83637,
        NPC_DOCKS_GRIMSHOT          = 83638,
        NPC_DOCKS_SHADOWBLADE       = 83639,
        NPC_WAVE_GRIMSHOT           = 83805,
        NPC_WAVE_VANGUARD           = 83806,
        NPC_WAVE_BLADEMASTER        = 83807,
        NPC_FIREBLADE_INVOKER       = 83811,
        NPC_WAVE_SHADOWBLADE        = 84359,
        NPC_BOMB_PICKUP             = 84092,
        NPC_BIG_CANNON              = 84229,
        GO_SHIP_TURRET              = 2601301,  // Iron Horde Turret, the twin guns on top of each ship (its firing is the model's Custom1)
        NPC_AIR_DEFENSE_CRYSTAL     = 84752,
        SPELL_CRYSTAL_LASER         = 169459,   // the crystal lasers an incoming shell down, as in Talador
        NPC_TURRET_MUZZLE           = 900131,   // invisible mark on each turret the shells leave from
        GO_PORTAL_TO_TALADOR        = 231620,

        SPELL_TIME_STOP             = 167264,   // Khadgar's 90 second spell: the Iron Horde is left hanging in the air
        SPELL_ARCANE_BLAST_VISUAL   = 63660,
        SPELL_DECK_FIRE             = 156191,   // Cosmetic - Large Fire 6FX
        SPELL_STRUCTURE_EXPLOSION   = 188744,
        SPELL_FIRE_EXPLOSION        = 120682,   // Horde Intro Cannonfire HITS: a shell bursting, with its blast
        SPELL_SHELL_IN_FLIGHT       = 86527,    // the shell on its way, as with Talador's air defense crystals
        NPC_INVISIBLE_STALKER       = 32780,
        SPELL_SHOULDER_CHARGE       = 157843,
        SPELL_HIDE_PIERCING_SHOT    = 157013,
        SPELL_STORMSTRIKE           = 160769,
        SPELL_WHIRLWIND             = 166782,
        SPELL_RIFLE_SHOT            = 159085,
        SPELL_LAVA_BURST            = 162074,
        SPELL_MORTAR_SHOT           = 159107,
        SPELL_BLACKROCK_GRENADES    = 160703,   // a bomb in hand: the action bar offers to throw it
        SPELL_BLACKROCK_GRENADE     = 161060,

        EVENT_TALK_TO_THAELIN       = 42809,
        EVENT_ENTER_IRON_STAR       = 42033,
        EVENT_REACH_HARBOR          = 42032,
        EVENT_TALK_TO_KHADGAR       = 42109,
        EVENT_PROTECT_KHADGAR       = 41623,
        EVENT_CONFRONT_BLACKHAND    = 41842,
        EVENT_TELEPORT_BATTLESHIP   = 40692,
        EVENT_BRANDT_DEFEATED       = 39193,
        EVENT_TELEPORT_FLAGSHIP     = 42038,
        EVENT_SURVIVE_BLACKHAND     = 40688,

        STEP_PREPARATIONS           = 0,
        STEP_IRON_STAR              = 1,
        STEP_BATTLE_IS_JOINED       = 2,
        STEP_IRON_HARBOR            = 3,
        STEP_THE_WEAPON             = 4,
        STEP_BLACKHAND              = 5,

        DATA_TALKED_TO_THAELIN      = 1,
        DATA_ENTERED_IRON_STAR      = 2,
        DATA_RIDE_ARRIVED           = 3,
        DATA_TALKED_TO_KHADGAR      = 4,
        DATA_TELEPORT_BATTLESHIP    = 5,
        DATA_TELEPORT_FLAGSHIP      = 6,
        DATA_BLACKHAND_DONE         = 7,
        DATA_CONFRONTED             = 8,
        DATA_STAR_RIDER             = 9,
        DISPLAY_IRON_STAR           = 56345,
        NPC_SHELL_IMPACT            = 900130,   // invisible mark the turrets' shells fly to, seen from afar

        CHANNEL_DURATION            = 90 * IN_MILLISECONDS,
        WAVE_INTERVAL               = 15 * IN_MILLISECONDS,
        EXIT_PORTAL_DELAY           = 8 * IN_MILLISECONDS,
    };

    // creature_text groups
    enum Talks
    {
        SAY_KHADGAR_TALADOR_ACCEPT  = 0,
        SAY_KHADGAR_CLEAR_THE_WAY   = 0,
        SAY_KHADGAR_FINISHED        = 1,
        SAY_KHADGAR_SPEAK_WITH_ME   = 2,
        SAY_KHADGAR_HELP_BLACKHAND  = 3,
        SAY_MARAAD_FOR_SHATTRATH    = 0,
        SAY_MARAAD_NO_MERCY         = 1,
        SAY_MARAAD_FORWARD          = 2,
        SAY_YREL_STEEL_YOURSELVES   = 0,
        SAY_YREL_CANNOT_HOLD        = 1,
        SAY_YREL_HAVE_HOPE          = 2,
        SAY_YREL_HOLD_OFF_BLACKHAND = 3,
        SAY_DUROTAN_DO_NOT_WAVER    = 0,
        SAY_DUROTAN_MORE_TIME       = 1,
        SAY_DUROTAN_SURVIVORS       = 2,
        SAY_DUROTAN_BROTHER         = 3,
        SAY_DRAKA_GUARD_UP          = 0,
        SAY_DRAKA_OF_COURSE         = 1,
        SAY_ORGRIM_INNOCENTS        = 0,
        SAY_ORGRIM_YOU_MUST_STOP    = 1,
        SAY_BLACKHAND_INNOCENT      = 0,
        SAY_BLACKHAND_DRAENOR_BURN  = 1,
        SAY_BLACKHAND_ALL_YOU_HAVE  = 2,
        SAY_BLACKHAND_WEAK          = 3,
        SAY_BLACKHAND_PLAY_WITH_FIRE = 4,
        EMOTE_BLACKHAND_MARKED      = 5,
        SAY_BRANDT_NUISANCE         = 0,
        SAY_BRANDT_END_THIS         = 1,
        SAY_BRANDT_TAKE_YOU_OUT     = 2,
    };

    enum Gossips
    {
        GOSSIP_MENU_THAELIN         = 17094,    // "Thanks. I'll meet them at the docks."
        GOSSIP_MENU_ENTER           = 65011,    // Talador, Khadgar: "Let us retake Shattrath."
        GOSSIP_MENU_BEGIN_SPELL     = 65012,    // "Begin your spell. I'll protect you!"
        GOSSIP_MENU_BATTLESHIP      = 65013,    // "I am ready to teleport to the ship."
        GOSSIP_MENU_FLAGSHIP        = 65014,    // "Then let's hurry!"
        TEXT_KHADGAR_HARBOR         = 24547,
        TEXT_KHADGAR_BATTLESHIP     = 24506,
        TEXT_KHADGAR_FLAGSHIP       = 920101,
        GOSSIP_ACTION_SCENARIO      = GOSSIP_ACTION_INFO_DEF + 1,
    };

    // the instance's scripted sequences
    enum Events
    {
        EVENT_CHANNEL_LINE_1        = 1,
        EVENT_CHANNEL_LINE_2,
        EVENT_CHANNEL_LINE_3,
        EVENT_CHANNEL_LINE_4,
        EVENT_CHANNEL_LINE_5,
        EVENT_CHANNEL_LINE_6,
        EVENT_CHANNEL_LINE_7,
        EVENT_AFTER_CHANNEL_1,
        EVENT_AFTER_CHANNEL_2,
        EVENT_AFTER_CHANNEL_3,
        EVENT_CONFRONT_1,
        EVENT_CONFRONT_2,
        EVENT_CONFRONT_3,
        EVENT_CONFRONT_4,
        EVENT_CONFRONT_5,
        EVENT_CONFRONT_6,
        EVENT_CONFRONT_KILL,
        EVENT_FLAGSHIP_1,
        EVENT_FLAGSHIP_2,
        EVENT_FLAGSHIP_3,
        EVENT_FLAGSHIP_4,
        EVENT_FLAGSHIP_5,
        EVENT_FLAGSHIP_6,
        EVENT_EXIT_PORTAL,
    };

    Position const ArchOfShatar         = { 3031.70f, 2831.60f, 67.00f };
    Position const EnterPosition        = { 3026.00f, 2826.00f, 67.02f, 0.901f };
    Position const BlackhandFight       = { 3670.50f, 3057.99f, 17.87f, 3.775f };
    Position const PlayerFlagship       = { 3696.30f, 3076.92f, 18.07f, 3.775f };   // far enough for the hero to walk up to Blackhand
    Position const YrelFlagship        = { 3665.01f, 3060.40f, 17.87f, 5.869f };
    Position const DurotanFlagship     = { 3665.66f, 3054.44f, 17.87f, 0.633f };
    Position const MaraadFlagship      = { 3671.16f, 3052.03f, 17.87f, 1.680f };
    // the deck of the flagship set ablaze by Blackhand's blast, from bow to stern
    G3D::Vector3 const FlagshipFires[] =
    {
        { 3651.86f, 3053.26f, 18.04f }, { 3660.06f, 3043.41f, 18.59f }, { 3659.89f, 3071.34f, 18.04f }, { 3662.54f, 3059.61f, 18.04f },
        { 3672.99f, 3073.07f, 18.24f }, { 3674.04f, 3044.01f, 18.15f }, { 3675.96f, 3056.03f, 17.81f }, { 3674.49f, 3085.89f, 18.24f },
        { 3687.32f, 3062.30f, 17.81f }, { 3690.31f, 3074.09f, 18.24f }, { 3690.78f, 3095.94f, 18.28f }, { 3695.62f, 3049.43f, 18.41f },
        { 3698.46f, 3061.40f, 18.10f }, { 3702.76f, 3074.46f, 17.88f }, { 3701.28f, 3089.98f, 17.88f }, { 3702.96f, 3107.67f, 18.68f },
        { 3713.03f, 3074.25f, 18.31f }, { 3714.76f, 3085.86f, 18.31f }, { 3713.11f, 3098.33f, 18.08f }, { 3713.76f, 3110.01f, 18.08f },
        { 3723.94f, 3119.20f, 18.35f }, { 3729.39f, 3079.09f, 18.31f }, { 3731.37f, 3106.54f, 18.08f }, { 3734.01f, 3094.73f, 17.67f },
        { 3734.66f, 3125.97f, 18.47f }, { 3743.65f, 3133.07f, 18.54f }, { 3747.76f, 3085.44f, 18.25f }, { 3746.37f, 3097.67f, 17.67f },
    };
    // after the film, at the foot of the flagship: Maraad and Orgrim lie side by side, Durotan kneels before them, Khadgar stands behind
    Position const EndMaraad            = { 3608.00f, 3051.00f, 1.95f, 3.385f };
    Position const EndOrgrim            = { 3608.50f, 3054.60f, 1.95f, 3.385f };
    Position const EndDurotan           = { 3605.00f, 3052.80f, 1.95f, 0.243f };
    Position const EndKhadgar           = { 3600.00f, 3052.50f, 1.95f, 0.243f };
    Position const EndPlayer            = { 3597.50f, 3047.50f, 1.96f, 0.600f };
    Position const StairsTop            = { 3446.00f, 3128.00f, 20.24f };   // where the defenders withdraw...
    Position const StairsFoot           = { 3496.00f, 3128.00f, 1.95f };    // ...by the stairs and nothing else
    Position const BrandtLanding        = { 3918.00f, 2952.00f, 18.70f };   // from his cannon down onto the deck
    Position const PlayerBattleship     = { 3912.00f, 2858.00f, 19.05f, 1.253f };
    Position const KhadgarBattleship    = { 3917.37f, 2941.37f, 18.70f, 5.015f };   // where he appears once B'randt has fallen
    // the waves run in from three piers, each soldier of a wave on its own lane, kept at quay level and clear of the wreckage;
    // they stop short of Khadgar and leave the heroes to come to them
    G3D::Vector3 const WaveLane00[] =
    {
        { 3590.09f, 3203.00f, 1.95f }, { 3573.09f, 3203.50f, 1.95f }, { 3554.35f, 3203.51f, 1.95f }, { 3551.03f, 3188.46f, 1.95f },
        { 3549.95f, 3177.19f, 1.95f }, { 3536.52f, 3175.62f, 1.95f }, { 3533.02f, 3168.23f, 1.95f }, { 3531.01f, 3162.35f, 1.95f },
        { 3529.87f, 3156.47f, 1.95f }, { 3529.21f, 3150.39f, 1.95f }, { 3528.60f, 3144.40f, 1.95f }, { 3528.12f, 3139.07f, 1.95f },
        { 3525.46f, 3134.83f, 1.95f },
    };
    G3D::Vector3 const WaveLane01[] =
    {
        { 3590.00f, 3200.00f, 1.95f }, { 3573.00f, 3200.50f, 1.95f }, { 3556.00f, 3201.00f, 1.95f }, { 3554.00f, 3188.00f, 1.95f },
        { 3552.00f, 3175.00f, 1.95f }, { 3537.82f, 3172.92f, 1.95f }, { 3535.86f, 3167.25f, 1.95f }, { 3533.91f, 3161.57f, 1.95f },
        { 3532.84f, 3156.03f, 1.95f }, { 3532.19f, 3150.07f, 1.95f }, { 3531.58f, 3144.10f, 1.95f }, { 3530.97f, 3138.13f, 1.95f },
        { 3528.02f, 3133.27f, 1.95f },
    };
    G3D::Vector3 const WaveLane02[] =
    {
        { 3589.91f, 3197.00f, 1.95f }, { 3572.91f, 3197.50f, 1.95f }, { 3557.65f, 3198.49f, 1.95f }, { 3556.97f, 3187.54f, 1.95f },
        { 3554.05f, 3172.81f, 1.95f }, { 3539.12f, 3170.22f, 1.95f }, { 3538.70f, 3166.27f, 1.95f }, { 3536.81f, 3160.79f, 1.95f },
        { 3535.81f, 3155.59f, 1.95f }, { 3535.17f, 3149.75f, 1.95f }, { 3534.56f, 3143.80f, 1.95f }, { 3533.82f, 3137.19f, 1.95f },
        { 3530.58f, 3131.71f, 1.95f },
    };
    G3D::Vector3 const WaveLane10[] =
    {
        { 3614.10f, 3062.86f, 1.95f }, { 3608.30f, 3061.03f, 1.95f }, { 3602.78f, 3058.90f, 1.95f }, { 3597.92f, 3056.95f, 1.95f },
        { 3592.92f, 3056.96f, 1.95f }, { 3589.72f, 3057.46f, 1.95f }, { 3590.04f, 3061.84f, 1.95f }, { 3588.79f, 3067.89f, 1.95f },
        { 3588.57f, 3074.96f, 1.95f }, { 3582.47f, 3079.31f, 1.95f }, { 3575.79f, 3079.97f, 1.95f }, { 3569.82f, 3080.49f, 1.95f },
        { 3564.67f, 3080.86f, 1.95f }, { 3560.70f, 3084.06f, 1.95f }, { 3555.99f, 3087.15f, 1.95f }, { 3552.88f, 3091.32f, 1.95f },
        { 3546.62f, 3093.48f, 1.95f }, { 3542.14f, 3095.58f, 1.95f }, { 3539.41f, 3099.32f, 1.95f }, { 3538.26f, 3104.58f, 1.95f },
        { 3536.76f, 3110.92f, 1.95f }, { 3533.26f, 3116.30f, 1.95f }, { 3528.78f, 3120.71f, 1.95f },
    };
    G3D::Vector3 const WaveLane11[] =
    {
        { 3615.00f, 3060.00f, 1.95f }, { 3609.29f, 3058.20f, 1.95f }, { 3603.86f, 3056.10f, 1.95f }, { 3598.46f, 3054.00f, 1.95f },
        { 3592.46f, 3054.00f, 1.95f }, { 3587.26f, 3055.74f, 1.95f }, { 3587.04f, 3061.73f, 1.95f }, { 3586.82f, 3067.72f, 1.95f },
        { 3586.01f, 3073.40f, 1.95f }, { 3581.50f, 3076.47f, 1.95f }, { 3575.53f, 3076.98f, 1.95f }, { 3569.55f, 3077.50f, 1.95f },
        { 3563.58f, 3078.06f, 1.95f }, { 3558.95f, 3081.62f, 1.95f }, { 3553.95f, 3084.95f, 1.95f }, { 3551.19f, 3088.84f, 1.95f },
        { 3545.49f, 3090.70f, 1.95f }, { 3540.22f, 3093.28f, 1.95f }, { 3536.68f, 3098.07f, 1.95f }, { 3535.34f, 3103.91f, 1.95f },
        { 3533.99f, 3109.76f, 1.95f }, { 3530.95f, 3114.38f, 1.95f }, { 3526.68f, 3118.57f, 1.95f },
    };
    G3D::Vector3 const WaveLane12[] =
    {
        { 3615.90f, 3057.14f, 1.95f }, { 3610.28f, 3055.37f, 1.95f }, { 3604.94f, 3053.30f, 1.95f }, { 3599.00f, 3051.05f, 1.95f },
        { 3592.00f, 3051.04f, 1.95f }, { 3584.80f, 3054.02f, 1.95f }, { 3584.04f, 3061.62f, 1.95f }, { 3583.83f, 3067.46f, 1.95f },
        { 3583.45f, 3071.84f, 1.95f }, { 3580.53f, 3073.63f, 1.95f }, { 3575.27f, 3073.99f, 1.95f }, { 3569.28f, 3074.51f, 1.95f },
        { 3562.49f, 3075.26f, 1.95f }, { 3557.20f, 3079.18f, 1.95f }, { 3551.91f, 3082.75f, 1.95f }, { 3549.50f, 3086.36f, 1.95f },
        { 3544.36f, 3087.92f, 1.95f }, { 3539.58f, 3092.52f, 1.95f }, { 3534.88f, 3097.24f, 1.95f }, { 3532.42f, 3103.24f, 1.95f },
        { 3531.22f, 3108.60f, 1.95f }, { 3528.64f, 3112.46f, 1.95f }, { 3524.58f, 3116.43f, 1.95f },
    };
    G3D::Vector3 const WaveLane20[] =
    {
        { 3640.22f, 3212.99f, 1.95f }, { 3634.24f, 3213.43f, 1.95f }, { 3628.36f, 3213.87f, 1.95f }, { 3622.26f, 3214.74f, 1.95f },
        { 3615.96f, 3214.38f, 1.95f }, { 3609.98f, 3214.03f, 1.95f }, { 3604.07f, 3213.73f, 1.95f }, { 3598.13f, 3213.66f, 1.95f },
        { 3592.13f, 3213.59f, 1.95f }, { 3585.83f, 3213.51f, 1.95f }, { 3579.81f, 3212.23f, 1.95f }, { 3573.95f, 3211.07f, 1.95f },
        { 3568.06f, 3209.90f, 1.95f }, { 3562.10f, 3207.67f, 1.95f }, { 3556.53f, 3204.73f, 1.95f }, { 3549.38f, 3201.09f, 1.95f },
        { 3549.10f, 3187.53f, 1.95f }, { 3549.98f, 3177.22f, 1.95f }, { 3537.62f, 3178.06f, 1.95f }, { 3533.82f, 3170.54f, 1.95f },
        { 3531.87f, 3164.87f, 1.95f }, { 3529.81f, 3158.81f, 1.95f }, { 3529.45f, 3152.75f, 1.95f }, { 3528.85f, 3146.84f, 1.95f },
        { 3528.27f, 3141.10f, 1.95f }, { 3527.16f, 3136.51f, 1.95f }, { 3523.39f, 3132.88f, 1.95f },
    };
    G3D::Vector3 const WaveLane21[] =
    {
        { 3640.00f, 3210.00f, 1.95f }, { 3634.02f, 3210.44f, 1.95f }, { 3628.03f, 3210.89f, 1.95f }, { 3622.13f, 3211.74f, 1.95f },
        { 3616.14f, 3211.39f, 1.95f }, { 3610.15f, 3211.03f, 1.95f }, { 3604.16f, 3210.73f, 1.95f }, { 3598.16f, 3210.66f, 1.95f },
        { 3592.16f, 3210.59f, 1.95f }, { 3586.16f, 3210.53f, 1.95f }, { 3580.42f, 3209.29f, 1.95f }, { 3574.53f, 3208.13f, 1.95f },
        { 3568.64f, 3206.96f, 1.95f }, { 3562.76f, 3205.80f, 1.95f }, { 3557.47f, 3202.99f, 1.95f }, { 3552.21f, 3200.11f, 1.95f },
        { 3552.10f, 3187.50f, 1.95f }, { 3552.00f, 3175.00f, 1.95f }, { 3538.62f, 3175.23f, 1.95f }, { 3536.66f, 3169.56f, 1.95f },
        { 3534.71f, 3163.89f, 1.95f }, { 3532.75f, 3158.22f, 1.95f }, { 3532.44f, 3152.51f, 1.95f }, { 3531.83f, 3146.54f, 1.95f },
        { 3531.22f, 3140.57f, 1.95f }, { 3529.75f, 3135.00f, 1.95f }, { 3525.51f, 3130.76f, 1.95f },
    };
    G3D::Vector3 const WaveLane22[] =
    {
        { 3639.78f, 3207.01f, 1.95f }, { 3633.80f, 3207.45f, 1.95f }, { 3627.70f, 3207.91f, 1.95f }, { 3622.00f, 3208.74f, 1.95f },
        { 3616.32f, 3208.40f, 1.95f }, { 3610.32f, 3208.03f, 1.95f }, { 3604.25f, 3207.73f, 1.95f }, { 3598.19f, 3207.66f, 1.95f },
        { 3592.19f, 3207.59f, 1.95f }, { 3586.49f, 3207.55f, 1.95f }, { 3581.03f, 3206.35f, 1.95f }, { 3575.11f, 3205.19f, 1.95f },
        { 3569.22f, 3204.02f, 1.95f }, { 3563.76f, 3202.97f, 1.95f }, { 3558.89f, 3200.35f, 1.95f }, { 3555.04f, 3199.13f, 1.95f },
        { 3555.10f, 3187.47f, 1.95f }, { 3554.02f, 3172.78f, 1.95f }, { 3539.62f, 3172.40f, 1.95f }, { 3539.50f, 3168.58f, 1.95f },
        { 3537.55f, 3162.91f, 1.95f }, { 3535.69f, 3157.63f, 1.95f }, { 3535.43f, 3152.27f, 1.95f }, { 3534.81f, 3146.24f, 1.95f },
        { 3534.17f, 3140.04f, 1.95f }, { 3532.34f, 3133.49f, 1.95f }, { 3527.63f, 3128.64f, 1.95f },
    };
    struct WaveLane { G3D::Vector3 const* Points; size_t Size; };
    WaveLane const WaveLanes[3][3] =
    {
        { { WaveLane00, std::extent<decltype(WaveLane00)>::value }, { WaveLane01, std::extent<decltype(WaveLane01)>::value }, { WaveLane02, std::extent<decltype(WaveLane02)>::value } },
        { { WaveLane10, std::extent<decltype(WaveLane10)>::value }, { WaveLane11, std::extent<decltype(WaveLane11)>::value }, { WaveLane12, std::extent<decltype(WaveLane12)>::value } },
        { { WaveLane20, std::extent<decltype(WaveLane20)>::value }, { WaveLane21, std::extent<decltype(WaveLane21)>::value }, { WaveLane22, std::extent<decltype(WaveLane22)>::value } },
    };
    Position const AllianceExit         = { 3590.36f, 1676.60f, 173.27f, 3.14f };   // Fort Wrynn, beside Yrel
    Position const HordeExit            = { 3252.60f, 1577.10f, 162.30f, 0.00f };   // Vol'jin's Pride, beside Durotan
    // Blackhand's own forces: the heroes' faction is only at war with theirs
    uint32 const WaveUnits[5]           = { NPC_WAVE_GRIMSHOT, NPC_WAVE_VANGUARD, NPC_WAVE_BLADEMASTER, NPC_FIREBLADE_INVOKER, NPC_WAVE_SHADOWBLADE };
    uint32 const IronHordeUnits[]       = { NPC_GROMKAR_VANGUARD, NPC_GROMKAR_GRIMSHOT, NPC_GROMKAR_SHADOWBLADE, NPC_BURNING_BLADEMASTER, NPC_GROMKAR_DEADEYE,
                                            NPC_IRON_BRUTE, NPC_DOCKS_BLADEMASTER, NPC_DOCKS_VANGUARD, NPC_DOCKS_GRIMSHOT, NPC_DOCKS_SHADOWBLADE,
                                            NPC_WAVE_GRIMSHOT, NPC_WAVE_VANGUARD, NPC_WAVE_BLADEMASTER, NPC_FIREBLADE_INVOKER, NPC_WAVE_SHADOWBLADE, NPC_GROMKAR_GRENADIER };

    // the heroes wait at the top of the harbor stairs, then hold its foot while Khadgar casts (Khadgar, Yrel, Durotan, Draka, Maraad)
    Position const HarborSpots[5]       =
    {
        { 3507.00f, 3128.00f, 1.80f, 0.0f }, { 3514.00f, 3124.00f, 1.95f, 0.0f }, { 3513.00f, 3137.00f, 1.95f, 0.0f },
        { 3509.00f, 3134.00f, 1.95f, 0.0f }, { 3521.00f, 3121.00f, 1.95f, 0.0f },   // Draka clear of the wrecked wagon, Yrel and Maraad of the war wagon
    };
    // where the shells land, away from the crystal: spots picked in game
    G3D::Vector3 const ShellTargets[] =
    {
        { 3430.43f, 3209.66f, 9.20f }, { 3570.38f, 3160.51f, 5.50f }, { 3516.24f, 2975.66f, 2.44f }, { 3447.87f, 3017.78f, 18.30f },
        { 3271.32f, 3224.90f, 22.46f },
    };
    float const SHELL_GROUND_SCALE      = 0.7f;    // the bursts on the ground are a third smaller
    float const CRYSTAL_SHIELD_RANGE    = 37.0f;   // the crystal brings shells down this far out, as in Talador
    float const SHELL_SPEED             = 85.0f;   // missile speed of the shell (120682)
    Position const ShipTops[3]          = { { 3937.71f, 2856.09f }, { 3766.70f, 3126.80f }, { 3821.82f, 3241.20f } };
    float const TIME_STOP_REACH         = 150.0f;
    float const KHADGAR_LEVITATION      = 4.0f;
    uint32 const POINT_WITHDRAW_FOOT    = 101;
    uint32 const POINT_STAIRS_FOOT      = 103;
    uint32 const POINT_WITHDRAW_TOP     = 102;
    float const KHADGAR_RUN_RATE        = 1.4f;
    uint32 const HERO_SCRATCH           = 5000;   // Blackhand and the heroes only scratch each other   // the spell's own reach is the whole map: the battleships must stay out of it

    // the Iron Star's way down the avenue to the top of the harbor stairs: straight ahead first, then kept to the middle,
    // clear of the walls at the height of the star
    G3D::Vector3 const IronStarPath[] =
    {
        { 3034.25f, 2837.86f, 67.16f }, { 3034.62f, 2842.85f, 67.10f }, { 3034.99f, 2847.84f, 67.09f }, { 3035.37f, 2852.83f, 67.09f },
        { 3035.74f, 2857.81f, 65.34f }, { 3036.11f, 2862.80f, 62.90f }, { 3036.48f, 2867.79f, 60.46f }, { 3036.86f, 2872.78f, 58.08f },
        { 3037.23f, 2877.77f, 55.56f }, { 3037.60f, 2882.76f, 53.44f }, { 3037.97f, 2887.74f, 50.92f }, { 3038.35f, 2892.73f, 48.90f },
        { 3038.72f, 2897.72f, 47.82f }, { 3039.09f, 2902.71f, 47.82f }, { 3040.81f, 2915.06f, 47.82f }, { 3044.21f, 2921.56f, 47.82f },
        { 3049.33f, 2929.55f, 47.82f }, { 3053.64f, 2938.01f, 47.82f }, { 3056.48f, 2947.41f, 47.82f }, { 3057.33f, 2957.12f, 47.82f },
        { 3055.17f, 2966.76f, 47.82f }, { 3053.09f, 2976.45f, 47.82f }, { 3051.68f, 2985.80f, 47.82f }, { 3046.05f, 2993.81f, 47.82f },
        { 3044.06f, 3003.79f, 47.82f }, { 3041.26f, 3013.19f, 47.82f }, { 3038.94f, 3022.72f, 47.82f }, { 3035.81f, 3032.15f, 47.82f },
        { 3030.81f, 3041.34f, 47.82f }, { 3027.27f, 3051.23f, 48.00f }, { 3027.90f, 3061.37f, 47.95f }, { 3029.63f, 3071.04f, 45.25f },
        { 3028.51f, 3080.93f, 40.79f }, { 3030.03f, 3089.90f, 36.45f }, { 3031.80f, 3096.50f, 33.50f }, { 3036.00f, 3102.50f, 33.50f },
        { 3044.00f, 3105.80f, 33.50f }, { 3053.00f, 3106.00f, 33.50f }, { 3066.04f, 3106.49f, 33.50f }, { 3075.95f, 3108.65f, 33.88f },
        { 3085.88f, 3108.31f, 33.82f }, { 3095.70f, 3106.93f, 32.23f }, { 3105.17f, 3106.39f, 27.73f }, { 3114.37f, 3106.10f, 23.49f },
        { 3123.74f, 3105.89f, 20.15f }, { 3133.09f, 3107.30f, 20.15f }, { 3141.15f, 3112.48f, 20.15f }, { 3149.44f, 3117.90f, 20.15f },
        { 3158.20f, 3122.31f, 20.15f }, { 3167.57f, 3125.16f, 20.15f }, { 3176.97f, 3127.91f, 20.15f }, { 3185.33f, 3132.83f, 20.15f },
        { 3192.74f, 3139.45f, 20.15f }, { 3200.49f, 3145.20f, 20.15f }, { 3209.55f, 3147.69f, 20.15f }, { 3219.41f, 3147.49f, 20.15f },
        { 3229.38f, 3146.91f, 20.15f }, { 3239.38f, 3146.76f, 20.15f }, { 3249.38f, 3146.75f, 20.15f }, { 3259.38f, 3146.75f, 20.15f },
        { 3269.38f, 3146.78f, 20.15f }, { 3279.36f, 3147.08f, 20.15f }, { 3289.32f, 3147.78f, 20.15f }, { 3299.25f, 3148.81f, 20.15f },
        { 3309.14f, 3150.15f, 20.15f }, { 3319.03f, 3150.89f, 20.15f }, { 3328.93f, 3150.15f, 20.15f }, { 3336.32f, 3148.95f, 20.15f },
        { 3348.70f, 3147.24f, 20.15f }, { 3358.69f, 3146.76f, 20.15f }, { 3368.52f, 3145.34f, 20.15f }, { 3377.01f, 3140.19f, 20.15f },
        { 3384.10f, 3133.15f, 20.15f }, { 3392.19f, 3127.36f, 20.15f }, { 3401.90f, 3125.29f, 20.15f }, { 3411.78f, 3123.98f, 20.15f },
        { 3421.63f, 3122.38f, 20.15f },
    };
    size_t const IronStarPathSize = std::extent<decltype(IronStarPath)>::value;
    // the parapet on the left of the stairs top, where the empty star crashes after curving away
    G3D::Vector3 const IronStarArc[] = { { 3421.63f, 3122.38f, 20.15f }, { 3431.00f, 3126.00f, 20.20f }, { 3438.00f, 3133.00f, 20.30f }, { 3443.00f, 3143.80f, 20.76f } };
    Position const IronStarCrash        = { 3443.00f, 3143.80f, 20.76f };

    // each hero's own way from the harbor to the pier where the flagship ran aground: quay level, straight lines,
    // ending in a row facing the ship (Yrel, Durotan, Maraad, Khadgar)
    G3D::Vector3 const HeroPath0[] = { { 3513.00f, 3119.00f, 1.95f }, { 3533.25f, 3113.00f, 1.95f }, { 3594.75f, 3050.50f, 1.95f }, { 3603.48f, 3049.06f, 1.95f } };
    G3D::Vector3 const HeroPath1[] = { { 3513.00f, 3137.00f, 1.95f }, { 3509.25f, 3136.00f, 1.95f }, { 3589.25f, 3054.00f, 1.96f }, { 3601.56f, 3056.82f, 1.95f } };
    G3D::Vector3 const HeroPath2[] = { { 3519.00f, 3113.00f, 1.95f }, { 3505.75f, 3098.00f, 1.95f }, { 3527.00f, 3073.25f, 1.95f }, { 3587.00f, 3072.00f, 1.95f }, { 3589.25f, 3054.00f, 1.96f }, { 3604.44f, 3045.18f, 1.97f } };
    G3D::Vector3 const HeroPath3[] = { { 3507.00f, 3128.00f, 1.80f }, { 3589.25f, 3054.00f, 1.96f }, { 3602.52f, 3052.94f, 1.95f } };
    struct HeroWay { G3D::Vector3 const* Points; size_t Size; Position End; };
    HeroWay const HeroWays[4] =
    {
        { HeroPath0, std::extent<decltype(HeroPath0)>::value, { 3603.48f, 3049.06f, 1.95f, 0.243f } },
        { HeroPath1, std::extent<decltype(HeroPath1)>::value, { 3601.56f, 3056.82f, 1.95f, 0.243f } },
        { HeroPath2, std::extent<decltype(HeroPath2)>::value, { 3604.44f, 3045.18f, 1.97f, 0.243f } },
        { HeroPath3, std::extent<decltype(HeroPath3)>::value, { 3602.52f, 3052.94f, 1.95f, 0.243f } },
    };
    uint32 const SPELL_TELEPORT_VISUAL = 41232;   // Teleport Visual Only

    inline bool HasBattleQuest(Player* player)
    {
        return player->GetQuestStatus(QUEST_BATTLE_ALLIANCE) == QUEST_STATUS_INCOMPLETE || player->GetQuestStatus(QUEST_BATTLE_HORDE) == QUEST_STATUS_INCOMPLETE;
    }

    // the player is marked for the scenario of the map, then the map builds it with its step tracker
    inline void EnterScenario(Player* player)
    {
        if (!player || player->GetMapId() == MAP_SHATTRATH_SCENARIO || !HasBattleQuest(player))
            return;

        player->SetScenarioId(SCENARIO_SHATTRATH);
        if (!player->TeleportTo(MAP_SHATTRATH_SCENARIO, EnterPosition.GetPositionX(), EnterPosition.GetPositionY(), EnterPosition.GetPositionZ(), EnterPosition.GetOrientation()))
        {
            player->SetScenarioId(0);
            TC_LOG_ERROR("scripts", "Battle for Shattrath: %s could not be sent into the scenario (map %u refused)", player->GetName(), uint32(MAP_SHATTRATH_SCENARIO));
        }
    }

    inline Scenario* GetScenario(Map* map)
    {
        Scenario* scenario = map ? sScenarioMgr->GetScenario(map->GetInstanceId()) : nullptr;
        return scenario && scenario->GetMap() == map ? scenario : nullptr;
    }

    inline uint8 GetStep(WorldObject* object)
    {
        Scenario* scenario = GetScenario(object->GetMap());
        return scenario ? scenario->GetCurrentStep() : STEP_PREPARATIONS;
    }

    inline bool IsWave(uint32 entry)
    {
        return std::find(std::begin(WaveUnits), std::end(WaveUnits), entry) != std::end(WaveUnits);
    }

    inline bool IsIronHorde(uint32 entry)
    {
        return std::find(std::begin(IronHordeUnits), std::end(IronHordeUnits), entry) != std::end(IronHordeUnits);
    }

    // Blackhand's forces and the defenders of the docks leave each other be: the waves are the heroes' fight
    inline bool KeepApart(Unit* me, Unit* who)
    {
        if (!who)
            return false;
        return (IsWave(me->GetEntry()) && who->GetEntry() == NPC_SHATTRATH_DEFENDER)
            || (me->GetEntry() == NPC_SHATTRATH_DEFENDER && IsWave(who->GetEntry()));
    }

    // creatures of an instance never give up a chase on their own: led too far, they break off and go home
    inline bool Leashed(Creature* me)
    {
        bool wave = IsWave(me->GetEntry());
        Position const& anchor = wave ? HarborSpots[0] : me->GetHomePosition();
        if (me->GetExactDist2d(&anchor) > (wave ? 70.0f : 40.0f))
            return true;
        // out of their reach: far away, or high above them
        if (Unit* victim = me->getVictim())
            if (me->GetExactDist2d(victim) > 50.0f || victim->GetPositionZ() - me->GetPositionZ() > 15.0f)
                return true;
        return false;
    }

    // held by Khadgar's spell: out of the fight for good
    inline bool IsFrozen(Unit* unit)
    {
        return unit->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
    }

    // creatures of this core do not pick fights with each other on their own: the battle in the streets is sought out
    inline Creature* FindFoe(Creature* me, float range)
    {
        std::list<Creature*> foes;
        // Blackhand's own forces come for the heroes and nobody else; the defenders keep to the fight of the docks
        if (IsWave(me->GetEntry()))
        {
            for (uint32 entry : { uint32(NPC_YREL), uint32(NPC_DUROTAN), uint32(NPC_DRAKA), uint32(NPC_MARAAD) })
                me->GetCreatureListWithEntryInGridAppend(foes, entry, range);
        }
        else if (IsIronHorde(me->GetEntry()))
        {
            for (uint32 entry : { uint32(NPC_SHATTRATH_DEFENDER), uint32(NPC_YREL), uint32(NPC_DUROTAN), uint32(NPC_DRAKA), uint32(NPC_MARAAD) })
                me->GetCreatureListWithEntryInGridAppend(foes, entry, range);
        }
        else
            for (uint32 entry : IronHordeUnits)
                if (me->GetEntry() != NPC_SHATTRATH_DEFENDER || !IsWave(entry))
                    me->GetCreatureListWithEntryInGridAppend(foes, entry, range);

        Creature* best = nullptr;
        for (Creature* foe : foes)
            if (foe->IsAlive() && !IsFrozen(foe) && me->IsHostileTo(foe) && (!best || me->GetDistance(foe) < me->GetDistance(best)))
                best = foe;
        return best;
    }
}

#endif // BATTLE_FOR_SHATTRATH_H
