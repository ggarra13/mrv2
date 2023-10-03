// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include "mrvNetwork/mrvImageOptions.h"
#include "mrvNetwork/mrvDisplayOptions.h"

namespace tl
{
    namespace timeline
    {

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
        }
    } // namespace timeline
} // namespace tl
