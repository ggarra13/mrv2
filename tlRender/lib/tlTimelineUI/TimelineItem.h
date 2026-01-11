// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// Copyright (c) 2024-Present Gonzalo Garramuño
// All rights reserved.

#pragma once

#include "IBasicItem.h"

#include <tlTimeline/Edit.h>
#include <tlTimeline/Player.h>

#include <vector>

namespace tl
{
    namespace gl
    {
        class GLFWWindow;
    }

    namespace TIMELINEUI
    {
        //! Track types.
        enum class TrackType { kNone, Video, Audio };

        //! Timeline item.
        //!
        //! \todo Add support for dragging clips to different tracks.
        class TimelineItem : public IItem
        {
        protected:
#ifdef OPENGL_BACKEND
            void _init(
                const std::shared_ptr<timeline::Player>&,
                const otio::SerializableObject::Retainer<otio::Stack>&,
                double scale, const ItemOptions&, const DisplayOptions&,
                const std::shared_ptr<ItemData>&,
                const std::shared_ptr<gl::GLFWWindow>&,
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent);

            
            TimelineItem();
#endif

#ifdef VULKAN_BACKEND
            void _init(
                const std::shared_ptr<timeline::Player>&,
                const otio::SerializableObject::Retainer<otio::Stack>&,
                double scale, const ItemOptions&, const DisplayOptions&,
                const std::shared_ptr<ItemData>&,
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent);

            TimelineItem(Fl_Vk_Context&);	
#endif

        public:
            virtual ~TimelineItem();

#ifdef OPENGL_BACKEND
            //! Create a new item.
            static std::shared_ptr<TimelineItem> create(
                const std::shared_ptr<timeline::Player>&,
                const otio::SerializableObject::Retainer<otio::Stack>&,
                double scale, const ItemOptions&, const DisplayOptions&,
                const std::shared_ptr<ItemData>&,
                const std::shared_ptr<gl::GLFWWindow>&,
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);
#endif

#ifdef VULKAN_BACKEND
            //! Create a new item.
            static std::shared_ptr<TimelineItem> create(
                const std::shared_ptr<timeline::Player>&,
                const otio::SerializableObject::Retainer<otio::Stack>&,
                double scale, const ItemOptions&, const DisplayOptions&,
                const std::shared_ptr<ItemData>&,
                Fl_Vk_Context& ctx,
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);
#endif
            
            //! Set whether the timeline is editable.
            void setEditable(bool);

            //! Set the action mode.
            void setEditMode(const timeline::EditMode);

            //! Set whether playback stops when scrubbing.
            void setStopOnScrub(bool);

            //! Returns whether a clip is getting dragged.
            bool isDraggingClip() const;

            //! Sets a callback for inserting items
            void setMoveCallback(const std::function<void(
                                     const std::vector<timeline::MoveData>&)>&);

            //! Observe whether scrubbing is in progress.
            std::shared_ptr<observer::IValue<bool> > observeScrub() const;

            //! Observe time scrubbing.
            std::shared_ptr<observer::IValue<otime::RationalTime> >
            observeTimeScrub() const;

            //! Set the frame markers.
            void setFrameMarkers(const std::vector<int>&);

            //! Get the minimum height.
            int getMinimumHeight() const;

            void setDisplayOptions(const DisplayOptions&) override;

            void setGeometry(const math::Box2i&) override;
            void sizeHintEvent(const ui::SizeHintEvent&) override;
            void
            drawOverlayEvent(const math::Box2i&, const ui::DrawEvent&) override;
            void mouseMoveEvent(ui::MouseMoveEvent&) override;
            void mousePressEvent(ui::MouseClickEvent&) override;
            void mouseReleaseEvent(ui::MouseClickEvent&) override;
            // void keyPressEvent(ui::KeyEvent&) override;
            // void keyReleaseEvent(ui::KeyEvent&) override;

            std::vector<const otio::Item*> getSelectedItems() const;
            
            
        protected:
            void _timeUnitsUpdate() override;

            void _releaseMouse() override;

#ifdef VULKAN_BACKEND
            Fl_Vk_Context& ctx;
#endif
            
        private:
            bool _isTrackVisible(int) const;

            void _mouseMoveEventFill(ui::MouseMoveEvent&);
            void _mouseMoveEventMove(ui::MouseMoveEvent&);
            void _mouseMoveEventRipple(ui::MouseMoveEvent&);
            void _mouseMoveEventRoll(ui::MouseMoveEvent&);
            void _mouseMoveEventSlice(ui::MouseMoveEvent&);
            void _mouseMoveEventSlide(ui::MouseMoveEvent&);
            void _mouseMoveEventSlip(ui::MouseMoveEvent&);
            void _mouseMoveEventTrim(ui::MouseMoveEvent&);
            
            void _mouseReleaseEventFill(ui::MouseClickEvent&);
            void _mouseReleaseEventMove(ui::MouseClickEvent&);
            void _mouseReleaseEventRipple(ui::MouseClickEvent&);
            void _mouseReleaseEventRoll(ui::MouseClickEvent&);
            void _mouseReleaseEventSlice(ui::MouseClickEvent&);
            void _mouseReleaseEventSlide(ui::MouseClickEvent&);
            void _mouseReleaseEventSlip(ui::MouseClickEvent&);
            void _mouseReleaseEventTrim(ui::MouseClickEvent&);

            bool _clampRangeToNeighborTransitions(const otio::Item* item,
                                                  const otime::TimeRange& proposedRange,
                                                  otime::TimeRange& clampedRange);
            
            void _drawInOutPoints(const math::Box2i&, const ui::DrawEvent&);
            math::Size2i
            _getLabelMaxSize(const std::shared_ptr<image::FontSystem>&) const;
            void _getTimeTicks(
                const std::shared_ptr<image::FontSystem>&, double& seconds,
                int& tick);
            void _drawTimeTicks(const math::Box2i&, const ui::DrawEvent&);
            void _drawFrameMarkers(const math::Box2i&, const ui::DrawEvent&);
            void _drawTimeLabels(const math::Box2i&, const ui::DrawEvent&);
            void _drawCacheInfo(const math::Box2i&, const ui::DrawEvent&);
            void _drawCurrentTime(const math::Box2i&, const ui::DrawEvent&);
            void _getTransitionItems(std::vector<IBasicItem*>& items,
                                     const int trackNumber,
                                     const otime::TimeRange& transitionRange);
            void _getTransitionTimeRanges(std::vector<otime::TimeRange>& items,
                                          const int trackNumber,
                                          const otime::TimeRange& transitionRange);
            void _addOneFrameGap(const otime::RationalTime& videoTime,
                                 otime::TimeRange& timeRange);
            bool _transitionIntersects(const std::shared_ptr<IItem> transition,
                                       const int transitionTrack,
                                       const otime::TimeRange& timeRange);
            void _tracksUpdate();
            void _textUpdate();
            void _storeUndo();

            TLRENDER_PRIVATE();
        };
    } // namespace TIMELINEUI
} // namespace tl
