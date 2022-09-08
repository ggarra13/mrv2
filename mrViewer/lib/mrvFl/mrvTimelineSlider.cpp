#include "mrvFl/mrvTimelineSlider.h"

#include <tlCore/Math.h>
#include <tlCore/StringFormat.h>

#include "mrvCore/mrvPreferences.h"

#include <FL/fl_draw.H>

#include "mrvFl/mrvTimelinePlayer.h"
#include "mrvFl/mrvHotkey.h"

#include "mrViewer.h"


namespace mrv
{
    struct TimelineSlider::Private
    {
        std::weak_ptr<system::Context> context;
        // mrv::TimelineThumbnailProvider* thumbnailProvider = nullptr;
        // std::map<otime::RationalTime, QImage> thumbnailImages;
        timeline::ColorConfigOptions colorConfigOptions;
        mrv::TimelinePlayer* timelinePlayer = nullptr;
        mrv::TimeUnits units = mrv::TimeUnits::Timecode;
        mrv::TimeObject* timeObject = nullptr;
        bool thumbnails = true;
        int64_t thumbnailRequestId = 0;
        bool stopOnScrub = true;
        ViewerUI*  ui    = nullptr;
    };


    TimelineSlider::TimelineSlider( int x, int y, int w, int h,
                                    char* l ) :
        Slider( x, y, w, h, l ),
        _p( new Private )
    {
        type( TICK_ABOVE );
        slider_type( kNORMAL );
        Slider::minimum( 1 );
        Slider::maximum( 50 );
    }

    TimelineSlider::~TimelineSlider()
    {
    }

    void TimelineSlider::main( ViewerUI* m )
    {
        _p->ui = m;
    }

    ViewerUI* TimelineSlider::main() const
    {
        return _p->ui;
    }

    void TimelineSlider::setContext(
        const std::shared_ptr<system::Context>& context )
    {
        _p->context = context;
    }


    int TimelineSlider::handle( int e )
    {
        TLRENDER_P();

        if ( e == FL_ENTER ) {
            window()->cursor( FL_CURSOR_DEFAULT );
            return 1;
        }
        else if ( e == FL_DRAG || e == FL_PUSH )
        {
            int X = Fl::event_x() - x();
            auto time = _posToTime( X );
            p.timelinePlayer->seek( time );
            return 1;
        }
        else if ( e == FL_LEAVE )
        {
            // Fl::remove_timeout( (Fl_Timeout_Handler)showwin, this );
            // if (win) win->hide();
        }
        else if ( e == FL_KEYDOWN )
        {
            unsigned int rawkey = Fl::event_key();
            int ok = p.ui->uiView->handle( e );
            if ( ok ) return ok;
        }
        Fl_Boxtype bx = box();
        box( FL_FLAT_BOX );
        int ok = Slider::handle( e );
        box( bx );
        return ok;
    }

    //! Set the timeline player.
    void TimelineSlider::setTimelinePlayer(mrv::TimelinePlayer* t)
    {
        TLRENDER_P();
        p.timelinePlayer = t;

        const auto& globalStartTime = t->globalStartTime();
        const auto& duration = t->duration();
        const double start = globalStartTime.value();

        Slider::minimum( start );
        Slider::maximum( start + duration.value() );
        value( start );

    }

    void TimelineSlider::draw()
    {
        TLRENDER_P();
        // @todo: handle drawing of cache lines
        double v = _timeToPos( p.timelinePlayer->currentTime() );
        value( v );

        draw_box();

        mrv::Recti r( x() + Fl::box_dx(box()),
                      y() + Fl::box_dy(box()),
                      w() - Fl::box_dw(box()),
                      h() - Fl::box_dh(box()) );
        draw_ticks( r, 10 );

        int X = r.x() + slider_position( value(), r.w() - 10 );
        int Y = r.y();
        int W = 10;
        int H = r.h();
        Fl_Color c = color();
        draw_box( FL_ROUND_UP_BOX, X, Y, W, H, c );
        clear_damage();
    }

    void TimelineSlider::setTimeObject(TimeObject* timeObject)
    {
        TLRENDER_P();
        if (timeObject == p.timeObject)
            return;
        p.timeObject = timeObject;
        if (p.timeObject)
        {
            p.units = p.timeObject->units();
            // connect(
            //     p.timeObject,
            //     SIGNAL(unitsChanged(tl::qt::TimeUnits)),
            //     SLOT(setUnits(tl::qt::TimeUnits)));
        }
    }


    void TimelineSlider::setUnits(mrv::TimeUnits value)
    {
        TLRENDER_P();
        if (value == p.units)
            return;
        p.units = value;
        redraw();
    }

    void TimelineSlider::setColorConfigOptions(
        const timeline::ColorConfigOptions& colorConfigOptions)
    {
        TLRENDER_P();
        if (colorConfigOptions == p.colorConfigOptions)
            return;
        p.colorConfigOptions = colorConfigOptions;
    }


    otime::RationalTime TimelineSlider::_posToTime(int value) const
    {
        TLRENDER_P();
        otime::RationalTime out = time::invalidTime;
        if (p.timelinePlayer)
        {
            const auto& globalStartTime = p.timelinePlayer->globalStartTime();
            const auto& duration = p.timelinePlayer->duration();
            out = otime::RationalTime(
                floor(math::clamp(value, 0, w()) /
                      static_cast<double>(w()) * (duration.value() - 1) +
                      globalStartTime.value()),
                duration.rate());
        }
        return out;
    }

    double TimelineSlider::_timeToPos(const otime::RationalTime& value) const
    {
        TLRENDER_P();
        double out = 0;
        if (p.timelinePlayer)
        {
            out = value.value();
        }
        return out;
    }

} // namespace mrv
