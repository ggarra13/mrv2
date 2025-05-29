// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlCore/Color.h>
#include <tlCore/Size.h>
#include <tlCore/Util.h>

namespace tl
{
    namespace timeline
    {
        //! Background type.
        enum class Background {
            Transparent,
            Solid,
            Checkers,
            Gradient,

            Count,
            First = Transparent
        };
        TLRENDER_ENUM(Background);
        TLRENDER_ENUM_SERIALIZE(Background);

        //! Background options.
        struct BackgroundOptions
        {
            Background type = Background::Transparent;
            image::Color4f color0 = image::Color4f(0.F, 0.F, 0.F);
            image::Color4f color1 = image::Color4f(0.F, 0.F, 0.F);
            math::Size2i checkersSize = math::Size2i(100, 100);

            bool operator==(const BackgroundOptions&) const;
            bool operator!=(const BackgroundOptions&) const;
        };

        //! \name Serialize
        ///@{

        void to_json(nlohmann::json&, const BackgroundOptions&);

        void from_json(const nlohmann::json&, BackgroundOptions&);

        ///@}
    } // namespace timeline
} // namespace tl

#include <tlTimeline/BackgroundOptionsInline.h>
