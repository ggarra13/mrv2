// $Id: Flu_Label.h,v 1.7 2004/03/28 14:06:57 jbryan Exp $

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

#include <stdlib.h>
#include <string.h>
#include <FL/Fl_Box.H>

#include "mrvFLU/Flu_Enumerations.h"

//! This class just provides an easier interface to making labels in FLTK
/*! All this class does is copy the label string to internal storage
  since FLTK needs non-transient memory for labels. */
class FLU_EXPORT Flu_Label : public Fl_Box
{

 public:

  //! Normal FLTK widget constructor
  Flu_Label( int x, int y, int w, int h, const char* l = 0 );

  //! Default destructor
  virtual ~Flu_Label();

  //! Overload of Fl_Box::draw()
  void draw();

  //! Set whether the label automatically resizes itself to fit the text. Default is \c false
  inline void auto_resize( bool b )
    { _autoSize = b; }

  //! Get whether the label automatically resizes itself to fit the text.
  inline bool auto_resize() const
    { return _autoSize; }

  //! Alias for label()
  inline void value( const char* l )
    { label(l); }

  //! \return the label
  inline const char* value() const
    { return _label; }

  //! Set the label to \b l
  void label( const char* l );

  //! \return the label
  inline const char* label() const
    { return _label; }

 protected:

  char* _label;
  bool _autoSize;

};
