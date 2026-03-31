// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <Imath/ImathVec.h>

#ifdef TLRENDER_EXR
#    include <ImfChromaticities.h>
#endif

#include <tlCore/Color.h>
#include <tlCore/Image.h>

namespace mrv
{

    using namespace tl;

    enum BrightnessType {
        kAsLuminance,
        kAsLumma,
        kAsLightness,
    };

    /**
     * Calculates the brightness a float value, based on a brightness type
     *
     * @param rgba original color
     * @param type brightness type (kAsLuminance, kAsLumma, kAsLightness)
     *
     * @return a float value representing the brightness.
     */
    float calculate_brightness(
        const image::Color4f& rgba, const mrv::BrightnessType type) noexcept;

    namespace color
    {

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
        
        /** 
         * Calculates Absolute Nits values from a PQ value.
         * 
         * @param v PQ value.
         * 
         * @return nits value (in 0...10000 range).
         */
        inline float pqToNits(float v) {
            return 10000.0f * inverse_st2084_eotf(v);
        }

        inline image::Color4f pqToNits(const image::Color4f& value)
        {
            return image::Color4f(pqToNits(value.r),
                                  pqToNits(value.g),
                                  pqToNits(value.b),
                                  value.a);
        }
        
        inline float pqToLinear(float v, const float reference_white = 100.F)
        {
            float nits = pqToNits(v);
            return nits / reference_white;
        }

        inline image::Color4f pqToLinear(const image::Color4f& value)
        {
            return image::Color4f(pqToLinear(value.r),
                                  pqToLinear(value.g),
                                  pqToLinear(value.b),
                                  value.a);
        }
        
        inline float srgbToLinear(float srgb)
        {
            if (srgb <= 0.04045f)
            {
                return srgb / 12.92f;
            }
            else
            {
                return std::pow((srgb + 0.055f) / 1.055f, 2.4f);
            }
        }
        
        inline image::Color4f srgbToLinear(const image::Color4f& value)
        {
            return image::Color4f(srgbToLinear(value.r),
                                  srgbToLinear(value.g),
                                  srgbToLinear(value.b),
                                  value.a);
        }
        
        extern Imath::V3f kD50_whitePoint;
        extern Imath::V3f kD65_whitePoint;
#ifdef TLRENDER_EXR
        extern Imf::Chromaticities kITU_709_chroma;
#endif

        enum Space {
            kRGB,
            kHSV,
            kHSL,
            kCIE_XYZ,
            kCIE_xyY,
            kCIE_Lab,
            kCIE_Luv,
            kYUV,     // Analog  PAL
            kYDbDr,   // Analog  SECAM/PAL-N
            kYIQ,     // Analog  NTSC
            kITU_601, // Digital PAL/NTSC
            kITU_709, // Digital HDTV
                      //       kYPbPr = 10,
            kLastColorSpace
        };

        /**
         * Convert a color::Space to a name
         *
         * @param space Color space to convert
         *
         * @return a const char* string in human readable form.
         *         For example, Linear RGB.
         */
        const char* space2name(const Space& space) noexcept;

        /**
         * Convert a color::Space to an id
         *
         * @param space Color space to convert
         *
         * @return A const char* string with the ID of the color space
         *         (ie. "RGB", or "HSV" for example)
         */
        const char* space2id(const Space& space) noexcept;

        /**
         * Convert a color::Space to a list of channels
         *
         * @param space color::Space to convert
         *
         * @return "R G B" or "H S V" for example.
         */
        const char* space2channels(const Space& space) noexcept;

        namespace rgb
        {
#ifdef TLRENDER_EXR
            image::Color4f to_xyz(
                const image::Color4f& rgb,
                const Imf::Chromaticities& chroma = kITU_709_chroma,
                const float Y = 1.0f) noexcept;
            image::Color4f to_xyY(
                const image::Color4f& rgb,
                const Imf::Chromaticities& chroma = kITU_709_chroma,
                const float Y = 1.0f) noexcept;
            image::Color4f to_lab(
                const image::Color4f& rgb,
                const Imf::Chromaticities& chroma = kITU_709_chroma,
                const float Y = 1.0f) noexcept;
            image::Color4f to_luv(
                const image::Color4f& rgb,
                const Imf::Chromaticities& chroma = kITU_709_chroma,
                const float Y = 1.0f) noexcept;
#endif
            image::Color4f to_hsv(const image::Color4f& rgb) noexcept;
            image::Color4f to_hsl(const image::Color4f& rgb) noexcept;
            image::Color4f to_yuv(const image::Color4f& rgb) noexcept;
            image::Color4f to_yiq(const image::Color4f& rgb) noexcept;
            image::Color4f to_YDbDr(const image::Color4f& rgb) noexcept;
            image::Color4f to_ITU601(const image::Color4f& rgb) noexcept;
            image::Color4f to_ITU709(const image::Color4f& rgb) noexcept;
            image::Color4f to_Rec2020(const image::Color4f& rgb) noexcept;
        } // namespace rgb

        namespace yuv
        {
            image::Color4f to_rgb(const image::Color4f& yuv256) noexcept;
        }

        namespace YPbPr
        {
            image::Color4f to_rgb(
                const image::Color4f& yPbPrFloat,
                const math::Vector4f& yuvCoefficients) noexcept;
        }

        namespace xyz
        {
        }

        namespace lab
        {
        }

        namespace luv
        {
        }

        void
        checkLevels(image::Color4f& rgba, const image::VideoLevels videoLevels);

        //! Convert tlRender's layer to more human readable ones.
        std::string layer(const std::string tlRenderLayer);

    } // namespace color

} // namespace mrv
