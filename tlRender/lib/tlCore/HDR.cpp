// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <vector>
#include <array> // For working with arrays in the JSON serialization

#include <tlCore/HDR.h>

#include <tlCore/Error.h>
#include <tlCore/Math.h>
#include <tlCore/String.h>

namespace tl
{
    namespace image
    {
        TLRENDER_ENUM_IMPL(HDRPrimaries, "Red", "Green", "Blue", "White");
        TLRENDER_ENUM_SERIALIZE_IMPL(HDRPrimaries);

        std::string primariesName(const math::Vector2f& r,
                                  const math::Vector2f& g,
                                  const math::Vector2f& b,
                                  const math::Vector2f& w)
        {
            using tl::math::fuzzyCompare;
            
            std::string out = "Unknown";
            if (fuzzyCompare(r.x, 0.708F) &&
                fuzzyCompare(r.y, 0.292F) &&
                fuzzyCompare(g.x, 0.170F) &&
                fuzzyCompare(g.y, 0.797F) &&
                fuzzyCompare(b.x, 0.131F) &&
                fuzzyCompare(b.y, 0.046F) &&
                fuzzyCompare(w.x, 0.3127F) &&
                fuzzyCompare(w.y, 0.3290F))
                out = "Rec. 2020 (BT.2020)";
            else if (fuzzyCompare(r.x, 0.680F) &&
                     fuzzyCompare(r.y, 0.320F) &&
                     fuzzyCompare(g.x, 0.265F) &&
                     fuzzyCompare(g.y, 0.690F) &&
                     fuzzyCompare(b.x, 0.150F) &&
                     fuzzyCompare(b.y, 0.060F) &&
                     fuzzyCompare(w.x, 0.3140F) &&
                     fuzzyCompare(w.y, 0.3510F))
                out = "DCI-P3 (DCI white)";
            else if (fuzzyCompare(r.x, 0.680F) &&
                     fuzzyCompare(r.y, 0.320F) &&
                     fuzzyCompare(g.x, 0.265F) &&
                     fuzzyCompare(g.y, 0.690F) &&
                     fuzzyCompare(b.x, 0.150F) &&
                     fuzzyCompare(b.y, 0.060F) &&
                     fuzzyCompare(w.x, 0.3127F) &&
                     fuzzyCompare(w.y, 0.3290F))
                out = "P3-D65 / Display P3 (D65 white)";
            else if (fuzzyCompare(r.x, 0.640F) &&
                     fuzzyCompare(r.y, 0.330F) &&
                     fuzzyCompare(g.x, 0.300F) &&
                     fuzzyCompare(g.y, 0.600F) &&
                     fuzzyCompare(b.x, 0.150F) &&
                     fuzzyCompare(b.y, 0.060F) &&
                     fuzzyCompare(w.x, 0.3127F) &&
                     fuzzyCompare(w.y, 0.3290F))
                out = "Rec. 709 (BT.709 / sRGB)";
            else if (fuzzyCompare(r.x, 0.670F) &&
                     fuzzyCompare(r.y, 0.330F) &&
                     fuzzyCompare(g.x, 0.210F) &&
                     fuzzyCompare(g.y, 0.710F) &&
                     fuzzyCompare(b.x, 0.140F) &&
                     fuzzyCompare(b.y, 0.080F) &&
                     fuzzyCompare(w.x, 0.310F) &&
                     fuzzyCompare(w.y, 0.316F))
                out = "BT470M";
            else if (fuzzyCompare(r.x, 0.640F) &&
                     fuzzyCompare(r.y, 0.330F) &&
                     fuzzyCompare(g.x, 0.290F) &&
                     fuzzyCompare(g.y, 0.600F) &&
                     fuzzyCompare(b.x, 0.150F) &&
                     fuzzyCompare(b.y, 0.060F) &&
                     fuzzyCompare(w.x, 0.3127F) &&
                     fuzzyCompare(w.y, 0.3290F))
                out = "NTSC / PAL / SECAM";
            else if (fuzzyCompare(r.x, 0.681F) &&
                     fuzzyCompare(r.y, 0.340F) &&
                     fuzzyCompare(g.x, 0.310F) &&
                     fuzzyCompare(g.y, 0.595F) &&
                     fuzzyCompare(b.x, 0.155F) &&
                     fuzzyCompare(b.y, 0.070F) &&
                     fuzzyCompare(w.x, 0.3127F) &&
                     fuzzyCompare(w.y, 0.3290F))
                out = "SMPTE240M";
            else if (fuzzyCompare(r.x, 0.681F) &&
                     fuzzyCompare(r.y, 0.319F) &&
                     fuzzyCompare(g.x, 0.243F) &&
                     fuzzyCompare(g.y, 0.692F) &&
                     fuzzyCompare(b.x, 0.145F) &&
                     fuzzyCompare(b.y, 0.049F) &&
                     fuzzyCompare(w.x, 0.310F) &&
                     fuzzyCompare(w.y, 0.316F))
                out = "Film";
            return out;
        }

        
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
