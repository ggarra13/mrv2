// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

//
// This is a nabbed copy of fltk1.3.x Fl_Pack, modified to allow the
// layout code to be called directly, instead of hidden in draw(). -erco
// 03/19/19
//

/* \file
   Pack widget . */

#pragma once

#include <FL/Fl_Group.H>
#include <FL/Fl_Scrollbar.H>

#include <iostream>

/**
   This widget was designed to add the functionality of compressing and
   aligning widgets.
   <P>If type() is Pack::HORIZONTAL all the children are
   resized to the height of the Pack, and are moved next to
   each other horizontally. If type() is not Pack::HORIZONTAL
   then the children are resized to the width and are stacked below each
   other.  Then the Pack resizes itself to surround the child
   widgets.
   <P>This widget is needed for the Fl_Tabs.
   In addition you may want to put the Pack inside an
   Fl_Scroll.

   <P>The resizable for Pack is set to NULL by default.</p>
   <P>See also: Fl_Group::resizable()
*/

namespace mrv {

class FL_EXPORT Pack : public Fl_Group {
  int spacing_;

public:
  enum { // values for type(int)
    VERTICAL = 0,
    HORIZONTAL = 1
  };

protected:
  virtual void draw() override;

public:
  Pack(int x, int y, int w, int h, const char *l = 0);
  virtual ~Pack();

  void end() {
    Fl_Group::end();
    layout();
  }

  /**
     Gets the number of extra pixels of blank space that are added
     between the children.
  */
  int spacing() const { return spacing_; }
  /**
     Sets the number of extra pixels of blank space that are added
     between the children.
  */
  void spacing(int i) { spacing_ = i; }
  /** Same as Fl_Group::type() */
  uchar horizontal() const { return type(); }
  void layout();
};

} // namespace mrv
