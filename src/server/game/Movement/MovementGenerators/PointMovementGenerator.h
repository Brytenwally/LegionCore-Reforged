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

#ifndef TRINITY_POINTMOVEMENTGENERATOR_H
#define TRINITY_POINTMOVEMENTGENERATOR_H

#include "FollowerReference.h"
#include "MovementGenerator.h"
#include "PathGenerator.h"
#include "Spell.h"

template<class T>
class PointMovementGenerator : public MovementGeneratorMedium<T, PointMovementGenerator<T>>
{
    public:
        static constexpr float NO_ORIENTATION = -1000.0f;

        // The face target and the effect data are kept by value/GUID: the generator can be (re)initialized after the spell or the target is gone
        explicit PointMovementGenerator(uint32 id, float x, float y, float z, bool generatePath, float speed = 0.0f, ObjectGuid const& faceTarget = ObjectGuid::Empty, Movement::SpellEffectExtraData const* spellEffectExtraData = nullptr, float finalOrientation = NO_ORIENTATION) : _movementId(id), _destination(x, y, z), _speed(speed), _faceTargetGuid(faceTarget), _finalOrientation(finalOrientation), _generatePath(generatePath), _recalculateSpeed(false), _interrupt(false)
        {
            if (spellEffectExtraData)
                _spellEffectExtra = *spellEffectExtraData;
        }

        MovementGeneratorType GetMovementGeneratorType() const override { return POINT_MOTION_TYPE; }

        void DoInitialize(T&);
        void DoFinalize(T&);
        void DoReset(T&);
        bool DoUpdate(T&, uint32);

        void UnitSpeedChanged() override { _recalculateSpeed = true; }

    private:
        void MovementInform(T&);

        uint32 _movementId;
        Position _destination;
        float _speed;
        ObjectGuid _faceTargetGuid;
        Optional<Movement::SpellEffectExtraData> _spellEffectExtra;
        float _finalOrientation;
        bool _generatePath;
        bool _recalculateSpeed;
        bool _interrupt;
};

class AssistanceMovementGenerator : public PointMovementGenerator<Creature>
{
    public:
        explicit AssistanceMovementGenerator(float _x, float _y, float _z) : PointMovementGenerator<Creature>(0, _x, _y, _z, true) { }

        MovementGeneratorType GetMovementGeneratorType() const override { return ASSISTANCE_MOTION_TYPE; }
        void Finalize(Unit&) override;
};

class EffectMovementGenerator : public MovementGenerator
{
    public:
        // arrivalMapId: a guarded jump (SkyholdDeparture.h) casts its arrival spell only on that map, alive
        explicit EffectMovementGenerator(uint32 id, uint32 arrivalSpellId = 0, ObjectGuid const& arrivalSpellTargetGuid = ObjectGuid::Empty, uint32 arrivalMapId = uint32(-1)) : _pointId(id), _arrivalSpellId(arrivalSpellId), _arrivalSpellTargetGuid(arrivalSpellTargetGuid), _arrivalMapId(arrivalMapId) { }

        MovementGeneratorType GetMovementGeneratorType() const override { return EFFECT_MOTION_TYPE; }

        void Initialize(Unit&) override { }
        void Finalize(Unit& unit) override;
        void Reset(Unit&) override { }
        bool Update(Unit& u, uint32) override;

    private:
        void MovementInform(Unit&);

        uint32 _pointId;
        uint32 _arrivalSpellId;
        ObjectGuid _arrivalSpellTargetGuid;
        uint32 _arrivalMapId;
};

#endif

