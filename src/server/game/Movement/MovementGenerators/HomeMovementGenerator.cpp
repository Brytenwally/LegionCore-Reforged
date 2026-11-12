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

#include "HomeMovementGenerator.h"
#include "Creature.h"
#include "CreatureAI.h"
#include "MoveSpline.h"
#include "MoveSplineInit.h"
#include "WorldPacket.h"
#include "Vehicle.h"

template<class T>
HomeMovementGenerator<T>::~HomeMovementGenerator() { }

template<>
HomeMovementGenerator<Creature>::~HomeMovementGenerator()
{
    delete _path;
}

template<class T>
void HomeMovementGenerator<T>::SetTargetLocation(T&) { }

template<>
void HomeMovementGenerator<Creature>::SetTargetLocation(Creature& owner)
{
    if (owner.HasUnitState(UNIT_STATE_ROOT | UNIT_STATE_STUNNED | UNIT_STATE_DISTRACTED))
    { // if we are ROOT/STUNNED/DISTRACTED even after aura clear, finalize on next update - otherwise we would get stuck in evade
        _skipToHome = true;
        return;
    }

    Movement::MoveSplineInit init(owner);
    float x, y, z, o;
    // The generator on top is this one, so asking it for a reset position never gave anything but the home position
    owner.GetHomePosition(x, y, z, o);
    init.SetFacing(o);
    owner.UpdateAllowedPositionZ(x, y, z);
    // Pathed, as in TrinityCore: MoveTo does not path by default here, so evading creatures walked
    // back to their spawn in a straight line, through walls. The destination is forced so that a path
    // cut short (spawn off the navmesh, on a roof or a bridge) still ends at the spawn; with no usable
    // path at all it goes straight, as before.
    init.MoveTo(x, y, z, true, true);
    init.SetWalk(false);
    init.Launch();

    _skipToHome = false;
    _arrived = false;

    owner.ClearUnitState(UNIT_STATE_ALL_STATE & ~UNIT_STATE_EVADE);
}

template<class T>
void HomeMovementGenerator<T>::DoInitialize(T& ) { }

template<>
void HomeMovementGenerator<Creature>::DoInitialize(Creature& owner)
{
    owner.SetNoSearchAssistance(false);
    SetTargetLocation(owner);
}

template<class T>
void HomeMovementGenerator<T>::DoFinalize(T& ) { }

template<>
void HomeMovementGenerator<Creature>::DoFinalize(Creature& owner)
{
    if (_arrived)
    {
        owner.ClearUnitState(UNIT_STATE_EVADE);
        owner.SetWalk(true);
        owner.LoadCreaturesAddon(true);
        if (owner.IsVehicle())
            owner.GetVehicleKit()->Reset(true);
        if (owner.AI())
            owner.AI()->JustReachedHome();
        owner.SetSpawnHealth();
    }
}

template<class T>
void HomeMovementGenerator<T>::DoReset(T &) { }

template<>
void HomeMovementGenerator<Creature>::DoReset(Creature& owner)
{
    DoInitialize(owner);
}

template<class T>
bool HomeMovementGenerator<T>::DoUpdate(T&, uint32)
{
    return false;
}

template<>
bool HomeMovementGenerator<Creature>::DoUpdate(Creature& owner, uint32 /*diff*/)
{
    _arrived = _skipToHome || owner.movespline->Finalized();
    return !_arrived;
}
