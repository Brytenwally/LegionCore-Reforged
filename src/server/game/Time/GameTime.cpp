////////////////////////////////////////////
///                                      ///
///              DEMONCORE               ///
///    Copyright 2026 Titans-Project     ///
///         All Rights Reserved.         ///
///                                      ///
///      Based on TrinityCore code       ///
///   Copyright 2008-2017 TrinityCore    ///
///   <http://www.trinitycore.org/>      ///
///                                      ///
////////////////////////////////////////////

#include "GameTime.h"
#include "Timer.h"
#include "Util.h"
#include <atomic>

namespace GameTime
{
    time_t const StartTime = time(nullptr);

    // written by the world thread, read by every map thread
    std::atomic<time_t> GameTime(time(nullptr));
    std::atomic<uint32> GameMSTime(0);

    std::atomic<SystemTimePoint> GameTimeSystemPoint(std::chrono::system_clock::now());
    std::atomic<TimePoint> GameTimeSteadyPoint(std::chrono::steady_clock::now());

    time_t GetStartTime()
    {
        return StartTime;
    }

    time_t GetGameTime()
    {
        return GameTime;
    }

    uint32 GetGameTimeMS()
    {
        return GameMSTime;
    }

    SystemTimePoint GetSystemTime()
    {
        return GameTimeSystemPoint;
    }

    TimePoint Now()
    {
        return GameTimeSteadyPoint;
    }

    template<>
    SystemTimePoint GetTime<std::chrono::system_clock>()
    {
        return GetSystemTime();
    }

    template<>
    TimePoint GetTime<std::chrono::steady_clock>()
    {
        return Now();
    }

    uint32 GetUptime()
    {
        return uint32(GameTime.load(std::memory_order_relaxed) - StartTime);
    }

    void UpdateGameTimers()
    {
        GameTime.store(time(nullptr), std::memory_order_relaxed);
        GameMSTime.store(getMSTime(), std::memory_order_relaxed);
        GameTimeSystemPoint.store(std::chrono::system_clock::now(), std::memory_order_relaxed);
        GameTimeSteadyPoint.store(std::chrono::steady_clock::now(), std::memory_order_relaxed);
    }
}
