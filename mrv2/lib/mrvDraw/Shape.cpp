// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include "mrvDraw/Shape.h"
#include "mrvNetwork/mrvCypher.h"

namespace tl
{
    namespace draw
    {
        void to_json(nlohmann::json& j, const Shape& value)
        {
            nlohmann::json matrix(value.matrix);
            nlohmann::json color(value.color);
            j["matrix"] = matrix;
            j["color"] = color;
            j["pen_size"] = value.pen_size;
            j["soft"] = value.soft;
            j["laser"] = value.laser;
            j["fade"] = value.fade;
        }

        void from_json(const nlohmann::json& j, Shape& value)
        {
            j.at("matrix").get_to(value.matrix);
            j.at("color").get_to(value.color);
            j.at("pen_size").get_to(value.pen_size);
            if (j.contains("soft"))
                j.at("soft").get_to(value.soft);
            if (j.contains("laser"))
            {
                j.at("laser").get_to(value.laser);
                j.at("fade").get_to(value.fade);
            }
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

        void to_json(nlohmann::json& j, const NoteShape& value)
        {
            to_json(j, static_cast<const Shape&>(value));
            const std::string& plainText = value.text;
            std::string encodedText = mrv::encode_string(plainText);
            j["text"] = encodedText;
            j["type"] = "Note";
        }

        void from_json(const nlohmann::json& j, NoteShape& value)
        {
            from_json(j, static_cast<Shape&>(value));
            std::string encodedText;
            j.at("text").get_to(encodedText);
            value.text = mrv::decode_string(encodedText);
        }

    } // namespace draw
} // namespace tl
