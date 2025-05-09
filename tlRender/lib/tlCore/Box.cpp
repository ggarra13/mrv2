// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlCore/Box.h>

#include <tlCore/Error.h>
#include <tlCore/String.h>

#include <sstream>

namespace tl
{
    namespace math
    {
        Box2i operator*(const Box2i& a, float b)
        {
            Box2i out;
            out.min = a.min * b;
            out.max = a.max * b;
            return out;
        }

        Box2f operator*(const Box2f& a, float b)
        {
            Box2f out;
            out.min = a.min * b;
            out.max = a.max * b;
            return out;
        }

        void to_json(nlohmann::json& json, const Box2i& value)
        {
            json = {value.min, value.max};
        }

        void to_json(nlohmann::json& json, const Box2f& value)
        {
            json = {value.min, value.max};
        }

        void from_json(const nlohmann::json& json, Box2i& value)
        {
            json.at(0).get_to(value.min);
            json.at(1).get_to(value.max);
        }

        void from_json(const nlohmann::json& json, Box2f& value)
        {
            json.at(0).get_to(value.min);
            json.at(1).get_to(value.max);
        }

        std::ostream& operator<<(std::ostream& os, const Box2i& value)
        {
            os << value.min.x << "," << value.min.y << "*" << value.max.x << ","
               << value.max.y;
            return os;
        }

        std::ostream& operator<<(std::ostream& os, const Box2f& value)
        {
            os << value.min.x << "," << value.min.y << "*" << value.max.x << ","
               << value.max.y;
            return os;
        }

        std::istream& operator>>(std::istream& is, Box2i& value)
        {
            std::string s;
            is >> s;
            auto split = string::split(s, '*');
            if (split.size() != 2)
            {
                throw error::ParseError();
            }
            {
                std::stringstream ss(split[0]);
                ss >> value.min;
            }
            {
                std::stringstream ss(split[1]);
                ss >> value.max;
            }
            return is;
        }

        std::istream& operator>>(std::istream& is, Box2f& value)
        {
            std::string s;
            is >> s;
            auto split = string::split(s, '*');
            if (split.size() != 2)
            {
                throw error::ParseError();
            }
            {
                std::stringstream ss(split[0]);
                ss >> value.min;
            }
            {
                std::stringstream ss(split[1]);
                ss >> value.max;
            }
            return is;
        }
    } // namespace math
} // namespace tl
