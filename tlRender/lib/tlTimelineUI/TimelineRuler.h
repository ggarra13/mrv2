// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// Copyright (c) 2024-Present Gonzalo Garramuño
// All rights reserved.

#pragma once

#include "IItem.h"

#include <tlTimeline/Player.h>

#include <tlUI/IWidget.h>

#include <vector>

namespace tl
{
    namespace TIMELINEUI
    {
        //! Timeline ruler.
        //!
        //! Draws the in/out points, time ticks, frame markers, time labels,
        //! cache information and current time of the timeline being played,
        //! and lets the user change the current frame with the mouse.
        //!
        //! There is one ruler per TimelineWidget (not one per TimelineItem).
        //! It is a sibling of the TimelineWidget -- it is attached to the
        //! TimelineWidget's parent -- and it is drawn on top of it.
        //!
        //! The ruler does not lay itself out in its parent: whatever geometry
        //! the parent assigns is ignored, and it occupies the top strip of the
        //! viewport given to setViewport().  It reports a zero size hint so
        //! that a parent layout does not reserve a row for it.
        class TimelineRuler : public ui::IWidget
        {
            TLRENDER_NON_COPYABLE(TimelineRuler);

        protected:
            void _init(
                const std::shared_ptr<timeline::ITimeUnitsModel>&,
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent);

            TimelineRuler();

        public:
            virtual ~TimelineRuler();

            //! Create a new ruler.
            static std::shared_ptr<TimelineRuler> create(
                const std::shared_ptr<timeline::ITimeUnitsModel>&,
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            //! \name Data
            ///@{

            //! Set the player.  This provides the time range, current time,
            //! in/out range and cache information, and is what gets seeked.
            void setPlayer(const std::shared_ptr<timeline::Player>&);

            //! Set the time range (call when the timeline is edited).
            void setTimeRange(const OTIO_NS::TimeRange&);

            //! Set the frame markers.
            void setFrameMarkers(const std::vector<int>&);

            ///@}

            //! \name View
            ///@{

            //! Set the timeline scale, in pixels per second.
            void setScale(double);

            //! Set the x position, in window coordinates, of the start of the
            //! timeline.  This is the viewport's left edge minus the scroll
            //! position.
            void setTimelineOrigin(int);

            //! Set the area in which the timeline is visible.  The ruler
            //! occupies the top of it, and the current time line extends
            //! down to its bottom.
            void setViewport(const math::Box2i&);

            //! Set the item options (only inputEnabled is used).
            void setItemOptions(const ItemOptions&);

            //! Set the display options.
            void setDisplayOptions(const DisplayOptions&);

            ///@}

            //! \name Scrubbing
            ///@{

            //! Set whether playback stops when scrubbing.
            void setStopOnScrub(bool);

            //! Observe whether scrubbing is in progress.
            std::shared_ptr<observer::IValue<bool> > observeScrub() const;

            //! Observe time scrubbing.
            std::shared_ptr<observer::IValue<OTIO_NS::RationalTime> >
            observeTimeScrub() const;

            ///@}

            //! Get the height of the ruler strip.  Valid after the first size
            //! hint pass.
            int getHeight() const;

            //! Convert a time to a window x position.
            int timeToPos(const OTIO_NS::RationalTime&) const;

            //! Convert a window x position to a time, clamped to the time
            //! range and rounded down to a frame.
            OTIO_NS::RationalTime posToTime(int) const;

            void setGeometry(const math::Box2i&) override;
            void sizeHintEvent(const ui::SizeHintEvent&) override;
            void drawEvent(const math::Box2i&, const ui::DrawEvent&) override;
            void mouseMoveEvent(ui::MouseMoveEvent&) override;
            void mousePressEvent(ui::MouseClickEvent&) override;
            void mouseReleaseEvent(ui::MouseClickEvent&) override;

        protected:
            void _releaseMouse() override;
            void _timeUnitsUpdate();

        private:
            void _geometryUpdate();
            void _scrub(int x);

            math::Size2i
            _getLabelMaxSize(const std::shared_ptr<image::FontSystem>&) const;
            void _getTimeTicks(
                const std::shared_ptr<image::FontSystem>&, double& seconds,
                int& tick) const;

            void _drawInOutPoints(const math::Box2i&, const ui::DrawEvent&);
            void _drawTimeTicks(const math::Box2i&, const ui::DrawEvent&);
            void _drawFrameMarkers(const math::Box2i&, const ui::DrawEvent&);
            void _drawTimeLabels(const math::Box2i&, const ui::DrawEvent&);
            void _drawCacheInfo(const math::Box2i&, const ui::DrawEvent&);
            void _drawCurrentTime(const math::Box2i&, const ui::DrawEvent&);

            TLRENDER_PRIVATE();
        };
    } // namespace TIMELINEUI
} // namespace tl
