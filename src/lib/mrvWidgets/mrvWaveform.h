// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.


#include <FL/Fl_Box.H>

#include <tlCore/Util.h>
#include <tlCore/Color.h>
#include <tlCore/Image.h>

#include "mrvCore/mrvColorAreaInfo.h"

class ViewerUI;

namespace mrv
{

    class Waveform : public Fl_Box
    {
    public:
        Waveform( int X, int Y, int W, int H, const char* L = 0 );
        ~Waveform();
    
        void draw() override;
        void resize(int X, int Y, int W, int H) override;
        
        void update(const area::Info& info);

        void main(ViewerUI* m);

        // HDR waveform support
        // maxValue: linear-light ceiling (e.g. 12.0 = 12× SDR white)
        // logScale: true = log2 mapping (recommended), false = linear
        void  setHDRMode(bool enabled,
                         float maxValue = 12.f,
                         bool  logScale = true);
        bool  isHDRMode()     const;
        float hdrMaxValue()   const;
        bool  isHDRLogScale() const;
        
    protected:
        void draw_grid();
        void draw_pixels();
        void draw_pixel(const int X, const image::Color4f& rgba );

        TLRENDER_PRIVATE();
    };

}  // namespace mrv


