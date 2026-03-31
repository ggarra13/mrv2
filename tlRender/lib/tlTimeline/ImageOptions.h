// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlCore/Util.h>

#include <nlohmann/json.hpp>

#include <iostream>
#include <string>
#include <vector>

namespace tl
{
    namespace timeline
    {
        //! Input video levels.
        enum class InputVideoLevels {
            FromFile,
            FullRange,
            LegalRange,
            LegalRangeHDR,

            Count,
            First = FromFile
        };
        TLRENDER_ENUM(InputVideoLevels);
        TLRENDER_ENUM_SERIALIZE(InputVideoLevels);

        //! Alpha channel blending.
        //!
        //! References:
        //! - https://microsoft.github.io/Win2D/html/PremultipliedAlpha.htm
        enum class AlphaBlend {
            kNone,
            Straight,
            Premultiplied,

            Count,
            First = kNone
        };
        TLRENDER_ENUM(AlphaBlend);
        TLRENDER_ENUM_SERIALIZE(AlphaBlend);

        //! Image filtering.
        enum class ImageFilter {
            Nearest,
            Linear,

            Count,
            First = Nearest
        };
        TLRENDER_ENUM(ImageFilter);
        TLRENDER_ENUM_SERIALIZE(ImageFilter);
        
        //! Image filters.
        struct ImageFilters
        {
            ImageFilter minify = ImageFilter::Linear;
            ImageFilter magnify = ImageFilter::Linear;

            bool operator==(const ImageFilters&) const;
            bool operator!=(const ImageFilters&) const;
        };

        void to_json(nlohmann::json& j, const ImageFilters& value);

        void from_json(const nlohmann::json& j, ImageFilters& value);

        //! Image options.
        struct ImageOptions
        {
            InputVideoLevels videoLevels = InputVideoLevels::FromFile;
            AlphaBlend alphaBlend = AlphaBlend::Straight;
            ImageFilters imageFilters;
            bool cache = true;

            bool operator==(const ImageOptions&) const;
            bool operator!=(const ImageOptions&) const;
        };

        void to_json(nlohmann::json& j, const ImageOptions& value);

        void from_json(const nlohmann::json& j, ImageOptions& value);
        
    } // namespace timeline
} // namespace tl

#include <tlTimeline/ImageOptionsInline.h>
