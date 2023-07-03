// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <FL/Fl_Slider.H>

namespace mrv
{

    class VolumeSlider : public Fl_Slider
    {
    public:
        VolumeSlider(int x, int y, int w, int h, const char* l = 0) :
            Fl_Slider(x, y, w, h, l)
        {
            type(FL_HORIZONTAL);
        }

        virtual ~VolumeSlider(){};

        int handle(int e) override;
    };

} // namespace mrv
