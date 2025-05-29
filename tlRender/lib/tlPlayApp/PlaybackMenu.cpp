// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlPlayApp/PlaybackMenu.h>

#include <tlPlayApp/App.h>

#include <tlTimelineUI/TimelineWidget.h>

namespace tl
{
    namespace play_app
    {
        struct PlaybackMenu::Private
        {
            std::shared_ptr<timeline::Player> player;

            std::map<std::string, std::shared_ptr<ui::Action> > actions;
            std::map<timeline::Playback, std::shared_ptr<ui::Action> >
                playbackItems;
            std::map<timeline::Loop, std::shared_ptr<ui::Action> > loopItems;

            std::shared_ptr<
                observer::ValueObserver<std::shared_ptr<timeline::Player> > >
                playerObserver;
            std::shared_ptr<observer::ValueObserver<timeline::Playback> >
                playbackObserver;
            std::shared_ptr<observer::ValueObserver<timeline::Loop> >
                loopObserver;
        };

        void PlaybackMenu::_init(
            const std::map<std::string, std::shared_ptr<ui::Action> >& actions,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            Menu::_init(context, parent);
            TLRENDER_P();

            p.actions = actions;
            addItem(p.actions["Stop"]);
            addItem(p.actions["Forward"]);
            addItem(p.actions["Reverse"]);
            addItem(p.actions["Toggle"]);
            addDivider();
            addItem(p.actions["JumpBack1s"]);
            addItem(p.actions["JumpBack10s"]);
            addItem(p.actions["JumpForward1s"]);
            addItem(p.actions["JumpForward10s"]);
            addDivider();
            addItem(p.actions["Loop"]);
            addItem(p.actions["Once"]);
            addItem(p.actions["PingPong"]);
            addDivider();
            addItem(p.actions["SetInPoint"]);
            addItem(p.actions["ResetInPoint"]);
            addItem(p.actions["SetOutPoint"]);
            addItem(p.actions["ResetOutPoint"]);

            p.playbackItems[timeline::Playback::Stop] = p.actions["Stop"];
            p.playbackItems[timeline::Playback::Forward] = p.actions["Forward"];
            p.playbackItems[timeline::Playback::Reverse] = p.actions["Reverse"];

            p.loopItems[timeline::Loop::Loop] = p.actions["Loop"];
            p.loopItems[timeline::Loop::Once] = p.actions["Once"];
            p.loopItems[timeline::Loop::PingPong] = p.actions["PingPong"];

            _playbackUpdate();
            _loopUpdate();

            p.playerObserver = observer::
                ValueObserver<std::shared_ptr<timeline::Player> >::create(
                    app->observePlayer(),
                    [this](const std::shared_ptr<timeline::Player>& value)
                    { _setPlayer(value); });
        }

        PlaybackMenu::PlaybackMenu() :
            _p(new Private)
        {
        }

        PlaybackMenu::~PlaybackMenu() {}

        std::shared_ptr<PlaybackMenu> PlaybackMenu::create(
            const std::map<std::string, std::shared_ptr<ui::Action> >& actions,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<PlaybackMenu>(new PlaybackMenu);
            out->_init(actions, app, context, parent);
            return out;
        }

        void
        PlaybackMenu::_setPlayer(const std::shared_ptr<timeline::Player>& value)
        {
            TLRENDER_P();
            p.playbackObserver.reset();
            p.loopObserver.reset();
            p.player = value;
            if (p.player)
            {
                p.playbackObserver =
                    observer::ValueObserver<timeline::Playback>::create(
                        p.player->observePlayback(),
                        [this](timeline::Playback) { _playbackUpdate(); });
                p.loopObserver =
                    observer::ValueObserver<timeline::Loop>::create(
                        p.player->observeLoop(),
                        [this](timeline::Loop) { _loopUpdate(); });
            }
        }

        void PlaybackMenu::_playbackUpdate()
        {
            TLRENDER_P();
            std::map<timeline::Playback, bool> values;
            for (const auto& value : timeline::getPlaybackEnums())
            {
                values[value] = false;
            }
            values
                [p.player ? p.player->observePlayback()->get()
                          : timeline::Playback::Stop] = true;
            for (auto i : values)
            {
                setItemChecked(p.playbackItems[i.first], i.second);
            }
        }

        void PlaybackMenu::_loopUpdate()
        {
            TLRENDER_P();
            std::map<timeline::Loop, bool> values;
            for (const auto& value : timeline::getLoopEnums())
            {
                values[value] = false;
            }
            values
                [p.player ? p.player->observeLoop()->get()
                          : timeline::Loop::Loop] = true;
            for (auto i : values)
            {
                setItemChecked(p.loopItems[i.first], i.second);
            }
        }
    } // namespace play_app
} // namespace tl
