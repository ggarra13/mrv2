// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <FL/fl_draw.H>
#include <FL/Fl_Button.H>

namespace mrv
{
    class ClipButton : public Fl_Button
    {
    public:
        ClipButton(int X, int Y, int W, int H, const char* L = 0) :
            Fl_Button(X, Y, W, H, L)
        {
            box(FL_ENGRAVED_BOX);
            selection_color(FL_YELLOW);
            align(
                FL_ALIGN_LEFT | FL_ALIGN_INSIDE | FL_ALIGN_IMAGE_NEXT_TO_TEXT);
            labelsize(12);
        }

        void draw() override
        {
            if (value())
                labelcolor(FL_BLACK);
            else
                labelcolor(FL_WHITE);
            Fl_Color col = value() ? selection_color() : color();
            draw_box(
                value() ? (down_box() ? down_box() : fl_down(box())) : box(),
                col);
            draw_backdrop();

#if 1
            fl_font(labelfont(), labelsize());
            fl_color(labelcolor());

            const int X = x() + Fl::box_dx(box());
            const int Y = y() + Fl::box_dy(box());
            const int W = w() - Fl::box_dw(box());
            const int H = h() - Fl::box_dh(box());

            fl_push_clip(X, Y, W, H);

            const int MX = 8; // margin
            
            Fl_Image* img = image();
            const int IW = img ? img->w() + MX : 0;
            const int IH = img ? img->h() : 0;
            fl_draw(label(), X + IW, Y + 20);
            if (img)
                img->draw(X, Y);

            fl_pop_clip();
#else
            draw_label();
#endif
            
        }
    };
} // namespace mrv
