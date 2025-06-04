// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

namespace tl
{
    namespace timeline
    {
        inline bool
        BackgroundOptions::operator==(const BackgroundOptions& other) const
        {
            return type == other.type && color0 == other.color0 &&
                   color1 == other.color1 && checkersSize == other.checkersSize;
        }

        inline bool
        BackgroundOptions::operator!=(const BackgroundOptions& other) const
        {
            return !(*this == other);
        }
    } // namespace timeline
} // namespace tl
