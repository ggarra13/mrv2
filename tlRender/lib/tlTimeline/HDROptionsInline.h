// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2024-Present Gonzalo Garramuño
// All rights reserved.

namespace tl
{
    namespace timeline
    {
        inline bool HDROptions::operator==(const HDROptions& other) const
        {
            return (tonemap == other.tonemap &&
                    peak_detection == other.peak_detection &&
                    peak_percentile == other.peak_percentile &&
                    peak_smoothing_period == other.peak_smoothing_period &&
                    peak_scene_low_limit == other.peak_scene_low_limit &&
                    peak_scene_high_limit == other.peak_scene_high_limit &&
                    gamutMapping == other.gamutMapping &&
                    algorithm == other.algorithm &&
                    hdrData == other.hdrData);
        }

        inline bool HDROptions::operator!=(const HDROptions& other) const
        {
            return !(*this == other);
        }
    } // namespace timeline
} // namespace tl
