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
            //! Debug HDR
            bool debug = false;
            
            bool tonemap = false;

            //! Trick for Windows HDR by sending it BT709 chromaticities.
            bool ScRGB = false;

            //! Peak detection variables.
            bool peak_detection = false;
            float peak_percentile = 100.F;
            float peak_smoothing_period = 20.F;
            float peak_scene_low_limit = 1.F;
            float peak_scene_high_limit = 3.F;

            //! Tone mapping data.
            HDRGamutMapping     gamutMapping = HDRGamutMapping::Auto;
            HDRTonemapAlgorithm algorithm = HDRTonemapAlgorithm::Hable;
            image::HDRData hdrData;

            bool operator==(const HDROptions&) const;
            bool operator!=(const HDROptions&) const;
        };
    } // namespace timeline
} // namespace tl

#include <tlTimeline/HDROptionsInline.h>
