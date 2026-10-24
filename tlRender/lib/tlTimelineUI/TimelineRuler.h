// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#pragma once

#include <tlTimelineUI/ItemOptions.h>

#include <tlTimeline/Player.h>

namespace tl
{
    namespace TIMELINEUI
    {
        //! The time ruler drawn above the timelines.
        //!
        //! Outside the scrolled timelines rather than part of one: several
        //! timelines shown together share a time axis, and one drawn inside
        //! the scroll area either scrolls away with its timeline or has to be
        //! pinned back to the top, where the others land on it.
        //!
        //! It is the player's ruler. The timelines the player is compared
        //! against are read against its axis, and the in/out points, frame
        //! markers and cache drawn here are its own.
        class TimelineRuler : public IItem
        {
            TLRENDER_NON_COPYABLE(TimelineRuler);

        protected:
            void _init(
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<ItemData>&,
                const std::shared_ptr<IWidget>& parent);

            TimelineRuler();

        public:
            virtual ~TimelineRuler();

            //! Create a new widget.
            static std::shared_ptr<TimelineRuler> create(
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<ItemData>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            //! Set the item data, which the labels are named through.
            void setItemData(const std::shared_ptr<ItemData>&);

            //! Set the player.
            void setPlayer(const std::shared_ptr<Player>&);

            //! Set the scale, in pixels per second.
            void setScale(double);

            //! Set how far the timelines are scrolled, so that the ruler is
            //! over the part of them it names.
            void setScrollPos(int);

            //! Set how far along the timeline is drawn.
            void setOffset(const OTIO_NS::RationalTime&);

            //! Set the frame markers.
            void setFrameMarkers(const std::vector<int>&);

            //! Set the markers.
            void setMarkers(const std::vector<Marker>&);

            //! Set the display options.
            void setDisplayOptions(const DisplayOptions&);

            //! Set the options.
            void setOptions(const ItemOptions&);

            //! Set whether playback stops when scrubbing.
            void setStopOnScrub(bool);

            //! Observe whether scrubbing is in progress.
            std::shared_ptr<ftk::IObservable<bool> > observeScrub() const;

            //! Observe time scrubbing.
            std::shared_ptr<ftk::IObservable<std::optional<OTIO_NS::RationalTime> > > observeTimeScrub() const;

            //! Convert a time to a position.
            int timeToPos(const OTIO_NS::RationalTime&) const;

            ftk::Size2I getSizeHint() const override;
            void sizeHintEvent(const ftk::SizeHintEvent&) override;
            void drawEvent(const math::Box2i&, const ftk::DrawEvent&) override;
            void mouseMoveEvent(ftk::MouseMoveEvent&) override;
            void mousePressEvent(ftk::MouseClickEvent&) override;
            void mouseReleaseEvent(ftk::MouseClickEvent&) override;
            void mouseLeaveEvent() override;

        private:
            void _drawInOutPoints(const math::Box2i&, const ftk::DrawEvent&);
            void _drawFrameMarkers(const math::Box2i&, const ftk::DrawEvent&);
            void _drawMarkers(const math::Box2i&, const ftk::DrawEvent&);
            void _drawCacheInfo(const math::Box2i&, const ftk::DrawEvent&);
            void _drawTimeLabels(const math::Box2i&, const ftk::DrawEvent&);
            void _drawTimeTicks(const math::Box2i&, const ftk::DrawEvent&);
            void _drawCurrentTime(const math::Box2i&, const ftk::DrawEvent&);

            OTIO_NS::RationalTime _posToTime(float) const;
            OTIO_NS::RationalTime _posToTimeClamped(float) const;
            std::string _timeLabel(const OTIO_NS::RationalTime&) const;
            math::Size2i _getLabelMaxSize(const std::shared_ptr<system::FontSystem>&) const;
            double _getSecondsInc(const std::shared_ptr<system::FontSystem>&);

            FTK_PRIVATE();
        };
    }
}
