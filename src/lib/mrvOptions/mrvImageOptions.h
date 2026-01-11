// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <nlohmann/json.hpp>

#include <tlCore/Image.h>

#include <tlTimeline/ImageOptions.h>

namespace tl
{
    namespace image
    {
        void to_json(nlohmann::json& j, const Mirror& value);

        void from_json(const nlohmann::json& j, Mirror& value);
    } // namespace image

    namespace timeline
    {

        void to_json(nlohmann::json& j, const ImageFilters& value);

        void from_json(const nlohmann::json& j, ImageFilters& value);

        void to_json(nlohmann::json& j, const ImageOptions& value);

        void from_json(const nlohmann::json& j, ImageOptions& value);
    } // namespace timeline
} // namespace tl
