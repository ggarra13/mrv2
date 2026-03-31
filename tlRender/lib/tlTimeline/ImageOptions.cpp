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
            InputVideoLevels, "FromFile", "FullRange", "LegalRange",
            "LegalRangeHDR");
        TLRENDER_ENUM_SERIALIZE_IMPL(InputVideoLevels);

        TLRENDER_ENUM_IMPL(AlphaBlend, "None", "Straight", "Premultiplied");
        TLRENDER_ENUM_SERIALIZE_IMPL(AlphaBlend);

        TLRENDER_ENUM_IMPL(ImageFilter, "Nearest", "Linear");
        TLRENDER_ENUM_SERIALIZE_IMPL(ImageFilter);
        
        void to_json(nlohmann::json& j, const ImageFilters& value)
        {
            j["minify"] = value.minify;
            j["magnify"] = value.magnify;
        }

        void from_json(const nlohmann::json& j, ImageFilters& value)
        {
            j.at("minify").get_to(value.minify);
            j.at("magnify").get_to(value.magnify);
        }

        void to_json(nlohmann::json& j, const ImageOptions& value)
        {
            nlohmann::json imageFilters(value.imageFilters);
            j["videoLevels"] = value.videoLevels;
            j["alphaBlend"] = value.alphaBlend;
            j["imageFilters"] = value.imageFilters;
        }

        void from_json(const nlohmann::json& j, ImageOptions& value)
        {
            j.at("videoLevels").get_to(value.videoLevels);
            j.at("alphaBlend").get_to(value.alphaBlend);
            j.at("imageFilters").get_to(value.imageFilters);
        }
    } // namespace timeline
} // namespace tl
