// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlUI/Style.h>

namespace tl
{
    namespace play_app
    {
        //! Style palettes.
        enum class StylePalette {
            Dark,
            Light,

            Count,
            First = Dark
        };
        TLRENDER_ENUM(StylePalette);
        TLRENDER_ENUM_SERIALIZE(StylePalette);

        //! Get the style palette.
        std::map<ui::ColorRole, image::Color4f> getStylePalette(StylePalette);
    } // namespace play_app
} // namespace tl
