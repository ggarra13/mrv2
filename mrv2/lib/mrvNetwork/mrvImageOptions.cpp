// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include "mrvNetwork/mrvImageOptions.h"

namespace tl
{
    namespace image
    {
        void to_json(nlohmann::json& j, const Mirror& value)
        {
            j["x"] = value.x;
            j["y"] = value.y;
        }

        void from_json(const nlohmann::json& j, Mirror& value)
        {
            j.at("x").get_to(value.x);
            j.at("y").get_to(value.y);
        }
    } // namespace image

    namespace timeline
    {
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
