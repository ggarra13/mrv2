// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <FL/Fl_Button.H>
#include <FL/Fl_Radio_Button.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Toggle_Button.H>

namespace mrv
{
    class Button : public Fl_Button
    {
        Fl_Color default_color;

    public:
        Button(int X, int Y, int W, int H, const char* L = 0);

        virtual int handle(int e);
        virtual void draw();

        void color(Fl_Color c);
        Fl_Color color() { return Fl_Button::color(); }
    };

    class CheckButton : public Fl_Check_Button
    {
    public:
        CheckButton(int X, int Y, int W, int H, const char* L = 0);

        virtual void draw();
    };

    class RadioButton : public Fl_Radio_Button
    {
    public:
        RadioButton(int X, int Y, int W, int H, const char* L = 0);

        virtual void draw();
    };

    class Toggle_Button : public Fl_Toggle_Button
    {
    public:
        Toggle_Button(int X, int Y, int W, int H, const char* L = 0);

        virtual void draw();
    };

} // namespace mrv
