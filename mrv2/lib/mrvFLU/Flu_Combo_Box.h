// $Id: Flu_Combo_Box.h,v 1.12 2004/03/24 02:49:00 jbryan Exp $

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

#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Group.H>

#include "mrvFLU/Flu_Enumerations.h"

//! This is a generic base class for implementing widgets with combo-box-like behavior (i.e. a pulldown menu where the "input" area is editable
class FLU_EXPORT Flu_Combo_Box : public Fl_Group
{

public:

  //! Normal FLTK widget constructor
  Flu_Combo_Box( int x, int y, int w, int h, const char *l = 0 );

  //! Default destructor
  ~Flu_Combo_Box();

  //! Get whether the input field can be edited. Default is \c true
  inline bool editable() const
    { return (int)(!input.readonly()); }

  //! Set whether the input field can be edited.
  inline void editable( bool b )
    { input.readonly( (int)(!b) ); }

  //! Get the string in the input field
  inline const char* value() const
    { return input.value(); }

  //! Set the string in the input field and the value of the popup box.
  void value( const char *v );

  //! Set the height of the popup box
  inline void pop_height( int h )
    { popHeight = h; }

  //! Get the height of the popup box
  inline int pop_height()
    { return popHeight; }

  //! Override of Fl_Group::handle()
  int handle( int );

  //! Override of Fl_Group::resize()
  void resize( int X, int Y, int W, int H );

  //! Set the function that will be called when the input area is interacted with
  inline void input_callback( void (*cb)(Fl_Widget*,void*), void* cbd = NULL )
    { _inputCB = cb; _inputCBD = cbd; }

  //! Publicly exposed input widget
  Fl_Input input;

protected:

  void (*_inputCB)(Fl_Widget*,void*);
  void* _inputCBD;

  virtual bool _value( const char *v ) = 0;
  virtual const char* _next() = 0;
  virtual const char* _previous() = 0;
  virtual void _hilight( int event, int x, int y ) = 0;

  void draw();

  void selected( const char *v );

  void set_combo_widget( Fl_Widget *w );

  uchar _valbox;
  bool _pushed, _popped;
  Fl_Widget *_cbox;
  int popHeight;

  static void input_cb( Fl_Widget*, void* v );

  class FLU_EXPORT Popup : public Fl_Double_Window
    {

    public:

      Popup( Flu_Combo_Box *b, Fl_Widget *c, int H );

      ~Popup();

      int handle( int event );

    protected:

      Flu_Combo_Box *combo;
      bool dragging;
      const char* selected;

    };
  friend class Popup;

};
