// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <algorithm>

#include <tlCore/Vector.h>

#include "mrvCore/mrvColorSpaces.h"

using tl::image::Color4f;

namespace
{

    /**
     * Color Space Info struct
     *
     */
    struct ColorSpaceInfo
    {
        const char* id;
        const char* name;
        const char* channels;
    };

    const ColorSpaceInfo kSpaceInfo[] = {
        // ID       Name                  Channels
        {"RGB", "Linear RGB", "R G B"},
        {"HSV", "Normalized HSV", "H S V"},
        {"HSL", "Normalized HSL", "H S L"},
        {"XYZ", "CIE XYZ", "X Y Z"},
        {"xyY", "CIE xyY", "x y Y"},
        {"Lab", "CIE L*a*b*", "L a b"},
        {"Luv", "CIE L*u*v*", "L u v"},
        {"YUV", "Analog PAL", "Y U V"},
        {"YDD", "Analog SECAM/PAL-N", "Y Db Dr"},
        {"YIQ", "Analog NTSC", "Y I Q"},
        {"601", "Digital PAL/NTSC", "Y Cb Cr"},
        {"709", "Digital HDTV", "Y Cb Cr"},
    };

    inline float cubeth(float v) noexcept
    {
        if (v > 0.008856f)
        {
            return (float)pow((double)v, 1.0 / 3.0);
        }
        else
        {
            return (float)(7.787037037037037037037037037037 * v + 16.0 / 116.0);
        }
    }

    inline float icubeth(float v) noexcept
    {
        if (v > 0.20689303448275862068965517241379f)
            return v * v * v;
        else if (v > 16.0f / 116.0f)
            return (
                float)((v - 16.0f / 116.0f) / 7.787037037037037037037037037037f);
        else
            return 0.0f;
    }

    /**
     * @brief Calculate hue for HSV or HSL
     *
     * @param rgb         original RGB pixel values
     * @param maxV        The max value of r, g, and b in rgb pixel
     * @param spanV       The (max-min) value between r,g,b channels in rgb
     * pixel
     *
     * @return a float representing the pixel hue [0..1]
     */
    inline float
    hue(const Color4f& rgb, const float maxV, const float spanV) noexcept
    {
        float h;
        if (rgb.r == maxV)
        {
            h = (rgb.g - rgb.b) / spanV;
        }
        else if (rgb.g == maxV)
        {
            h = 2.0f + (rgb.b - rgb.r) / spanV;
        }
        else
        { // ( rgb.b == maxV )
            h = 4.0f + (rgb.r - rgb.g) / spanV;
        }
        if (h < 0.0f)
        {
            h += 6;
        }
        h /= 6;
        return h;
    }

} // namespace

namespace mrv
{

    /**
     * @brief Calculate the brightness of a rgba color, based on type.
     *
     * @param rgba Color to calculate brightness for.
     * @param type One of Brightness Type.
     *
     * @return the brightness of the color.
     */
    float calculate_brightness(
        const Color4f& rgba, const BrightnessType type) noexcept
    {
        float L;
        switch (type)
        {
        case kAsLuminance:
            L = (0.2126f * rgba.r + 0.7152f * rgba.g + 0.0722f * rgba.b);
            break;
        case kAsLightness:
            L = (0.2126f * rgba.r + 0.7152f * rgba.g + 0.0722f * rgba.b);
            if (L >= 0.008856f)
                L = 116 * (pow(L, 1.0f / 3.0f)) - 16;

            // normalize it to 0...1, instead of 0...100
            L = L / 100.f;
            break;
        case kAsLumma:
        default:
            L = (rgba.r + rgba.g + rgba.b) / 3.0f;
            break;
        }
        return L;
    }

    namespace color
    {

        Imf::Chromaticities kITU_709_chroma;
        Imath::V3f kD50_whitePoint(0.3457f, 0.3585f, 0.2958f);
        Imath::V3f kD65_whitePoint(0.3127f, 0.3290f, 0.3582f);

        /**
         * @brief Convert a color::Space to a name
         *
         * @param space color::Space to convert
         *
         * @return name of the color space.
         */
        const char* space2name(const Space& space) noexcept
        {
            return kSpaceInfo[(unsigned)space].name;
        }

        /**
         * @brief Convert a color::Space to an ID.
         *
         * @param space color::Space to convert.
         *
         * @return ID of the color space.
         */
        const char* space2id(const Space& space) noexcept
        {
            return kSpaceInfo[(unsigned)space].id;
        }

        /**
         * @brief Convert a color::Space to channels.
         *
         * @param space color::Space to convert.
         *
         * @return Channels of the color spacce.
         */
        const char* space2channels(const Space& space) noexcept
        {
            return kSpaceInfo[(unsigned)space].channels;
        }

        namespace rgb
        {
            /**
             * @brief Convert color::rgb to xyz.
             *
             * @param rgb RGB color to convert.
             * @param chroma Chromatiticies used in XYZ conversion.
             * @param Y Luminosity.
             *
             * @return an XYZ color.
             */
            Color4f to_xyz(
                const Color4f& rgb, const Imf::Chromaticities& chroma,
                const float Y) noexcept
            {
                Color4f r(rgb);
                const Imath::M44f& m = RGBtoXYZ(chroma, Y);
                Imath::V3f* v = (Imath::V3f*)&r;
                *v = *v * m;
                return r;
            }

            /**
             * @brief  Convert color::rgb to xyY.
             *
             * @param rgb RGB color to convert.
             * @param chroma Chromatiticies used in XYZ conversion.
             * @param Y Luminosity.
             *
             * @return an xyY color.
             */
            Color4f to_xyY(
                const Color4f& rgb, const Imf::Chromaticities& chroma,
                const float Y) noexcept
            {
                Color4f xyy(to_xyz(rgb, chroma, Y));

                float sum = xyy.r + xyy.g + xyy.b;

                xyy.b = xyy.g;

                if (sum == 0.0f)
                {
                    xyy.r = chroma.white.x;
                    xyy.g = chroma.white.y;
                }
                else
                {
                    sum = 1.0f / sum;
                    xyy.r *= sum;
                    xyy.g *= sum;
                }

                return xyy;
            }

            /**
             * @brief Convert an rgb color to lab.
             *
             * @param rgb RGB color to convert.
             * @param chroma Chromaticities used in conversion.
             * @param Y Luminosity.
             *
             * @return Lab color.
             */
            Color4f to_lab(
                const Color4f& rgb, const Imf::Chromaticities& chroma,
                const float Y) noexcept
            {
                Color4f lab(to_xyz(rgb, chroma, Y));

                float Xn = cubeth(lab.r / chroma.white.x);
                float Yn = cubeth(lab.g / chroma.white.y);
                float Zn =
                    cubeth(lab.b / (1.0f - chroma.white.x - chroma.white.y));

                lab.r = (116.0f * Yn - 16.0f);
                lab.g = (500.0f * (Xn - Yn));
                lab.b = (200.0f * (Yn - Zn));
                return lab;
            }

            /**
             * @brief Convert an rgb color to LUV.
             *
             * @param rgb RGB color to convert.
             * @param chroma Chromaticities used in conversion.
             * @param Y Luminosity used in conversion.
             *
             * @return A LUV color.
             */
            Color4f to_luv(
                const Color4f& rgb, const Imf::Chromaticities& chroma,
                const float Y) noexcept
            {
                Color4f luv(to_xyz(rgb, chroma, Y));

                float cwz = (1.0f - chroma.white.x - chroma.white.y);

                float yr = luv.g / chroma.white.y;

                float D = 1.0f / (luv.r + 15.0f * luv.g + 3 * luv.b);
                float Dn =
                    1.0f / (chroma.white.x + 15.0f * chroma.white.y + 3 * cwz);

                float u1 = (4.0f * luv.r) * D;
                float v1 = (9.0f * luv.g) * D;

                float ur = (4.0f * chroma.white.x) * Dn;
                float vr = (4.0f * chroma.white.y) * Dn;

                if (yr > 0.008856f)
                {
                    float Yn = powf(yr, 1.0f / 3.0f);
                    luv.r = 116.0f * Yn - 16.0f;
                }
                else
                {
                    luv.r = 903.3f * yr;
                }
                luv.g = 13.0f * luv.r * (u1 - ur);
                luv.b = 13.0f * luv.r * (v1 - vr);
                return luv;
            }

            /**
             * @brief Convert a RGB color to HSV
             *
             * @param rgb RGB color in [0-1] range.
             *
             * @return HSV color
             */
            Color4f to_hsv(const Color4f& rgb) noexcept
            {
                float minV = std::min(rgb.r, std::min(rgb.g, rgb.b));
                float maxV = std::max(rgb.r, std::max(rgb.g, rgb.b));
                float h, s, v;
                float spanV = maxV - minV;
                v = maxV;
                s = (maxV != 0.0f) ? (spanV / maxV) : 0.0f;
                if (s == 0)
                    h = 0;
                else
                {
                    h = hue(rgb, maxV, spanV);
                }
                return Color4f(h, s, v, rgb.a);
            }

            /**
             * @brief Convert a RGB color to HSL
             *
             * @param rgb RGB color in [0-1] range.
             *
             * @return HSL color
             */
            Color4f to_hsl(const Color4f& rgb) noexcept
            {
                float minV = std::min(rgb.r, std::min(rgb.g, rgb.b));
                float maxV = std::max(rgb.r, std::max(rgb.g, rgb.b));
                float spanV = maxV - minV;
                float sumV = maxV + minV;
                float h = hue(rgb, maxV, spanV);
                float l = sumV * 0.5f;
                float s;
                if (maxV == minV)
                    s = 0.0f;
                else if (l <= 0.5f)
                {
                    s = spanV / sumV; // or:  spanV / (2*l)
                }
                else
                {
                    s = spanV / (2.0f - sumV); // or: spanV / (2-(2*l))
                }
                return Color4f(h, s, l, rgb.a);
            }

            //! Analog NTSC
            Color4f to_yiq(const Color4f& rgb) noexcept
            {
                return Color4f(
                    rgb.r * 0.299f + rgb.g * 0.587f + rgb.b * 0.114f,
                    -rgb.r * 0.595716f - rgb.g * 0.274453f - rgb.b * 0.321263f,
                    rgb.r * 0.211456f - rgb.g * 0.522591f + rgb.b * 0.31135f);
            }

            //! Analog PAL
            Color4f to_yuv(const Color4f& rgb) noexcept
            {
                return Color4f(
                    rgb.r * 0.299f + rgb.g * 0.587f + rgb.b * 0.114f,
                    -rgb.r * 0.14713f - rgb.g * 0.28886f + rgb.b * 0.436f,
                    rgb.r * 0.615f - rgb.g * 0.51499f - rgb.b * 0.10001f);
            }

            //! Analog Secam/PAL-N
            Color4f to_YDbDr(const Color4f& rgb) noexcept
            {
                return Color4f(
                    rgb.r * 0.299f + rgb.g * 0.587f + rgb.b * 0.114f,
                    -rgb.r * 0.450f - rgb.g * 0.883f + rgb.b * 1.333f,
                    -rgb.r * 1.333f + rgb.g * 1.116f + rgb.b * 0.217f);
            }

            //! ITU. 601 or CCIR 601  (Digital PAL and NTSC )
            Color4f to_ITU601(const Color4f& rgb) noexcept
            {
                return Color4f(
                    16.f + rgb.r * 65.481f + rgb.g * 128.553f + rgb.b * 24.966f,
                    128.f - rgb.r * 37.797f - rgb.g * 74.203f + rgb.b * 112.0f,
                    128.f + rgb.r * 112.0f - rgb.g * 93.786f - rgb.b * 18.214f);
            }

            //! ITU. 709  (Digital HDTV )
            Color4f to_ITU709(const Color4f& rgb) noexcept
            {
                return Color4f(
                    rgb.r * 0.299f + rgb.g * 0.587f + rgb.b * 0.114f,
                    -rgb.r * 0.299f - rgb.g * 0.587f + rgb.b * 0.886f,
                    rgb.r * 0.701f - rgb.g * 0.587f - rgb.b * 0.114f);
            }

        } // namespace rgb

        namespace yuv
        {
            //! Analog PAL
            Color4f to_rgb(const Color4f& yuv) noexcept
            {
                float y2 = 1.164f * (yuv.r - 16);
                Color4f rgb(
                    y2 + 2.018f * (yuv.g - 128),
                    y2 - 0.813f * (yuv.b - 128) - 0.391f * (yuv.g - 128),
                    y2 + 1.596f * (yuv.b - 128));
                return rgb;
            }
        } // namespace yuv

        namespace YPbPr
        {
            /**
             * @brief Convert a YPbPr color to rgb.
             *
             * @param YPbPr the YPbPr color.
             * @param yuvCoefficients YUV coefficients used in conversion
             *
             * @return an RGB color.
             */
            image::Color4f to_rgb(
                const image::Color4f& YPbPr,
                const math::Vector4f& yuvCoefficients) noexcept
            {
                Color4f c;
                c.r = Imath::clamp(YPbPr.r, 0.0f, 1.0f);
                c.g = Imath::clamp(YPbPr.g, -0.5f, 0.5f);
                c.b = Imath::clamp(YPbPr.b, -0.5f, 0.5f);
                c.a = YPbPr.a;

                const float y = c.r;
                const float cb = c.g;
                const float cr = c.b;
                c.r = y + (yuvCoefficients.x * cr);
                c.g = y - (yuvCoefficients.z * cb) - (yuvCoefficients.w * cr);
                c.b = y + (yuvCoefficients.y * cb);

                Imath::clamp(c.r, 0.0f, 1.0f);
                Imath::clamp(c.g, 0.0f, 1.0f);
                Imath::clamp(c.b, 0.0f, 1.0f);
                return c;
            }
        } // namespace YPbPr

        /**
         * Limits an RGB color to full range or legal range video levels.
         *
         * @param rgba Original RGB color (modified in place)
         * @param videoLevels VideoLevels enum.
         */
        void
        checkLevels(image::Color4f& rgba, const image::VideoLevels videoLevels)
        {
            if (videoLevels == image::VideoLevels::FullRange)
            {
                rgba.g = rgba.g - 0.5f;
                rgba.b = rgba.b - 0.5f;
            }
            else if (videoLevels == image::VideoLevels::LegalRange)
            {
                rgba.r = (rgba.r - (16.0 / 255.0)) * (255.0 / (235.0 - 16.0));
                rgba.g =
                    (rgba.g - (16.0 / 255.0)) * (255.0 / (240.0 - 16.0)) - 0.5;
                rgba.b =
                    (rgba.b - (16.0 / 255.0)) * (255.0 / (240.0 - 16.0)) - 0.5;
            }
        }

    } // namespace color

} // namespace mrv
