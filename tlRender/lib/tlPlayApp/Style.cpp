// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlPlayApp/Style.h>

#include <tlCore/Error.h>
#include <tlCore/String.h>

namespace tl
{
    namespace play_app
    {
        TLRENDER_ENUM_IMPL(StylePalette, "Dark", "Light");
        TLRENDER_ENUM_SERIALIZE_IMPL(StylePalette);

        std::map<ui::ColorRole, image::Color4f>
        getStylePalette(StylePalette value)
        {
            auto out = ui::defaultColorRoles();
            switch (value)
            {
            case StylePalette::Light:
                out[ui::ColorRole::Window] = image::Color4f(.9F, .9F, .9F);
                out[ui::ColorRole::Base] = image::Color4f(1.F, 1.F, 1.F);
                out[ui::ColorRole::Button] = image::Color4f(.8F, .8F, .8F);
                out[ui::ColorRole::Text] = image::Color4f(0.F, 0.F, 0.F);
                out[ui::ColorRole::TextDisabled] =
                    image::Color4f(.5F, .5F, .5F);
                out[ui::ColorRole::Border] = image::Color4f(.6F, .6F, .6F);
                out[ui::ColorRole::Hover] = image::Color4f(0.F, 0.F, 0.F, .1F);
                out[ui::ColorRole::Pressed] =
                    image::Color4f(0.F, 0.F, 0.F, .2F);
                out[ui::ColorRole::Checked] = image::Color4f(.6F, .7F, 1.F);
                out[ui::ColorRole::KeyFocus] = image::Color4f(.3F, .4F, 1.F);

                out[ui::ColorRole::InOut] = image::Color4f(.4F, .5F, .9F);
                out[ui::ColorRole::VideoCache] = image::Color4f(.3F, .7F, .7F);
                out[ui::ColorRole::AudioCache] = image::Color4f(.5F, .3F, .7F);
                out[ui::ColorRole::VideoClip] = image::Color4f(.5F, .7F, .7F);
                out[ui::ColorRole::VideoGap] = image::Color4f(.55F, .61F, .61F);
                out[ui::ColorRole::AudioClip] = image::Color4f(.6F, .55F, .7F);
                out[ui::ColorRole::AudioGap] = image::Color4f(.55F, .54F, .6F);
                out[ui::ColorRole::Transition] = image::Color4f(.7F, .6F, .6F);
                break;
            default:
                break;
            }
            return out;
        }
    } // namespace play_app
} // namespace tl
