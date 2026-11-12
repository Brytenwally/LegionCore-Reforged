////////////////////////////////////////////
///                                      ///
///              DEMONCORE               ///
///    Copyright 2026 Titans-Project     ///
///         All Rights Reserved.         ///
///                                      ///
////////////////////////////////////////////

#ifndef TRINITY_GENERICMOVEMENTGENERATOR_H
#define TRINITY_GENERICMOVEMENTGENERATOR_H

#include "MovementGenerator.h"
#include "MoveSplineInit.h"
#include "Timer.h"

class Unit;

enum MovementGeneratorType : uint8;

class GenericMovementGenerator : public MovementGenerator
{
    public:
        explicit GenericMovementGenerator(Movement::MoveSplineInit&& splineInit, MovementGeneratorType type, uint32 id, uint32 arrivalSpellId = 0, ObjectGuid const& arrivalSpellTargetGuid = ObjectGuid::Empty)
            : _splineInit(std::move(splineInit)), _type(type), _pointId(id), _duration(0), _launched(false), _arrivalSpellId(arrivalSpellId), _arrivalSpellTargetGuid(arrivalSpellTargetGuid) { }

        void Initialize(Unit&) override;
        void Finalize(Unit&) override;
        void Reset(Unit&) override { }
        bool Update(Unit&, uint32) override;
        MovementGeneratorType GetMovementGeneratorType() const override { return _type; }

    private:
        void MovementInform(Unit&);

        Movement::MoveSplineInit _splineInit;
        MovementGeneratorType _type;
        uint32 _pointId;
        TimeTrackerSmall _duration;
        bool _launched;

        uint32 _arrivalSpellId;
        ObjectGuid _arrivalSpellTargetGuid;
};

#endif