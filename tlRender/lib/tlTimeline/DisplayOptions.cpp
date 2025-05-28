// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlTimeline/DisplayOptions.h>

#include <tlCore/Error.h>
#include <tlCore/String.h>

#include <algorithm>
#include <array>

namespace tl
{
    namespace timeline
    {
        TLRENDER_ENUM_IMPL(
            Channels, "Color", "Red", "Green", "Blue", "Alpha", "Lumma");
        TLRENDER_ENUM_SERIALIZE_IMPL(Channels);
        
        TLRENDER_ENUM_IMPL(
            HDRInformation, "From File", "False", "True");
        TLRENDER_ENUM_SERIALIZE_IMPL(HDRInformation);

        math::Matrix4x4f brightness(const math::Vector3f& value)
        {
            return math::Matrix4x4f(
                value.x, 0.F, 0.F, 0.F, 0.F, value.y, 0.F, 0.F, 0.F, 0.F,
                value.z, 0.F, 0.F, 0.F, 0.F, 1.F);
        }

        math::Matrix4x4f contrast(const math::Vector3f& value)
        {
            return math::Matrix4x4f(
                       1.F, 0.F, 0.F, -.5F, 0.F, 1.F, 0.F, -.5F, 0.F, 0.F, 1.F,
                       -.5F, 0.F, 0.F, 0.F, 1.F) *
                   math::Matrix4x4f(
                       value.x, 0.F, 0.F, 0.F, 0.F, value.y, 0.F, 0.F, 0.F, 0.F,
                       value.z, 0.F, 0.F, 0.F, 0.F, 1.F) *
                   math::Matrix4x4f(
                       1.F, 0.F, 0.F, .5F, 0.F, 1.F, 0.F, .5F, 0.F, 0.F, 1.F,
                       .5F, 0.F, 0.F, 0.F, 1.F);
        }

        math::Matrix4x4f saturation(const math::Vector3f& value)
        {
            const math::Vector3f s(
                (1.F - value.x) * .3086F, (1.F - value.y) * .6094F,
                (1.F - value.z) * .0820F);
            return math::Matrix4x4f(
                s.x + value.x, s.y, s.z, 0.F, s.x, s.y + value.y, s.z, 0.F, s.x,
                s.y, s.z + value.z, 0.F, 0.F, 0.F, 0.F, 1.F);
        }

        math::Matrix4x4f tint(float v)
        {
            const float c = cos(v * M_PI * 2.F);
            const float c2 = 1.F - c;
            const float c3 = 1.F / 3.F * c2;
            const float s = sin(v * M_PI * 2.F);
            const float sq = sqrtf(1.F / 3.F);
            return math::Matrix4x4f(
                c + c2 / 3.F, c3 - sq * s, c3 + sq * s, 0.F, c3 + sq * s,
                c + c3, c3 - sq * s, 0.F, c3 - sq * s, c3 + sq * s, c + c3, 0.F,
                0.F, 0.F, 0.F, 1.F);
        }

        math::Matrix4x4f color(const Color& in)
        {
            return brightness(in.brightness) * contrast(in.contrast) *
                   saturation(in.saturation) * tint(in.tint);
        }
    } // namespace timeline
} // namespace tl
