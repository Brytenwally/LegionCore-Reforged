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

#ifndef __GAMETIME_H
#define __GAMETIME_H

#include "Define.h"
#include "Duration.h"

namespace GameTime
{
    // Server start time
    TC_GAME_API time_t GetStartTime();

    // Current server time (unix) in seconds
    TC_GAME_API time_t GetGameTime();

    // Milliseconds since server start
    TC_GAME_API uint32 GetGameTimeMS();

    /// Current chrono system_clock time point
    TC_GAME_API SystemTimePoint GetSystemTime();

    /// Current chrono steady_clock time point
    TC_GAME_API TimePoint Now();

    /// Current chrono Clock time point
    template<typename Clock>
    typename Clock::time_point GetTime();

    /// Uptime (in secs)
    TC_GAME_API uint32 GetUptime();

    void UpdateGameTimers();
};

#endif
