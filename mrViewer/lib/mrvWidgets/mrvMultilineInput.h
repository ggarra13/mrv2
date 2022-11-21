#pragma once

#include <tlCore/Vector.h>

#include <FL/Fl_Multiline_Input.H>
#include <FL/Enumerations.H>

namespace mrv
{
    using namespace tl;

    class MultilineInput : public Fl_Multiline_Input
    {
    public:
        MultilineInput( int X, int Y, int W, int H, char* l = 0 ) :
            Fl_Multiline_Input( X, Y, W, H, l )
            {
                box( FL_ROUNDED_BOX );
                color( FL_FREE_COLOR );
                wrap( false );
                tab_nav( false );
                pos.x = X;
                pos.y = Y;
            };
        ~MultilineInput() override;
        
        int accept();
        
        int  textsize() const;
        void textsize(int x);
        void recalc();

        int handle( int e ) override;
        void draw()         override;
    public:
        double _font_size;
        math::Vector2i pos;
        math::Vector2i viewPos;
        double         viewZoom = 1.F;
    };

} // namespace mrv
