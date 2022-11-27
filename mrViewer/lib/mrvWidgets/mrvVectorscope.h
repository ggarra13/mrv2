

#pragma once

#include <FL/Fl_Group.H>

#include <tlCore/Util.h>
#include <tlCore/Color.h>
#include <tlCore/Image.h>

#include "mrvFl/mrvColorAreaInfo.h"

class ViewerUI;

namespace mrv
{
    using namespace tl;

    class Vectorscope : public Fl_Group
    {
    public:
        Vectorscope( int X, int Y, int W, int H, const char* L = 0 );
        ~Vectorscope();

        virtual void draw() override;

        void update( const area::Info& info );

        
        void main( ViewerUI* m ) {
            ui = m;
        };
        ViewerUI* main() {
            return ui;
        };

    protected:
        void draw_grid() noexcept;
        void draw_pixel( imaging::Color4f& rgb ) const noexcept;
        void draw_pixels() const noexcept;

        int           diameter;
        imaging::Size renderSize;
        math::BBox2i  box;
        imaging::Color4f* image = nullptr;
        ViewerUI* ui;
    };

}  // namespace mrv
