// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlPlayApp/SecondaryWindow.h>

#include <tlPlayApp/App.h>

#include <tlPlay/ColorModel.h>
#include <tlPlay/FilesModel.h>
#include <tlPlay/RenderModel.h>
#include <tlPlay/Settings.h>
#include <tlPlay/ViewportModel.h>

#include <tlTimelineUI/TimelineViewport.h>

namespace tl
{
    namespace play_app
    {
        struct SecondaryWindow::Private
        {
            std::shared_ptr<timelineui::TimelineViewport> viewport;

            std::shared_ptr<
                observer::ValueObserver<std::shared_ptr<timeline::Player> > >
                playerObserver;
            std::shared_ptr<observer::ValueObserver<timeline::CompareOptions> >
                compareOptionsObserver;
            std::shared_ptr<observer::ValueObserver<timeline::OCIOOptions> >
                ocioOptionsObserver;
            std::shared_ptr<observer::ValueObserver<timeline::LUTOptions> >
                lutOptionsObserver;
            std::shared_ptr<observer::ValueObserver<timeline::ImageOptions> >
                imageOptionsObserver;
            std::shared_ptr<observer::ValueObserver<timeline::DisplayOptions> >
                displayOptionsObserver;
            std::shared_ptr<
                observer::ValueObserver<timeline::BackgroundOptions> >
                backgroundOptionsObserver;
            std::shared_ptr<observer::ValueObserver<image::PixelType> >
                colorBufferObserver;
        };

        void SecondaryWindow::_init(
            const std::shared_ptr<ui_app::Window>& window,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context)
        {
            const bool shareContexts =
                app->getSettings()->getValue<bool>("OpenGL/ShareContexts");
            Window::_init(
                "tlplay 2", context, shareContexts ? window : nullptr);
            TLRENDER_P();

            p.viewport = timelineui::TimelineViewport::create(context);
            p.viewport->setParent(shared_from_this());

            p.playerObserver = observer::
                ValueObserver<std::shared_ptr<timeline::Player> >::create(
                    app->observePlayer(),
                    [this](const std::shared_ptr<timeline::Player>& value)
                    { _p->viewport->setPlayer(value); });

            p.compareOptionsObserver =
                observer::ValueObserver<timeline::CompareOptions>::create(
                    app->getFilesModel()->observeCompareOptions(),
                    [this](const timeline::CompareOptions& value)
                    { _p->viewport->setCompareOptions(value); });

            p.ocioOptionsObserver =
                observer::ValueObserver<timeline::OCIOOptions>::create(
                    app->getColorModel()->observeOCIOOptions(),
                    [this](const timeline::OCIOOptions& value)
                    { _p->viewport->setOCIOOptions(value); });

            p.lutOptionsObserver =
                observer::ValueObserver<timeline::LUTOptions>::create(
                    app->getColorModel()->observeLUTOptions(),
                    [this](const timeline::LUTOptions& value)
                    { _p->viewport->setLUTOptions(value); });

            p.imageOptionsObserver =
                observer::ValueObserver<timeline::ImageOptions>::create(
                    app->getRenderModel()->observeImageOptions(),
                    [this](const timeline::ImageOptions& value)
                    { _p->viewport->setImageOptions({value}); });

            p.displayOptionsObserver =
                observer::ValueObserver<timeline::DisplayOptions>::create(
                    app->getViewportModel()->observeDisplayOptions(),
                    [this](const timeline::DisplayOptions& value)
                    { _p->viewport->setDisplayOptions({value}); });

            p.backgroundOptionsObserver =
                observer::ValueObserver<timeline::BackgroundOptions>::create(
                    app->getViewportModel()->observeBackgroundOptions(),
                    [this](const timeline::BackgroundOptions& value)
                    { _p->viewport->setBackgroundOptions(value); });

            p.colorBufferObserver =
                observer::ValueObserver<image::PixelType>::create(
                    app->getRenderModel()->observeColorBuffer(),
                    [this](image::PixelType value)
                    { _p->viewport->setColorBuffer(value); });
        }

        SecondaryWindow::SecondaryWindow() :
            _p(new Private)
        {
        }

        SecondaryWindow::~SecondaryWindow()
        {
            TLRENDER_P();
            _makeCurrent();
            auto i = std::find(_children.begin(), _children.end(), p.viewport);
            if (i != _children.end())
            {
                _children.erase(i);
            }
        }

        std::shared_ptr<SecondaryWindow> SecondaryWindow::create(
            const std::shared_ptr<ui_app::Window>& window,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context)
        {
            auto out = std::shared_ptr<SecondaryWindow>(new SecondaryWindow);
            out->_init(window, app, context);
            return out;
        }

        void SecondaryWindow::setView(
            const tl::math::Vector2i& pos, double zoom, bool frame)
        {
            TLRENDER_P();
            p.viewport->setViewPosAndZoom(pos, zoom);
            p.viewport->setFrameView(frame);
        }
    } // namespace play_app
} // namespace tl
