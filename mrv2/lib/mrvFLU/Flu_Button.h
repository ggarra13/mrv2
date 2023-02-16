// $Id: Flu_Button.h,v 1.11 2003/12/21 15:58:31 jbryan Exp $

/***************************************************************
 *                FLU - FLTK Utility Widgets
 *  Copyright (C) 2002 Ohio Supercomputer Center, Ohio State University
 *
 * This file and its content is protected by a software license.
 * You should have received a copy of this license with this file.
 * If not, please contact the Ohio Supercomputer Center immediately:
 * Attn: Jason Bryan Re: FLU 1224 Kinnear Rd, Columbus, Ohio 43212
 *
 ***************************************************************/

#pragma once

#include <FL/Fl_Button.H>
#include <FL/Fl_Image.H>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mrvFLU/flu_export.h"

//! This class extends Fl_Button to make a more attractive alternative that
//! hilights as the mouse enters/leaves and automatically grayscales any image
//! for deactivation
class FLU_EXPORT Flu_Button : public Fl_Button {
public:
  //! Normal FLTK widget constructor
  Flu_Button(int X, int Y, int W, int H, const char *l = 0);

  //! Default destructor
  virtual ~Flu_Button();

  //! Override of Fl_Widget::color()
  inline void color(unsigned c) {
    col = (Fl_Color)c;
    Fl_Button::color(col);
  }

  //! Override of Fl_Widget::color()
  inline Fl_Color color() const { return col; }

  //! Override of Fl_Widget::selection_color()
  inline void selection_color(unsigned c) {
    sCol = (Fl_Color)c;
    Fl_Button::selection_color(sCol);
  }

  //! Override of Fl_Widget::selection_color()
  inline Fl_Color selection_color() const { return sCol; }

  //! Set the box to use when the mouse is over the button. If this is \c
  //! FL_NO_BOX, then the regular box() is used
  inline void enter_box(Fl_Boxtype b) { eBox = b; }

  //! Get the box to use when the mouse enters
  inline Fl_Boxtype enter_box() const { return eBox; }

  //! Override of Fl_Widget::image()
  void image(Fl_Image *i);

  //! Override of Fl_Widget::image()
  inline void image(Fl_Image &i) { image(&i); }

  //! Override of Fl_Button::handle()
  int handle(int event);

  // Override of Fl_Button::draw()
  void draw();

protected:
  bool retBtn, linkBtn, overLink;

private:
  void checkLink();
  int labelSize[4];
  bool hover;
  Fl_Color col, sCol;
  Fl_Image *inactiveImg;
  Fl_Boxtype eBox;
};
