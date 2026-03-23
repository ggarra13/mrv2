// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlTimeline/DisplayOptions.h>

#include <tlCore/Error.h>
#include <tlCore/String.h>

#include <algorithm>
#include <array>

namespace tl
{
    namespace timeline
    {
        TLRENDER_ENUM_IMPL(
            Channels, "Color", "Red", "Green", "Blue", "Alpha", "Lumma");
        TLRENDER_ENUM_SERIALIZE_IMPL(Channels);
        
        TLRENDER_ENUM_IMPL(
            HDRInformation, "From File", "Inactive", "Active");
        TLRENDER_ENUM_SERIALIZE_IMPL(HDRInformation);

        math::Matrix4x4f brightness(const math::Vector3f& value)
        {
            return math::Matrix4x4f(
                value.x, 0.F, 0.F, 0.F, 0.F, value.y, 0.F, 0.F, 0.F, 0.F,
                value.z, 0.F, 0.F, 0.F, 0.F, 1.F);
        }

        math::Matrix4x4f contrast(const math::Vector3f& value)
        {
            return math::Matrix4x4f(
                       1.F, 0.F, 0.F, -.5F, 0.F, 1.F, 0.F, -.5F, 0.F, 0.F, 1.F,
                       -.5F, 0.F, 0.F, 0.F, 1.F) *
                   math::Matrix4x4f(
                       value.x, 0.F, 0.F, 0.F, 0.F, value.y, 0.F, 0.F, 0.F, 0.F,
                       value.z, 0.F, 0.F, 0.F, 0.F, 1.F) *
                   math::Matrix4x4f(
                       1.F, 0.F, 0.F, .5F, 0.F, 1.F, 0.F, .5F, 0.F, 0.F, 1.F,
                       .5F, 0.F, 0.F, 0.F, 1.F);
        }

        math::Matrix4x4f saturation(const math::Vector3f& value)
        {
            const math::Vector3f s(
                (1.F - value.x) * .3086F, (1.F - value.y) * .6094F,
                (1.F - value.z) * .0820F);
            return math::Matrix4x4f(
                s.x + value.x, s.y, s.z, 0.F, s.x, s.y + value.y, s.z, 0.F, s.x,
                s.y, s.z + value.z, 0.F, 0.F, 0.F, 0.F, 1.F);
        }

        math::Matrix4x4f tint(float v)
        {
            const float c = cos(v * M_PI * 2.F);
            const float c2 = 1.F - c;
            const float c3 = 1.F / 3.F * c2;
            const float s = sin(v * M_PI * 2.F);
            const float sq = sqrtf(1.F / 3.F);
            return math::Matrix4x4f(
                c + c2 / 3.F, c3 - sq * s, c3 + sq * s, 0.F, c3 + sq * s,
                c + c3, c3 - sq * s, 0.F, c3 - sq * s, c3 + sq * s, c + c3, 0.F,
                0.F, 0.F, 0.F, 1.F);
        }

        math::Matrix4x4f color(const Color& in)
        {
            return brightness(in.brightness) * contrast(in.contrast) *
                   saturation(in.saturation) * tint(in.tint);
        }

        void to_json(nlohmann::json& j, const Color& value)
        {
            nlohmann::json add(value.add);
            nlohmann::json brightness(value.brightness);
            nlohmann::json contrast(value.contrast);
            nlohmann::json saturation(value.saturation);
            j["enabled"] = value.enabled;
            j["add"] = add;
            j["brightness"] = brightness;
            j["contrast"] = contrast;
            j["saturation"] = saturation;
            j["tint"] = value.tint;
            j["invert"] = value.invert;
        }

        void from_json(const nlohmann::json& j, Color& value)
        {
            j.at("enabled").get_to(value.enabled);
            j.at("add").get_to(value.add);
            j.at("brightness").get_to(value.brightness);
            j.at("contrast").get_to(value.contrast);
            j.at("saturation").get_to(value.saturation);
            j.at("tint").get_to(value.tint);
            j.at("invert").get_to(value.invert);
        }

        void to_json(nlohmann::json& j, const Levels& value)
        {
            j["enabled"] = value.enabled;
            j["inLow"] = value.inLow;
            j["inHigh"] = value.inHigh;
            j["gamma"] = value.gamma;
            j["outLow"] = value.outLow;
            j["outHigh"] = value.outHigh;
        }

        void from_json(const nlohmann::json& j, Levels& value)
        {
            j.at("enabled").get_to(value.enabled);
            j.at("inLow").get_to(value.inLow);
            j.at("inHigh").get_to(value.inHigh);
            j.at("gamma").get_to(value.gamma);
            j.at("outLow").get_to(value.outLow);
            j.at("outHigh").get_to(value.outHigh);
        }

        void to_json(nlohmann::json& j, const EXRDisplay& value) {}

        void from_json(const nlohmann::json& j, EXRDisplay& value) {}

        void to_json(nlohmann::json& j, const SoftClip& value)
        {
            j["enabled"] = value.enabled;
            j["value"] = value.value;
        }

        void from_json(const nlohmann::json& j, SoftClip& value)
        {
            j.at("enabled").get_to(value.enabled);
            j.at("value").get_to(value.value);
        }

        void to_json(nlohmann::json& j, const Normalize& value)
        {
            j["enabled"] = value.enabled;
            j["minimum"] = value.minimum;
            j["maximum"] = value.maximum;
        }

        void from_json(const nlohmann::json& j, Normalize& value)
        {
            j.at("enabled").get_to(value.enabled);
            j.at("minimum").get_to(value.minimum);
            j.at("maximum").get_to(value.maximum);
        }

        void to_json(nlohmann::json& j, const DisplayOptions& value)
        {
            nlohmann::json mirror(value.mirror);
            nlohmann::json color(value.color);
            nlohmann::json levels(value.levels);
            nlohmann::json softClip(value.softClip);
            nlohmann::json imageFilters(value.imageFilters);
            j["channels"] = value.channels;
            j["mirror"] = mirror;
            j["color"] = color;
            j["levels"] = levels;
            j["softClip"] = softClip;
            j["imageFilters"] = imageFilters;
            j["videoLevels"] = value.videoLevels;
            j["normalize"] = value.normalize;
            j["hdrInfo"] = value.hdrInfo;
            j["ignoreChromaticities"] = value.ignoreChromaticities;
            j["invalidValues"] = value.invalidValues;
        }

        void from_json(const nlohmann::json& j, DisplayOptions& value)
        {
            j.at("channels").get_to(value.channels);
            j.at("mirror").get_to(value.mirror);
            j.at("color").get_to(value.color);
            j.at("levels").get_to(value.levels);
            j.at("softClip").get_to(value.softClip);
            j.at("imageFilters").get_to(value.imageFilters);
            j.at("videoLevels").get_to(value.videoLevels);
            j.at("normalize").get_to(value.normalize);
            j.at("hdrInfo").get_to(value.hdrInfo);
            j.at("ignoreChromaticities").get_to(value.ignoreChromaticities);
            j.at("invalidValues").get_to(value.invalidValues);
        }
        
    } // namespace timeline
} // namespace tl
