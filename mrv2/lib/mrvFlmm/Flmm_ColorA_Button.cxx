//
// "$Id:$"
//
// Flmm_ColorA_Button source file for the FLMM extension to FLTK.
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

/** \class Flmm_ColorA_Button
 * A button that shows an RGB color set with alpha channel.
 *
 * The ColorA Button is derived form Fl_Button and shows a color chip
 * and an additional transparent overlay representing an alpha value.
 * This button is useful to give the user the choice of a 
 * color/transparency combination by opening the fl_color_a_chooser.
 */

#include "Flmm_ColorA_Button.h"
#include <FL/Fl.H>
#include <FL/fl_draw.H>

/**
 * Create a colored button with alpha channel;
 *
 * ColorA Button is works just like a regular button. It displays
 * a color chip in any color plus a transparency information (alpha).
 *
 * \param x, y, w, h position and size of the widget
 * \param t optional label text
 */
Flmm_ColorA_Button::Flmm_ColorA_Button(int x, int y, int w, int h, char const *t) 
: Fl_Button(x, y, w, h, t) {
}

/**
 * Set the color of the color chip.
 *
 * This sets the color and alpha of the ColorA chip in 8 bit color space.
 * \param R, G, B red, green and blue color component
 * \param A alpha component; 0 is fully transparent, 255 is opaque
 */
void Flmm_ColorA_Button::chip_color(uchar R, uchar G, uchar B, uchar A) {
  r_ = R; g_ = G; b_ = B; a_ = A;
  r = R/255.0f; g = G/255.0f; b = B/255.0f; a = A/255.0f; 
  redraw();
}

/**
 * Set the color of the color chip.
 *
 * This sets the color and alpha of the ColorA chip in floating point space.
 * \param R, G, B red, green and blue color component
 * \param A alpha component; 0.0 is fully transparent, 1.0 is opaque
 */
void Flmm_ColorA_Button::chip_color(float R, float G, float B, float A) {
  r = R; g = G; b = B; a = A;
  r_ = (uchar)(R*255.0f);
  g_ = (uchar)(G*255.0f);
  b_ = (uchar)(B*255.0f);
  a_ = (uchar)(A*255.0f);
  redraw();
}

/**
 * Set the color of the color chip.
 *
 * This sets the color and alpha from the FLTK color lookup table.
 * Since the LUT has no alpha component, we assume alpha to be 1
 * (opaque).
 * .
 * \param col index of lookup table entry
 */
void Flmm_ColorA_Button::chip_color(Fl_Color col) {
  uchar R, G, B;
  Fl::get_color(col, R, G, B);
  chip_color(R, G, B);
}

void Flmm_ColorA_Button::draw() {  
  if (type() == FL_HIDDEN_BUTTON) return;
  Fl_Color col = value() ? selection_color() : color();
  draw_box(value() ? (down_box()?down_box():fl_down(box())) : box(), col);
  if (a_==255) {
    fl_rectf(x()+Fl::box_dx(box()),
             y()+Fl::box_dy(box()),
             w()-Fl::box_dw(box()),
             h()-Fl::box_dh(box()), r_, g_, b_);
  } else {
    th = h()-Fl::box_dh(box());
    fl_draw_image(generate_achip, this, 
	   x()+Fl::box_dx(box()),
	   y()+Fl::box_dy(box()),
	   w()-Fl::box_dw(box()), th);
  }
  draw_label();
  if (Fl::focus() == this) draw_focus();
}


void Flmm_ColorA_Button::generate_achip(void* vv, int X, int Y, int W, uchar* buf) {
  Flmm_ColorA_Button *v = (Flmm_ColorA_Button*)vv;
  if (v->th==0) return;
  float a = v->a, ia = 1.0f-a;
  float vr = v->r, vg = v->g, vb = v->b;
  uchar ro = (uchar)(vr*255.0f);
  uchar go = (uchar)(vg*255.0f);
  uchar bo = (uchar)(vb*255.0f);
  uchar rg = (uchar)(255*(a*vr + ia*0.8f));
  uchar gg = (uchar)(255*(a*vg + ia*0.4f));
  uchar bg = (uchar)(255*(a*vb + ia*0.4f));
  uchar r, g, b;
  if ((Y&0x07)==4) {
    r = rg; g = gg; b = bg;
  } else {
    r = (uchar)(255*(a*vr + ia*0.8f));
    g = (uchar)(255*(a*vg + ia*0.8f));
    b = (uchar)(255*(a*vb + ia*0.8f));
  }
  int xa = (int)((float)W/(float)v->th*(float)(v->th-Y));
  for (int x = X; x < X+W; x++) {
    if (x>xa) {
      if ((x&0x07)==4) {
	*buf++ = rg; *buf++ = gg; *buf++ = bg;
      } else {
	*buf++ = r; *buf++ = g; *buf++ = b;
      }
    } else {
      *buf++ = ro; *buf++ = go; *buf++ = bo;
    }
  }
}

//
// End of "$Id:$".
//
