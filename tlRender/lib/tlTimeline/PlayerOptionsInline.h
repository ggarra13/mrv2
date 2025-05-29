// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

namespace tl
{
    namespace timeline
    {
        inline bool
        PlayerCacheOptions::operator==(const PlayerCacheOptions& other) const
        {
            return readAhead == other.readAhead &&
                   readBehind == other.readBehind;
        }

        inline bool
        PlayerCacheOptions::operator!=(const PlayerCacheOptions& other) const
        {
            return !(*this == other);
        }

        inline bool PlayerOptions::operator==(const PlayerOptions& other) const
        {
            return cache == other.cache && timerMode == other.timerMode &&
                   audioBufferFrameCount == other.audioBufferFrameCount &&
                   muteTimeout == other.muteTimeout &&
                   sleepTimeout == other.sleepTimeout &&
                   currentTime == other.currentTime;
        }

        inline bool PlayerOptions::operator!=(const PlayerOptions& other) const
        {
            return !(*this == other);
        }
    } // namespace timeline
} // namespace tl
