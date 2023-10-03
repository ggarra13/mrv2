// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <FL/Fl_Box.H>

namespace mrv
{

    class ResizableBar : public Fl_Box
    {
        int orig_w;
        int last_x;
        int min_w; // min width for widget right of us
        int min_x; // max width for widget left of us
    public:
        ResizableBar(int X, int Y, int W, int H, const char* L = 0);
        void draw() override;
        int handle(int e) override;
        void resize(int X, int Y, int W, int H) override;
        void HandleDrag(int diff);
    };

} // namespace mrv
