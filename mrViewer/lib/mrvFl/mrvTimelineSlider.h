#pragma once

#include <tlCore/ColorConfig.h>

#include <mrvCore/mrvTimeObject.h>


#include <mrvFl/mrvTimelinePlayer.h>
#include "mrvFl/mrvTimecode.h"
#include "mrvFl/mrvSlider.h"


class ViewerUI;

namespace mrv
{
    class TimelinePlayer;

    class TimelineSlider : public mrv::Slider
    {
    public:
        TimelineSlider( int x, int y, int w, int h, char* l = 0 );
        ~TimelineSlider();


        virtual int  handle( int e ) override;
        virtual void draw()          override;

        //! Set the viewer handle
        void main( ViewerUI* m );

        //! Return the viewer handle
        ViewerUI* main() const;

        //! Set the tlRender context
        void setContext(
            const std::shared_ptr<system::Context>& context);

        //! Set the time object.
        void setTimeObject(mrv::TimeObject*);

        //! Set the color configuration.
        void setColorConfig(const imaging::ColorConfig&);

        //! Set the timeline player.
        void setTimelinePlayer(mrv::TimelinePlayer*);

        //! Get the time units.
        mrv::TimeUnits units() const;

        //! Get whether thumbnails are displayed.
        bool hasThumbnails() const;

        //! Get whether playback is stopped when scrubbing.
        bool hasStopOnScrub() const;

        //Q_SLOTS
    public:
        //! Set the time units.
        void setUnits(mrv::TimeUnits);

        //! Set whether thumbnails are displayed.
        void setThumbnails(bool);

        //! Set whether playback is stopped when scrubbing.
        void setStopOnScrub(bool);

    protected:

        static Timecode::Display _display;

    private:
        otime::RationalTime _posToTime(int) const;
        double _timeToPos(const otime::RationalTime&) const;

        void _thumbnailsUpdate();

        TLRENDER_PRIVATE();
    };

} // namespace mrv
