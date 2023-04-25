// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include "mrvDraw/Point.h"

namespace tl
{
    namespace draw
    {
        void to_json(nlohmann::json& json, const Point& value)
        {
            json = nlohmann::json{
                {"x", value.x},
                {"y", value.y},
            };
        }

        void from_json(const nlohmann::json& json, Point& value)
        {
            json.at("x").get_to(value.x);
            json.at("y").get_to(value.y);
        }
    } // namespace draw
} // namespace tl
