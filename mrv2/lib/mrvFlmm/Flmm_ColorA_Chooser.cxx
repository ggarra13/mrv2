//
// "$Id:$"
//
// Flmm_ColorA_Chooser source file for the FLMM extension to FLTK.
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

/** \class Flmm_ColorA_Chooser
 * Color chooser widget that allows display and selection of an alpha value.
 *
 * The colorA chooser widget displays a color wheel and sliders
 * for intensity and transparency (alpha). If built into a larger
 * dialog box, the size of this widget should be 245x145. 
 * 
 * The recommended way to access a the alpha colo chooser is via
 * a call to flmm_color_a_chooser.
 */

/** \fn Flmm_ColorA_Chooser::mode()
 * Return the current display mode of the color chooser.
 *
 * \return the current value display mode: rgb, byte, hex or hsv
 */

/** \fn Flmm_ColorA_Chooser::hue() const
 * Return the current hue value.
 */

/** \fn Flmm_ColorA_Chooser::saturation() const
 * Return the current saturation.
 */

/** \fn Flmm_ColorA_Chooser::value() const
 * Return the intensity.
 */

/** \fn Flmm_ColorA_Chooser::r() const
 * Return the value of the red color component.
 */

/** \fn Flmm_ColorA_Chooser::g() const
 * Return the value of the green color component.
 */

/** \fn Flmm_ColorA_Chooser::b() const
 * Return the value of the blue color component.
 */

/** \fn Flmm_ColorA_Chooser::a() const
 * Return the value of the alpha component.
 *
 * \return 0 for transparent, or 1 for opaque or any value inside that range.
 */

#include <FL/Fl.H>
#include "Flmm_ColorA_Chooser.h"
#include "Flmm_ColorA_Button.h"
#include <FL/fl_draw.H>
#include <FL/math.h>

#include <iostream>

#include "mrvCore/mrvMath.h"

#include <stdio.h>

#define CIRCLE 1
#define UPDATE_HUE_BOX 1

static Flmm_ColorA_Window* chooserWindow = nullptr;

/**
 * Convert hsv color components to rgb.
 *
 * This function converts the hue, saturation and intensity values 
 * into red, green and blue components.
 */
void Flmm_ColorA_Chooser::hsv2rgb(
	double H, double S, double V, double& R, double& G, double& B) {
  Fl_Color_Chooser::hsv2rgb(H, S, V, R, G, B);
}

/**
 * Convert rgb color components to hsv.
 *
 * This function converts the red, green and blue values 
 * into hue, saturation and intensity components.
 */
void Flmm_ColorA_Chooser::rgb2hsv(
	double R, double G, double B, double& H, double& S, double& V) {
  Fl_Color_Chooser::rgb2hsv(R, G, B, H, S, V);
}

enum {M_RGB, M_BYTE, M_HEX, M_HSV}; // modes
static Fl_Menu_Item mode_menu[] = {
  {"rgb"},
  {"byte"},
  {"hex"},
  {"hsv"},
  {0}
};

int Flmm_Value_Input::format(char* buf) {
  Flmm_ColorA_Chooser* c = (Flmm_ColorA_Chooser*)parent();
  if (c->mode() == M_HEX) return snprintf(buf, 5, "0x%02X", int(value()));
  else return Fl_Valuator::format(buf);
}

void Flmm_ColorA_Chooser::set_valuators() {
  switch (mode()) {
  case M_RGB:
    rvalue.range(0,1); rvalue.step(1,1000); rvalue.value(r_);
    gvalue.range(0,1); gvalue.step(1,1000); gvalue.value(g_);
    bvalue.range(0,1); bvalue.step(1,1000); bvalue.value(b_);
    avalue.range(0,1); avalue.step(1,1000); avalue.value(a_);
    break;
  case M_BYTE:
  case M_HEX:
    rvalue.range(0,255); rvalue.step(1); rvalue.value(int(255*r_+.5));
    gvalue.range(0,255); gvalue.step(1); gvalue.value(int(255*g_+.5));
    bvalue.range(0,255); bvalue.step(1); bvalue.value(int(255*b_+.5));
    avalue.range(0,255); avalue.step(1); avalue.value(int(255*a_+.5));
    break;
  case M_HSV:
    rvalue.range(0,6); rvalue.step(1,1000); rvalue.value(hue_);
    gvalue.range(0,1); gvalue.step(1,1000); gvalue.value(saturation_);
    bvalue.range(0,1); bvalue.step(1,1000); bvalue.value(value_);
    avalue.range(0,1); avalue.step(1,1000); avalue.value(a_);
    break;
  }
}

/**
 * Set a new color.
 *
 * Set the displayed color and alpha to new values.
 *
 * \param R, G, B red green and blue color components
 * \param A alpha component
 * \return 0, if no values changed over the previous settings, else 1
 */
int Flmm_ColorA_Chooser::rgb(double R, double G, double B, double A) {
  if (R == r_ && G == g_ && B == b_ && A == a_) return 0;
  double pa = a_;
  r_ = R; g_ = G; b_ = B; a_ = A;
  double ph = hue_;
  double ps = saturation_;
  double pv = value_;
  rgb2hsv(R,G,B,hue_,saturation_,value_);
  set_valuators();
  set_changed();
  if (value_ != pv) {
#ifdef UPDATE_HUE_BOX
    huebox.damage(FL_DAMAGE_SCROLL);
#endif
    valuebox.damage(FL_DAMAGE_EXPOSE);
  }
  if (hue_ != ph || saturation_ != ps) {
    huebox.damage(FL_DAMAGE_EXPOSE); 
    valuebox.damage(FL_DAMAGE_SCROLL);
  }
  if (hue_ != ph || saturation_ != ps || a_ != pa) {
    alphabox.damage(FL_DAMAGE_SCROLL);
  }
  return 1;
}

/**
 * Set a new color.
 *
 * Set the displayed color and alpha to new values.
 *
 * \param H, S, V hue, saturation and intensity color components
 * \param A alpha component
 * \return 0, if no values changed over the previous settings, else 1
 */
int Flmm_ColorA_Chooser::hsv(double H, double S, double V, double A) {
  H = fmod(H,6.0); if (H < 0.0) H += 6.0;
  if (S < 0.0) S = 0.0; else if (S > 1.0) S = 1.0;
  if (V < 0.0) V = 0.0; else if (V > 1.0) V = 1.0;
  if (H == hue_ && S == saturation_ && V == value_ && A == a_) return 0;
  double pa = a_;
  double ph = hue_;
  double ps = saturation_;
  double pv = value_;
  hue_ = H; saturation_ = S; value_ = V; a_ = A;
  if (value_ != pv) {
#ifdef UPDATE_HUE_BOX
    huebox.damage(FL_DAMAGE_SCROLL);
#endif
    valuebox.damage(FL_DAMAGE_EXPOSE);}
  if (hue_ != ph || saturation_ != ps) {
    huebox.damage(FL_DAMAGE_EXPOSE); 
    valuebox.damage(FL_DAMAGE_SCROLL);
  }
  if (hue_ != ph || saturation_ != ps || a_ != pa) {
    alphabox.damage(FL_DAMAGE_SCROLL);
  }
  hsv2rgb(H,S,V,r_,g_,b_);
  set_valuators();
  set_changed();
  return 1;
}

////////////////////////////////////////////////////////////////

static void tohs(double x, double y, double& h, double& s) {
#ifdef CIRCLE
  x = 2*x-1;
  y = 1-2*y;
  s = sqrt(x*x+y*y); if (s > 1.0) s = 1.0;
  h = (3.0/M_PI)*atan2(y,x);
  if (h<0) h += 6.0;
#else
  h = fmod(6.0*x,6.0); if (h < 0.0) h += 6.0;
  s = 1.0-y; if (s < 0.0) s = 0.0; else if (s > 1.0) s = 1.0;
#endif
}

int Flmm_HueBox::handle(int e) {
  static double ih, is;
  Flmm_ColorA_Chooser* c = (Flmm_ColorA_Chooser*)parent();
  switch (e) {
  case FL_PUSH:
    if (Fl::visible_focus()) {
      Fl::focus(this);
      redraw();
    }
    ih = c->hue();
    is = c->saturation();
  case FL_DRAG: {
    double Xf, Yf, H, S;
    Xf = (Fl::event_x()-x()-Fl::box_dx(box()))/double(w()-Fl::box_dw(box()));
    Yf = (Fl::event_y()-y()-Fl::box_dy(box()))/double(h()-Fl::box_dh(box()));
    tohs(Xf, Yf, H, S);
    if (fabs(H-ih) < 3*6.0/w()) H = ih;
    if (fabs(S-is) < 3*1.0/h()) S = is;
    if (Fl::event_state(FL_CTRL)) H = ih;
    if (c->hsv(H, S, c->value(),c->a())) c->do_callback();
    } return 1;
  case FL_FOCUS :
  case FL_UNFOCUS :
    if (Fl::visible_focus()) {
      redraw();
      return 1;
    }
    else return 1;
  case FL_KEYBOARD :
    return handle_key(Fl::event_key());
  default:
    return 0;
  }
}

static void generate_image(void* vv, int X, int Y, int W, uchar* buf) {
  Flmm_HueBox* v = (Flmm_HueBox*)vv;
  int iw = v->w()-Fl::box_dw(v->box());
  double Yf = double(Y)/(v->h()-Fl::box_dh(v->box()));
#ifdef UPDATE_HUE_BOX
  const double V = ((Flmm_ColorA_Chooser*)(v->parent()))->value();
#else
  const double V = 1.0;
#endif
  for (int x = X; x < X+W; x++) {
    double Xf = double(x)/iw;
    double H,S; tohs(Xf,Yf,H,S);
    double r,g,b;
    Flmm_ColorA_Chooser::hsv2rgb(H,S,V,r,g,b);
    *buf++ = uchar(255*r+.5);
    *buf++ = uchar(255*g+.5);
    *buf++ = uchar(255*b+.5);
  }
}

int Flmm_HueBox::handle_key(int key) {
  int w1 = w()-Fl::box_dw(box())-6;
  int h1 = h()-Fl::box_dh(box())-6;
  Flmm_ColorA_Chooser* c = (Flmm_ColorA_Chooser*)parent();

#ifdef CIRCLE
  int X = int(.5*(cos(c->hue()*(M_PI/3.0))*c->saturation()+1) * w1);
  int Y = int(.5*(1-sin(c->hue()*(M_PI/3.0))*c->saturation()) * h1);
#else
  int X = int(c->hue()/6.0*w1);
  int Y = int((1-c->saturation())*h1);
#endif

  switch (key) {
    case FL_Up :
      Y -= 3;
      break;
    case FL_Down :
      Y += 3;
      break;
    case FL_Left :
      X -= 3;
      break;
    case FL_Right :
      X += 3;
      break;
    default :
      return 0;
  }

  double Xf, Yf, H, S;
  Xf = (double)X/(double)w1;
  Yf = (double)Y/(double)h1;
  tohs(Xf, Yf, H, S);
  if (c->hsv(H, S, c->value(),c->a())) c->do_callback();

  return 1;
}

void Flmm_HueBox::draw() {
  if (damage()&FL_DAMAGE_ALL) draw_box();
  int x1 = x()+Fl::box_dx(box());
  int yy1 = y()+Fl::box_dy(box());
  int w1 = w()-Fl::box_dw(box());
  int h1 = h()-Fl::box_dh(box());
  if (damage() == FL_DAMAGE_EXPOSE) fl_clip(x1+px,yy1+py,6,6);
  fl_draw_image(generate_image, this, x1, yy1, w1, h1);
  if (damage() == FL_DAMAGE_EXPOSE) fl_pop_clip();
  Flmm_ColorA_Chooser* c = (Flmm_ColorA_Chooser*)parent();
#ifdef CIRCLE
  int X = int(.5*(cos(c->hue()*(M_PI/3.0))*c->saturation()+1) * (w1-6));
  int Y = int(.5*(1-sin(c->hue()*(M_PI/3.0))*c->saturation()) * (h1-6));
#else
  int X = int(c->hue()/6.0*(w1-6));
  int Y = int((1-c->saturation())*(h1-6));
#endif
  if (X < 0) X = 0; else if (X > w1-6) X = w1-6;
  if (Y < 0) Y = 0; else if (Y > h1-6) Y = h1-6;
  //  fl_color(c->value()>.75 ? FL_BLACK : FL_WHITE);
  draw_box(FL_UP_BOX,x1+X,yy1+Y,6,6,Fl::focus() == this ? FL_FOREGROUND_COLOR : FL_GRAY);
  px = X; py = Y;
}

////////////////////////////////////////////////////////////////

int Flmm_ValueBox::handle(int e) {
  static double iv;
  Flmm_ColorA_Chooser* c = (Flmm_ColorA_Chooser*)parent();
  switch (e) {
    case FL_PUSH:
      if (Fl::visible_focus()) {
        Fl::focus(this);
        redraw();
      }
      iv = c->value();
    case FL_DRAG: {
      double Yf;
      Yf = 1-(Fl::event_y()-y()-Fl::box_dy(box()))/double(h()-Fl::box_dh(box()));
      if (fabs(Yf-iv)<(3*1.0/h())) Yf = iv;
      if (c->hsv(c->hue(),c->saturation(),Yf,c->a())) c->do_callback();
    } return 1;
    case FL_FOCUS :
    case FL_UNFOCUS :
      if (Fl::visible_focus()) {
        redraw();
        return 1;
      }
      else return 1;
    case FL_KEYBOARD :
      return handle_key(Fl::event_key());
    default:
      return 0;
  }
}

static double tr, tg, tb;
static void generate_vimage(void* vv, int X, int Y, int W, uchar* buf) {
  Flmm_ValueBox* v = (Flmm_ValueBox*)vv;
  double Yf = 255*(1.0-double(Y)/(v->h()-Fl::box_dh(v->box())));
  uchar r = uchar(tr*Yf+.5);
  uchar g = uchar(tg*Yf+.5);
  uchar b = uchar(tb*Yf+.5);
  for (int x = X; x < X+W; x++) {
    *buf++ = r; *buf++ = g; *buf++ = b;
  }
}

void Flmm_ValueBox::draw() {
  if (damage()&FL_DAMAGE_ALL) draw_box();
  Flmm_ColorA_Chooser* c = (Flmm_ColorA_Chooser*)parent();
  c->hsv2rgb(c->hue(),c->saturation(),1.0,tr,tg,tb);
  int x1 = x()+Fl::box_dx(box());
  int yy1 = y()+Fl::box_dy(box());
  int w1 = w()-Fl::box_dw(box());
  int h1 = h()-Fl::box_dh(box());
  if (damage() == FL_DAMAGE_EXPOSE) fl_clip(x1,yy1+py,w1,6);
  fl_draw_image(generate_vimage, this, x1, yy1, w1, h1);
  if (damage() == FL_DAMAGE_EXPOSE) fl_pop_clip();
  int Y = int((1-c->value()) * (h1-6));
  if (Y < 0) Y = 0; else if (Y > h1-6) Y = h1-6;
  draw_box(FL_UP_BOX,x1,yy1+Y,w1,6,Fl::focus() == this ? FL_FOREGROUND_COLOR : FL_GRAY);
  py = Y;
}

int Flmm_ValueBox::handle_key(int key) {
  int h1 = h()-Fl::box_dh(box())-6;
  Flmm_ColorA_Chooser* c = (Flmm_ColorA_Chooser*)parent();
  
  int Y = int((1-c->value()) * h1);
  if (Y < 0) Y = 0; else if (Y > h1) Y = h1;
  
  switch (key) {
    case FL_Up :
      Y -= 3;
      break;
    case FL_Down :
      Y += 3;
      break;
    default :
      return 0;
  }
  
  double Yf;
  Yf = 1-((double)Y/(double)h1);
  if (c->hsv(c->hue(),c->saturation(),Yf,c->a())) c->do_callback();
  
  return 1;
}

////////////////////////////////////////////////////////////////

int Flmm_AlphaBox::handle(int e) {
  static double iv;
  Flmm_ColorA_Chooser* c = (Flmm_ColorA_Chooser*)parent();
  switch (e) {
    case FL_PUSH:
      if (Fl::visible_focus()) {
        Fl::focus(this);
        redraw();
      }
      iv = c->a();
    case FL_DRAG: {
      double a;
      a = 1-(Fl::event_y()-y()-Fl::box_dy(box()))/double(h()-Fl::box_dh(box()));
      if (a<0.0) a = 0.0; if (a>1.0) a = 1.0; 
      if (c->rgb(c->r(),c->g(),c->b(),a)) c->do_callback();
    } return 1;
    case FL_FOCUS :
    case FL_UNFOCUS :
      if (Fl::visible_focus()) {
        redraw();
        return 1;
      }
      else return 1;
    case FL_KEYBOARD :
      return handle_key(Fl::event_key());
    default:
      return 0;
  }
}

//++ static double tr, tg, tb;
static void generate_aimage(void* vv, int X, int Y, int W, uchar* buf) {
  Flmm_AlphaBox* v = (Flmm_AlphaBox*)vv;
  double Yf = double(Y)/(v->h()-Fl::box_dh(v->box())), iYf = 1.0-Yf;
  uchar rg = (uchar)(255*(iYf*tr + Yf*0.8));
  uchar gg = (uchar)(255*(iYf*tg + Yf*0.4));
  uchar bg = (uchar)(255*(iYf*tb + Yf*0.4));
  uchar r, g, b;
  if ((Y&0x07)==4) {
    r = rg; g = gg; b = bg;
  } else {
    r = (uchar)(255*(iYf*tr + Yf*0.8));
    g = (uchar)(255*(iYf*tg + Yf*0.8));
    b = (uchar)(255*(iYf*tb + Yf*0.8));
  }
  for (int x = X; x < X+W; x++) {
    if ((x&0x07)==4) {
      *buf++ = rg; *buf++ = gg; *buf++ = bg;
    } else {
      *buf++ = r; *buf++ = g; *buf++ = b;
    }
  }
}

void Flmm_AlphaBox::draw() {
  if (damage()&FL_DAMAGE_ALL) draw_box();
  Flmm_ColorA_Chooser* c = (Flmm_ColorA_Chooser*)parent();
  c->hsv2rgb(c->hue(),c->saturation(),1.0,tr,tg,tb);
  int x1 = x()+Fl::box_dx(box());
  int yy1 = y()+Fl::box_dy(box());
  int w1 = w()-Fl::box_dw(box());
  int h1 = h()-Fl::box_dh(box());
  if (damage() == FL_DAMAGE_EXPOSE) fl_clip(x1,yy1+py,w1,6);
  fl_draw_image(generate_aimage, this, x1, yy1, w1, h1);
  if (damage() == FL_DAMAGE_EXPOSE) fl_pop_clip();
  int Y = int((1-c->a()) * (h1-6));
  if (Y < 0) Y = 0; else if (Y > h1-6) Y = h1-6;
  draw_box(FL_UP_BOX,x1,yy1+Y,w1,6,Fl::focus() == this ? FL_FOREGROUND_COLOR : FL_GRAY);
  py = Y;
}

int Flmm_AlphaBox::handle_key(int key) {
  int h1 = h()-Fl::box_dh(box())-6;
  Flmm_ColorA_Chooser* c = (Flmm_ColorA_Chooser*)parent();
  
  int Y = int((1-c->a()) * h1);
  if (Y < 0) Y = 0; else if (Y > h1) Y = h1;
  
  switch (key) {
    case FL_Up :
      Y -= 3;
      break;
    case FL_Down :
      Y += 3;
      break;
    default :
      return 0;
  }
  
  double a;
  a = 1-((double)Y/(double)h1);
  if (c->rgb(c->r(),c->g(),c->b(),a)) c->do_callback();
  return 1;
}

////////////////////////////////////////////////////////////////

void Flmm_ColorA_Chooser::rgba_cb(Fl_Widget* o, void*) {
  Flmm_ColorA_Chooser* c = (Flmm_ColorA_Chooser*)(o->parent());
  double R = c->rvalue.value();
  double G = c->gvalue.value();
  double B = c->bvalue.value();
  double A = c->avalue.value();
  if (c->mode() == M_HSV) {
    if (c->hsv(R,G,B,A)) c->do_callback();
    return;
  }
  if (c->mode() != M_RGB) {
    R = R/255;
    G = G/255;
    B = B/255;
    A = A/255;
  }
  if (c->rgb(R,G,B,A)) c->do_callback();
}

void Flmm_ColorA_Chooser::mode_cb(Fl_Widget* o, void*) {
  Flmm_ColorA_Chooser* c = (Flmm_ColorA_Chooser*)(o->parent());
  // force them to redraw even if value is the same:
  c->rvalue.value(-1);
  c->gvalue.value(-1);
  c->bvalue.value(-1);
  c->avalue.value(-1);
  c->set_valuators();
}

////////////////////////////////////////////////////////////////

/** 
 * Create a new color chooser widget with alpha setting.
 *
 * The colorA chooser widget displays a color wheel and sliders for intensity 
 * and transparency (alpha). If built into a larger dialog box, the size of 
 * this widget should be 245x145.
 * 
 * The recommended way to access a the alpha colo chooser is via a call to 
 * flmm_color_a_chooser. 
 *
 * \param X, Y, W, H position and size of new widget
 * \param L optional label text
 */
Flmm_ColorA_Chooser::Flmm_ColorA_Chooser(int X, int Y, int W, int H, const char* L)
  : Fl_Group(0,0,265,145,L),
    huebox(0,0,145,145),
    valuebox(145,0,20,145),
    alphabox(165,0,20,145),
    choice(190,  0,55,25),
    rvalue(190, 30,55,25),
    gvalue(190, 60,55,25),
    bvalue(190, 90,55,25),
    avalue(190,120,55,25),
    resize_box(0,0,145,145)
{
  end();
  resizable(resize_box);
  resize(X,Y,W,H);
  r_ = g_ = b_ = a_ = 0;
  hue_ = 0.0;
  saturation_ = 0.0;
  value_ = 0.0;
  huebox.box(FL_DOWN_FRAME);
  valuebox.box(FL_DOWN_FRAME);
  alphabox.box(FL_DOWN_FRAME);
  choice.menu(mode_menu);
  set_valuators();
  rvalue.callback(rgba_cb);
  gvalue.callback(rgba_cb);
  bvalue.callback(rgba_cb);
  avalue.callback(rgba_cb);
  choice.callback(mode_cb);
  choice.box(FL_THIN_UP_BOX);
  choice.textfont(FL_HELVETICA_BOLD_ITALIC);
}

////////////////////////////////////////////////////////////////
// Flmm_ColorA_Chooser():

#include <FL/Fl_Box.H>
#include <FL/Fl_Return_Button.H>

static void chooser_cb(Fl_Object* o, void* vv) {
  Flmm_ColorA_Chooser* c = (Flmm_ColorA_Chooser*)o;
  ColorAChip* v = (ColorAChip*)vv;
  v->chip_color((float)c->r(), (float)c->g(), (float)c->b(), (float)c->a());
  v->damage(FL_DAMAGE_EXPOSE);
}

static void change_color_cb(ColorAChip* o, void* v)
{
  Flmm_ColorA_Chooser* c = (Flmm_ColorA_Chooser*)v;
  double r, g, b, a;
  o->rgba(r, g, b, a);
  c->rgb(r, g, b, a);
  c->redraw();
  c->do_callback();
}

extern const char* fl_ok;
extern const char* fl_cancel;

Flmm_ColorA_Window::Flmm_ColorA_Window(int W, int H, const char* L) :
    Fl_Double_Window(W, H, L),
    chooser(10, 10, 265, 145),
    ok_color(10, 160, 120, 25),
    ok_button(10, 195, 120, 25, fl_ok),
    cancel_color(135, 160, 120, 25),
    cancel_button(135, 195, 120, 25, fl_cancel)
{
    for (int i = 0; i < 7; ++i)
    {
        saved[i] = new SavedAChip(265, 10 + i * 30, 30, 30);
        saved[i]->callback((Fl_Callback*)change_color_cb, &chooser);
        saved[i]->chip_color(0.F, 0.F, 0.F, 1.F);
    }
  resizable(chooser);
  size_range(W,H);
  chooser.callback(chooser_cb, &ok_color);
}

int Flmm_ColorA_Window::run(double& r, double& g, double& b, double& a)
{
    ok_color.chip_color((float)r, (float)g, (float)b, (float)a);
    cancel_color.chip_color((float)r, (float)g, (float)b, (float)a);
    chooser.rgb(r,g,b,a);
    show();
    while (shown()) {
        Fl::wait();
        for (;;) {
            Fl_Widget* o = Fl::readqueue();
            if (!o) break;
            if (o == &ok_button) {
                chooserWindow->hide();
                r = chooser.r();
                g = chooser.g();
                b = chooser.b();
                a = chooser.a();

                // Check  if color is already on the list
                const double fuzzFactor = 0.005F;
                double sr = 0, sg = 0, sb = 0, sa = 0;
                for (int i = 0; i < 7; ++i)
                {
                    saved[i]->rgba(sr, sg, sb, sa);
                    if (mrv::is_equal(sr, r, fuzzFactor) &&
                        mrv::is_equal(sg, g, fuzzFactor) &&
                        mrv::is_equal(sb, b, fuzzFactor) &&
                        mrv::is_equal(sa, a, fuzzFactor))
                    {
                        return 1;
                    }
                }


                // Move the colors down.
                for (int i = 5; i >= 0; --i)
                {
                    saved[i]->rgba(sr, sg, sb, sa);
                    saved[i+1]->chip_color(
                        (float)sr, (float)sg, (float)sb, (float)sa);
                }

                // Add lates color at the top.
                saved[0]->chip_color(
                    (float)r, (float)g, (float)b, (float)a);
                return 1;
            }
            if (o == this || o == &cancel_button)
            {
                chooserWindow->hide();
                return 0;
            }
        }
    }
    return 0;
}
/**
 * A dialog that lets the user choose a color and an alpha componenet.
 *
 * This dialog box is based on FLTK's native fl_color_chooser, adding
 * the ability to display and choose an alpha value (transparency).
 * All components are in double format. An alpha value of 0.0 means full 
 * transparency, 1.0 is opaque.
 *
 * \param name title of color chooser
 * \param r, g, b set and return color components
 * \param a set and return alpha component
 */
int flmm_color_a_chooser(
    const char* name, double& r, double& g, double& b, double& a)
{
    if (!chooserWindow)
        chooserWindow = new Flmm_ColorA_Window(305,230,name);
    chooserWindow->end();
    chooserWindow->set_modal();
    chooserWindow->hotspot(chooserWindow);
    return chooserWindow->run(r, g, b, a);
}

/**
 * A dialog that lets the user choose a color and an alpha componenet.
 *
 * This dialog box is based on FLTK's native fl_color_chooser, adding
 * the ability to display and choose an alpha value (transparency).
 * All components are in float format. An alpha value of 0.0 means full 
 * transparency, 1.0 is opaque.
 *
 * \param name title of color chooser
 * \param r, g, b set and return color components
 * \param a set and return alpha component
 */
int flmm_color_a_chooser(const char* name, float& r, float& g, float& b, float &a) {
  double dr = r, dg = g, db = b, da = a;
  if (flmm_color_a_chooser(name,dr,dg,db,da)) {
    r = dr; g = dg; b = db; a = da;
    return 1;
  }
  return 0;
}

/**
 * A dialog that lets the user choose a color and an alpha componenet.
 *
 * This dialog box is based on FLTK's native fl_color_chooser, adding
 * the ability to display and choose an alpha value (transparency).
 * All components are in unsigned char format. An alpha value of 0 means full 
 * transparency, 255 is opaque.
 *
 * \param name title of color chooser
 * \param r, g, b set and return color components
 * \param a set and return alpha component
 */
int flmm_color_a_chooser(const char* name, uchar& r, uchar& g, uchar& b, uchar &a) {
  double dr = r/255.0;
  double dg = g/255.0;
  double db = b/255.0;
  double da = a/255.0;
  if (flmm_color_a_chooser(name,dr,dg,db,da)) {
    r = uchar(255*dr+.5);
    g = uchar(255*dg+.5);
    b = uchar(255*db+.5);
    a = uchar(255*da+.5);
    return 1;
  }
  return 0;
}

//
// End of "$Id:$".
//
