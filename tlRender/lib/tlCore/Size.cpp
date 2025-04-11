// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlCore/Size.h>

#include <tlCore/Error.h>
#include <tlCore/String.h>

#include <sstream>

namespace tl
{
    namespace math
    {
        void to_json(nlohmann::json& json, const Size2i& value)
        {
            json = {value.w, value.h};
        }

        void to_json(nlohmann::json& json, const Size2f& value)
        {
            json = {value.w, value.h};
        }

        void from_json(const nlohmann::json& json, Size2i& value)
        {
            json.at(0).get_to(value.w);
            json.at(1).get_to(value.h);
        }

        void from_json(const nlohmann::json& json, Size2f& value)
        {
            json.at(0).get_to(value.w);
            json.at(1).get_to(value.h);
        }

        std::ostream& operator<<(std::ostream& os, const Size2i& value)
        {
            os << value.w << "x" << value.h;
            return os;
        }

        std::ostream& operator<<(std::ostream& os, const Size2f& value)
        {
            os << value.w << "x" << value.h;
            return os;
        }

        std::istream& operator>>(std::istream& is, Size2i& value)
        {
            std::string s;
            is >> s;
            auto split = string::split(s, 'x');
            if (split.size() != 2)
            {
                throw error::ParseError();
            }
            {
                std::stringstream ss(split[0]);
                ss >> value.w;
            }
            {
                std::stringstream ss(split[1]);
                ss >> value.h;
            }
            return is;
        }

        std::istream& operator>>(std::istream& is, Size2f& value)
        {
            std::string s;
            is >> s;
            auto split = string::split(s, 'x');
            if (split.size() != 2)
            {
                throw error::ParseError();
            }
            {
                std::stringstream ss(split[0]);
                ss >> value.w;
            }
            {
                std::stringstream ss(split[1]);
                ss >> value.h;
            }
            return is;
        }
    } // namespace math
} // namespace tl
