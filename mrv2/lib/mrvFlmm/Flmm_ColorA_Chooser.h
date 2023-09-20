//
// "$Id$"
//
// Flmm_ColorA_Chooser header file for the FLMM extension to FLTK.
// Code based on Fl_Color_Chooser of the FLTK library.
//
// Copyright 2002-2004 by Bill Spitzak, Matthias Melcher and others.
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Library General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Library General Public License for more details.
//
// You should have received a copy of the GNU Library General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
// USA.
//
// Please report all bugs and problems to "flmm@matthiasm.com".
//

#pragma once

#include <FL/Fl_Color_Chooser.H>
#include <FL/Fl_Double_Window.H>
#include "Flmm_ColorA_Button.h"

class FL_EXPORT Flmm_HueBox : public Fl_Widget
{
    int px, py;

protected:
    void draw();
    int handle_key(int);

public:
    int handle(int);
    Flmm_HueBox(int X, int Y, int W, int H) :
        Fl_Widget(X, Y, W, H)
    {
        px = py = 0;
    }
};

class FL_EXPORT Flmm_ValueBox : public Fl_Widget
{
    int py;

protected:
    void draw();
    int handle_key(int);

public:
    int handle(int);
    Flmm_ValueBox(int X, int Y, int W, int H) :
        Fl_Widget(X, Y, W, H)
    {
        py = 0;
    }
};

class FL_EXPORT Flmm_AlphaBox : public Fl_Widget
{
    int py;

protected:
    void draw();
    int handle_key(int);

public:
    int handle(int);
    Flmm_AlphaBox(int X, int Y, int W, int H) :
        Fl_Widget(X, Y, W, H)
    {
        py = 0;
    }
};

class FL_EXPORT Flmm_Value_Input : public Fl_Value_Input
{
public:
    int format(char*);
    Flmm_Value_Input(int X, int Y, int W, int H) :
        Fl_Value_Input(X, Y, W, H)
    {
    }
};

class FL_EXPORT Flmm_ColorA_Chooser : public Fl_Group
{
    Flmm_HueBox huebox;
    Flmm_ValueBox valuebox;
    Flmm_AlphaBox alphabox;
    Fl_Choice choice;
    Flcc_Value_Input rvalue;
    Flcc_Value_Input gvalue;
    Flcc_Value_Input bvalue;
    Flcc_Value_Input avalue;

    Fl_Box resize_box;
    double hue_, saturation_, value_;
    double r_, g_, b_, a_;
    void set_valuators();
    static void rgba_cb(Fl_Widget*, void*);
    static void mode_cb(Fl_Widget*, void*);

public:
    int mode() { return choice.value(); }
    double hue() const { return hue_; }
    double saturation() const { return saturation_; }
    double value() const { return value_; }
    double r() const { return r_; }
    double g() const { return g_; }
    double b() const { return b_; }
    double a() const { return a_; }
    int hsv(double, double, double, double = 1.0);
    int rgb(double, double, double, double = 1.0);
    static void hsv2rgb(double, double, double, double&, double&, double&);
    static void rgb2hsv(double, double, double, double&, double&, double&);
    Flmm_ColorA_Chooser(int, int, int, int, const char* = 0);
};

class ColorAChip : public Flmm_ColorA_Button
{
    int handle(int event) { return Fl_Widget::handle(event); }

public:
    ColorAChip(int X, int Y, int W, int H) :
        Flmm_ColorA_Button(X, Y, W, H)
    {
        box(FL_ENGRAVED_FRAME);
    }
};

class SavedAChip : public Flmm_ColorA_Button
{
public:
    SavedAChip(int X, int Y, int W, int H) :
        Flmm_ColorA_Button(X, Y, W, H)
    {
        box(FL_ENGRAVED_FRAME);
    }
};

class FL_EXPORT Flmm_ColorA_Window : public Fl_Double_Window
{
    Flmm_ColorA_Chooser chooser;
    ColorAChip ok_color;
    Fl_Return_Button ok_button;
    ColorAChip cancel_color;
    Fl_Button cancel_button;
    SavedAChip* saved[7];

public:
    Flmm_ColorA_Window(int, int, const char* L = 0);
    int run(double& r, double& g, double& b, double& a);
};

FL_EXPORT int flmm_color_a_chooser(
    const char* name, double& r, double& g, double& b, double& a);
FL_EXPORT int
flmm_color_a_chooser(const char* name, float& r, float& g, float& b, float& a);
FL_EXPORT int
flmm_color_a_chooser(const char* name, uchar& r, uchar& g, uchar& b, uchar& a);
