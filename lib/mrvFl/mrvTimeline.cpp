#include <FL/fl_draw.H>

// #include <mrvCore/mrvI8N.h>
#include <mrvCore/mrvRectangle.h>
#include "mrvCore/mrvColor.h"
#include "mrvCore/mrvPreferences.h"

#include "mrvFl/mrvTimecode.h"
#include "mrvFl/mrvTimeline.h"
#include "mrvFlGL/mrvTimelineViewport.h"
#include "mrvFl/mrvHotkey.h"

#include "mrViewer.h"

namespace
{
// Maximum number of frames to show cacheline for.  Setting it too high can
// impact GUI playback when the image/movies are that long.
unsigned kMAX_FRAMES = 5000;
double kMinFrame = std::numeric_limits<double>::min();
double kMaxFrame = std::numeric_limits<double>::max();
}


namespace mrv
{

mrv::Timecode::Display Timeline::_display = Timecode::kFrames;

Timeline::Timeline( int x, int y, int w, int h, char* l ) :
mrv::Slider( x, y, w, h, l ),
m_draw_annotation( true ),
m_draw_cache( true ),
uiMain( NULL )
{
    type( TICK_ABOVE );
    slider_type( kNORMAL );
    Fl_Slider::minimum( 1 );
    Fl_Slider::maximum( 50 );
}

Timeline::~Timeline()
{
    uiMain = NULL;
}



int Timeline::handle( int e )
{
    if ( e == FL_ENTER ) {
        window()->cursor( FL_CURSOR_DEFAULT );
        return 1;
    }
    else if ( e == FL_MOVE || e == FL_DRAG || e == FL_PUSH )
    {
        window()->cursor( FL_CURSOR_DEFAULT );
        event_x = Fl::event_x();
        int X = x()+Fl::box_dx(box());
        int Y = y()+Fl::box_dy(box());
        int W = w()-Fl::box_dw(box());
        int H = h()-Fl::box_dh(box());

        // HERE, SET A VALUE TO CURRENT HOVERING PLACE,
        // *NOT* TO value().
        double val;
        if (minimum() == maximum())
            val = 0.5;
        else {
            val = (value()-minimum())/(maximum()-minimum());
            if (val > 1.0) val = 1.0;
            else if (val < 0.0) val = 0.0;
        }

        int ww = W;
        int mx = Fl::event_x()-X;
        int S;
        static int offcenter;

        S = int(slider_size()*ww+.5); if (S >= ww) return 0;
        int T = H / 2+1;
        if (type()==FL_HOR_NICE_SLIDER) T += 4;
        if (S < T) S = T;

        if ( e == FL_MOVE ) {
            int xx = int(val*(ww-S)+.5);
            offcenter = mx-xx;
            if (offcenter < 0) offcenter = 0;
            else if (offcenter > S) offcenter = S;
            else return 1;
        }

        int xx = mx-offcenter;
        double v = 0;
        if (xx < 0) {
            xx = 0;
            offcenter = mx; if (offcenter < 0) offcenter = 0;
        } else if (xx > (ww-S)) {
            xx = ww-S;
            offcenter = mx-xx; if (offcenter > S) offcenter = S;
        }
        v = round(xx*(maximum()-minimum())/(ww-S) + minimum());
        //frame = clamp(v);  // @todo

        // @todo:
        // if ( uiMain->uiPrefs->uiPrefsTimelineThumbnails->value() )
        // {
        // if ( Fl::has_timeout( (Fl_Timeout_Handler)showwin, this ) )
        //     Fl::remove_timeout( (Fl_Timeout_Handler)showwin, this );
        // Fl::add_timeout( 0.01, (Fl_Timeout_Handler)showwin, this );
        // }
    }
    else if ( e == FL_LEAVE )
    {
        // Fl::remove_timeout( (Fl_Timeout_Handler)showwin, this );
        // if (win) win->hide();
    }
    else if ( e == FL_KEYDOWN )
    {
        unsigned int rawkey = Fl::event_key();
        // if ( kPlayBack.match( rawkey ) ||
        //      kPlayFwd.match( rawkey ) ||
        //      kPlayDirection.match( rawkey ) ||
        //      kPlayFwdTwiceSpeed.match( rawkey ) ||
        //      kPlayBackHalfSpeed.match( rawkey ) ||
        //      kShapeFrameStepFwd.match( rawkey ) ||
        //      kShapeFrameStepBack.match( rawkey ) ||
        //      kStop.match( rawkey ) )
        // {
        //     return uiMain->uiView->handle( e );
        // }
    }
    Fl_Boxtype bx = box();
    box( FL_FLAT_BOX );
    int ok = mrv::Slider::handle( e );
    box( bx );
    return ok;
    // if ( r != 0 ) return r;
    // return uiMain->uiView->handle( e );
}

/**
 * Main widget drawing routine
 *
 */
void Timeline::draw()
{
    Fl_Slider::draw();
}

int Timeline::draw_coordinate( double value, int w )
{
    double A = minimum();
    double B = maximum();

    // @todo:
    // if ( uiMain->uiPrefs->uiPrefsTimelineSelectionDisplay->value() )
    // {
    //     A = display_minimum();
    //     B = display_maximum();
    // }

    if (B == A) return 0;
    bool flip = B < A;
    if (flip) {A = B; B = minimum();}
    if (!horizontal()) flip = !flip;
    // if both are negative, make the range positive:
    if (B <= 0) {flip = !flip; double t = A; A = -B; B = -t; value = -value;}
    double fraction;
    if (!(slider_type() & kLOG)) {
        // linear slider
    fraction = (value-A)/(B-A+1);
  } else if (A > 0) {
    // logatithmic slider
    if (value <= A) fraction = 0;
    else fraction = (::log(value)-::log(A))/(::log(B)-::log(A));
  } else if (A == 0) {
    // squared slider
    if (value <= 0) fraction = 0;
    else fraction = sqrt(value/B);
  } else {
    // squared signed slider
    if (value < 0) fraction = (1-sqrt(value/A))*.5;
    else fraction = (1+sqrt(value/B))*.5;
  }
  if (flip) fraction = 1-fraction;
  w -= slider_size(); if (w <= 0) return 0;
  return int(fraction*w+.5);
}

int Timeline::slider_position( double value, int w )
{
    double A = minimum();
    double B = maximum();

    // @todo:
    // if ( uiMain->uiPrefs->uiPrefsTimelineSelectionDisplay->value() )
    // {
    //     A = display_minimum();
    //     B = display_maximum();
    // }

    if (B == A) return 0;
    bool flip = B < A;
    if (flip) {A = B; B = minimum();}
    if (!horizontal()) flip = !flip;
    // if both are negative, make the range positive:
    if (B <= 0) {flip = !flip; double t = A; A = -B; B = -t; value = -value;}
    double fraction;
    if (!(slider_type() & kLOG)) {
        // linear slider
    fraction = (value-A)/(B-A+1);
  } else if (A > 0) {
    // logatithmic slider
    if (value <= A) fraction = 0;
    else fraction = (::log(value)-::log(A))/(::log(B)-::log(A));
  } else if (A == 0) {
    // squared slider
    if (value <= 0) fraction = 0;
    else fraction = sqrt(value/B);
  } else {
    // squared signed slider
    if (value < 0) fraction = (1-sqrt(value/A))*.5;
    else fraction = (1+sqrt(value/B))*.5;
  }
  if (flip) fraction = 1-fraction;
  w -= slider_size(); if (w <= 0) return 0;
  if (fraction >= 1) return w;
  else if (fraction <= 0) return 0;
  else return int(fraction*w+.5);
}

} // namespace mrv
