// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

namespace tl
{
    namespace timeline
    {
        inline bool
        PlayerCacheInfo::operator==(const PlayerCacheInfo& other) const
        {
            return videoPercentage == other.videoPercentage &&
                   videoFrames == other.videoFrames &&
                   audioFrames == other.audioFrames;
        }

        inline bool
        PlayerCacheInfo::operator!=(const PlayerCacheInfo& other) const
        {
            return !(*this == other);
        }
    } // namespace timeline
} // namespace tl
