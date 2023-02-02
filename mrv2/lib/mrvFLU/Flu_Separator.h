// $Id: Flu_Separator.h,v 1.6 2003/08/20 16:29:42 jbryan Exp $

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

/* fltk includes */
#include <FL/Fl.H>
#include <FL/Fl_Widget.H>
#include <FL/fl_draw.H>

#include "mrvFLU/Flu_Enumerations.h"

//! A simple class that draws a separator line using the current box type
class FLU_EXPORT Flu_Separator : public Fl_Widget
{
 public:

  enum {
    HORIZONTAL,
    VERTICAL
  };

  //! Normal FLTK constructor. Default type() is \b HORIZONTAL
  Flu_Separator( int X, int Y, int W, int H, const char *l = 0 );

  //! Get the type
  inline int type() const
    { return _type; }

  //! Set the type
  inline void type( int t )
    { _type = t; }

 protected:

  int _type;

  void draw();

};
