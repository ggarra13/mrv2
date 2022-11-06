#pragma once

#include "mrvCore/mrvRectangle.h"

#include <FL/Fl_Slider.H>

namespace mrv {

class Slider : public Fl_Slider
{
  public:
    enum SliderType {
    kNORMAL = 0,
    kLOG    = 1,
    };

    enum Ticks
    {
    TICK_ABOVE = 1,
    TICK_BELOW = 2,
    TICK_BOTH  = 3,
    NO_TICK
    };

  public:

    Slider( int x, int y, int w, int h, const char* l = 0 ) :
    Fl_Slider( x, y, w, h, l ),
    _slider_type( kNORMAL ),
    _tick_color( FL_BLACK ),
    tick_size_( 4 )
    {
        type( FL_HORIZONTAL );
    }

    bool log() const { return _slider_type & kLOG; }


    inline void ticks( Ticks t ) { _ticks = t; }
    inline Ticks ticks() const { return _ticks; }

    inline int tick_size() const { return tick_size_; }
    inline void tick_size(int i) { tick_size_ = i; }

    inline void tick_color( Fl_Color c ) { _tick_color = c; }

    inline SliderType slider_type() const { return _slider_type; }
    inline void slider_type( enum SliderType x ) { _slider_type = x; }

    virtual void draw();
    virtual int handle(int e);

  protected:
    double position_value( int X, int w );
    int slider_position( double p, int w );
    void draw_ticks( const mrv::Recti& r, int min_spacing );

    SliderType _slider_type;
    Ticks      _ticks;
    Fl_Color   _tick_color;
    int        tick_size_;
};

}
