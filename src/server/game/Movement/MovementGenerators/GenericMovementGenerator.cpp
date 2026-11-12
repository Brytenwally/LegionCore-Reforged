////////////////////////////////////////////
///                                      ///
///              DEMONCORE               ///
///    Copyright 2026 Titans-Project     ///
///         All Rights Reserved.         ///
///                                      ///
////////////////////////////////////////////

#include "GenericMovementGenerator.h"
#include "Creature.h"
#include "CreatureAI.h"
#include "MoveSpline.h"
#include "ObjectAccessor.h"
#include "Unit.h"

void GenericMovementGenerator::Initialize(Unit& /*owner*/)
{
    _duration.Reset(_splineInit.Launch());
    _launched = true;
}

bool GenericMovementGenerator::Update(Unit& owner, uint32 diff)
{
    _duration.Update(diff);
    if (_duration.Passed())
        return false;

    return !owner.movespline->Finalized();
}

void GenericMovementGenerator::Finalize(Unit& owner)
{
    // Removed before it was launched, or while its spline is still running (replaced/cleared): no arrival
    if (_launched && (_duration.Passed() || owner.movespline->Finalized()))
        MovementInform(owner);
}

void GenericMovementGenerator::MovementInform(Unit& owner)
{
    if (_arrivalSpellId)
        owner.CastSpell(ObjectAccessor::GetUnit(owner, _arrivalSpellTargetGuid), _arrivalSpellId, true);

    if (Creature* creature = owner.ToCreature())
        if (creature->AI())
            creature->AI()->MovementInform(_type, _pointId);
}