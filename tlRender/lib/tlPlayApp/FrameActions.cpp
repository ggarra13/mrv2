// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlPlayApp/FrameActions.h>

#include <tlPlayApp/App.h>
#include <tlPlayApp/MainWindow.h>

#include <tlCore/StringFormat.h>

namespace tl
{
    namespace play_app
    {
        struct FrameActions::Private
        {
            std::map<std::string, std::shared_ptr<ui::Action> > actions;
        };

        void FrameActions::_init(
            const std::shared_ptr<MainWindow>& mainWindow,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context)
        {
            TLRENDER_P();

            auto appWeak = std::weak_ptr<App>(app);
            p.actions["Start"] = std::make_shared<ui::Action>(
                "Go To Start", "TimeStart", ui::Key::Home, 0,
                [appWeak]
                {
                    if (auto app = appWeak.lock())
                    {
                        if (auto player = app->observePlayer()->get())
                        {
                            player->start();
                        }
                    }
                });
            p.actions["Start"]->toolTip =
                string::Format("Go to the start frame\n"
                               "\n"
                               "Shortcut: {0}")
                    .arg(ui::getLabel(
                        p.actions["Start"]->shortcut,
                        p.actions["Start"]->shortcutModifiers));

            p.actions["End"] = std::make_shared<ui::Action>(
                "Go To End", "TimeEnd", ui::Key::End, 0,
                [appWeak]
                {
                    if (auto app = appWeak.lock())
                    {
                        if (auto player = app->observePlayer()->get())
                        {
                            player->end();
                        }
                    }
                });
            p.actions["End"]->toolTip =
                string::Format("Go to the end frame\n"
                               "\n"
                               "Shortcut: {0}")
                    .arg(ui::getLabel(
                        p.actions["End"]->shortcut,
                        p.actions["End"]->shortcutModifiers));

            p.actions["Prev"] = std::make_shared<ui::Action>(
                "Previous Frame", "FramePrev", ui::Key::Left, 0,
                [appWeak]
                {
                    if (auto app = appWeak.lock())
                    {
                        if (auto player = app->observePlayer()->get())
                        {
                            player->framePrev();
                        }
                    }
                });
            p.actions["Prev"]->toolTip =
                string::Format("Go to the previous frame\n"
                               "\n"
                               "Shortcut: {0}")
                    .arg(ui::getLabel(
                        p.actions["Prev"]->shortcut,
                        p.actions["Prev"]->shortcutModifiers));

            p.actions["PrevX10"] = std::make_shared<ui::Action>(
                "Previous Frame X10", ui::Key::Left,
                static_cast<int>(ui::KeyModifier::Shift),
                [appWeak]
                {
                    if (auto app = appWeak.lock())
                    {
                        if (auto player = app->observePlayer()->get())
                        {
                            player->timeAction(
                                timeline::TimeAction::FramePrevX10);
                        }
                    }
                });

            p.actions["PrevX100"] = std::make_shared<ui::Action>(
                "Previous Frame X100", ui::Key::Left,
                static_cast<int>(ui::KeyModifier::Control),
                [appWeak]
                {
                    if (auto app = appWeak.lock())
                    {
                        if (auto player = app->observePlayer()->get())
                        {
                            player->timeAction(
                                timeline::TimeAction::FramePrevX100);
                        }
                    }
                });

            p.actions["Next"] = std::make_shared<ui::Action>(
                "Next Frame", "FrameNext", ui::Key::Right, 0,
                [appWeak]
                {
                    if (auto app = appWeak.lock())
                    {
                        if (auto player = app->observePlayer()->get())
                        {
                            player->frameNext();
                        }
                    }
                });
            p.actions["Next"]->toolTip =
                string::Format("Go to the next frame\n"
                               "\n"
                               "Shortcut: {0}")
                    .arg(ui::getLabel(
                        p.actions["Next"]->shortcut,
                        p.actions["Next"]->shortcutModifiers));

            p.actions["NextX10"] = std::make_shared<ui::Action>(
                "Next Frame X10", ui::Key::Right,
                static_cast<int>(ui::KeyModifier::Shift),
                [appWeak]
                {
                    if (auto app = appWeak.lock())
                    {
                        if (auto player = app->observePlayer()->get())
                        {
                            player->timeAction(
                                timeline::TimeAction::FrameNextX10);
                        }
                    }
                });

            p.actions["NextX100"] = std::make_shared<ui::Action>(
                "Next Frame X100", ui::Key::Right,
                static_cast<int>(ui::KeyModifier::Control),
                [appWeak]
                {
                    if (auto app = appWeak.lock())
                    {
                        if (auto player = app->observePlayer()->get())
                        {
                            player->timeAction(
                                timeline::TimeAction::FrameNextX100);
                        }
                    }
                });

            auto mainWindowWeak = std::weak_ptr<MainWindow>(mainWindow);
            p.actions["FocusCurrent"] = std::make_shared<ui::Action>(
                "Focus Current Frame", ui::Key::F,
                static_cast<int>(ui::KeyModifier::Control),
                [mainWindowWeak]
                {
                    if (auto mainWindow = mainWindowWeak.lock())
                    {
                        mainWindow->focusCurrentFrame();
                    }
                });
        }

        FrameActions::FrameActions() :
            _p(new Private)
        {
        }

        FrameActions::~FrameActions() {}

        std::shared_ptr<FrameActions> FrameActions::create(
            const std::shared_ptr<MainWindow>& mainWindow,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context)
        {
            auto out = std::shared_ptr<FrameActions>(new FrameActions);
            out->_init(mainWindow, app, context);
            return out;
        }

        const std::map<std::string, std::shared_ptr<ui::Action> >&
        FrameActions::getActions() const
        {
            return _p->actions;
        }
    } // namespace play_app
} // namespace tl
