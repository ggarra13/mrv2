// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlUI/IWidget.h>

#include <tlTimeline/BackgroundOptions.h>
#include <tlTimeline/Player.h>

namespace tl
{
    namespace timelineui_vk
    {
        //! Timeline viewport.
        class TimelineViewport : public ui::IWidget
        {
            TLRENDER_NON_COPYABLE(TimelineViewport);

        protected:
            void _init(
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent);

            TimelineViewport();

        public:
            virtual ~TimelineViewport();

            //! Create a new widget.
            static std::shared_ptr<TimelineViewport> create(
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            //! Set the comparison options.
            void setCompareOptions(const timeline::CompareOptions&);

            //! Set the comparison callback.
            void setCompareCallback(
                const std::function<void(timeline::CompareOptions)>&);

            //! Set the OpenColorIO options.
            void setOCIOOptions(const timeline::OCIOOptions&);

            //! Set the LUT options.
            void setLUTOptions(const timeline::LUTOptions&);

            //! Set the image options.
            void setImageOptions(const std::vector<timeline::ImageOptions>&);

            //! Set the display options.
            void
            setDisplayOptions(const std::vector<timeline::DisplayOptions>&);

            //! Set the background options.
            void setBackgroundOptions(const timeline::BackgroundOptions&);

            //! Get the color buffer type.
            image::PixelType getColorBuffer() const;

            //! Set the color buffer type.
            void setColorBuffer(image::PixelType);

            //! Set the timeline player.
            void setPlayer(const std::shared_ptr<timeline::Player>&);

            //! Get the view position.
            const math::Vector2i& getViewPos() const;

            //! Get the view zoom.
            double getViewZoom() const;

            //! Set the view position and zoom.
            void setViewPosAndZoom(const math::Vector2i&, double);

            //! Set the view zoom.
            void
            setViewZoom(double, const math::Vector2i& focus = math::Vector2i());

            //! Get whether the view is framed automatically.
            bool hasFrameView() const;

            //! Observe whether the view is framed automatically.
            std::shared_ptr<observer::IValue<bool> > observeFrameView() const;

            //! Set whether the view is framed automatically.
            void setFrameView(bool);

            //! Set the view framed callback.
            void setFrameViewCallback(const std::function<void(bool)>&);

            //! Set the view zoom to 1:1.
            void viewZoom1To1();

            //! Zoom the view in.
            void viewZoomIn();

            //! Zoom the view out.
            void viewZoomOut();

            //! Set the view position and zoom callback.
            void setViewPosAndZoomCallback(
                const std::function<void(const math::Vector2i&, double)>&);

            //! Get the frames per second.
            double getFPS() const;

            //! Observe the frames per second.
            std::shared_ptr<observer::IValue<double> > observeFPS() const;

            //! Get the number of dropped frames during playback.
            size_t getDroppedFrames() const;

            //! Observe the number of dropped frames during playback..
            std::shared_ptr<observer::IValue<size_t> >
            observeDroppedFrames() const;

            void setGeometry(const math::Box2i&) override;
            void sizeHintEvent(const ui::SizeHintEvent&) override;
            void drawEvent(const math::Box2i&, const ui::DrawEvent&) override;
            void mouseMoveEvent(ui::MouseMoveEvent&) override;
            void mousePressEvent(ui::MouseClickEvent&) override;
            void mouseReleaseEvent(ui::MouseClickEvent&) override;
            void scrollEvent(ui::ScrollEvent&) override;
            void keyPressEvent(ui::KeyEvent&) override;
            void keyReleaseEvent(ui::KeyEvent&) override;

        protected:
            void _releaseMouse() override;

        private:
            math::Size2i _getRenderSize() const;
            math::Vector2i _getViewportCenter() const;
            void _frameView();

            void _droppedFramesUpdate(const otime::RationalTime&);

            TLRENDER_PRIVATE();
        };
    } // namespace timelineui_vk
} // namespace tl
