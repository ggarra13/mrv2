
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

        int Xoffset = 70;
        if ( L == nullptr ) Xoffset = 3;
        auto uiValueW = new Widget<Fl_Float_Input>( X+Xoffset, Y, 50, H, L );
        uiValue = uiValueW;
	uiValue->color( (Fl_Color)-1733777408 );
	uiValue->textcolor( FL_BLACK );
        uiValue->labelsize( 12 );
        auto uiSliderW = new Widget<Fl_Hor_Slider>( X+Xoffset+50, Y,
                                                    W-Xoffset-50-13, H );
        uiSlider = uiSliderW;
        uiSlider->when(FL_WHEN_CHANGED);
        auto uiResetW  = new Widget<Fl_Button>( X+W-13, Y, 10, H, "@-31+" );
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

        default_value( 0.0F );
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
        check_size();
    }

    void HorSlider::maximum( double mx ) noexcept
    {
        uiSlider->maximum( mx );
        check_size();
    }


    void HorSlider::check_size() noexcept
    {
        return;
        double mn = uiSlider->minimum();
        double mx = uiSlider->maximum();
        if ( mn < -1000 || mx > 1000 ) {
            uiValue->size( 70, uiValue->h() );
            uiSlider->resize( x()+uiValue->x()+70, uiSlider->y(),
                              w()-70-13-uiValue->x(), uiSlider->h() );
        }
    }
    
    void HorSlider::range( double mn, double mx ) noexcept
    {
        uiSlider->bounds( mn, mx );
        check_size();
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
