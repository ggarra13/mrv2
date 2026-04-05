// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.
// Copyright Contributors to the feather-tk project.

#include <tlCore/Size.h>
#include <tlCore/String.h>

#include <sstream>

namespace tl
{
    namespace math
    {
    
        std::string to_string(const Size2i& value)
        {
            std::stringstream ss;
            ss << value.w << " " << value.h;
            return ss.str();
        }

        std::string to_string(const Size2f& value)
        {
            std::stringstream ss;
            ss << value.w << " " << value.h;
            return ss.str();
        }

        std::string to_string(const Size3f& value)
        {
            std::stringstream ss;
            ss << value.w << " " << value.h << " " << value.d;
            return ss.str();
        }

        bool from_string(const std::string& s, Size2i& value)
        {
            bool out = false;
            const auto pieces = string::split(s, ' ');
            if (2 == pieces.size())
            {
                value.w = std::atoi(pieces[0].c_str());
                value.h = std::atoi(pieces[1].c_str());
                out = true;
            }
            return out;
        }

        bool from_string(const std::string& s, Size2f& value)
        {
            bool out = false;
            const auto pieces = string::split(s, ' ');
            if (2 == pieces.size())
            {
                value.w = std::atof(pieces[0].c_str());
                value.h = std::atof(pieces[1].c_str());
                out = true;
            }
            return out;
        }

        bool from_string(const std::string& s, Size3f& value)
        {
            bool out = false;
            const auto pieces = string::split(s, ' ');
            if (3 == pieces.size())
            {
                value.w = std::atof(pieces[0].c_str());
                value.h = std::atof(pieces[1].c_str());
                value.d = std::atof(pieces[2].c_str());
                out = true;
            }
            return out;
        }

        void to_json(nlohmann::json& json, const Size2i& value)
        {
            json = { value.w, value.h };
        }

        void to_json(nlohmann::json& json, const Size2f& value)
        {
            json = { value.w, value.h };
        }

        void to_json(nlohmann::json& json, const Size3f& value)
        {
            json = { value.w, value.h , value.d };
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

        void from_json(const nlohmann::json& json, Size3f& value)
        {
            json.at(0).get_to(value.w);
            json.at(1).get_to(value.h);
            json.at(2).get_to(value.d);
        }
    }
}
