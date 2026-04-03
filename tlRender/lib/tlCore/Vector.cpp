// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlCore/String.h>
#include <tlCore/Vector.h>

#include <sstream>

namespace tl
{
    namespace math
    {
        std::string to_string(const Vector2i& value)
        {
            std::stringstream ss;
            ss << value.x << " " << value.y;
            return ss.str();
        }

        std::string to_string(const Vector2f& value)
        {
            std::stringstream ss;
            ss << value.x << " " << value.y;
            return ss.str();
        }

        std::string to_string(const Vector3f& value)
        {
            std::stringstream ss;
            ss << value.x << " " << value.y << " " << value.z;
            return ss.str();
        }

        std::string to_string(const Vector4f& value)
        {
            std::stringstream ss;
            ss << value.x << " " << value.y << " " << value.z << " " << value.w;
            return ss.str();
        }

        bool from_string(const std::string& s, Vector2i& value)
        {
            bool out = false;
            const auto pieces = string::split(s, ' ');
            if (2 == pieces.size())
            {
                value.x = std::atoi(pieces[0].c_str());
                value.y = std::atoi(pieces[1].c_str());
                out = true;
            }
            return out;
        }

        bool from_string(const std::string& s, Vector2f& value)
        {
            bool out = false;
            const auto pieces = string::split(s, ' ');
            if (2 == pieces.size())
            {
                value.x = std::atof(pieces[0].c_str());
                value.y = std::atof(pieces[1].c_str());
                out = true;
            }
            return out;
        }

        bool from_string(const std::string& s, Vector3f& value)
        {
            bool out = false;
            const auto pieces = string::split(s, ' ');
            if (3 == pieces.size())
            {
                value.x = std::atof(pieces[0].c_str());
                value.y = std::atof(pieces[1].c_str());
                value.z = std::atof(pieces[2].c_str());
                out = true;
            }
            return out;
        }

        bool from_string(const std::string& s, Vector4f& value)
        {
            bool out = false;
            const auto pieces = string::split(s, ' ');
            if (4 == pieces.size())
            {
                value.x = std::atof(pieces[0].c_str());
                value.y = std::atof(pieces[1].c_str());
                value.z = std::atof(pieces[2].c_str());
                value.w = std::atof(pieces[3].c_str());
                out = true;
            }
            return out;
        }

        void to_json(nlohmann::json& json, const Vector2i& value)
        {
            json = { value.x, value.y };
        }

        void to_json(nlohmann::json& json, const Vector2f& value)
        {
            json = { value.x, value.y };
        }

        void to_json(nlohmann::json& json, const Vector3f& value)
        {
            json = { value.x, value.y, value.z };
        }

        void to_json(nlohmann::json& json, const Vector4f& value)
        {
            json = { value.x, value.y, value.z, value.w };
        }

        void from_json(const nlohmann::json& json, Vector2i& value)
        {
            json.at(0).get_to(value.x);
            json.at(1).get_to(value.y);
        }

        void from_json(const nlohmann::json& json, Vector2f& value)
        {
            json.at(0).get_to(value.x);
            json.at(1).get_to(value.y);
        }

        void from_json(const nlohmann::json& json, Vector3f& value)
        {
            json.at(0).get_to(value.x);
            json.at(1).get_to(value.y);
            json.at(2).get_to(value.z);
        }

        void from_json(const nlohmann::json& json, Vector4f& value)
        {
            json.at(0).get_to(value.x);
            json.at(1).get_to(value.y);
            json.at(2).get_to(value.z);
            json.at(3).get_to(value.w);
        }
    }
}
