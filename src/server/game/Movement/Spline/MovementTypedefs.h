////////////////////////////////////////////
///                                      ///
///              DEMONCORE               ///
///    Copyright 2026 Titans-Project     ///
///         All Rights Reserved.         ///
///                                      ///
////////////////////////////////////////////

#ifndef TRINITYSERVER_TYPEDEFS_H
#define TRINITYSERVER_TYPEDEFS_H

#include "Common.h"
#include <atomic>
#include <limits>

enum MonsterMoveType
{
    MONSTER_MOVE_NORMAL         = 0,
    MONSTER_MOVE_FACING_SPOT    = 1,
    MONSTER_MOVE_FACING_TARGET  = 2,
    MONSTER_MOVE_FACING_ANGLE   = 3
};

namespace Movement
{
    inline uint32 SecToMS(float sec)
    {
        return static_cast<uint32>(sec * 1000.f);
    }

    inline float MSToSec(uint32 ms)
    {
        return ms / 1000.f;
    }

    float computeFallTime(float path_length, bool isSafeFall);
    float computeFallElevation(float t_passed, bool isSafeFall, float start_velocity = 0.0f);

    // Shared by every map thread, so increments must be atomic. Wrap-around at the type maximum is the natural overflow.
    template<class T, T limit>
    class counter
    {
        static_assert(limit == std::numeric_limits<T>::max(), "counter relies on natural overflow to wrap");

    public:
        counter() : m_counter(0) { }

        void Increase() { ++m_counter; }

        T NewId() { return ++m_counter; }
        T getCurrent() const { return m_counter.load(); }

    private:
        std::atomic<T> m_counter;
    };

    typedef counter<uint32, 0xFFFFFFFF> UInt32Counter;

    float constexpr gravity = static_cast<float>(19.29110527038574);
    extern UInt32Counter splineIdGen;
    extern std::string MovementFlagsToString(uint32 flags);
    extern std::string MovementFlags2ToString(uint32 flags);
}

#endif // TRINITYSERVER_TYPEDEFS_H
