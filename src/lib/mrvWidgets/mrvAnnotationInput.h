// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <FL/Fl.H>
#include <FL/Enumerations.H>
#include <FL/Fl_Multiline_Input.H>

namespace mrv
{
    // Colors for the annotation panel.  Always built with the 3-argument
    // fl_rgb_color() so a custom theme cannot remap them.
    namespace ann_colors
    {
        inline Fl_Color panel_bg()   { return fl_rgb_color(0x26, 0x26, 0x26); }
        inline Fl_Color header_bg()  { return fl_rgb_color(0x33, 0x33, 0x33); }
        inline Fl_Color current_bg() { return fl_rgb_color(0x46, 0x35, 0x27); }
        inline Fl_Color edit_bg()    { return fl_rgb_color(0x17, 0x17, 0x17); }
        inline Fl_Color border()     { return fl_rgb_color(0xC9, 0x93, 0x42); }
        inline Fl_Color selection()  { return fl_rgb_color(0x8A, 0x65, 0x2E); }
        inline Fl_Color text()       { return fl_rgb_color(0xDD, 0xDD, 0xDD); }
        inline Fl_Color date_text()  { return fl_rgb_color(0x8C, 0x8C, 0x8C); }
    } // namespace ann_colors

    class AnnotationInput : public Fl_Multiline_Input
    {
    public:
        AnnotationInput(int X, int Y, int W, int H, const char* L = 0) :
            Fl_Multiline_Input(X, Y, W, H, L)
        {
        }
        ~AnnotationInput() {};

        // When locked the input is a plain, borderless, read-only label:
        // it ignores every event (so it never takes focus) but keeps
        // its normal text color (unlike deactivate()).
        void locked(bool v)
        {
            locked_ = v;
            redraw();
        }
        bool locked() const { return locked_; }

        int handle(int e) override;

    protected:
        void draw() override;

    private:
        bool locked_ = false;
    };

} // namespace mrv
