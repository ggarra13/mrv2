// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2024-Present Gonzalo Garramuño
// All rights reserved.

namespace tl
{
    namespace timeline
    {
        inline bool HDROptions::operator==(const HDROptions& other) const
        {
            // passthru is ignored on purpose
            return (tonemap == other.tonemap &&
                    peak_detection == other.peak_detection &&
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
