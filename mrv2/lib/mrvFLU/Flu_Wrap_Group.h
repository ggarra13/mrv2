// $Id: Flu_Wrap_Group.h,v 1.10 2004/01/27 21:44:21 jbryan Exp $

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
#include <FL/Fl_Group.H>
#include <FL/Fl_Scrollbar.H>
#include <FL/fl_draw.H>

#include "mrvFLU/Flu_Enumerations.h"

//! This class provides an alternative to Fl_Group that automatically arranges
//! the children either left to right and top to bottom (for type() == \c
//! FL_VERTICAL), or top to bottom and left to right (for type() == \c
//! FL_HORIZONTAL), within the available size of the group, with a scrollbar
//! turning on if they don't all fit
/*! This class is a group with a scrollbar and an \b Fl_Group inside (both
  publicly exposed). The \b Fl_Group contains the actual child widgets of this
  group.

  Most of the \b Fl_Group member functions are reimplemented here in a
  pass-through fashion to the internal group. This means that casual use of a
  descendent instance will be almost exactly the same as for a regular \b
  Fl_Group, with any additional access provided directly through member \b
  group.

  The goal of this class is to provide a group that dynamically and evenly
  distributes its children within a fixed space, similar to those available in
  other GUI toolkits.
*/
class FLU_EXPORT Flu_Wrap_Group : public Fl_Group {
public:
  class Scrollbar : public Fl_Scrollbar {
  public:
    Scrollbar(int x, int y, int w, int h, const char *l = 0);
    int handle(int event);
  };

  //! Normal FLTK constructor
  Flu_Wrap_Group(int x, int y, int w, int h, const char *l = 0);

  //! \return the widget that is visibly above \b w in the group, or \c NULL if
  //! no such widget exists
  Fl_Widget *above(Fl_Widget *w);

  //! \return the widget that is visibly below \b w in the group, or \c NULL if
  //! no such widget exists
  Fl_Widget *below(Fl_Widget *w);

  //! Override of Fl_Group::draw()
  void draw();

  //! \return the widget that is visibly to the left of \b w in the group, or \c
  //! NULL if no such widget exists
  Fl_Widget *left(Fl_Widget *w);

  //! \return the widget that is logically after \b w in the groups order, or \c
  //! NULL if no such widget exists
  Fl_Widget *next(Fl_Widget *w);

  //! Set the offset for where the first child starts
  inline void offset(int x, int y) {
    _offset[0] = x, _offset[1] = y;
    redraw();
  }

  //! \return the x offset for where the first child starts
  inline int offset_x() const { return _offset[0]; }

  //! \return the y offset for where the first child starts
  inline int offset_y() const { return _offset[1]; }

  //! \return the widget that is logically before \b w in the groups order, or
  //! \c NULL if no such widget exists
  Fl_Widget *previous(Fl_Widget *w);

  //! Override of Fl_Group::resize()
  void resize(int x, int y, int w, int h);

  //! \return the widget that is visibly to the right of \b w in the group, or
  //! \c NULL if no such widget exists
  Fl_Widget *right(Fl_Widget *w);

  //! Scroll the group so that the given widget is shown (usually aligned to the
  //! left/top)
  void scroll_to(const Fl_Widget *w);

  //! Scroll the group so that the given widget is shown (usually aligned to the
  //! left/top)
  inline void scroll_to(const Fl_Widget &w) { scroll_to(&w); }

  //! Scroll the group to the beginning of the list
  void scroll_to_beginning();

  //! Scroll the group to the end of the list
  void scroll_to_end();

  //! Set the spacing between children
  inline void spacing(int x, int y) {
    _spacing[0] = x, _spacing[1] = y;
    redraw();
  }

  //! \return the x spacing between children
  inline int spacing_x() const { return _spacing[0]; }

  //! \return the y spacing between children
  inline int spacing_y() const { return _spacing[1]; }

  //! Set the wrap type. Must be either \c FL_VERTICAL (children wrap according
  //! to the width, with a vertical scrollbar) or \c FL_HORIZONTAL (children
  //! wrap according to the height, with a horizontal scrollbar). Default is \c
  //! FL_HORIZONTAL
  void type(int t);

  //! Get the wrap type
  inline int type() const { return _type; }

  /*! \name Pass-through functions for the internal Fl_Group
   * These are strictly for convenience. Only the most commonly called functions
   * have been re-implemented. You can also explicitly access the group object
   * for more control.
   */
  //@{

  inline Fl_Widget *const *array() const { return group.array(); }

  inline int find(const Fl_Widget *w) const { return group.find(w); }

  inline int find(const Fl_Widget &w) const { return group.find(w); }

  inline void clear() { group.clear(); }

  inline Fl_Widget *child(int n) const { return group.child(n); }

  inline int children() const { return group.children(); }

  inline void begin() { group.begin(); }

  inline void end() {
    group.end();
    Fl_Group::end();
  }

  inline void resizable(Fl_Widget *box) { group.resizable(box); }

  inline void resizable(Fl_Widget &box) { group.resizable(box); }

  inline Fl_Widget *resizable() const { return group.resizable(); }

  inline void add(Fl_Widget &w) { group.add(w); }

  inline void add(Fl_Widget *w) { group.add(w); }

  inline void insert(Fl_Widget &w, int n) { group.insert(w, n); }

  inline void insert(Fl_Widget &w, Fl_Widget *beforethis) {
    group.insert(w, beforethis);
  }

  inline void remove(Fl_Widget &w) { group.remove(w); }

  inline void add_resizable(Fl_Widget &box) { group.add_resizable(box); }

  //@}

  Scrollbar scrollbar;
  Fl_Group group;

protected:
  inline static void _scrollCB(Fl_Widget *, void *arg) {
    ((Flu_Wrap_Group *)arg)->redraw();
  }

  int layout(bool sbVisible, bool doScrollTo, int *measure = 0);

  const Fl_Widget *scrollTo;
  int _offset[2], _spacing[2], _type;
};
