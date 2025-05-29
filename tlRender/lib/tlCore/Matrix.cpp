// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlCore/Matrix.h>

#include <tlCore/Error.h>
#include <tlCore/String.h>

#include <sstream>

namespace tl
{
    namespace math
    {
        void to_json(nlohmann::json& json, const Matrix3x3f& value)
        {
            json = {
                {value.e[0], value.e[1], value.e[2]},
                {value.e[3], value.e[4], value.e[5]},
                {value.e[6], value.e[7], value.e[8]}};
        }

        void to_json(nlohmann::json& json, const Matrix4x4f& value)
        {
            json = {
                {value.e[0], value.e[1], value.e[2], value.e[3]},
                {value.e[4], value.e[5], value.e[6], value.e[7]},
                {value.e[8], value.e[9], value.e[10], value.e[11]},
                {value.e[12], value.e[13], value.e[14], value.e[15]}};
        }

        void from_json(const nlohmann::json& json, Matrix3x3f& value)
        {
            json.at(0).at(0).get_to(value.e[0]);
            json.at(0).at(1).get_to(value.e[1]);
            json.at(0).at(2).get_to(value.e[2]);
            json.at(1).at(0).get_to(value.e[3]);
            json.at(1).at(1).get_to(value.e[4]);
            json.at(1).at(2).get_to(value.e[5]);
            json.at(2).at(0).get_to(value.e[6]);
            json.at(2).at(1).get_to(value.e[7]);
            json.at(2).at(2).get_to(value.e[8]);
        }

        void from_json(const nlohmann::json& json, Matrix4x4f& value)
        {
            json.at(0).at(0).get_to(value.e[0]);
            json.at(0).at(1).get_to(value.e[1]);
            json.at(0).at(2).get_to(value.e[2]);
            json.at(0).at(3).get_to(value.e[3]);
            json.at(1).at(0).get_to(value.e[4]);
            json.at(1).at(1).get_to(value.e[5]);
            json.at(1).at(2).get_to(value.e[6]);
            json.at(1).at(3).get_to(value.e[7]);
            json.at(2).at(0).get_to(value.e[8]);
            json.at(2).at(1).get_to(value.e[9]);
            json.at(2).at(2).get_to(value.e[10]);
            json.at(2).at(3).get_to(value.e[11]);
            json.at(3).at(0).get_to(value.e[12]);
            json.at(3).at(1).get_to(value.e[13]);
            json.at(3).at(2).get_to(value.e[14]);
            json.at(3).at(3).get_to(value.e[15]);
        }

        std::ostream& operator<<(std::ostream& os, const Matrix3x3f& value)
        {
            std::vector<std::string> s;
            for (size_t i = 0; i < 9; ++i)
            {
                std::stringstream ss;
                ss << value.e[i];
                s.push_back(ss.str());
            }
            os << string::join(s, ',');
            return os;
        }

        std::ostream& operator<<(std::ostream& os, const Matrix4x4f& value)
        {
            std::vector<std::string> s;
            for (size_t i = 0; i < 16; ++i)
            {
                std::stringstream ss;
                ss << value.e[i];
                s.push_back(ss.str());
            }
            os << string::join(s, ',');
            return os;
        }

        std::istream& operator>>(std::istream& is, Matrix3x3f& value)
        {
            std::string s;
            is >> s;
            auto split = string::split(s, ',');
            if (split.size() != 9)
            {
                throw error::ParseError();
            }
            for (size_t i = 0; i < 9; ++i)
            {
                std::stringstream ss(split[i]);
                ss >> value.e[i];
            }
            return is;
        }

        std::istream& operator>>(std::istream& is, Matrix4x4f& value)
        {
            std::string s;
            is >> s;
            auto split = string::split(s, ',');
            if (split.size() != 16)
            {
                throw error::ParseError();
            }
            for (size_t i = 0; i < 16; ++i)
            {
                std::stringstream ss(split[i]);
                ss >> value.e[i];
            }
            return is;
        }
    } // namespace math
} // namespace tl
