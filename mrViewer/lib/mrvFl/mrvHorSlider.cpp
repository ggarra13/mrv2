
#include <cmath>

#include "mrvHorSlider.h"

#include "mrvFunctional.h"

namespace mrv
{

    HorSlider::HorSlider( int X, int Y, int W, int H, const char* L ) :
        Fl_Group( X, Y, W, H ),
        default_value_( 1.0 )
    {
        begin();

        auto uiValueW = new Widget<Fl_Float_Input>( X+90, Y, 50, H, L );
        uiValue = uiValueW;
        uiValue->labelsize( 12 );
        auto uiSliderW = new Widget<Fl_Hor_Slider>( X+90+uiValue->w(), Y,
                                                    W-150, H );
        uiSlider = uiSliderW;
        uiSlider->when(FL_WHEN_CHANGED);
        auto uiResetW  = new Widget<Fl_Button>( X+W-10, Y, 10, H, "@-31+" );
        uiReset = uiResetW;
        uiReset->box( FL_NO_BOX );
        end();
        resizable( uiSlider );

        uiSliderW->callback([=](auto s) {
            double v = s->value();
            char buf[32];
            snprintf( buf, 32, "%6.2f", v );
            uiValue->value( buf );
            do_callback();
        } );
                
        uiValueW->callback( [=]( auto o ) {
            double v = atof( o->value() );
            uiSlider->value( v );
            do_callback();
        } );
                
        uiResetW->callback( [=]( auto o ) {
            uiSlider->value( default_value_ );
            uiSlider->do_callback();
        } );

        range( 0.F, 10.F );
        step( 0.1F );
    }

    void HorSlider::default_value( double d ) noexcept
    {
        default_value_ = d;
        uiSlider->value(d);
        uiSlider->do_callback();
    }

    void HorSlider::minimum( double mn ) noexcept
    {
        uiSlider->minimum( mn );
    }

    void HorSlider::maximum( double mx ) noexcept
    {
        uiSlider->maximum( mx );
    }


    void HorSlider::range( double mn, double mx ) noexcept
    {
        uiSlider->bounds( mn, mx );
    }

    void HorSlider::step( double s ) noexcept
    {
        uiSlider->step( s );
    }

    void HorSlider::value( double x ) noexcept {
        uiSlider->value( x );
        uiSlider->do_callback();
    }

    
    double HorSlider::value() const noexcept { return uiSlider->value(); }

}
