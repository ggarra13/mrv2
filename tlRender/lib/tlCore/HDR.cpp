// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <vector>
#include <array> // For working with arrays in the JSON serialization

#include <tlCore/HDR.h>

#include <tlCore/Error.h>
#include <tlCore/String.h>

namespace tl
{
    namespace image
    {
        TLRENDER_ENUM_IMPL(HDRPrimaries, "Red", "Green", "Blue", "White");
        TLRENDER_ENUM_SERIALIZE_IMPL(HDRPrimaries);

        void to_json(nlohmann::json& json, const HDRBezier& value)
        {
            json = nlohmann::json{
                {"targetLuma", value.targetLuma},
                {"kneeX", value.kneeX},
                {"kneeY", value.kneeY},
                {"numAnchors", value.numAnchors},
                {"anchors",
                 std::vector<float>(
                     value.anchors, value.anchors + value.numAnchors)}};
        }

        void from_json(const nlohmann::json& json, HDRBezier& value)
        {
            json.at("targetLuma").get_to(value.targetLuma);
            json.at("kneeX").get_to(value.kneeX);
            json.at("kneeY").get_to(value.kneeY);
            json.at("numAnchors").get_to(value.numAnchors);

            // Deserialize the anchors array
            std::vector<float> anchorsVec =
                json.at("anchors").get<std::vector<float>>();
            value.numAnchors = static_cast<uint8_t>(anchorsVec.size());

            // Copy the vector contents back into the array
            std::copy(
                anchorsVec.begin(), anchorsVec.begin() + value.numAnchors,
                value.anchors);
        }

        void to_json(nlohmann::json& json, const HDRData& value)
        {
            json = nlohmann::json{
                {"eotf", value.eotf},
                {"primaries", value.primaries},
                {"displayMasteringLuminance", value.displayMasteringLuminance},
                {"maxCLL", value.maxCLL},
                {"maxFALL", value.maxFALL},
                {"sceneMax0", value.sceneMax[0]},
                {"sceneMax1", value.sceneMax[1]},
                {"sceneMax2", value.sceneMax[2]},
                {"sceneAvg", value.sceneAvg},
                {"ootf", value.ootf},
                {"maxPQY", value.maxPQY},
                {"avgPQY", value.avgPQY},
            };
        }

        void from_json(const nlohmann::json& json, HDRData& value)
        {
            json.at("eotf").get_to(value.eotf);
            json.at("primaries").get_to(value.primaries);
            json.at("displayMasteringLuminance")
                .get_to(value.displayMasteringLuminance);
            json.at("maxCLL").get_to(value.maxCLL);
            json.at("maxFALL").get_to(value.maxFALL);
            json.at("sceneMax0").get_to(value.sceneMax[0]);
            json.at("sceneMax1").get_to(value.sceneMax[1]);
            json.at("sceneMax2").get_to(value.sceneMax[2]);
            json.at("sceneAvg").get_to(value.sceneAvg);
            json.at("ootf").get_to(value.ootf);
            json.at("maxPQY").get_to(value.maxPQY);
            json.at("avgPQY").get_to(value.avgPQY);
        }
    } // namespace image
} // namespace tl
