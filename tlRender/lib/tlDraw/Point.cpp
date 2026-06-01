// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include "tlDraw/Point.h"

namespace tl
{
    namespace draw
    {
        void to_json(nlohmann::json& json, const Point& value)
        {
            json = nlohmann::json{
                {"x", value.x},
                {"y", value.y},
                {"pressure", value.pressure},
            };
        }

        void from_json(const nlohmann::json& json, Point& value)
        {
            json.at("x").get_to(value.x);
            json.at("y").get_to(value.y);
            // Backward-compatible: old files without "pressure" default to 1.0
            if (json.contains("pressure"))
                json.at("pressure").get_to(value.pressure);
            else
                value.pressure = 1.0f;
        }
    } // namespace draw
} // namespace tl
