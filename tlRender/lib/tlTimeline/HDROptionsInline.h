// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

namespace tl
{
    namespace timeline
    {
        inline bool HDROptions::operator==(const HDROptions& other) const
        {
            // passthru is ignored on purpose
            return (tonemap == other.tonemap &&
                    algorithm == other.algorithm &&
                    hdrData == other.hdrData);
        }

        inline bool HDROptions::operator!=(const HDROptions& other) const
        {
            return !(*this == other);
        }
    } // namespace timeline
} // namespace tl
