// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTimelineUI/IItem.h>

#include <tlTimeline/Edit.h>
#include <tlTimeline/Player.h>

namespace tl
{
    namespace gl
    {
        class GLFWWindow;
    }

    namespace timelineui
    {
        //! Track types.
        enum class TrackType { None, Video, Audio };

        //! Timeline item.
        //!
        //! \todo Add a selection model.
        //! \todo Add support for dragging clips to different tracks.
        //! \todo Add support for adjusting clip handles.
        //! \todo Add support for undo/redo.
        //! \todo Add an option for viewing/playing individual clips ("solo"
        //! mode).
        class TimelineItem : public IItem
        {
        protected:
            void _init(
                const std::shared_ptr<timeline::Player>&,
                const otio::SerializableObject::Retainer<otio::Stack>&,
                double scale, const ItemOptions&, const DisplayOptions&,
                const std::shared_ptr<ItemData>&,
                const std::shared_ptr<gl::GLFWWindow>&,
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent);

            TimelineItem();

        public:
            virtual ~TimelineItem();

            //! Create a new item.
            static std::shared_ptr<TimelineItem> create(
                const std::shared_ptr<timeline::Player>&,
                const otio::SerializableObject::Retainer<otio::Stack>&,
                double scale, const ItemOptions&, const DisplayOptions&,
                const std::shared_ptr<ItemData>&,
                const std::shared_ptr<gl::GLFWWindow>&,
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            //! Set whether the timeline is editable.
            void setEditable(bool);

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

        protected:
            void _timeUnitsUpdate() override;

            void _releaseMouse() override;

        private:
            bool _isTrackVisible(int) const;

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

            void _tracksUpdate();
            void _textUpdate();

            TLRENDER_PRIVATE();
        };
    } // namespace timelineui
} // namespace tl
