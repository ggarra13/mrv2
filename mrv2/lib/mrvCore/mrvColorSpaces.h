// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <ImfChromaticities.h>
#include <tlCore/Color.h>
#include <tlCore/Image.h>

namespace mrv {

using namespace tl;

enum BrightnessType {
  kAsLuminance,
  kAsLumma,
  kAsLightness,
};

float calculate_brightness(const imaging::Color4f &rgba,
                           const mrv::BrightnessType type) noexcept;

namespace color {

extern Imath::V3f kD50_whitePoint;
extern Imath::V3f kD65_whitePoint;
extern Imf::Chromaticities kITU_709_chroma;

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

const char *space2name(const Space &space) noexcept;
const char *space2id(const Space &space) noexcept;
const char *space2channels(const Space &space) noexcept;

namespace rgb {
imaging::Color4f to_xyz(const imaging::Color4f &rgb,
                        const Imf::Chromaticities &chroma = kITU_709_chroma,
                        const float Y = 1.0f) noexcept;
imaging::Color4f to_xyY(const imaging::Color4f &rgb,
                        const Imf::Chromaticities &chroma = kITU_709_chroma,
                        const float Y = 1.0f) noexcept;
imaging::Color4f to_lab(const imaging::Color4f &rgb,
                        const Imf::Chromaticities &chroma = kITU_709_chroma,
                        const float Y = 1.0f) noexcept;
imaging::Color4f to_luv(const imaging::Color4f &rgb,
                        const Imf::Chromaticities &chroma = kITU_709_chroma,
                        const float Y = 1.0f) noexcept;
imaging::Color4f to_hsv(const imaging::Color4f &rgb) noexcept;
imaging::Color4f to_hsl(const imaging::Color4f &rgb) noexcept;
imaging::Color4f to_yuv(const imaging::Color4f &rgb) noexcept;
imaging::Color4f to_yiq(const imaging::Color4f &rgb) noexcept;
imaging::Color4f to_YDbDr(const imaging::Color4f &rgb) noexcept;
imaging::Color4f to_ITU601(const imaging::Color4f &rgb) noexcept;
imaging::Color4f to_ITU709(const imaging::Color4f &rgb) noexcept;
} // namespace rgb

namespace yuv {
imaging::Color4f to_rgb(const imaging::Color4f &yuv256) noexcept;
}

namespace YPbPr {
imaging::Color4f to_rgb(const imaging::Color4f &yPbPrFloat,
                        const math::Vector4f &yuvCoefficients) noexcept;
}

namespace xyz {}

namespace lab {}

namespace luv {}

void checkLevels(imaging::Color4f &rgba,
                 const imaging::VideoLevels videoLevels);

} // namespace color

} // namespace mrv
