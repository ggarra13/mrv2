// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlPlayApp/WindowMenu.h>

#include <tlPlayApp/App.h>
#include <tlPlayApp/MainWindow.h>

namespace tl
{
    namespace play_app
    {
        struct WindowMenu::Private
        {
            std::map<std::string, std::shared_ptr<ui::Action> > actions;
            std::map<std::string, std::shared_ptr<ui::Menu> > menus;

            std::shared_ptr<observer::ValueObserver<bool> > fullScreenObserver;
            std::shared_ptr<observer::ValueObserver<bool> > floatOnTopObserver;
            std::shared_ptr<observer::ValueObserver<bool> > secondaryObserver;
            std::shared_ptr<observer::ValueObserver<WindowOptions> >
                optionsObserver;
        };

        void WindowMenu::_init(
            const std::map<std::string, std::shared_ptr<ui::Action> >& actions,
            const std::shared_ptr<MainWindow>& mainWindow,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            Menu::_init(context, parent);
            TLRENDER_P();

            p.actions = actions;

            p.menus["Resize"] = addSubMenu("Resize");
            auto mainWindowWeak = std::weak_ptr<MainWindow>(mainWindow);
            auto action = std::make_shared<ui::Action>(
                "1280x720",
                [mainWindowWeak]
                {
                    if (auto mainWindow = mainWindowWeak.lock())
                    {
                        mainWindow->setWindowSize(math::Size2i(1280, 720));
                    }
                });
            p.menus["Resize"]->addItem(action);
            action = std::make_shared<ui::Action>(
                "1920x1080",
                [mainWindowWeak]
                {
                    if (auto mainWindow = mainWindowWeak.lock())
                    {
                        mainWindow->setWindowSize(math::Size2i(1920, 1080));
                    }
                });
            p.menus["Resize"]->addItem(action);

            addDivider();
            addItem(p.actions["FullScreen"]);
            addItem(p.actions["FloatOnTop"]);
            addDivider();
            addItem(p.actions["Secondary"]);
            addDivider();
            addItem(p.actions["FileToolBar"]);
            addItem(p.actions["CompareToolBar"]);
            addItem(p.actions["WindowToolBar"]);
            addItem(p.actions["ViewToolBar"]);
            addItem(p.actions["ToolsToolBar"]);
            addItem(p.actions["Timeline"]);
            addItem(p.actions["BottomToolBar"]);
            addItem(p.actions["StatusToolBar"]);

            p.fullScreenObserver = observer::ValueObserver<bool>::create(
                mainWindow->observeFullScreen(), [this](bool value)
                { setItemChecked(_p->actions["FullScreen"], value); });

            p.floatOnTopObserver = observer::ValueObserver<bool>::create(
                mainWindow->observeFloatOnTop(), [this](bool value)
                { setItemChecked(_p->actions["FloatOnTop"], value); });

            p.secondaryObserver = observer::ValueObserver<bool>::create(
                app->observeSecondaryWindow(), [this](bool value)
                { setItemChecked(_p->actions["Secondary"], value); });

            p.optionsObserver = observer::ValueObserver<WindowOptions>::create(
                mainWindow->observeWindowOptions(),
                [this](const WindowOptions& value)
                {
                    setItemChecked(
                        _p->actions["FileToolBar"], value.fileToolBar);
                    setItemChecked(
                        _p->actions["CompareToolBar"], value.compareToolBar);
                    setItemChecked(
                        _p->actions["WindowToolBar"], value.windowToolBar);
                    setItemChecked(
                        _p->actions["ViewToolBar"], value.viewToolBar);
                    setItemChecked(
                        _p->actions["ToolsToolBar"], value.toolsToolBar);
                    setItemChecked(_p->actions["Timeline"], value.timeline);
                    setItemChecked(
                        _p->actions["BottomToolBar"], value.bottomToolBar);
                    setItemChecked(
                        _p->actions["StatusToolBar"], value.statusToolBar);
                });
        }

        WindowMenu::WindowMenu() :
            _p(new Private)
        {
        }

        WindowMenu::~WindowMenu() {}

        std::shared_ptr<WindowMenu> WindowMenu::create(
            const std::map<std::string, std::shared_ptr<ui::Action> >& actions,
            const std::shared_ptr<MainWindow>& mainWindow,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<WindowMenu>(new WindowMenu);
            out->_init(actions, mainWindow, app, context, parent);
            return out;
        }

        void WindowMenu::close()
        {
            Menu::close();
            TLRENDER_P();
            for (const auto menu : p.menus)
            {
                menu.second->close();
            }
        }
    } // namespace play_app
} // namespace tl
