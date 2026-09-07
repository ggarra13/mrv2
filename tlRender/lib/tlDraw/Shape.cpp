// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.


#include <tlDraw/Shape.h>

#include <cstdio>
#include <ctime>

namespace tl
{
    std::string current_timestamp_string()
    {
        std::time_t t = std::time(nullptr);
        std::tm tmv;
#if defined(_WIN32)
        localtime_s(&tmv, &t);
#else
        localtime_r(&t, &tmv);
#endif
        char buf[32];
        std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M", &tmv);
        return std::string(buf);
    }

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
            j.at("soft").get_to(value.soft);
            j.at("laser").get_to(value.laser);
            j.at("fade").get_to(value.fade);
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
            j["text"] = value.text;
            j["color"] = value.color;
            j["date"] = value.date;
            j["type"] = "Note";
        }

        void from_json(const nlohmann::json& j, NoteShape& value)
        {
            from_json(j, static_cast<Shape&>(value));
            j.at("text").get_to(value.text);
            if (j.contains("color"))
                j.at("color").get_to(value.color);
            else
                value.color = FL_RED;
            if (j.contains("date"))
                j.at("date").get_to(value.date);
            else
                value.date = current_timestamp_string();
        }

    } // namespace draw
} // namespace tl
