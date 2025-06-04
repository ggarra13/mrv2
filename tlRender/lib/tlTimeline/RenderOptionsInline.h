// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

namespace tl
{
    namespace timeline
    {
        inline bool RenderOptions::operator==(const RenderOptions& other) const
        {
            return clear == other.clear && clearColor == other.clearColor &&
                   colorBuffer == other.colorBuffer &&
                   textureCacheByteCount == other.textureCacheByteCount;
        }

        inline bool RenderOptions::operator!=(const RenderOptions& other) const
        {
            return !(*this == other);
        }
    } // namespace timeline
} // namespace tl
