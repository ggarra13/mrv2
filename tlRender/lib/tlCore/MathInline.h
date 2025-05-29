// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <cmath>

namespace tl
{
    namespace math
    {
        constexpr float deg2rad(float value)
        {
            return value / 360.F * pi2;
        }

        constexpr float rad2deg(float value)
        {
            return value / pi2 * 360.F;
        }

        template <typename T> constexpr T clamp(T value, T min, T max)
        {
            return std::min(std::max(value, min), max);
        }

        template <class T, class U> constexpr T lerp(U value, T min, T max)
        {
            return min + T(value * (max - min));
        }

        constexpr float smoothStep(float value, float min, float max)
        {
            return lerp(value * value * (3.F - (2.F * value)), min, max);
        }

        constexpr double smoothStep(double value, double min, double max)
        {
            return lerp(value * value * (3. - (2. * value)), min, max);
        }

        inline size_t digits(int value)
        {
            size_t out = 0;
            if (value != 0)
            {
                if (value < 0)
                {
                    //! \bug Should the minus sign be included?
                    ++out;
                }
                while (value)
                {
                    value /= 10;
                    ++out;
                }
            }
            else
            {
                out = 1;
            }
            return out;
        }

        inline bool fuzzyCompare(double a, double b, double e)
        {
            return fabs(a - b) < e;
        }

        inline bool fuzzyCompare(float a, float b, float e)
        {
            return fabsf(a - b) < e;
        }
    } // namespace math
} // namespace tl
