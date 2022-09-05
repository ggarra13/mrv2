#ifndef mrvVolumeSlider_h
#define mrvVolumeSlider_h

#include <FL/Fl_Slider.H>

namespace mrv {

class VolumeSlider : public Fl_Slider
{
  public:
    VolumeSlider( int x, int y, int w, int h, const char* l = 0 ) :
    Fl_Slider( x, y, w, h, l )
    {
        type( FL_HORIZONTAL );
    }

    virtual ~VolumeSlider() {};

    virtual int handle(int e);
};

}


#endif
