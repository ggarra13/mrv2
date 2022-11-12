#pragma once

#include <FL/Fl_Hor_Slider.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Float_Input.H>
#include <FL/Fl_Group.H>

namespace mrv
{

    class HorSlider : public Fl_Group
    {
    protected:
        Fl_Float_Input*  uiValue;
        Fl_Hor_Slider*  uiSlider;
        Fl_Button*      uiReset;
        double          default_value_;
    public:
        HorSlider( int X, int Y, int W, int H, const char* L = 0 );

        void default_value( double d ) noexcept;
        void minimum( double mn ) noexcept;
        void maximum( double mx ) noexcept;
        void range( double mn, double mx ) noexcept;
        void step( double s ) noexcept;
        void value( double x ) noexcept;
        double value() const noexcept;

        void check_size() noexcept;
        
        inline void setEnabled( bool a )
            {
                if ( a )
                {
                    uiValue->activate();
                    uiSlider->activate();
                    uiReset->activate();
                }
                else
                {
                    uiValue->deactivate();
                    uiSlider->deactivate();
                    uiReset->deactivate();
                }
            }
    };


}
