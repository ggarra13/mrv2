// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once


#include <tlCore/Color.h>
#include <tlCore/Image.h>

namespace mrv
{
    namespace color
    {
        using namespace tl;
        image::Color4f fromVoidPtr(const void* ptr, const image::PixelType pixelType);
        
        // ─────────────────────────────────────────────────────────────────────────
        // ST.2084 (PQ) Inverse EOTF: Maps [0, 1] PQ → [0, 1] Linear (1.0 = 10k nits)
        // ──────────────────────────────────────────────────────────────────────
        inline float inverse_st2084_eotf(float N) noexcept
        {
            if (N <= 0.f) return 0.f;
    
            constexpr float m1 = 2610.f / 16384.f;
            constexpr float m2 = (2523.f / 4096.f) * 128.f;
            constexpr float c1 = 3424.f / 4096.f;
            constexpr float c2 = (2413.f / 4096.f) * 32.f;
            constexpr float c3 = (2392.f / 4096.f) * 32.f;

            float Npw = std::pow(N, 1.f / m2);
            float num = std::max(Npw - c1, 0.f);
            float den = c2 - c3 * Npw;
    
            return std::pow(num / den, 1.f / m1);
        }
        
    }
}
