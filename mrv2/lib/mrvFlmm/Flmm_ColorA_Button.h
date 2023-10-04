//
// "$Id$"
//
// Flmm_ColorA_Button header file for the FLMM extension to FLTK.
//
// Copyright 2002-2004 by Matthias Melcher.
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

#include <FL/Fl_Button.H>

class Flmm_ColorA_Button : public Fl_Button
{
    uchar r_, g_, b_, a_;
    float r, g, b, a;
    int th;
    static void generate_achip(void*, int, int, int, uchar*);
    void draw();

public:
    Flmm_ColorA_Button(int, int, int, int, char const* t = 0L);
    void chip_color(uchar, uchar, uchar, uchar a = 255);
    void chip_color(float, float, float, float a = 1.0f);
    void chip_color(Fl_Color);
    void rgba(double& ro, double& go, double& bo, double& ao);
};
