

#pragma once

#include <FL/Fl_Gl_Window.H>

#include <tlCore/Util.h>
#include <tlCore/BBox.h>
#include <tlCore/ISystem.h>

#include "mrvFl/mrvColorAreaInfo.h"

class ViewerUI;

namespace mrv
{
    using namespace tl;

    class Histogram : public Fl_Gl_Window
    {
    public:
        enum Type
        {
            kLinear,
            kLog,
            kSqrt,
        };

        enum Channel
        {
            kRGB,
            kRed,
            kGreen,
            kBlue,
            kLumma,
        };

    public:
        Histogram( int X, int Y, int W, int H, const char* L = 0 );

        void channel( Channel c ) {
            _channel = c;
            redraw();
        }
        Channel channel() const    {
            return _channel;
        }

        void histogram_type( Type c ) {
            _histtype = c;
            redraw();
        };
        Type histogram_type() const {
            return _histtype;
        };

        virtual void draw() override;

        void update( const area::Info& info );

        
        void main( ViewerUI* m ) {
            ui = m;
        };
        ViewerUI* main() {
            return ui;
        };

    protected:
        void   draw_grid( const math::BBox2i& r );
        void draw_pixels( const math::BBox2i& r );

        void count_pixel( const uint8_t* rgb );

        inline float histogram_scale( float val, float maxVal );

        Channel      _channel;
        Type         _histtype;

        float maxLumma;
        float lumma[256];

        float maxColor;

        float red[256];
        float green[256];
        float blue[256];

        ViewerUI* ui;
    };

}  // namespace mrv
