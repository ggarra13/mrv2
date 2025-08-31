

#include <tlTimeline/HDROptions.h>
// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-Present Gonzalo Garramuño
// All rights reserved.

#include <vector>
#include <array> // For working with arrays in the JSON serialization

#include <tlCore/HDR.h>

#include <tlCore/Error.h>
#include <tlCore/String.h>

namespace tl
{
    namespace timeline
    {
        TLRENDER_ENUM_IMPL(
            HDRGamutMapping,
            "Auto",
            "Clip",
            "Perceptual",
            "Relative",
            "Saturation",
            "Absolute",
            "Desaturate",
            "Darken",
            "Highlight",
            "Linear");
        TLRENDER_ENUM_SERIALIZE_IMPL(HDRGamutMapping);
        
        TLRENDER_ENUM_IMPL(
            HDRTonemapAlgorithm, "ST2094_40", "ST2094_10", "Clip", "BT2390",
            "BT2446A", "Spline", "Reinhard", "Mobius", "Hable", "Gamma",
            "Linear", "Linear Light", "None");
        TLRENDER_ENUM_SERIALIZE_IMPL(HDRTonemapAlgorithm);
    } // namespace timeline
} // namespace tl
