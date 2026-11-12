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

#ifndef TRINITY_RANDOMMOTIONGENERATOR_H
#define TRINITY_RANDOMMOTIONGENERATOR_H

#include "MovementGenerator.h"
#include "PathGenerator.h"
#include "Timer.h"

template<class T>
class RandomMovementGenerator : public MovementGeneratorMedium<T, RandomMovementGenerator<T>>
{
    public:
        explicit RandomMovementGenerator(float distance = 0.0f) : _path(nullptr), _timer(0), _reference(0.f, 0.f, 0.f), _requestedDistance(distance), _wanderDistance(distance), _wanderSteps(0), _stalled(false), _airborne(false) { }

        MovementGeneratorType GetMovementGeneratorType() const override { return RANDOM_MOTION_TYPE; }

        void Pause(uint32 timer = 0) override;
        void Resume(uint32 overrideTimer = 0) override;

        void DoInitialize(T&);
        void DoFinalize(T&);
        void DoReset(T&);
        bool DoUpdate(T&, uint32);

    private:
        void SetRandomLocation(T&);

        std::unique_ptr<PathGenerator> _path;
        TimeTracker _timer;
        Position _reference;
        float _requestedDistance; // 0 = use the creature's respawn radius
        float _wanderDistance;
        uint8 _wanderSteps;
        bool _stalled;
        bool _airborne;     // flying wanderer: each leg is chained before the previous one ends
};
#endif

