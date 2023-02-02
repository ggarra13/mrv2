// $Id: Flu_Combo_List.h,v 1.6 2004/03/24 02:49:00 jbryan Exp $

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



#ifndef _FLU_COMBO_LIST_H
#define _FLU_COMBO_LIST_H

#include <FL/Fl_Hold_Browser.H>

#include "mrvFLU/Flu_Combo_Box.h"

//! Just like the Fl_Choice widget except the input area is editable
class FLU_EXPORT Flu_Combo_List : public Flu_Combo_Box
{

public:

  //! Normal FLTK widget constructor
  Flu_Combo_List( int x, int y, int w, int h, const char *l = 0 );

  //! Default destructor
  ~Flu_Combo_List();

  //! Publicly exposed list widget (instance of Fl_Hold_Browser)
  Fl_Hold_Browser list;

 protected:
  void add( const char* v, void* c = NULL ) { list.add(v, c); }

  bool _value( const char *v );
  const char* _next();
  const char* _previous();
  void _hilight( int event, int x, int y );

  inline static void _cb( Fl_Widget *w, void *arg )
    { ((Flu_Combo_List*)arg)->cb(); }
  void cb();

};

#endif
