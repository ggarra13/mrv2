// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlPlayApp/ViewActions.h>

#include <tlPlayApp/App.h>
#include <tlPlayApp/MainWindow.h>
#include <tlPlayApp/Viewport.h>

#include <tlPlay/ViewportModel.h>

namespace tl
{
    namespace play_app
    {
        struct ViewActions::Private
        {
            std::map<std::string, std::shared_ptr<ui::Action> > actions;
        };

        void ViewActions::_init(
            const std::shared_ptr<MainWindow>& mainWindow,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context)
        {
            TLRENDER_P();

            auto mainWindowWeak = std::weak_ptr<MainWindow>(mainWindow);
            p.actions["Frame"] = std::make_shared<ui::Action>(
                "Frame", "ViewFrame",
                [mainWindowWeak](bool value)
                {
                    if (auto mainWindow = mainWindowWeak.lock())
                    {
                        mainWindow->getViewport()->setFrameView(value);
                    }
                });
            p.actions["Frame"]->toolTip = "Frame the view to fit the window";

            p.actions["Zoom1To1"] = std::make_shared<ui::Action>(
                "Zoom 1:1", "ViewZoom1To1",
                [mainWindowWeak]
                {
                    if (auto mainWindow = mainWindowWeak.lock())
                    {
                        mainWindow->getViewport()->viewZoom1To1();
                    }
                });
            p.actions["Zoom1To1"]->toolTip = "Set the view zoom to 1:1";

            p.actions["ZoomIn"] = std::make_shared<ui::Action>(
                "Zoom In",
                [mainWindowWeak]
                {
                    if (auto mainWindow = mainWindowWeak.lock())
                    {
                        mainWindow->getViewport()->viewZoomIn();
                    }
                });

            p.actions["ZoomOut"] = std::make_shared<ui::Action>(
                "Zoom Out",
                [mainWindowWeak]
                {
                    if (auto mainWindow = mainWindowWeak.lock())
                    {
                        mainWindow->getViewport()->viewZoomOut();
                    }
                });

            auto appWeak = std::weak_ptr<App>(app);
            p.actions["Red"] = std::make_shared<ui::Action>(
                "Red Channel", ui::Key::R, 0,
                [appWeak](bool value)
                {
                    if (auto app = appWeak.lock())
                    {
                        auto displayOptions =
                            app->getViewportModel()->getDisplayOptions();
                        displayOptions.channels =
                            value ? timeline::Channels::Red
                                  : timeline::Channels::Color;
                        app->getViewportModel()->setDisplayOptions(
                            displayOptions);
                    }
                });

            p.actions["Green"] = std::make_shared<ui::Action>(
                "Green Channel", ui::Key::G, 0,
                [appWeak](bool value)
                {
                    if (auto app = appWeak.lock())
                    {
                        auto displayOptions =
                            app->getViewportModel()->getDisplayOptions();
                        displayOptions.channels =
                            value ? timeline::Channels::Green
                                  : timeline::Channels::Color;
                        app->getViewportModel()->setDisplayOptions(
                            displayOptions);
                    }
                });

            p.actions["Blue"] = std::make_shared<ui::Action>(
                "Blue Channel", ui::Key::B, 0,
                [appWeak](bool value)
                {
                    if (auto app = appWeak.lock())
                    {
                        auto displayOptions =
                            app->getViewportModel()->getDisplayOptions();
                        displayOptions.channels =
                            value ? timeline::Channels::Blue
                                  : timeline::Channels::Color;
                        app->getViewportModel()->setDisplayOptions(
                            displayOptions);
                    }
                });

            p.actions["Alpha"] = std::make_shared<ui::Action>(
                "Alpha Channel", ui::Key::A, 0,
                [appWeak](bool value)
                {
                    if (auto app = appWeak.lock())
                    {
                        auto displayOptions =
                            app->getViewportModel()->getDisplayOptions();
                        displayOptions.channels =
                            value ? timeline::Channels::Alpha
                                  : timeline::Channels::Color;
                        app->getViewportModel()->setDisplayOptions(
                            displayOptions);
                    }
                });

            p.actions["MirrorHorizontal"] = std::make_shared<ui::Action>(
                "Mirror Horizontal", ui::Key::H, 0,
                [appWeak](bool value)
                {
                    if (auto app = appWeak.lock())
                    {
                        auto displayOptions =
                            app->getViewportModel()->getDisplayOptions();
                        displayOptions.mirror.x = value;
                        app->getViewportModel()->setDisplayOptions(
                            displayOptions);
                    }
                });

            p.actions["MirrorVertical"] = std::make_shared<ui::Action>(
                "Mirror Vertical", ui::Key::V, 0,
                [appWeak](bool value)
                {
                    if (auto app = appWeak.lock())
                    {
                        auto displayOptions =
                            app->getViewportModel()->getDisplayOptions();
                        displayOptions.mirror.y = value;
                        app->getViewportModel()->setDisplayOptions(
                            displayOptions);
                    }
                });

            p.actions["MinifyNearest"] = std::make_shared<ui::Action>(
                "Nearest",
                [appWeak](bool value)
                {
                    if (auto app = appWeak.lock())
                    {
                        auto displayOptions =
                            app->getViewportModel()->getDisplayOptions();
                        displayOptions.imageFilters.minify =
                            timeline::ImageFilter::Nearest;
                        app->getViewportModel()->setDisplayOptions(
                            displayOptions);
                    }
                });

            p.actions["MinifyLinear"] = std::make_shared<ui::Action>(
                "Linear",
                [appWeak](bool value)
                {
                    if (auto app = appWeak.lock())
                    {
                        auto displayOptions =
                            app->getViewportModel()->getDisplayOptions();
                        displayOptions.imageFilters.minify =
                            timeline::ImageFilter::Linear;
                        app->getViewportModel()->setDisplayOptions(
                            displayOptions);
                    }
                });

            p.actions["MagnifyNearest"] = std::make_shared<ui::Action>(
                "Nearest",
                [appWeak](bool value)
                {
                    if (auto app = appWeak.lock())
                    {
                        auto displayOptions =
                            app->getViewportModel()->getDisplayOptions();
                        displayOptions.imageFilters.magnify =
                            timeline::ImageFilter::Nearest;
                        app->getViewportModel()->setDisplayOptions(
                            displayOptions);
                    }
                });

            p.actions["MagnifyLinear"] = std::make_shared<ui::Action>(
                "Linear",
                [appWeak](bool value)
                {
                    if (auto app = appWeak.lock())
                    {
                        auto displayOptions =
                            app->getViewportModel()->getDisplayOptions();
                        displayOptions.imageFilters.magnify =
                            timeline::ImageFilter::Linear;
                        app->getViewportModel()->setDisplayOptions(
                            displayOptions);
                    }
                });

            p.actions["HUD"] = std::make_shared<ui::Action>(
                "HUD", ui::Key::H, static_cast<int>(ui::KeyModifier::Control),
                [mainWindowWeak](bool value)
                {
                    if (auto mainWindow = mainWindowWeak.lock())
                    {
                        mainWindow->getViewport()->setHUD(value);
                    }
                });
            p.actions["HUD"]->toolTip = "Toggle the HUD (Heads Up Display)";
        }

        ViewActions::ViewActions() :
            _p(new Private)
        {
        }

        ViewActions::~ViewActions() {}

        std::shared_ptr<ViewActions> ViewActions::create(
            const std::shared_ptr<MainWindow>& mainWindow,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context)
        {
            auto out = std::shared_ptr<ViewActions>(new ViewActions);
            out->_init(mainWindow, app, context);
            return out;
        }

        const std::map<std::string, std::shared_ptr<ui::Action> >&
        ViewActions::getActions() const
        {
            return _p->actions;
        }
    } // namespace play_app
} // namespace tl
