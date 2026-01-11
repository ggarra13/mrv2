// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include "TimelineItem.h"

#include <vector>

namespace tl
{
    namespace TIMELINEUI
    {
        //! Timeline widget.
        //!
        //! \tool Adjust the current frame label to stay visible on the right
        //! side of the timeline widget.
        class TimelineWidget : public ui::IWidget
        {
            TLRENDER_NON_COPYABLE(TimelineWidget);

        protected:
            void _init(
                const std::shared_ptr<timeline::ITimeUnitsModel>&,
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent);

#ifdef OPENGL_BACKEND
            TimelineWidget();
#endif

#ifdef VULKAN_BACKEND
            TimelineWidget(Fl_Vk_Context& ctx);
#endif

        public:
            virtual ~TimelineWidget();

#ifdef OPENGL_BACKEND
            //! Create a new widget.
            static std::shared_ptr<TimelineWidget> create(
                const std::shared_ptr<timeline::ITimeUnitsModel>&,
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);
#endif

#ifdef VULKAN_BACKEND
            //! Create a new widget.
            static std::shared_ptr<TimelineWidget> create(
                const std::shared_ptr<timeline::ITimeUnitsModel>&,
                Fl_Vk_Context& ctx,
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);
#endif

            //! Set the timeline player.
            void setPlayer(const std::shared_ptr<timeline::Player>&);

            //! \name Editing
            ///@{

            //! Get whether the timeline is editable.
            bool isEditable() const;

            //! Observe whether the timeline is editable.
            std::shared_ptr<observer::IValue<bool> > observeEditable() const;

            //! Set whether the timeline is editable.
            void setEditable(bool);

            //! Set action mode.
            void setEditMode(const timeline::EditMode);
            
            ///@}

            //! \name View
            ///@{

            //! Set the view zoom.
            void setViewZoom(double);

            //! Set the view zoom.
            void setViewZoom(double, const tl::math::Vector2i& focus);

            //! Frame the view.
            void frameView();

            //! Get whether the view is framed automatically.
            bool hasFrameView() const;

            //! Observe whether the view is framed automatically.
            std::shared_ptr<observer::IValue<bool> > observeFrameView() const;

            //! Set whether the view is framed automatically.
            void setFrameView(bool);

            //! Get whether the scroll bars are visible.
            bool areScrollBarsVisible() const;

            //! Set whether the scroll bars are visible.
            void setScrollBarsVisible(bool);

            //! Get whether to automatically scroll to the current frame.
            bool hasScrollToCurrentFrame() const;

            //! Observe whether to automatically scroll to the current frame.
            std::shared_ptr<observer::IValue<bool> >
            observeScrollToCurrentFrame() const;

            //! Set whether to automatically scroll to the current frame.
            void setScrollToCurrentFrame(bool);

            //! Get the mouse scroll key modifier.
            ui::KeyModifier getScrollKeyModifier() const;

            //! Set the mouse scroll key modifier.
            void setScrollKeyModifier(ui::KeyModifier);

            //! Get the mouse wheel scale.
            float getMouseWheelScale() const;

            //! Set the mouse wheel scale.
            void setMouseWheelScale(float);

            ///@}

            //! \name Scrubbing
            ///@{

            //! Get whether to stop playback when scrubbing.
            bool hasStopOnScrub() const;

            //! Observe whether to stop playback when scrubbing.
            std::shared_ptr<observer::IValue<bool> > observeStopOnScrub() const;

            //! Set whether to stop playback when scrubbing.
            void setStopOnScrub(bool);

            //! Observe whether scrubbing is in progress.
            std::shared_ptr<observer::IValue<bool> > observeScrub() const;

            //! Observe time scrubbing.
            std::shared_ptr<observer::IValue<otime::RationalTime> >
            observeTimeScrub() const;

            ///@}

            //! \name Frame Markers
            ///@{

            //! Get the frame markers.
            const std::vector<int>& getFrameMarkers() const;

            //! Set the frame markers.
            void setFrameMarkers(const std::vector<int>&);

            ///@}

            //! \name Options
            ///@{

            //! Get the item options.
            const ItemOptions& getItemOptions() const;

            //! Observe the item options.
            std::shared_ptr<observer::IValue<ItemOptions> >
            observeItemOptions() const;

            //! Set the item options.
            void setItemOptions(const ItemOptions&);

            //! Get the display options.
            const DisplayOptions& getDisplayOptions() const;

            //! Observe the display options.
            std::shared_ptr<observer::IValue<DisplayOptions> >
            observeDisplayOptions() const;

            //! Set the display options.
            void setDisplayOptions(const DisplayOptions&);

            ///@}

            //! Get timeline item geometry.
            const math::Box2i& getTimelineItemGeometry() const;

            //! Get timeline scale.
            double getScale() const;

            //! Return whether a clip is getting dragged.
            bool isDraggingClip() const;

            //! Sets a callback for moving items.
            void setMoveCallback(const std::function<void(
                                     const std::vector<timeline::MoveData>&)>&);

            void setGeometry(const math::Box2i&) override;
            void tickEvent(bool, bool, const ui::TickEvent&) override;
            void sizeHintEvent(const ui::SizeHintEvent&) override;
            void mouseMoveEvent(ui::MouseMoveEvent&) override;
            void mousePressEvent(ui::MouseClickEvent&) override;
            void mouseReleaseEvent(ui::MouseClickEvent&) override;
            void scrollEvent(ui::ScrollEvent&) override;
            void keyPressEvent(ui::KeyEvent&) override;
            void keyReleaseEvent(ui::KeyEvent&) override;

            std::vector<const otio::Item*> getSelectedItems() const;
            
        protected:
            void _releaseMouse() override;

#ifdef VULKAN_BACKEND
            Fl_Vk_Context& ctx;
#endif

        private:
            void _setViewZoom(
                double zoomNew, double zoomPrev, const math::Vector2i& focus,
                const math::Vector2i& scrollPos);

            double _getTimelineScale() const;
            double _getTimelineScaleMax() const;

            void _setItemScale();
            void _setItemScale(const std::shared_ptr<IWidget>&, double);
            void _setItemOptions(
                const std::shared_ptr<IWidget>&, const ItemOptions&);
            void _setDisplayOptions(
                const std::shared_ptr<IWidget>&, const DisplayOptions&);

            void _scrollUpdate();
            void _timelineUpdate();

            TLRENDER_PRIVATE();
        };
    } // namespace TIMELINEUI
} // namespace tl
