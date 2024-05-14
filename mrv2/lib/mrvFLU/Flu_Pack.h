//
// Pack header file for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2020 by Bill Spitzak and others.
//
// This library is free software. Distribution and use rights are outlined in
// the file "COPYING" which should have been included with this file.  If this
// file is missing or damaged, see the license at:
//
//     https://www.fltk.org/COPYING.php
//
// Please see the following page on how to report bugs and issues:
//
//     https://www.fltk.org/bugs.php
//

/* \file
   Flu_Pack widget . */

#ifndef Flu_Pack_H
#define Flu_Pack_H

#include <FL/Fl_Group.H>

/**
  This widget was designed to add the functionality of compressing and
  aligning widgets.

  If type() is Flu_Pack::HORIZONTAL all the children are
  resized to the height of the Flu_Pack, and are moved next to
  each other horizontally. If type() is not Flu_Pack::HORIZONTAL
  then the children are resized to the width and are stacked below each
  other.  Then the Flu_Pack resizes itself to surround the child widgets.

  You may want to put the Flu_Pack inside an Fl_Scroll.

  The \p 'resizable()' for Flu_Pack is set to NULL by default. Its behavior
  is slightly different than in a normal Fl_Group widget: only if the
  resizable() widget is the last widget in the group it is extended to take
  the full available width or height, respectively, of the Flu_Pack group.

  \note You can nest Flu_Pack widgets or put them inside Fl_Scroll widgets
    or inside other group widgets but their behavior can sometimes be
    <i>"surprising"</i>. This is partly due to the fact that Flu_Pack widgets
    resize themselves during their draw() operation, trying to react on
    their child widgets resizing themselves during \b their draw() operations
    which can be confusing. If you want to achieve special resize behavior
    of nested group widgets it can sometimes be easier to derive your own
    specialized group widget than to try to make nested Flu_Pack widgets
    behave as expected.

  \see Fl_Group::resizable()
*/
class FL_EXPORT Flu_Pack : public Fl_Group {
  int spacing_;

public:
  enum { // values for type(int)
    VERTICAL = 0,
    HORIZONTAL = 1
  };

protected:
  void draw() FL_OVERRIDE;

public:
  Flu_Pack(int X, int Y, int W, int H, const char *L = 0);

  /**
    Gets the number of extra pixels of blank space that are added
    between the children.
  */
  int spacing() const {return spacing_;}

  /**
    Sets the number of extra pixels of blank space that are added
    between the children.
  */
  void spacing(int i) {spacing_ = i;}

  /** Returns non-zero if Flu_Pack alignment is horizontal.

    \returns non-zero if Flu_Pack alignment is horizontal (Flu_Pack::HORIZONTAL)

    \note Currently the return value is the same as Fl_Group::type(), but
      this may change in the future. Do not set any other values than the
      following with Flu_Pack::type():
      - Flu_Pack::VERTICAL (Default)
      - Flu_Pack::HORIZONTAL

    See class Flu_Pack documentation for details.
  */
  uchar horizontal() const {return type();}

  void resize(int X, int Y, int W, int H) FL_OVERRIDE;
  
  int handle(int event) FL_OVERRIDE;

  void draw_child(Fl_Widget& o) const;
  void update_child(Fl_Widget& o) const;
};

#endif
