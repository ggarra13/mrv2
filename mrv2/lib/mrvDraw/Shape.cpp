// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include "mrvDraw/Shape.h"

namespace tl
{
    namespace draw
    {
        void to_json(nlohmann::json& j, const Shape& value)
        {
            nlohmann::json matrix(value.matrix);
            nlohmann::json color(value.color);
            j["order"] = value.order;
            j["matrix"] = matrix;
            j["color"] = color;
            j["pen_size"] = value.pen_size;
        }

        void from_json(const nlohmann::json& j, Shape& value)
        {
            j.at("order").get_to(value.order);
            j.at("matrix").get_to(value.matrix);
            j.at("color").get_to(value.color);
            j.at("pen_size").get_to(value.pen_size);
        }

        void to_json(nlohmann::json& j, const PathShape& value)
        {
            to_json(j, static_cast<const Shape&>(value));
            nlohmann::json pnts(value.pts);
            j["pts"] = pnts;
        }

        void from_json(const nlohmann::json& j, PathShape& value)
        {
            from_json(j, static_cast<Shape&>(value));
            j.at("pts").get_to(value.pts);
        }

    } // namespace draw
} // namespace tl
