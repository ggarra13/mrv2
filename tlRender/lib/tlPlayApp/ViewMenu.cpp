// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlPlayApp/ViewMenu.h>

#include <tlPlayApp/App.h>
#include <tlPlayApp/MainWindow.h>
#include <tlPlayApp/Viewport.h>

#include <tlPlay/ViewportModel.h>

namespace tl
{
    namespace play_app
    {
        struct ViewMenu::Private
        {
            std::map<std::string, std::shared_ptr<ui::Action> > actions;
            std::map<std::string, std::shared_ptr<Menu> > menus;

            std::shared_ptr<observer::ValueObserver<bool> > frameViewObserver;
            std::shared_ptr<observer::ValueObserver<bool> > hudObserver;
            std::shared_ptr<observer::ValueObserver<timeline::DisplayOptions> >
                displayOptionsObserver;
        };

        void ViewMenu::_init(
            const std::map<std::string, std::shared_ptr<ui::Action> >& actions,
            const std::shared_ptr<MainWindow>& mainWindow,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            Menu::_init(context, parent);
            TLRENDER_P();

            p.actions = actions;

            addItem(p.actions["Frame"]);
            addItem(p.actions["Zoom1To1"]);
            addItem(p.actions["ZoomIn"]);
            addItem(p.actions["ZoomOut"]);
            addDivider();
            addItem(p.actions["Red"]);
            addItem(p.actions["Green"]);
            addItem(p.actions["Blue"]);
            addItem(p.actions["Alpha"]);
            addDivider();
            addItem(p.actions["MirrorHorizontal"]);
            addItem(p.actions["MirrorVertical"]);
            addDivider();

            p.menus["MinifyFilter"] = addSubMenu("Minify Filter");
            p.menus["MinifyFilter"]->addItem(p.actions["MinifyNearest"]);
            p.menus["MinifyFilter"]->addItem(p.actions["MinifyLinear"]);

            p.menus["MagnifyFilter"] = addSubMenu("Magnify Filter");
            p.menus["MagnifyFilter"]->addItem(p.actions["MagnifyNearest"]);
            p.menus["MagnifyFilter"]->addItem(p.actions["MagnifyLinear"]);

            addDivider();
            addItem(p.actions["HUD"]);

            p.frameViewObserver = observer::ValueObserver<bool>::create(
                mainWindow->getViewport()->observeFrameView(),
                [this](bool value)
                { setItemChecked(_p->actions["Frame"], value); });

            p.hudObserver = observer::ValueObserver<bool>::create(
                mainWindow->getViewport()->observeHUD(), [this](bool value)
                { setItemChecked(_p->actions["HUD"], value); });

            p.displayOptionsObserver =
                observer::ValueObserver<timeline::DisplayOptions>::create(
                    app->getViewportModel()->observeDisplayOptions(),
                    [this](const timeline::DisplayOptions& value)
                    {
                        setItemChecked(
                            _p->actions["Red"],
                            timeline::Channels::Red == value.channels);
                        setItemChecked(
                            _p->actions["Green"],
                            timeline::Channels::Green == value.channels);
                        setItemChecked(
                            _p->actions["Blue"],
                            timeline::Channels::Blue == value.channels);
                        setItemChecked(
                            _p->actions["Alpha"],
                            timeline::Channels::Alpha == value.channels);

                        setItemChecked(
                            _p->actions["MirrorHorizontal"], value.mirror.x);
                        setItemChecked(
                            _p->actions["MirrorVertical"], value.mirror.y);

                        _p->menus["MinifyFilter"]->setItemChecked(
                            _p->actions["MinifyNearest"],
                            timeline::ImageFilter::Nearest ==
                                value.imageFilters.minify);
                        _p->menus["MinifyFilter"]->setItemChecked(
                            _p->actions["MinifyLinear"],
                            timeline::ImageFilter::Linear ==
                                value.imageFilters.minify);

                        _p->menus["MagnifyFilter"]->setItemChecked(
                            _p->actions["MagnifyNearest"],
                            timeline::ImageFilter::Nearest ==
                                value.imageFilters.magnify);
                        _p->menus["MagnifyFilter"]->setItemChecked(
                            _p->actions["MagnifyLinear"],
                            timeline::ImageFilter::Linear ==
                                value.imageFilters.magnify);
                    });
        }

        ViewMenu::ViewMenu() :
            _p(new Private)
        {
        }

        ViewMenu::~ViewMenu() {}

        std::shared_ptr<ViewMenu> ViewMenu::create(
            const std::map<std::string, std::shared_ptr<ui::Action> >& actions,
            const std::shared_ptr<MainWindow>& mainWindow,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<ViewMenu>(new ViewMenu);
            out->_init(actions, mainWindow, app, context, parent);
            return out;
        }
    } // namespace play_app
} // namespace tl
