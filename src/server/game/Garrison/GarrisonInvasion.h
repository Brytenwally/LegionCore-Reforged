////////////////////////////////////////////
///                                      ///
///              DEMONCORE               ///
///    Copyright 2026 Titans-Project     ///
///         All Rights Reserved.         ///
///                                      ///
////////////////////////////////////////////

#ifndef GarrisonInvasion_h__
#define GarrisonInvasion_h__

#include "Define.h"
#include <algorithm>
#include <vector>

// Garrison invasions (6.1): an Apexis assault quest angers a faction, whose invasion of the garrison can then be
// fought once until the weekly reset. Shared by the quest gating in Player and the event script.
namespace GarrisonInvasion
{
    enum Type : uint8
    {
        TYPE_IRON_HORDE     = 0,
        TYPE_SHADOW_COUNCIL = 1,
        TYPE_OGRE           = 2,
        TYPE_PRIMALS        = 3,
        TYPE_GOREN          = 4,
        TYPE_SHADOWMOON     = 5,
        MAX_TYPES           = 6
    };

    enum Medal : uint8
    {
        MEDAL_BRONZE        = 0,
        MEDAL_SILVER        = 1,
        MEDAL_GOLD          = 2,
        MEDAL_PLATINUM      = 3,
        MAX_MEDALS          = 4
    };

    struct TypeInfo
    {
        uint32 QuestAlliance;
        uint32 QuestHorde;
        uint32 ScenarioId;
        uint32 PendingTracking;     // hidden weekly quest: the invasion can be fought this week
    };

    inline TypeInfo const Types[MAX_TYPES] =
    {
        { 36655, 35935, 704, 50222 },   // Iron Horde
        { 36830, 36831, 733, 50223 },   // Shadow Council
        { 36650, 36623, 724, 50224 },   // Ogre (Gorian)
        { 36171, 36653, 707, 50225 },   // Primals (Botani)
        { 36376, 35142, 499, 50312 },   // Breakers (Goren)
        { 36412, 36414, 715, 50320 },   // Shadowmoon
    };

    struct MedalInfo
    {
        uint32 Points;
        uint32 Item;
        uint32 RewardTracking;      // hidden weekly quest: this bag was received this week
    };

    inline MedalInfo const Medals[MAX_MEDALS] =
    {
        {  300, 120320, 50334 },    // Invader's Abandoned Sack
        {  600, 120319, 50344 },    // Invader's Damaged Cache
        { 1000, 116980, 50345 },    // Invader's Forgotten Treasure
        { 1300, 122163, 50346 },    // Routed Invader's Crate of Spoils
    };

    // every variant of each Apexis assault (task quest, daily, missive) for both factions
    struct AssaultInfo
    {
        Type InvasionType;
        std::vector<uint32> Quests;
    };

    inline AssaultInfo const Assaults[] =
    {
        { TYPE_IRON_HORDE,     { 36678, 36693, 36818, 36819, 37629, 37630, 38181, 38195 } },   // Mok'gol Watchpost
        { TYPE_IRON_HORDE,     { 36674, 36696, 36822, 36823, 37585, 37622, 38178, 38191 } },   // Iron Siegeworks
        { TYPE_IRON_HORDE,     { 36383, 36489, 36686, 36701, 40941, 40942 } },                 // the Pit
        { TYPE_IRON_HORDE,     { 36684, 36698 } },                                             // Battle in Ashran
        { TYPE_SHADOW_COUNCIL, { 36338, 36649, 36667, 36808, 37635, 37636, 38187, 38201 } },   // Shattrath Harbor
        { TYPE_SHADOW_COUNCIL, { 36685, 36699, 36807, 36809, 40943, 40944 } },                 // Heart of Shattrath
        { TYPE_OGRE,           { 35637, 35641, 36648, 36669, 37623, 37624, 38176, 38189 } },   // Stonefury Cliffs
        { TYPE_OGRE,           { 36820, 36821, 37523, 37524, 38194, 38202, 38204, 38205 } },   // Gorian Proving Grounds
        { TYPE_OGRE,           { 36687, 36700 } },                                             // Challenge at the Ring of Blood
        { TYPE_PRIMALS,        { 36681, 36690, 36860, 38184, 38198 } },                        // Lost Veil Anzu
        { TYPE_PRIMALS,        { 36676, 36695, 36814, 36815, 38179, 38192 } },                 // Everbloom Wilds
        { TYPE_GOREN,          { 36675, 36697, 36826, 36827, 37625, 37626, 38177, 38190 } },   // Magnarok
        { TYPE_GOREN,          { 36677, 36694, 36816, 36817, 37627, 37628, 38180, 38193 } },   // Broken Precipice
        { TYPE_SHADOWMOON,     { 35974, 35977, 36679, 36692, 37633, 37634, 38182, 38196 } },   // Darktide Roost
        { TYPE_SHADOWMOON,     { 36490, 36491, 36682, 36689, 38185, 38199 } },                 // Pillars of Fate
        { TYPE_SHADOWMOON,     { 36069, 36070, 36683, 36688, 38186, 38200 } },                 // Skettis
        { TYPE_SHADOWMOON,     { 36347, 36348, 36680, 36691, 37631, 37632, 38183, 38197 } },   // Socrethar's Rise
    };

    inline Type GetTypeByAssaultQuest(uint32 questId)
    {
        for (AssaultInfo const& assault : Assaults)
            if (std::find(assault.Quests.begin(), assault.Quests.end(), questId) != assault.Quests.end())
                return assault.InvasionType;

        return MAX_TYPES;
    }

    inline Type GetTypeByInvasionQuest(uint32 questId)
    {
        for (uint8 type = 0; type < MAX_TYPES; ++type)
            if (Types[type].QuestAlliance == questId || Types[type].QuestHorde == questId)
                return Type(type);

        return MAX_TYPES;
    }
}

#endif // GarrisonInvasion_h__
