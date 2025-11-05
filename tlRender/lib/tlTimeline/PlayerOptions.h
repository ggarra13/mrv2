// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlCore/Time.h>

namespace tl
{
    namespace timeline
    {
        //! Playback modes.
        enum class Playback {
            Stop,
            Forward,
            Reverse,

            Count,
            First = Stop
        };
        TLRENDER_ENUM(Playback);
        TLRENDER_ENUM_SERIALIZE(Playback);

        //! Timer modes.
        enum class TimerMode {
            System,
            Audio,

            Count,
            First = System
        };
        TLRENDER_ENUM(TimerMode);
        TLRENDER_ENUM_SERIALIZE(TimerMode);

        //! Timeline player cache options.
        struct PlayerCacheOptions
        {
            //! Cache read ahead.
            otime::RationalTime readAhead = otime::RationalTime(5.0, 1.0);

            //! Cache read behind.
            otime::RationalTime readBehind = otime::RationalTime(0.5, 1.0);

            bool operator==(const PlayerCacheOptions&) const;
            bool operator!=(const PlayerCacheOptions&) const;
        };

        //! Timeline player options.
        struct PlayerOptions
        {
            //! Cache options.
            PlayerCacheOptions cache;

            //! Timer mode.
            TimerMode timerMode = TimerMode::System;

            //! Audio buffer frame count.
            size_t audioBufferFrameCount = 2048;

            //! Timeout for muting the audio when playback stutters.
            std::chrono::milliseconds muteTimeout =
                std::chrono::milliseconds(500);

            //! Timeout to sleep each tick.
            std::chrono::milliseconds sleepTimeout =
                std::chrono::milliseconds(5);

            //! Current time.
            otime::RationalTime currentTime = time::invalidTime;

            //! Start playback direction.
            Playback playback = Playback::Forward;

            bool operator==(const PlayerOptions&) const;
            bool operator!=(const PlayerOptions&) const;
        };
    } // namespace timeline
} // namespace tl

#include <tlTimeline/PlayerOptionsInline.h>
