// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlPlayApp/AudioMenu.h>

#include <tlPlayApp/App.h>

#include <tlPlay/AudioModel.h>

namespace tl
{
    namespace play_app
    {
        struct AudioMenu::Private
        {
            std::map<std::string, std::shared_ptr<ui::Action> > actions;

            std::shared_ptr<observer::ValueObserver<float> > volumeObserver;
            std::shared_ptr<observer::ValueObserver<bool> > muteObserver;
        };

        void AudioMenu::_init(
            const std::map<std::string, std::shared_ptr<ui::Action> >& actions,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            Menu::_init(context, parent);
            TLRENDER_P();

            p.actions = actions;

            addItem(p.actions["VolumeUp"]);
            addItem(p.actions["VolumeDown"]);
            addItem(p.actions["Mute"]);

            p.volumeObserver = observer::ValueObserver<float>::create(
                app->getAudioModel()->observeVolume(),
                [this](float value)
                {
                    setItemEnabled(_p->actions["VolumeUp"], value < 1.F);
                    setItemEnabled(_p->actions["VolumeDown"], value > 0.F);
                });

            p.muteObserver = observer::ValueObserver<bool>::create(
                app->getAudioModel()->observeMute(), [this](bool value)
                { setItemChecked(_p->actions["Mute"], value); });
        }

        AudioMenu::AudioMenu() :
            _p(new Private)
        {
        }

        AudioMenu::~AudioMenu() {}

        std::shared_ptr<AudioMenu> AudioMenu::create(
            const std::map<std::string, std::shared_ptr<ui::Action> >& actions,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<AudioMenu>(new AudioMenu);
            out->_init(actions, app, context, parent);
            return out;
        }
    } // namespace play_app
} // namespace tl
