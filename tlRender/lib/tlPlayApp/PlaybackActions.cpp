// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlPlayApp/PlaybackActions.h>

#include <tlPlayApp/App.h>

#include <tlTimelineUI/TimelineWidget.h>

#include <tlCore/StringFormat.h>

namespace tl
{
    namespace play_app
    {
        struct PlaybackActions::Private
        {
            std::map<std::string, std::shared_ptr<ui::Action> > actions;
            timeline::Playback playbackPrev = timeline::Playback::Forward;
        };

        void PlaybackActions::_init(
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context)
        {
            TLRENDER_P();

            auto appWeak = std::weak_ptr<App>(app);
            p.actions["Stop"] = std::make_shared<ui::Action>(
                "Stop", "PlaybackStop", ui::Key::K, 0,
                [appWeak](bool value)
                {
                    if (auto app = appWeak.lock())
                    {
                        if (auto player = app->observePlayer()->get())
                        {
                            player->setPlayback(timeline::Playback::Stop);
                        }
                    }
                });
            p.actions["Stop"]->toolTip =
                string::Format("Stop playback\n"
                               "\n"
                               "Shortcut: {0}")
                    .arg(ui::getLabel(
                        p.actions["Stop"]->shortcut,
                        p.actions["Stop"]->shortcutModifiers));

            p.actions["Forward"] = std::make_shared<ui::Action>(
                "Forward", "PlaybackForward", ui::Key::L, 0,
                [appWeak](bool value)
                {
                    if (auto app = appWeak.lock())
                    {
                        if (auto player = app->observePlayer()->get())
                        {
                            player->setPlayback(timeline::Playback::Forward);
                        }
                    }
                });
            p.actions["Forward"]->toolTip =
                string::Format("Forward playback\n"
                               "\n"
                               "Shortcut: {0}")
                    .arg(ui::getLabel(
                        p.actions["Forward"]->shortcut,
                        p.actions["Forward"]->shortcutModifiers));

            p.actions["Reverse"] = std::make_shared<ui::Action>(
                "Reverse", "PlaybackReverse", ui::Key::J, 0,
                [appWeak](bool value)
                {
                    if (auto app = appWeak.lock())
                    {
                        if (auto player = app->observePlayer()->get())
                        {
                            player->setPlayback(timeline::Playback::Reverse);
                        }
                    }
                });
            p.actions["Reverse"]->toolTip =
                string::Format("Reverse playback\n"
                               "\n"
                               "Shortcut: {0}")
                    .arg(ui::getLabel(
                        p.actions["Reverse"]->shortcut,
                        p.actions["Reverse"]->shortcutModifiers));

            p.actions["Toggle"] = std::make_shared<ui::Action>(
                "Toggle Playback", ui::Key::Space, 0,
                [this, appWeak]
                {
                    if (auto app = appWeak.lock())
                    {
                        if (auto player = app->observePlayer()->get())
                        {
                            const timeline::Playback playback =
                                player->observePlayback()->get();
                            player->setPlayback(
                                timeline::Playback::Stop == playback
                                    ? _p->playbackPrev
                                    : timeline::Playback::Stop);
                            if (playback != timeline::Playback::Stop)
                            {
                                _p->playbackPrev = playback;
                            }
                        }
                    }
                });

            p.actions["JumpBack1s"] = std::make_shared<ui::Action>(
                "Jump Back 1s", ui::Key::J,
                static_cast<int>(ui::KeyModifier::Shift),
                [appWeak]
                {
                    if (auto app = appWeak.lock())
                    {
                        if (auto player = app->observePlayer()->get())
                        {
                            player->timeAction(
                                timeline::TimeAction::JumpBack1s);
                        }
                    }
                });

            p.actions["JumpBack10s"] = std::make_shared<ui::Action>(
                "Jump Back 10s", ui::Key::J,
                static_cast<int>(ui::KeyModifier::Control),
                [appWeak]
                {
                    if (auto app = appWeak.lock())
                    {
                        if (auto player = app->observePlayer()->get())
                        {
                            player->timeAction(
                                timeline::TimeAction::JumpBack10s);
                        }
                    }
                });

            p.actions["JumpForward1s"] = std::make_shared<ui::Action>(
                "Jump Forward 1s", ui::Key::L,
                static_cast<int>(ui::KeyModifier::Shift),
                [appWeak]
                {
                    if (auto app = appWeak.lock())
                    {
                        if (auto player = app->observePlayer()->get())
                        {
                            player->timeAction(
                                timeline::TimeAction::JumpForward1s);
                        }
                    }
                });

            p.actions["JumpForward10s"] = std::make_shared<ui::Action>(
                "Jump Forward 10s", ui::Key::L,
                static_cast<int>(ui::KeyModifier::Control),
                [appWeak]
                {
                    if (auto app = appWeak.lock())
                    {
                        if (auto player = app->observePlayer()->get())
                        {
                            player->timeAction(
                                timeline::TimeAction::JumpForward10s);
                        }
                    }
                });

            p.actions["Loop"] = std::make_shared<ui::Action>(
                "Loop Playback",
                [appWeak]
                {
                    if (auto app = appWeak.lock())
                    {
                        if (auto player = app->observePlayer()->get())
                        {
                            player->setLoop(timeline::Loop::Loop);
                        }
                    }
                });

            p.actions["Once"] = std::make_shared<ui::Action>(
                "Playback Once",
                [appWeak]
                {
                    if (auto app = appWeak.lock())
                    {
                        if (auto player = app->observePlayer()->get())
                        {
                            player->setLoop(timeline::Loop::Once);
                        }
                    }
                });

            p.actions["PingPong"] = std::make_shared<ui::Action>(
                "Ping-Pong Playback",
                [appWeak]
                {
                    if (auto app = appWeak.lock())
                    {
                        if (auto player = app->observePlayer()->get())
                        {
                            player->setLoop(timeline::Loop::PingPong);
                        }
                    }
                });

            p.actions["SetInPoint"] = std::make_shared<ui::Action>(
                "Set In Point", ui::Key::I, 0,
                [appWeak]
                {
                    if (auto app = appWeak.lock())
                    {
                        if (auto player = app->observePlayer()->get())
                        {
                            player->setInPoint();
                        }
                    }
                });

            p.actions["ResetInPoint"] = std::make_shared<ui::Action>(
                "Reset In Point", ui::Key::I,
                static_cast<int>(ui::KeyModifier::Shift),
                [appWeak]
                {
                    if (auto app = appWeak.lock())
                    {
                        if (auto player = app->observePlayer()->get())
                        {
                            player->resetInPoint();
                        }
                    }
                });

            p.actions["SetOutPoint"] = std::make_shared<ui::Action>(
                "Set Out Point", ui::Key::O, 0,
                [appWeak]
                {
                    if (auto app = appWeak.lock())
                    {
                        if (auto player = app->observePlayer()->get())
                        {
                            player->setOutPoint();
                        }
                    }
                });

            p.actions["ResetOutPoint"] = std::make_shared<ui::Action>(
                "Reset Out Point", ui::Key::O,
                static_cast<int>(ui::KeyModifier::Shift),
                [appWeak]
                {
                    if (auto app = appWeak.lock())
                    {
                        if (auto player = app->observePlayer()->get())
                        {
                            player->resetOutPoint();
                        }
                    }
                });
        }

        PlaybackActions::PlaybackActions() :
            _p(new Private)
        {
        }

        PlaybackActions::~PlaybackActions() {}

        std::shared_ptr<PlaybackActions> PlaybackActions::create(
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context)
        {
            auto out = std::shared_ptr<PlaybackActions>(new PlaybackActions);
            out->_init(app, context);
            return out;
        }

        const std::map<std::string, std::shared_ptr<ui::Action> >&
        PlaybackActions::getActions() const
        {
            return _p->actions;
        }
    } // namespace play_app
} // namespace tl
