// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

//! Convenience macro for making a class non-copyable.
#define TLRENDER_NON_COPYABLE(CLASS)                                           \
    CLASS(const CLASS&) = delete;                                              \
    CLASS& operator=(const CLASS&) = delete

//! Convenience macro for private implementations.
//!
//! Required includes:
//! * memory
#define TLRENDER_PRIVATE()                                                     \
    struct Private;                                                            \
    std::unique_ptr<Private> _p

//! Define a variable, "p", that references the private implementation.
#define TLRENDER_P() auto& p = *_p

//! Convenience macro for enum utilities.
//!
//! Required includes:
//! * string
//! * vector
#define TLRENDER_ENUM(ENUM)                                             \
    std::vector<ENUM> get##ENUM##Enums();                               \
    std::vector<std::string> get##ENUM##Labels();                       \
    std::string getLabel(ENUM);                                         \
    std::string to_string(ENUM);                                        \
    bool from_string(const std::string&, ENUM&);                        

//! Implementation macro for enum utilities.
//!
//! Required includes:
//! * array
#define TLRENDER_ENUM_IMPL(ENUM, ...)                                          \
    std::vector<ENUM> get##ENUM##Enums()                                       \
    {                                                                          \
        std::vector<ENUM> out;                                                 \
        for (std::size_t i = 0; i < static_cast<std::size_t>(ENUM::Count);     \
             ++i)                                                              \
        {                                                                      \
            out.push_back(static_cast<ENUM>(i));                               \
        }                                                                      \
        return out;                                                            \
    }                                                                          \
                                                                               \
    std::vector<std::string> get##ENUM##Labels()                               \
    {                                                                          \
        return {__VA_ARGS__};                                                  \
    }                                                                          \
                                                                               \
    std::string getLabel(ENUM value)                                           \
    {                                                                          \
        const std::array<std::string, static_cast<std::size_t>(ENUM::Count)> \
            data = {__VA_ARGS__};                                       \
        return data[static_cast<std::size_t>(value)];                   \
    }                                                                   \
                                                                        \
    std::string to_string(ENUM value)                                   \
    {                                                                   \
        return getLabel(value);                                         \
    }                                                                   \
                                                                        \
    bool from_string(const std::string& s, ENUM& value)                 \
    {                                                                   \
        bool out = false;                                               \
        const auto& labels = get##ENUM##Labels();                       \
        const auto i = std::find_if(                                    \
            labels.begin(),                                             \
            labels.end(),                                               \
            [s](const std::string& value)                               \
                {                                                       \
                    return string::compare(s, value, string::Compare::CaseInsensitive); \
                });                                                     \
        if (i != labels.end())                                          \
        {                                                               \
            value = static_cast<ENUM>(i - labels.begin());              \
            out = true;                                                 \
        }                                                               \
        return out;                                                     \
    }

//! Convenience macro for serializing enums.
//!
//! Required includes:
//! * iostream
//! * nlohmann/json.hpp
#define TLRENDER_ENUM_SERIALIZE(ENUM)                                          \
    std::ostream& operator<<(std::ostream&, ENUM);                             \
    std::istream& operator>>(std::istream&, ENUM&);                            \
    void to_json(nlohmann::json&, ENUM);                                       \
    void from_json(const nlohmann::json&, ENUM&)

//! Implementation macro for serializing enums.
//!
//! Required includes:
//! * tlCore/Error.h
//! * tlCore/String.h
//! * algorithm
#define TLRENDER_ENUM_SERIALIZE_IMPL(ENUM, ...)                                \
    std::ostream& operator<<(std::ostream& os, ENUM in)                        \
    {                                                                          \
        os << get##ENUM##Labels()[static_cast<std::size_t>(in)];               \
        return os;                                                             \
    }                                                                          \
                                                                               \
    std::istream& operator>>(std::istream& is, ENUM& out)                      \
    {                                                                          \
        std::string s;                                                         \
        is >> s;                                                               \
        const auto labels = get##ENUM##Labels();                               \
        const auto i = std::find_if(                                           \
            labels.begin(), labels.end(),                                      \
            [s](const std::string& value)                                      \
            {                                                                  \
                return tl::string::compare(                                    \
                    s, value, tl::string::Compare::CaseInsensitive);           \
            });                                                                \
        if (i == labels.end())                                                 \
        {                                                                      \
            throw tl::error::ParseError();                                     \
        }                                                                      \
        out = static_cast<ENUM>(i - labels.begin());                           \
        return is;                                                             \
    }                                                                          \
                                                                               \
    void to_json(nlohmann::json& json, ENUM in)                                \
    {                                                                          \
        json = get##ENUM##Labels()[static_cast<std::size_t>(in)];              \
    }                                                                          \
                                                                               \
    void from_json(const nlohmann::json& json, ENUM& out)                      \
    {                                                                          \
        std::string s;                                                         \
        json.get_to(s);                                                        \
        const auto labels = get##ENUM##Labels();                               \
        const auto i = std::find_if(                                           \
            labels.begin(), labels.end(),                                      \
            [s](const std::string& value) {                                    \
                return string::compare(                                        \
                    s, value, string::Compare::CaseInsensitive);               \
            });                                                                \
        if (i == labels.end())                                                 \
        {                                                                      \
            throw tl::error::ParseError();                                     \
        }                                                                      \
        out = static_cast<ENUM>(i - labels.begin());                           \
    }
