// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlTimeline/DisplayOptions.h>

#include <tlCore/Error.h>
#include <tlCore/String.h>
#include <tlCore/StringFormat.h>

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
            AspectRatioType, "Pixel", "Display");
        TLRENDER_ENUM_SERIALIZE_IMPL(AspectRatioType);

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
        {            return math::Matrix4x4f(
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

        std::string getLabel(const AspectRatio& value)
        {
            return string::Format("{0}:{1}").
                arg(value.num).
                arg(value.den);
        }

        std::string getLabel(const AspectRatioOptions& value)
        {
            return string::Format("{0} {1}").
                arg(getLabel(value.value)).
                arg(value.type);
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

        void to_json(nlohmann::json& json, const AspectRatio& in)
        {
            json["num"] = in.num;
            json["den"] = in.den;
        }

        void from_json(const nlohmann::json& json, AspectRatio& out)
        {
            json.at("num").get_to(out.num);
            json.at("den").get_to(out.den);
        }

        void to_json(nlohmann::json& json, const AspectRatioOptions& in)
        {
            json["value"] = in.value;
            json["type"] = to_string(in.type);
        }

        void from_json(const nlohmann::json& json, AspectRatioOptions& out)
        {
            json.at("value").get_to(out.value);
            from_string(json.at("type").get<std::string>(), out.type);
        }

        void to_json(nlohmann::json& j, const DisplayOptions& value)
        {
            nlohmann::json mirror(value.mirror);
            nlohmann::json color(value.color);
            nlohmann::json levels(value.levels);
            nlohmann::json softClip(value.softClip);
            nlohmann::json imageFilters(value.imageFilters);
            nlohmann::json aspect(value.aspect);
            j["channels"] = value.channels;
            j["aspect"] = aspect;
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
            if (j.contains("aspect"))
                j.at("aspect").get_to(value.aspect);
            else
                value.aspect = AspectRatioOptions();
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

        float getAspectRatio(
            const image::Info& info,
            const AspectRatioOptions& options)
        {
            float out = 0.F;
            if (options.value.isValid())
            {
                switch (options.type)
                {
                case AspectRatioType::Pixel:
                    out = info.getAspect() * options.value;
                    break;
                case AspectRatioType::Display:
                    out = options.value;
                    break;
                default: break;
                }
            }
            else
            {
                out = info.getAspect();
            }
            return out;
        }

        math::Size2i getRenderSize(
            const image::Info& info,
            const AspectRatioOptions& options)
        {
            math::Size2i out;
            if (options.value.isValid())
            {
                switch (options.type)
                {
                case AspectRatioType::Pixel:
                    out.w = info.size.w * options.value;
                    out.h = info.size.h;
                    break;
                case AspectRatioType::Display:
                    out.w = info.size.h * options.value;
                    out.h = info.size.h;
                    break;
                default: break;
                }
            }
            else
            {
                out.w = info.size.w * info.size.pixelAspectRatio;
                out.h = info.size.h;
            }
            return out;
        }

        math::Box2i getBox(
            const math::Box2i& box,
            const image::Info& info,
            const AspectRatioOptions& options,
            BoxHAlign hAlign,
            BoxVAlign vAlign)
        {
            math::Box2i out;
            const math::Size2i boxSize = box.getSize();
            const float boxAspect = math::aspectRatio(boxSize);
            const float aspect = getAspectRatio(info, options);
            if (boxAspect > aspect)
            {
                const int w = boxSize.h * aspect;
                const int h = boxSize.h;
                int x = box.min.x;
                switch (hAlign)
                {
                case BoxHAlign::Center:
                    x += boxSize.w / 2.F - (boxSize.h * aspect) / 2.F;
                    break;
                case BoxHAlign::Right:
                    x += boxSize.w - w;
                    break;
                default: break;
                }
                out = math::Box2i(x, box.min.y, w, h);
            }
            else
            {
                const int w = boxSize.w;
                const int h = boxSize.w / aspect;
                int y = box.min.y;
                switch (vAlign)
                {
                case BoxVAlign::Center:
                    y += boxSize.h / 2.F - (boxSize.w / aspect) / 2.F;
                    break;
                case BoxVAlign::Bottom:
                    y += boxSize.h - h;
                    break;
                default: break;
                }
                out = math::Box2i(box.min.x, y, w, h);
            }
            return out;
        }

    } // namespace timeline
} // namespace tl
