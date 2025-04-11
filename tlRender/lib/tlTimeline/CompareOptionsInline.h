// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

namespace tl
{
    namespace timeline
    {
        inline bool
        CompareOptions::operator==(const CompareOptions& other) const
        {
            return mode == other.mode && wipeCenter == other.wipeCenter &&
                   wipeRotation == other.wipeRotation &&
                   overlay == other.overlay;
        }

        inline bool
        CompareOptions::operator!=(const CompareOptions& other) const
        {
            return !(*this == other);
        }
    } // namespace timeline
} // namespace tl
