// $Id: Flu_Label.cpp,v 1.6 2004/10/21 15:21:07 jbryan Exp $

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



#include <FL/fl_draw.H>
#include "mrvFl/FLU/Flu_Label.h"

Flu_Label :: Flu_Label( int x, int y, int w, int h, const char* l )
  : Fl_Box( x, y, w, h, 0 )
{
  _autoSize = false;
  align( FL_ALIGN_LEFT | FL_ALIGN_WRAP | FL_ALIGN_INSIDE );
  _label = NULL;
  label( l );
  box( FL_NO_BOX );
  clear_visible_focus();
}

Flu_Label :: ~Flu_Label()
{
  if( _label )
    delete[] _label;
}

void Flu_Label :: draw()
{
  if( _autoSize )
    {
      fl_font( labelfont(), labelsize() );
      int W = 0, H = 0;
      fl_measure( label(), W, H );
      if( W != w() || H != h() )
        resize( x(), y(), W, H );
    }
  Fl_Box::draw();
}

void Flu_Label :: label( const char* l )
{
  if( _label )
    delete[] _label;
  if( l == NULL )
    {
      _label = new char[1];
      _label[0] = '\0';
    }
  else
    {
      _label = new char[strlen(l)+1];
      strcpy( _label, l );
    }
  Fl_Box::label( _label );
  redraw();
}
