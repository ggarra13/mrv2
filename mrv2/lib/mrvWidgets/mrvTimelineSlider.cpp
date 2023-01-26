// SPDX-License-Identifier: BSD-3-Clause
// mrv2 (mrViewer2)
// Copyright Contributors to the mrv2 Project. All rights reserved.


#include <FL/names.h>
#include <FL/fl_draw.H>
#include <FL/Fl.H>

#include <tlCore/StringFormat.h>

#include "mrvWidgets/mrvTimelineSlider.h"

#include "mrvFl/mrvPreferences.h"
#include "mrvFl/mrvTimelinePlayer.h"
#include "mrvFl/mrvHotkey.h"

#include "mrvGL/mrvThumbnailCreator.h"

#include "mrvTools/mrvToolsCallbacks.h"

#include "mrViewer.h"

#include "mrvFl/mrvIO.h"

#include <FL/platform.H>


namespace mrv
{
    namespace
    {
        const int stripeSize = 5;
        const int handleSize = 10;
        const char* kModule = "timelineslider";
    }


    struct TimelineSlider::Private
    {
        std::weak_ptr<system::Context> context;
        ThumbnailCreator* thumbnailCreator = nullptr;
        //! List of images in filmstrip.  For now, we store only one.
        std::vector<Fl_RGB_Image*> images;
        timeline::ColorConfigOptions colorConfigOptions;
        timeline::LUTOptions lutOptions;
        mrv::TimelinePlayer* timelinePlayer = nullptr;
        mrv::TimeUnits units = mrv::TimeUnits::Timecode;
        mrv::TimeObject* timeObject = nullptr;
        bool thumbnails = true;
        int64_t thumbnailRequestId = 0;
        bool stopOnScrub = true;
        ViewerUI*  ui    = nullptr;

        Fl_Double_Window* thumbnailWindow = nullptr;  // thumbnail window
        Fl_Box*           box   = nullptr;

        int x, width;
    };


    TimelineSlider::TimelineSlider( int x, int y, int w, int h,
                                    char* l ) :
        Slider( x, y, w, h, l ),
        _p( new Private )
    {
        type( TICK_ABOVE );
        slider_type( kNORMAL );
    }

    TimelineSlider::~TimelineSlider()
    {
        _deleteThumbnails();
        delete _p->thumbnailCreator;

        // Note we don't delete here the thumbnailWindow as it is a child of
        // the main window and will get deleted once the main window is
        // deleted.
    }

    void TimelineSlider::_deleteThumbnails()
    {
        TLRENDER_P();
        for ( auto& t : p.images )
        {
            delete t;
        }
    }

    void TimelineSlider::main( ViewerUI* m )
    {
        _p->ui = m;
    }


    void TimelineSlider::setContext(
        const std::shared_ptr<system::Context>& context )
    {
        TLRENDER_P();

        p.context = context;
        if ( !p.thumbnailCreator )
            p.thumbnailCreator = new ThumbnailCreator( context );
    }


    ThumbnailCreator* TimelineSlider::thumbnailCreator()
    {
        return _p->thumbnailCreator;
    }

    void TimelineSlider::hideThumbnail_cb( TimelineSlider* t )
    {
        t->hideThumbnail();
    }
    
    void TimelineSlider::hideThumbnail()
    {
        TLRENDER_P();
        if ( ! p.thumbnailWindow  ) return;
        
        p.thumbnailWindow->hide();
    }

    int TimelineSlider::_requestThumbnail( bool fetch )
    {
        TLRENDER_P();
        const auto& player = p.timelinePlayer;
        if ( ! player ) return 0;

        if ( ! p.ui->uiPrefs->uiPrefsTimelineThumbnails->value() )
        {
            hideThumbnail();
            return 0;
        }
        int W = 128; int H = 90;
        int X = Fl::event_x_root() - p.ui->uiMain->x() - W / 2;
        //int Y = Fl::event_y_root() - Fl::event_y() - H - 20;
        int Y = window()->y() - H - 20;
        if ( X < 0 ) X = 0;
        else if ( X+W/2 > x()+w() ) X -= W/2;

        char buffer[64];
        if ( ! p.thumbnailWindow  )
        {
            // Open a thumbnail window just above the timeline
            Fl_Group::current( p.ui->uiMain );
            p.thumbnailWindow = new Fl_Double_Window( X, Y, W, H );
            p.thumbnailWindow->clear_border();
            p.thumbnailWindow->set_non_modal();
            p.thumbnailWindow->callback( (Fl_Callback*)0 );
            p.thumbnailWindow->begin();

            p.box = new Fl_Box( 2, 2, W-2, H-2 );
            p.box->box( FL_FLAT_BOX );
            p.box->labelcolor( fl_contrast( p.box->labelcolor(),
                                            p.box->color() ) );
            p.thumbnailWindow->end();
            p.thumbnailWindow->show();
        }
        p.thumbnailWindow->resize( X, Y, W, H );
#ifdef _WIN32
        // Without this, the window would not show
        p.thumbnailWindow->show();
#endif

        const auto path   = player->path();
        imaging::Size size( p.box->w(), p.box->h() - 24 );
        const auto& time = _posToTime( Fl::event_x() - x() );

        if ( p.thumbnailRequestId )
          {
            p.thumbnailCreator->cancelRequests( p.thumbnailRequestId );
          }

        if ( fetch )
        {
            p.thumbnailCreator->initThread();
            p.thumbnailRequestId =
                p.thumbnailCreator->request( path.get(), time, size,
                                             single_thumbnail_cb,
                                             (void*)this,
                                             p.colorConfigOptions,
                                             p.lutOptions );
        }
        timeToText( buffer, time, _p->units );
        p.box->copy_label( buffer );
        return 1;
    }

    int TimelineSlider::handle( int e )
    {
        TLRENDER_P();
        if ( !p.timelinePlayer ||
             !p.timelinePlayer->timelinePlayer() ) return 0;

        if ( e == FL_ENTER ) {
            window()->cursor( FL_CURSOR_DEFAULT );
            if ( p.thumbnailWindow ) p.thumbnailWindow->show();
            _requestThumbnail();
            return 1;
        }
        else if ( e == FL_DRAG || e == FL_PUSH )
        {
            int X = Fl::event_x() - x();
            const auto& time = _posToTime( X );
            p.timelinePlayer->seek( time );
            _requestThumbnail( (e == FL_PUSH) );
            return 1;
        }
        else if ( e == FL_MOVE )
        {
            _requestThumbnail();
            return 1;
        }
        else if ( e == FL_LEAVE || e == FL_HIDE )
        {
            if ( p.thumbnailCreator && p.thumbnailRequestId )
                p.thumbnailCreator->cancelRequests( p.thumbnailRequestId );
            Fl::add_timeout( 0.005, (Fl_Timeout_Handler) hideThumbnail_cb,
                             this );
            return 1;
        }
        else if ( e == FL_RELEASE )
        {
            if ( filesTool ) filesTool->redraw();
            if ( compareTool ) compareTool->redraw();
        }
        return Slider::handle( e );
    }

    //! Set the timeline player.
    void TimelineSlider::setTimelinePlayer(mrv::TimelinePlayer* t)
    {
        TLRENDER_P();
        p.timelinePlayer = t;
        if ( t )
        {
            const auto& range = t->timeRange();
            const auto& inOutRange = t->inOutRange();
            const auto& duration = range.end_time_inclusive() -
                                   range.start_time();
            const double start = range.start_time().to_frames();

            Slider::minimum( start );
            Slider::maximum( start + duration.to_frames() );
            value( start );
            if ( p.ui )
            {
                TimelineClass* c = p.ui->uiTimeWindow;
                if ( inOutRange.start_time() == range.start_time() )
                    c->uiStartButton->value(0);
                else
                    c->uiStartButton->value(1);
                if ( inOutRange.end_time_inclusive() ==
                     range.end_time_inclusive() )
                    c->uiEndButton->value(0);
                else
                    c->uiEndButton->value(1);
            }
            redraw();
        }
    }


    char* TimelineSlider::print_tick(char* buffer, double v)
    {
        TimelineClass* c = _p->ui->uiTimeWindow;
        otio::RationalTime time( v, c->uiFrame->time().rate() );
        timeToText( buffer, time, _p->units );
        return buffer;
    }

    void TimelineSlider::draw_ticks(const tl::math::BBox2i& r, int min_spacing)
    {
        int x1, sx1, y1, sy1, x2, y2, w;
        sx1 = x1 = x2 = r.x()+(slider_size()-1)/2;
        y1 = r.y(); y2 = r.max.y;
        sy1 = y1+1+r.h()/4;
        w = r.w();

        if (w <= 0) return;
        double A = minimum();
        double B = maximum();
        if (A > B) {A = B; B = minimum();}

        double mul = 1; // how far apart tick marks are
        double div = 1;
        int smallmod = 5; // how many tick marks apart "larger" ones are
        int nummod = 10; // how many tick marks apart numbers are
        int powincr = 10000;

        double derivative = (B-A)*min_spacing/w;
        if (derivative < step()) derivative = step();
        while (mul*5 <= derivative) mul *= 10;
        while (mul > derivative*2*div) div *= 10;
        if (derivative*div > mul*2) {mul *= 5; smallmod = 2;}
        else if (derivative*div > mul) {mul *= 2; nummod = 5;}

        fl_push_clip( r.x(), r.y(), r.w(), r.h() );

        Fl_Color textcolor = fl_rgb_color( 120, 120, 120 );
        if ( Preferences::schemes.name == "Black" )
        {
            _tick_color = fl_rgb_color( 70, 70, 70 );
        }
        else
        {
            _tick_color = FL_BLACK;
        }
        Fl_Color linecolor = _tick_color;

        fl_color(linecolor);
        fl_font( fl_font(), labelsize() );

        float yt = y1+fl_size()-fl_descent();
        double v; char buffer[20]; char* p; int t; float x, y;

        v = maximum();
        t = slider_position( v, w );
        int maxx = x1+t+1;
        p = print_tick(buffer, v);
        float width = fl_width(p); maxx -= 2*width;

        for (int n = 0; ; n++) {
            // every ten they get further apart for log slider:
            if (n > powincr) {mul *= 10; n = (n-1)/10+1;}
            v = mul*n/div;
            if (v >= fabs(A) && v >= fabs(B)) break;
            if (n%smallmod) {
                if (v > A && v < B) {
                    t = slider_position(v, w);
                    fl_line(sx1+t, sy1, x2+t, y2);
                }
                if (v && -v > A && -v < B) {
                    t = slider_position(-v, w);
                    fl_line(sx1+t, sy1, x2+t, y2);
                }
            }
            else
            {
                if (v > A && v < B) {
                    t = slider_position(v, w);
                    fl_line(x1+t, y1, x2+t, y2);
                    if (n%nummod == 0) {
                        p = print_tick(buffer, v);
                        x =  x1+t+1;
                        y = yt;
                        if (x < r.x()+3*min_spacing ||
                            x >= maxx ||
                            x >= r.max.x-5*min_spacing ) ;
                        else {
                            fl_color(textcolor);
                            fl_draw( p, x, y );
                            fl_color(linecolor);
                        }
                    }
                }
                if (v && -v > A && -v < B) {
                    t = slider_position(-v, w);
                    fl_line(x1+t, y1, x2+t, y2);
                    if (n%nummod == 0) {
                        p = print_tick(buffer, v);
                        x = x1+t+1;
                        y = yt;
                        if (x < r.x()+3*min_spacing ||
                            x >= maxx ||
                            x >= r.max.x-5*min_spacing) ;
                        else {
                            fl_color(textcolor);
                            fl_draw(p, x,y);
                            fl_color(linecolor);
                        }
                    }
                }
            }
        }

        // draw the end ticks with numbers:

        v = minimum();
        t = slider_position( v, w );
        fl_line( x1+t, y1, x2+t, y2 );
        p = print_tick(buffer, v);
        x = x1+t+1;
        y = yt;
        fl_color( textcolor );
        fl_draw( p, x,y );
        fl_color( linecolor );

        v = maximum();
        t = slider_position( v, w );
        fl_line( x1+t, y1, x2+t, y2 );
        p = print_tick( buffer, v );
        x = x1+t+1;
        width = fl_width(p); if (x+width > r.max.x) x -= 2+width;
        y = yt;
        fl_color( textcolor );
        fl_draw( p, x, y );

        fl_pop_clip();
    }

    void TimelineSlider::draw()
    {

        TLRENDER_P();
        bool valid = p.timelinePlayer;
        otio::RationalTime time;
        if ( valid )
        {
            time = p.timelinePlayer->currentTime();
            value( time.value() );
        }

        draw_box();

        p.x = x() + Fl::box_dx(box());
        p.width = w() - Fl::box_dw(box());
        int x0, x1;
        int y1 = y() + Fl::box_dy(box());
        int h1 = h() - Fl::box_dh(box());

        //
        fl_push_clip( p.x, y1, p.width, h1 );

        tl::math::BBox2i r( p.x, y1, p.width, h1 );
        const int Y = r.y();
        const int H = r.h();


        // Draw cached frames.
        fl_color( fl_rgb_color( 40, 190, 40 ) );
        fl_line_style( FL_SOLID, 1 );

        if ( valid )
        {
            const auto& cacheInfo = p.timelinePlayer->cacheInfo();
            int y2 = y1 + h1 - stripeSize;
            for (const auto& i : cacheInfo.videoFrames)
            {
                x0 = _timeToPos(i.start_time());
                x1 = _timeToPos(i.end_time_inclusive());
                fl_rectf(x0, y2, x1 - x0, stripeSize );
            }

            fl_color( fl_rgb_color( 190, 190, 40 ) );
            y2 = y1 + h1 - stripeSize * 2;
            for (const auto& i : cacheInfo.audioFrames)
            {
                x0 = _timeToPos(i.start_time());
                x1 = _timeToPos(i.end_time_inclusive());
                fl_rectf(x0, y2, x1 - x0, stripeSize);
            }
        }


        int spacing = 10;
        if ( p.units == TimeUnits::Timecode )  spacing = 20;

        draw_ticks( r, spacing );


        // Draw frame range lines
        if ( valid )
        {
            const auto& iorange = p.timelinePlayer->inOutRange();
            const auto& range = p.timelinePlayer->timeRange();
            if ( iorange != range )
            {
                fl_color( FL_CYAN );
                int X = _timeToPos( iorange.start_time() );
                const int Y2 = Y + H;
                fl_line_style( FL_SOLID, 3 );
                fl_line( X, Y, X, Y2 );
                X = _timeToPos( iorange.end_time_inclusive() );
                fl_line( X, Y, X, Y2 );
            }

            const auto& frames = p.timelinePlayer->getAnnotationFrames();
            fl_color( FL_RED );
            fl_line_style( FL_SOLID, 3 );
            const auto& duration = range.end_time_inclusive() -
                                   range.start_time();
            for ( const auto frame : frames )
            {
                otime::RationalTime time( frame, duration.rate() );
                int X = _timeToPos( time );
                const int Y2 = Y + H;
                fl_line( X, Y, X, Y2 );
            }
        }

        fl_line_style(0);

        int X;
        if ( valid )
            X = _timeToPos( time ) - handleSize / 2;
        else
            X = slider_position( value(), p.width ) - handleSize / 2;
        const int W = handleSize;
        Fl_Color c = fl_lighter( color() );
        draw_box( FL_ROUND_UP_BOX, X, Y, W, H, c );
        clear_damage();

        fl_pop_clip();
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
        }
    }

    TimeUnits TimelineSlider::units() const
    {
        return _p->units;
    }

    bool TimelineSlider::hasThumbnails() const
    {
        return _p->thumbnails;
    }

    bool TimelineSlider::hasStopOnScrub() const
    {
        return _p->stopOnScrub;
    }


    void TimelineSlider::setUnits(TimeUnits value)
    {
        TLRENDER_P();
        if (value == p.units)
            return;
        p.units = value;
        TimelineClass* c = _p->ui->uiTimeWindow;
        c->uiStartFrame->setUnits( value );
        c->uiEndFrame->setUnits( value );
        c->uiFrame->setUnits( value );
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


    otime::RationalTime TimelineSlider::_posToTime(int value) const noexcept
    {
        TLRENDER_P();
        otime::RationalTime out = time::invalidTime;
        if (p.timelinePlayer && p.timelinePlayer->timelinePlayer() )
        {
            const int width = p.width;
            const auto& range = p.timelinePlayer->timeRange();
            const auto& duration = range.end_time_inclusive() -
                                   range.start_time();
            out = otime::RationalTime(
                floor(math::clamp(value, 0, width) /
                      static_cast<double>(width) * duration.value() +
                      range.start_time().value()),
                duration.rate());
        }
        return out;
    }

    double
    TimelineSlider::_timeToPos(const otime::RationalTime& value) const noexcept
    {
        TLRENDER_P();
        double out = 0;
        if (p.timelinePlayer && p.timelinePlayer->timelinePlayer() )
        {
            const auto& range = p.timelinePlayer->timeRange();
            const auto& duration = range.end_time_inclusive() -
                                   range.start_time();

            out = p.x +
                  (value.value() - range.start_time().value()) /
                  (duration.value() > 1 ? (duration.value() - 1) : 1) *
                  p.width;
        }
        return out;
    }

    void TimelineSlider::single_thumbnail(
        const int64_t id,
        const std::vector< std::pair<otime::RationalTime,
        Fl_RGB_Image*> >& thumbnails )
    {
        TLRENDER_P();

        if (id == p.thumbnailRequestId)
        {
            for (const auto& i : thumbnails)
            {
                p.images.push_back( i.second );
                Fl_Image* image = p.box->image();
                p.box->image( i.second );
                if ( image )
                {
                    // Removes all elements with the value of image.
                    p.images.erase(std::remove(p.images.begin(),
                                               p.images.end(), image),
                                   p.images.end());
                    // Delete the old image
                    delete image;
                }
            }
            p.box->redraw();
        }
        else
        {
            for (const auto& i : thumbnails)
            {
                delete i.second;
            }
        }
    }

    void TimelineSlider::single_thumbnail_cb(
        const int64_t id,
        const std::vector< std::pair<otime::RationalTime,
        Fl_RGB_Image*> >& thumbnails, void* data )
    {
        TimelineSlider* self = static_cast< TimelineSlider* >( data );
        self->single_thumbnail( id, thumbnails );
    }

    void change_timeline_display( mrv::PopupMenu* menu )
    {
        select_character( menu, true );
    }

} // namespace mrv
