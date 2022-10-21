#pragma once

#include <FL/Fl_Hor_Slider.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Float_Input.H>
#include <FL/Fl_Group.H>

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
      HorSlider( int X, int Y, int W, int H, const char* L = 0 );

      void setDefaultValue( double d ) noexcept;
      void setRange( double mn, double mx ) noexcept;
      void setStep( double s ) noexcept;
      void value( double x ) noexcept;

      double value() const noexcept;
    };


}
