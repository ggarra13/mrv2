// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlCore/Range.h>

#include <tlCore/Error.h>
#include <tlCore/String.h>

#include <sstream>

namespace tl
{
    namespace math
    {
        void to_json(nlohmann::json& json, const IntRange& value)
        {
            json = {value.getMin(), value.getMax()};
        }
        
        void to_json(nlohmann::json& json, const Int64Range& value)
        {
            json = {value.getMin(), value.getMax()};
        }

        void to_json(nlohmann::json& json, const SizeTRange& value)
        {
            json = {value.getMin(), value.getMax()};
        }

        void to_json(nlohmann::json& json, const FloatRange& value)
        {
            json = {value.getMin(), value.getMax()};
        }

        void to_json(nlohmann::json& json, const DoubleRange& value)
        {
            json = {value.getMin(), value.getMax()};
        }

        void from_json(const nlohmann::json& json, IntRange& value)
        {
            int min = 0;
            int max = 0;
            json.at(0).get_to(min);
            json.at(1).get_to(max);
            value = IntRange(min, max);
        }
        
        void from_json(const nlohmann::json& json, Int64Range& value)
        {
            int64_t min = 0;
            int64_t max = 0;
            json.at(0).get_to(min);
            json.at(1).get_to(max);
            value = Int64Range(min, max);
        }

        void from_json(const nlohmann::json& json, SizeTRange& value)
        {
            size_t min = 0;
            size_t max = 0;
            json.at(0).get_to(min);
            json.at(1).get_to(max);
            value = SizeTRange(min, max);
        }

        void from_json(const nlohmann::json& json, FloatRange& value)
        {
            float min = 0.F;
            float max = 0.F;
            json.at(0).get_to(min);
            json.at(1).get_to(max);
            value = FloatRange(min, max);
        }

        void from_json(const nlohmann::json& json, DoubleRange& value)
        {
            double min = 0.0;
            double max = 0.0;
            json.at(0).get_to(min);
            json.at(1).get_to(max);
            value = DoubleRange(min, max);
        }

        std::ostream& operator<<(std::ostream& os, const IntRange& value)
        {
            os << value.getMin() << "-" << value.getMax();
            return os;
        }
        
        std::ostream& operator<<(std::ostream& os, const Int64Range& value)
        {
            os << value.getMin() << "-" << value.getMax();
            return os;
        }

        std::ostream& operator<<(std::ostream& os, const SizeTRange& value)
        {
            os << value.getMin() << "-" << value.getMax();
            return os;
        }

        std::ostream& operator<<(std::ostream& os, const FloatRange& value)
        {
            os << value.getMin() << "-" << value.getMax();
            return os;
        }

        std::ostream& operator<<(std::ostream& os, const DoubleRange& value)
        {
            os << value.getMin() << "-" << value.getMax();
            return os;
        }

        std::istream& operator>>(std::istream& is, IntRange& value)
        {
            std::string s;
            is >> s;
            auto split = string::split(s, '-');
            if (split.size() != 2)
            {
                throw error::ParseError();
            }
            int min = 0;
            int max = 0;
            {
                std::stringstream ss(split[0]);
                ss >> min;
            }
            {
                std::stringstream ss(split[1]);
                ss >> max;
            }
            value = IntRange(min, max);
            return is;
        }


        std::istream& operator>>(std::istream& is, Int64Range& value)
        {
            std::string s;
            is >> s;
            auto split = string::split(s, '-');
            if (split.size() != 2)
            {
                throw error::ParseError();
            }
            int64_t min = 0;
            int64_t max = 0;
            {
                std::stringstream ss(split[0]);
                ss >> min;
            }
            {
                std::stringstream ss(split[1]);
                ss >> max;
            }
            value = Int64Range(min, max);
            return is;
        }
        
        std::istream& operator>>(std::istream& is, SizeTRange& value)
        {
            std::string s;
            is >> s;
            auto split = string::split(s, '-');
            if (split.size() != 2)
            {
                throw error::ParseError();
            }
            size_t min = 0;
            size_t max = 0;
            {
                std::stringstream ss(split[0]);
                ss >> min;
            }
            {
                std::stringstream ss(split[1]);
                ss >> max;
            }
            value = SizeTRange(min, max);
            return is;
        }

        std::istream& operator>>(std::istream& is, FloatRange& value)
        {
            std::string s;
            is >> s;
            auto split = string::split(s, '-');
            if (split.size() != 2)
            {
                throw error::ParseError();
            }
            float min = 0.F;
            float max = 0.F;
            {
                std::stringstream ss(split[0]);
                ss >> min;
            }
            {
                std::stringstream ss(split[1]);
                ss >> max;
            }
            value = FloatRange(min, max);
            return is;
        }

        std::istream& operator>>(std::istream& is, DoubleRange& value)
        {
            std::string s;
            is >> s;
            auto split = string::split(s, '-');
            if (split.size() != 2)
            {
                throw error::ParseError();
            }
            double min = 0.0;
            double max = 0.0;
            {
                std::stringstream ss(split[0]);
                ss >> min;
            }
            {
                std::stringstream ss(split[1]);
                ss >> max;
            }
            value = DoubleRange(min, max);
            return is;
        }
    } // namespace math
} // namespace tl
