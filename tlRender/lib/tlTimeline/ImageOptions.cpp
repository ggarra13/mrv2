// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlTimeline/ImageOptions.h>

#include <tlCore/Error.h>
#include <tlCore/String.h>

#include <algorithm>
#include <array>

namespace tl
{
    namespace timeline
    {
        TLRENDER_ENUM_IMPL(
            InputVideoLevels, "FromFile", "FullRange", "LegalRange");
        TLRENDER_ENUM_SERIALIZE_IMPL(InputVideoLevels);

        TLRENDER_ENUM_IMPL(AlphaBlend, "None", "Straight", "Premultiplied");
        TLRENDER_ENUM_SERIALIZE_IMPL(AlphaBlend);

        TLRENDER_ENUM_IMPL(ImageFilter, "Nearest", "Linear");
        TLRENDER_ENUM_SERIALIZE_IMPL(ImageFilter);
    } // namespace timeline
} // namespace tl
