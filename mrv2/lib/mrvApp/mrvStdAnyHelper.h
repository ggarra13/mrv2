// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <any>

#include "mrvFl/mrvIO.h"

inline std::string anyName(const std::any& value)
{
    // Get type information using typeid
    const std::type_info& typeInfo = value.type();

    // Convert type information to a human-readable string
    return typeInfo.name();
}

template <typename T> inline T std_any_cast(const std::any& value)
{
    T out;
    try
    {
        out = std::any_cast<T>(value);
    }
    catch (const std::bad_any_cast& e)
    {
        const char* kModule = "any";
        LOG_ERROR(e.what() << " is " << anyName(value));
    }
    return out;
}

template <> inline bool std_any_cast(const std::any& value)
{
    bool out = false; // Default value in case of bad_any_cast
    try
    {
        out = std::any_cast<bool>(value);
    }
    catch (const std::bad_any_cast& e)
    {
        const char* kModule = "any";
        LOG_ERROR(e.what() << " expected bool is " << anyName(value));
    }
    return out;
}

template <> inline int std_any_cast(const std::any& value)
{
    int out = 0; // Default value in case of bad_any_cast
    try
    {
        out = std::any_cast<int>(value);
    }
    catch (const std::bad_any_cast& e)
    {
        const char* kModule = "any";
        LOG_ERROR(e.what() << " expected int is " << anyName(value));
    }
    return out;
}

template <> inline float std_any_cast(const std::any& value)
{
    float out = 0.0F; // Default value in case of bad_any_cast
    try
    {
        out = std::any_cast<float>(value);
    }
    catch (const std::bad_any_cast& e)
    {
        const char* kModule = "any";
        LOG_ERROR(e.what() << " expected float is " << anyName(value));
    }
    return out;
}

template <> inline double std_any_cast(const std::any& value)
{
    double out = 0.0; // Default value in case of bad_any_cast
    try
    {
        out = std::any_cast<double>(value);
    }
    catch (const std::bad_any_cast& e)
    {
        const char* kModule = "any";
        LOG_ERROR(e.what() << " expected double is " << anyName(value));
    }
    return out;
}
