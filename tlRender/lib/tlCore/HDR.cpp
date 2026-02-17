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

        HDRData nameToPrimaries(const std::string& input)
        {
            HDRData out;
            std::string name = string::toUpper(input);

            // Extract view gamut if present (e.g., "(P3 D65)")
            std::string viewGamut;
            size_t start = name.rfind('(');
            size_t end = name.rfind(')');
            if (start != std::string::npos && end != std::string::npos &&
                end > start)
            {
                viewGamut = name.substr(start + 1, end - start - 1);
                // Remove the extracted part from name to avoid interference
                name = name.substr(0, start) + name.substr(end + 1);
            }

            // Trim any extra spaces
            auto trim = [](std::string& s) {
                s.erase(0, s.find_first_not_of(" \t"));
                s.erase(s.find_last_not_of(" \t") + 1);
            };
            trim(name);
            trim(viewGamut);


            if (name.find("SDR") != std::string::npos)
            {
                out.eotf = EOTF_BT709;
                out.primaries[Red]   = {0.640F, 0.330F};
                out.primaries[Green] = {0.300F, 0.600F};
                out.primaries[Blue]  = {0.150F, 0.060F};
                out.primaries[White] = {0.3127F, 0.3290F};
            }                         
            // --- 1. Wide Gamut / HDR (BT.2020 and BT.2100) ---
            else if (name.find("2020") != std::string::npos ||
                     name.find("2100") != std::string::npos ||
                     name.find("HDR") != std::string::npos)
            {
                // Default to 2020 SDR curve
                out.eotf = EOTF_BT2020; 

                // Check for specific HDR Transfer Functions
                if (name.find("PQ") != std::string::npos ||
                    name.find("ST2084") != std::string::npos)
                {
                    out.eotf = EOTF_BT2100_PQ;
                }
                else if (name.find("HLG") != std::string::npos)
                {
                    out.eotf = EOTF_BT2100_HLG;
                }

                out.primaries[Red]   = {0.708F, 0.292F};
                out.primaries[Green] = {0.170F, 0.797F};
                out.primaries[Blue]  = {0.131F, 0.046F};
                out.primaries[White] = {0.3127F, 0.3290F};
            }
            // --- 2. DCI-P3 / Display P3 ---
            else if (name.find("P3") != std::string::npos)
            {
                // P3 is usually SDR (Gamma 2.6 for DCI or sRGB-like for Display P3)
                // Since the enum lacks Gamma 2.6, BT709 is the closest "SDR" curve.
                out.eotf = EOTF_BT709; 

                if (name.find("PQ") != std::string::npos ||
                    name.find("ST2084") != std::string::npos)
                    out.eotf = EOTF_BT2100_PQ;
                else if (name.find("HLG") != std::string::npos)
                    out.eotf = EOTF_BT2100_HLG;

                out.primaries[Red]   = {0.680F, 0.320F};
                out.primaries[Green] = {0.265F, 0.690F};
                out.primaries[Blue]  = {0.150F, 0.060F};
        
                if (name.find("DCI") != std::string::npos)
                {
                    out.primaries[White] = {0.3140F, 0.3510F};
                }
                else // D65 (Display P3)
                {
                    out.primaries[White] = {0.3127F, 0.3290F};
                }
            }
            // --- 3. Standard Definition (Legacy) ---
            else if (name.find("NTSC") != std::string::npos || 
                     name.find("PAL")  != std::string::npos || 
                     name.find("601")  != std::string::npos)
            {
                out.eotf = EOTF_BT601;
                out.primaries[Red]   = {0.640F, 0.330F};
                out.primaries[Green] = {0.290F, 0.600F};
                out.primaries[Blue]  = {0.150F, 0.060F};
                out.primaries[White] = {0.3127F, 0.3290F};
            }
            // --- 4. HD SDR (BT.709) ---
            else if (name.find("709") != std::string::npos)
            {
                out.eotf = EOTF_BT709;
                out.primaries[Red]   = {0.640F, 0.330F};
                out.primaries[Green] = {0.300F, 0.600F};
                out.primaries[Blue]  = {0.150F, 0.060F};
                out.primaries[White] = {0.3127F, 0.3290F};
            }
            // --- 5. Custom / Film / Others ---
            else if (name.find("FILM") != std::string::npos ||
                     name.find("BT470M") != std::string::npos)
            {
                out.eotf = EOTF_BT601;
                out.primaries[Red]   = { 0.670F, 0.330F };
                out.primaries[Green] = { 0.210F, 0.710F };
                out.primaries[Blue]  = { 0.140F, 0.080F };
                out.primaries[White] = { 0.310F, 0.316F };
            }
            // --- 6. SDR Rec. 1886 ---
            else if (name.find("1886") != std::string::npos)
            {
                out.eotf = EOTF_BT709;
                out.primaries[Red]   = {0.640F, 0.330F};
                out.primaries[Green] = {0.300F, 0.600F};
                out.primaries[Blue]  = {0.150F, 0.060F};
                out.primaries[White] = {0.3127F, 0.3290F};
            }
            // --- Default Fallback (Standard sRGB/Rec709) ---
            else
            {
                out.eotf = EOTF_BT709;
                out.primaries[Red]   = {0.640F, 0.330F};
                out.primaries[Green] = {0.300F, 0.600F};
                out.primaries[Blue]  = {0.150F, 0.060F};
                out.primaries[White] = {0.3127F, 0.3290F};
            }

            // Override primaries with view gamut if present and matched
            if (!viewGamut.empty())
            {
                // Recursively apply matching to viewGamut
                // (without EOTF override)
                HDRData viewData = nameToPrimaries(viewGamut);
                out.primaries[Red]   = viewData.primaries[Red];
                out.primaries[Green] = viewData.primaries[Green];
                out.primaries[Blue]  = viewData.primaries[Blue];
                out.primaries[White] = viewData.primaries[White];
                // EOTF remains from display (container)
            }

            return out;
        }
        
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

        std::string primariesName(const std::array<math::Vector2f, HDRPrimaries::Count> primaries)
        {
            return primariesName(primaries[0], primaries[1], primaries[2], primaries[3]);
        }

        std::string primariesName(const HDRData& value)
        {
            std::string out;
            out = primariesName(value.primaries);
            if (value.eotf == EOTF_BT2100_HLG)
                out = "Rec. 2100-HLG";
            else if (value.eotf == EOTF_BT2100_PQ)
                out = "Rec. 2100-PQ";
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
                {"isDolbyVision", value.isDolbyVision},
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
            if (json.contains("isDolbyVision"))
            {
                json.at("isDolbyVision").get_to(value.isDolbyVision);
                json.at("maxPQY").get_to(value.maxPQY);
                json.at("avgPQY").get_to(value.avgPQY);
            }
            else
            {
                value.isDolbyVision = false;
                value.maxPQY = 0.F;
                value.avgPQY = 0.F;
            }
        }
    } // namespace image
} // namespace tl
