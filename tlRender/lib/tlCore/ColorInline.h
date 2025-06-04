// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <algorithm>

namespace tl
{
    namespace image
    {
        inline Color4f::Color4f() :
            r(0.F),
            g(0.F),
            b(0.F),
            a(0.F)
        {
        }

        inline Color4f::Color4f(float r, float g, float b, float a) :
            r(r),
            g(g),
            b(b),
            a(a)
        {
        }

        inline bool Color4f::operator==(const Color4f& other) const
        {
            return r == other.r && g == other.g && b == other.b && a == other.a;
        }

        inline bool Color4f::operator!=(const Color4f& other) const
        {
            return !(*this == other);
        }

        inline image::Color4f lighter(const image::Color4f& color, float amount)
        {
            return image::Color4f(
                color.r + amount, color.g + amount, color.b + amount, color.a);
        }

        inline image::Color4f darker(const image::Color4f& color, float amount)
        {
            return image::Color4f(
                color.r - amount, color.g - amount, color.b - amount, color.a);
        }
    } // namespace image
} // namespace tl
