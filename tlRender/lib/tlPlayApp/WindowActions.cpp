// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlPlayApp/WindowActions.h>

#include <tlPlayApp/App.h>
#include <tlPlayApp/MainWindow.h>

#include <tlCore/StringFormat.h>

namespace tl
{
    namespace play_app
    {
        struct WindowActions::Private
        {
            std::map<std::string, std::shared_ptr<ui::Action> > actions;
        };

        void WindowActions::_init(
            const std::shared_ptr<MainWindow>& mainWindow,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context)
        {
            TLRENDER_P();

            auto appWeak = std::weak_ptr<App>(app);
            p.actions["FullScreen"] = std::make_shared<ui::Action>(
                "Full Screen", "WindowFullScreen", ui::Key::U, 0,
                [appWeak](bool value)
                {
                    if (auto app = appWeak.lock())
                    {
                        app->getMainWindow()->setFullScreen(value);
                    }
                });
            p.actions["FullScreen"]->toolTip =
                string::Format("Toggle the window full screen\n"
                               "\n"
                               "Shortcut: {0}")
                    .arg(ui::getLabel(
                        p.actions["FullScreen"]->shortcut,
                        p.actions["FullScreen"]->shortcutModifiers));

            p.actions["FloatOnTop"] = std::make_shared<ui::Action>(
                "Float On Top",
                [appWeak](bool value)
                {
                    if (auto app = appWeak.lock())
                    {
                        app->getMainWindow()->setFloatOnTop(value);
                    }
                });

            p.actions["Secondary"] = std::make_shared<ui::Action>(
                "Secondary", "WindowSecondary", ui::Key::Y, 0,
                [appWeak](bool value)
                {
                    if (auto app = appWeak.lock())
                    {
                        app->setSecondaryWindow(value);
                    }
                });
            p.actions["Secondary"]->toolTip =
                string::Format("Toggle the secondary window\n"
                               "\n"
                               "Shortcut: {0}")
                    .arg(ui::getLabel(
                        p.actions["Secondary"]->shortcut,
                        p.actions["Secondary"]->shortcutModifiers));

            auto mainWindowWeak = std::weak_ptr<MainWindow>(mainWindow);
            p.actions["FileToolBar"] = std::make_shared<ui::Action>(
                "File Tool Bar",
                [mainWindowWeak](bool value)
                {
                    if (auto mainWindow = mainWindowWeak.lock())
                    {
                        auto options = mainWindow->getWindowOptions();
                        options.fileToolBar = value;
                        mainWindow->setWindowOptions(options);
                    }
                });

            p.actions["CompareToolBar"] = std::make_shared<ui::Action>(
                "Compare Tool Bar",
                [mainWindowWeak](bool value)
                {
                    if (auto mainWindow = mainWindowWeak.lock())
                    {
                        auto options = mainWindow->getWindowOptions();
                        options.compareToolBar = value;
                        mainWindow->setWindowOptions(options);
                    }
                });

            p.actions["WindowToolBar"] = std::make_shared<ui::Action>(
                "Window Tool Bar",
                [mainWindowWeak](bool value)
                {
                    if (auto mainWindow = mainWindowWeak.lock())
                    {
                        auto options = mainWindow->getWindowOptions();
                        options.windowToolBar = value;
                        mainWindow->setWindowOptions(options);
                    }
                });

            p.actions["ViewToolBar"] = std::make_shared<ui::Action>(
                "View Tool Bar",
                [mainWindowWeak](bool value)
                {
                    if (auto mainWindow = mainWindowWeak.lock())
                    {
                        auto options = mainWindow->getWindowOptions();
                        options.viewToolBar = value;
                        mainWindow->setWindowOptions(options);
                    }
                });

            p.actions["ToolsToolBar"] = std::make_shared<ui::Action>(
                "Tools Tool Bar",
                [mainWindowWeak](bool value)
                {
                    if (auto mainWindow = mainWindowWeak.lock())
                    {
                        auto options = mainWindow->getWindowOptions();
                        options.toolsToolBar = value;
                        mainWindow->setWindowOptions(options);
                    }
                });

            p.actions["Timeline"] = std::make_shared<ui::Action>(
                "Timeline",
                [mainWindowWeak](bool value)
                {
                    if (auto mainWindow = mainWindowWeak.lock())
                    {
                        auto options = mainWindow->getWindowOptions();
                        options.timeline = value;
                        mainWindow->setWindowOptions(options);
                    }
                });

            p.actions["BottomToolBar"] = std::make_shared<ui::Action>(
                "Bottom Tool Bar",
                [mainWindowWeak](bool value)
                {
                    if (auto mainWindow = mainWindowWeak.lock())
                    {
                        auto options = mainWindow->getWindowOptions();
                        options.bottomToolBar = value;
                        mainWindow->setWindowOptions(options);
                    }
                });

            p.actions["StatusToolBar"] = std::make_shared<ui::Action>(
                "Status Tool Bar",
                [mainWindowWeak](bool value)
                {
                    if (auto mainWindow = mainWindowWeak.lock())
                    {
                        auto options = mainWindow->getWindowOptions();
                        options.statusToolBar = value;
                        mainWindow->setWindowOptions(options);
                    }
                });
        }

        WindowActions::WindowActions() :
            _p(new Private)
        {
        }

        WindowActions::~WindowActions() {}

        std::shared_ptr<WindowActions> WindowActions::create(
            const std::shared_ptr<MainWindow>& mainWindow,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context)
        {
            auto out = std::shared_ptr<WindowActions>(new WindowActions);
            out->_init(mainWindow, app, context);
            return out;
        }

        const std::map<std::string, std::shared_ptr<ui::Action> >&
        WindowActions::getActions() const
        {
            return _p->actions;
        }
    } // namespace play_app
} // namespace tl
