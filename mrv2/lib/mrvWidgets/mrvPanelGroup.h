// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <iostream>

/* fltk includes */
#include <FL/Fl_Button.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Scroll.H>

#include "mrvDockGroup.h"
#include "mrvDragButton.h"
#include "mrvPack.h"
#include "mrvPanelWindow.h"

namespace mrv {

class PanelGroup : public Fl_Group {
private:
  // control variables
  bool _docked;
  DockGroup *dock;

  // constructor helper function
  void create_dockable_group(const char *lbl);
  void create_docked(DockGroup *d, const char *lbl);
  void create_floating(DockGroup *d, int state, int x, int y, int w, int h,
                       const char *l);

protected:
  // Widgets used by the toolbar
  Fl_Button *dismiss;
  DragButton *dragger;
  Fl_Button *docker;
  PanelWindow *tw;
  Fl_Scroll *scroll;
  Pack *pack;
  Fl_Group *group;

  // Sets whether window is docked or not.
  void docked(bool r);

  // Defines which dock the group can dock into
  inline void set_dock(DockGroup *w) { dock = w; }
  // get the dock group ID
  inline DockGroup *get_dock(void) { return dock; }

public:
  // Constructors for docked/floating window
  PanelGroup(DockGroup *d, int f, int x, int y, int w, int h,
             const char *l = 0);
  // Debug element sizes
  void debug(const char *text) const;

  // Get the toolwindow or null if docked
  DragButton *get_dragger() const { return dragger; }
  Fl_Group *get_group() const { return group; }
  Pack *get_pack() const { return pack; }
  Fl_Scroll *get_scroll() const { return scroll; }
  PanelWindow *get_window() const { return tw; }

  Fl_Image *image() const { return dragger->image(); }
  void image(Fl_Image *img) { dragger->image(img); }

  // Recalculate the sizes
  void layout();

  // methods for hiding/showing *all* the floating windows
  static void show_all(void);
  static void hide_all(void);

  // Tests whether window is docked or not.
  bool docked() { return _docked; }

  // generic callback function for the dock/undock checkbox
  void dock_grp(void *v);
  void undock_grp(void *v);

  // generic callback function for the dismiss button
  static void cb_dismiss(Fl_Button *, void *v);

  inline void callback(Fl_Callback *c, void *d) { dismiss->callback(c, d); }

  inline const char *label() const { return dragger->label(); }
  // wrap some basic Fl_Group functions to access the enclosed pack
  inline void clear() { pack->clear(); }
  inline void begin() { pack->begin(); }
  void end();
  void resize(int X, int Y, int W, int H);
  inline void resizable(Fl_Widget *box) { pack->resizable(box); }
  inline void resizable(Fl_Widget &box) { pack->resizable(box); }
  inline Fl_Widget *resizable() const { return pack->resizable(); }
  inline void add(Fl_Widget *w) { pack->add(w); }
  inline void add(Fl_Widget &w) { add(&w); }
  inline void insert(Fl_Widget &w, int n) { pack->insert(w, n); }
  inline void insert(Fl_Widget &w, Fl_Widget *beforethis) {
    pack->insert(w, beforethis);
  }
  inline void remove(Fl_Widget &w) { pack->remove(w); }
  inline void remove(Fl_Widget *w) { pack->remove(w); }
  inline int children() const { return pack->children(); }
};

} // namespace mrv
