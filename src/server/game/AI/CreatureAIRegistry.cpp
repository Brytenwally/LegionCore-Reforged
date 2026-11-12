////////////////////////////////////////////
///                                      ///
///              DEMONCORE               ///
///    Copyright 2026 Titans-Project     ///
///         All Rights Reserved.         ///
///                                      ///
///      Based on TrinityCore code       ///
///   Copyright 2008-2012 TrinityCore    ///
///   <http://www.trinitycore.org/>      ///
///   Copyright 2005-2009 MaNGOS         ///
///   <http://getmangos.com/>            ///
///                                      ///
////////////////////////////////////////////

#include "PassiveAI.h"
#include "ReactorAI.h"
#include "CombatAI.h"
#include "GuardAI.h"
#include "PetAI.h"
#include "TotemAI.h"
#include "RandomMovementGenerator.h"
#include "MovementGeneratorImpl.h"
#include "CreatureAIRegistry.h"
#include "WaypointMovementGenerator.h"
#include "CreatureAIFactory.h"
#include "SmartAI.h"

#include <atomic>

namespace AIRegistry
{
    // Read from the map threads, which start before Initialize() runs.
    static std::atomic<bool> s_initialise(false);

    bool IsInitialized() { return s_initialise; }

    void Initialize()
    {
        (new CreatureAIFactory<NullCreatureAI>("NullCreatureAI"))->RegisterSelf();
        (new CreatureAIFactory<TriggerAI>("TriggerAI"))->RegisterSelf();
        (new CreatureAIFactory<AggressorAI>("AggressorAI"))->RegisterSelf();
        (new CreatureAIFactory<ReactorAI>("ReactorAI"))->RegisterSelf();
        (new CreatureAIFactory<PassiveAI>("PassiveAI"))->RegisterSelf();
        (new CreatureAIFactory<CritterAI>("CritterAI"))->RegisterSelf();
        (new CreatureAIFactory<GuardAI>("GuardAI"))->RegisterSelf();
        (new CreatureAIFactory<PetAI>("PetAI"))->RegisterSelf();
        (new CreatureAIFactory<AnyPetAI>("AnyPetAI"))->RegisterSelf();
        (new CreatureAIFactory<TotemAI>("TotemAI"))->RegisterSelf();
        (new CreatureAIFactory<CombatAI>("CombatAI"))->RegisterSelf();
        (new CreatureAIFactory<CasterAI>("CasterAI"))->RegisterSelf();
        (new CreatureAIFactory<ArcherAI>("ArcherAI"))->RegisterSelf();
        (new CreatureAIFactory<TurretAI>("TurretAI"))->RegisterSelf();
        (new CreatureAIFactory<VehicleAI>("VehicleAI"))->RegisterSelf();
        (new CreatureAIFactory<SmartAI>("SmartAI"))->RegisterSelf();
        (new CreatureAIFactory<BattlePetAI>("BattlePetAI"))->RegisterSelf();

        (new GameObjectAIFactory<GameObjectAI>("GameObjectAI"))->RegisterSelf();
        (new GameObjectAIFactory<SmartGameObjectAI>("SmartGameObjectAI"))->RegisterSelf();

        (new IdleMovementFactory())->RegisterSelf();
        (new MovementGeneratorFactory<RandomMovementGenerator<Creature> >(RANDOM_MOTION_TYPE))->RegisterSelf();
        (new MovementGeneratorFactory<WaypointMovementGenerator<Creature> >(WAYPOINT_MOTION_TYPE))->RegisterSelf();

        s_initialise = true;
    }
}

