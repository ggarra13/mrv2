// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlCore/Vector.h>

#include <tlCore/Error.h>
#include <tlCore/String.h>

#include <sstream>

namespace tl
{
    namespace math
    {
        void to_json(nlohmann::json& json, const Vector2i& value)
        {
            json = {value.x, value.y};
        }

        void to_json(nlohmann::json& json, const Vector2f& value)
        {
            json = {value.x, value.y};
        }

        void to_json(nlohmann::json& json, const Vector3f& value)
        {
            json = {value.x, value.y, value.z};
        }

        void to_json(nlohmann::json& json, const Vector4f& value)
        {
            json = {value.x, value.y, value.z, value.w};
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

        std::ostream& operator<<(std::ostream& os, const Vector2i& value)
        {
            os << value.x << "," << value.y;
            return os;
        }

        std::ostream& operator<<(std::ostream& os, const Vector2f& value)
        {
            os << value.x << "," << value.y;
            return os;
        }

        std::ostream& operator<<(std::ostream& os, const Vector3f& value)
        {
            os << value.x << "," << value.y << "," << value.z;
            return os;
        }

        std::ostream& operator<<(std::ostream& os, const Vector4f& value)
        {
            os << value.x << "," << value.y << "," << value.z << "," << value.w;
            return os;
        }

        std::istream& operator>>(std::istream& is, Vector2i& value)
        {
            std::string s;
            is >> s;
            auto split = string::split(s, ',');
            if (split.size() != 2)
            {
                throw error::ParseError();
            }
            {
                std::stringstream ss(split[0]);
                ss >> value.x;
            }
            {
                std::stringstream ss(split[1]);
                ss >> value.y;
            }
            return is;
        }

        std::istream& operator>>(std::istream& is, Vector2f& value)
        {
            std::string s;
            is >> s;
            auto split = string::split(s, ',');
            if (split.size() != 2)
            {
                throw error::ParseError();
            }
            {
                std::stringstream ss(split[0]);
                ss >> value.x;
            }
            {
                std::stringstream ss(split[1]);
                ss >> value.y;
            }
            return is;
        }

        std::istream& operator>>(std::istream& is, Vector3f& value)
        {
            std::string s;
            is >> s;
            auto split = string::split(s, ',');
            if (split.size() != 3)
            {
                throw error::ParseError();
            }
            {
                std::stringstream ss(split[0]);
                ss >> value.x;
            }
            {
                std::stringstream ss(split[1]);
                ss >> value.y;
            }
            {
                std::stringstream ss(split[2]);
                ss >> value.z;
            }
            return is;
        }

        std::istream& operator>>(std::istream& is, Vector4f& value)
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
                ss >> value.x;
            }
            {
                std::stringstream ss(split[1]);
                ss >> value.y;
            }
            {
                std::stringstream ss(split[2]);
                ss >> value.z;
            }
            {
                std::stringstream ss(split[3]);
                ss >> value.w;
            }
            return is;
        }
    } // namespace math
} // namespace tl
