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

    } // namespace color

} // namespace mrv
