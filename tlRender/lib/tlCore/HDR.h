// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlCore/Range.h>
#include <tlCore/Util.h>
#include <tlCore/Vector.h>

#include <array>

namespace tl
{
    namespace image
    {
        //! HDR color primaries.
        enum HDRPrimaries {
            Red = 0,
            Green = 1,
            Blue = 2,
            White = 3,

            Count,
            First = Red
        };
        TLRENDER_ENUM(HDRPrimaries);
        TLRENDER_ENUM_SERIALIZE(HDRPrimaries);

        //! Bezier curve for HDR metadata
        struct HDRBezier
        {
            float targetLuma = 100.F;
            float kneeX = 0.5F;
            float kneeY = 0.5F;
            uint8_t numAnchors = 0;
            float anchors[15];

            bool operator==(const HDRBezier&) const;
            bool operator!=(const HDRBezier&) const;
        };

        //! \name Serialize
        ///@{

        void to_json(nlohmann::json&, const HDRBezier&);

        void from_json(const nlohmann::json&, HDRBezier&);

        ///@}

        enum EOTFType : uint8_t {
            EOTF_BT601 = 0,
            EOTF_BT709 = 1,
            EOTF_BT2020 = 2,
            EOTF_BT2100_HLG = 3,
            EOTF_BT2100_PQ = 4
        };

        //! HDR data.
        struct HDRData
        {
            uint8_t eotf = EOTFType::EOTF_BT709;
            //! Default Rec. 2020 color primaries (red, green, blue, white).
            std::array<math::Vector2f, HDRPrimaries::Count> primaries = {
                math::Vector2f(.708F, .292F), math::Vector2f(.170F, .797F),
                math::Vector2f(.131F, .046F), math::Vector2f(.3127F, .3290F)};
            math::FloatRange displayMasteringLuminance =
                math::FloatRange(0.F, 1000.F);
            float maxCLL = 1000.F;
            float maxFALL = 400.F;

            //! HDR10+ Metadata
            float sceneMax[3] = {0.F, 0.F, 0.F};
            float sceneAvg = 0.F;
            HDRBezier ootf;

            //! HDR CieY Metadata (DolbyVision)
            float maxPQY = 0.F;
            float avgPQY = 0.F;

            bool operator==(const HDRData&) const;
            bool operator!=(const HDRData&) const;
        };

        //! \name Serialize
        ///@{

        void to_json(nlohmann::json&, const HDRData&);

        void from_json(const nlohmann::json&, HDRData&);

        ///@}
    } // namespace image
} // namespace tl

#include <tlCore/HDRInline.h>
