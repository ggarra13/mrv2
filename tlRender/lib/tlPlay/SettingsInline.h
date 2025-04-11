// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

namespace tl
{
    namespace play
    {
        template <typename T>
        inline T Settings::getValue(const std::string& key) const
        {
            T out = T();
            try
            {
                out = _values.at(key);
            }
            catch (const std::exception&)
            {
            }
            return out;
        }

        template <> inline int Settings::getValue(const std::string& key) const
        {
            int out = 0;
            try
            {
                out = _values.at(key);
            }
            catch (const std::exception&)
            {
            }
            return out;
        }

        template <>
        inline float Settings::getValue(const std::string& key) const
        {
            float out = 0.F;
            try
            {
                out = _values.at(key);
            }
            catch (const std::exception&)
            {
            }
            return out;
        }

        template <>
        inline double Settings::getValue(const std::string& key) const
        {
            double out = 0.0;
            try
            {
                out = _values.at(key);
            }
            catch (const std::exception&)
            {
            }
            return out;
        }

        template <>
        inline size_t Settings::getValue(const std::string& key) const
        {
            size_t out = 0;
            try
            {
                out = _values.at(key);
            }
            catch (const std::exception&)
            {
            }
            return out;
        }

        template <typename T>
        inline void Settings::setDefaultValue(const std::string& key, T in)
        {
            nlohmann::json json = in;
            _defaultValues[key] = json;
            const auto i = _values.find(key);
            if (i == _values.end())
            {
                _values[key] = json;
                _observer->setAlways(key);
            }
        }

        template <typename T>
        inline void Settings::setValue(const std::string& key, T in)
        {
            nlohmann::json json = in;
            const auto i = _values.find(key);
            if (i == _values.end() || json != _values[key])
            {
                _values[key] = json;
                _observer->setAlways(key);
            }
        }
    } // namespace play
} // namespace tl
