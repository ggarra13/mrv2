// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlCore/Color.h>

#include <tlCore/Error.h>
#include <tlCore/String.h>

#include <sstream>

namespace tl
{
    namespace image
    {
        void to_json(nlohmann::json& json, const Color4f& value)
        {
            json = {value.r, value.g, value.b, value.a};
        }

        void from_json(const nlohmann::json& json, Color4f& value)
        {
            json.at(0).get_to(value.r);
            json.at(1).get_to(value.g);
            json.at(2).get_to(value.b);
            json.at(3).get_to(value.a);
        }

        std::ostream& operator<<(std::ostream& os, const Color4f& value)
        {
            os << value.r << "," << value.g << "," << value.b << "," << value.a;
            return os;
        }

        std::istream& operator>>(std::istream& is, Color4f& value)
        {
            std::string s;
            is >> s;
            auto split = string::split(s, ',');
            if (split.size() != 4)
            {
                throw error::ParseError();
            }
            {
                std::stringstream ss(split[0]);
                ss >> value.r;
            }
            {
                std::stringstream ss(split[1]);
                ss >> value.g;
            }
            {
                std::stringstream ss(split[2]);
                ss >> value.b;
            }
            {
                std::stringstream ss(split[3]);
                ss >> value.a;
            }
            return is;
        }
    } // namespace image
} // namespace tl
