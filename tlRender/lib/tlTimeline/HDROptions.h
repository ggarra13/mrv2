// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Gonzalo Garramuño
// All rights reserved.

#include <tlCore/HDR.h>

namespace tl
{
    namespace timeline
    {
        enum class HDRGamutMapping {
            Auto,
            Clip,
            Perceptual,
            Relative,
            Saturation,
            Absolute,
            Desaturate,
            Darken,
            Highlight,
            Linear,
            Count,
            First = Auto
        };
        TLRENDER_ENUM(HDRGamutMapping);
        TLRENDER_ENUM_SERIALIZE(HDRGamutMapping);
        
        enum class HDRTonemapAlgorithm {
            ST2094_40,
            ST2094_10,
            Clip,
            BT2390,
            BT2446A,
            Spline,
            Reinhard,
            Mobius,
            Hable,
            Gamma,
            Linear,
            LinearLight,
            kNone,
            Count,
            First = ST2094_40
        };
        TLRENDER_ENUM(HDRTonemapAlgorithm);
        TLRENDER_ENUM_SERIALIZE(HDRTonemapAlgorithm);

        //! Tonemap options.
        struct HDROptions
        {
            bool passthru = false;
            bool tonemap = false;
            HDRGamutMapping     gamutMapping = HDRGamutMapping::Auto;
            HDRTonemapAlgorithm algorithm = HDRTonemapAlgorithm::Hable;
            image::HDRData hdrData;

            bool operator==(const HDROptions&) const;
            bool operator!=(const HDROptions&) const;
        };
    } // namespace timeline
} // namespace tl

#include <tlTimeline/HDROptionsInline.h>
