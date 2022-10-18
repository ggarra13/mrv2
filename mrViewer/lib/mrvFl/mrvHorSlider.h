#pragma once

#include <FL/Fl_Hor_Slider.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Float_Input.H>
#include <FL/Fl_Group.H>

#include "mrvFunctional.h"

namespace mrv
{


    class HorSlider : public Fl_Group
    {
    public:
        Fl_Float_Input*  uiValue;
        Fl_Hor_Slider*  uiSlider;
        Fl_Button*      uiReset;
        double           default_value;
    public:
        HorSlider( int X, int Y, int W, int H, const char* L = 0 ) :
            Fl_Group( X, Y, W, H )
            {
                begin();
                
                auto uiValueW = new Widget<Fl_Float_Input>( X+90, Y, 30, H, L );
                uiValue = uiValueW;
                uiValue->labelsize( 12 );
                auto uiSliderW = new Widget<Fl_Hor_Slider>( X+120, Y, W-130, H );
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
                    snprintf( buf, 32, "%.2g", v );
                    uiValue->value( buf );
                } );
                
                uiValueW->callback( [=]( auto o ) {
                    double v = atof( o->value() );
                    uiSlider->value( v );
                } );
                
                uiResetW->callback( [=]( auto o ) {
                    uiSlider->value( default_value );
                    uiSlider->do_callback();
                } );
            }

        void setDefaultValue( double d )
            {
                default_value = d;
                uiSlider->value(d);
                uiSlider->do_callback();
            }

        void setRange( double mn, double mx )
            {
                uiSlider->bounds( mn, mx );
            }

        void setStep( double s )
            {
                uiSlider->step( s );
            }

        double value() const { return uiSlider->value(); }
    };


}
