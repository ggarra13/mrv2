// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlPlayApp/AudioActions.h>

#include <tlPlayApp/App.h>

#include <tlPlay/AudioModel.h>

namespace tl
{
    namespace play_app
    {
        struct AudioActions::Private
        {
            std::map<std::string, std::shared_ptr<ui::Action> > actions;
        };

        void AudioActions::_init(
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context)
        {
            TLRENDER_P();

            auto appWeak = std::weak_ptr<App>(app);
            p.actions["VolumeUp"] = std::make_shared<ui::Action>(
                "Volume Up", ui::Key::Period, 0,
                [appWeak]
                {
                    if (auto app = appWeak.lock())
                    {
                        app->getAudioModel()->volumeUp();
                    }
                });

            p.actions["VolumeDown"] = std::make_shared<ui::Action>(
                "Volume Down", ui::Key::Comma, 0,
                [appWeak]
                {
                    if (auto app = appWeak.lock())
                    {
                        app->getAudioModel()->volumeDown();
                    }
                });

            p.actions["Mute"] = std::make_shared<ui::Action>(
                "Mute", "Mute", ui::Key::M, 0,
                [appWeak](bool value)
                {
                    if (auto app = appWeak.lock())
                    {
                        app->getAudioModel()->setMute(value);
                    }
                });
        }

        AudioActions::AudioActions() :
            _p(new Private)
        {
        }

        AudioActions::~AudioActions() {}

        std::shared_ptr<AudioActions> AudioActions::create(
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context)
        {
            auto out = std::shared_ptr<AudioActions>(new AudioActions);
            out->_init(app, context);
            return out;
        }

        const std::map<std::string, std::shared_ptr<ui::Action> >&
        AudioActions::getActions() const
        {
            return _p->actions;
        }
    } // namespace play_app
} // namespace tl
