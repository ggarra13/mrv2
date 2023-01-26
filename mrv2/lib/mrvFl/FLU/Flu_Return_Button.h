// $Id: Flu_Return_Button.h,v 1.2 2003/08/20 16:29:42 jbryan Exp $

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

#include "mrvFl/FLU/Flu_Button.h"

//! This class extends Flu_Button to make a button similar to Fl_Return_Button
class FLU_EXPORT Flu_Return_Button : public Flu_Button
{
 public:

  //! Normal FLTK widget constructor
  Flu_Return_Button( int X,int Y,int W,int H,const char *l = 0 );

  //! Default destructor
  ~Flu_Return_Button();

};
