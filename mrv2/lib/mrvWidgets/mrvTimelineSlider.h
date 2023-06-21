// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <tlTimeline/IRender.h>

#include "mrvFl/mrvTimeObject.h"
#include "mrvFl/mrvTimelinePlayer.h"

#include "mrvWidgets/mrvSlider.h"

class Fl_RGB_Image;
class ViewerUI;

namespace mrv
{
    class TimelinePlayer;
    class ThumbnailCreator;

    class TimelineSlider : public Slider
    {
    public:
        TimelineSlider(int x, int y, int w, int h, char* l = 0);
        ~TimelineSlider();

        virtual int handle(int e) override;
        virtual void draw() override;

        //! Hide the thumbnail at least until user enters the timeline slider
        //! again.
        void hideThumbnail();

        //! @bug: A static one time timeout callback used to avoid a bug
        //! FLTK when hiding a window from an event of another widget.
        static void hideThumbnail_cb(TimelineSlider* t);

        //! Set the viewer handle
        void main(ViewerUI* m);

        //! Set the tlRender context
        void setContext(const std::shared_ptr<system::Context>& context);

        //! Set the LUT configuration.
        void setLUTOptions(const timeline::LUTOptions&);

        //! Set the color configuration.
        void setColorConfigOptions(const timeline::ColorConfigOptions&);

        //! Set the timeline player.
        void setTimelinePlayer(mrv::TimelinePlayer*);

        ThumbnailCreator* thumbnailCreator();

        //! Get the time units.
        TimeUnits units() const;

        //! Get whether thumbnails are displayed.
        bool hasThumbnails() const;

        //! Get whether playback is stopped when scrubbing.
        bool hasStopOnScrub() const;

        void single_thumbnail(
            const int64_t,
            const std::vector<
                std::pair<otime::RationalTime, Fl_RGB_Image*> >&);

        static void single_thumbnail_cb(
            const int64_t,
            const std::vector< std::pair<otime::RationalTime, Fl_RGB_Image*> >&,
            void* data);

        //! Set the time units.
        void setUnits(TimeUnits);

        //! Set whether thumbnails are displayed.
        void setThumbnails(bool);

        //! Set whether playback is stopped when scrubbing.
        void setStopOnScrub(bool);

    private:
        char* print_tick(char* buf, const double value);
        void draw_ticks(const tl::math::BBox2i& r, int min_spacing);

        //! Consider the timeRange and the inOutRange and return the further
        //! extremes of both.
        otime::TimeRange _validRange() const noexcept;

        otime::RationalTime _posToTime(int) const noexcept;
        double _timeToPos(const otime::RationalTime&) const noexcept;

        int _requestThumbnail(bool fetch = true);
        void _deleteThumbnails();
        void _thumbnailsUpdate();

        TLRENDER_PRIVATE();
    };

} // namespace mrv
