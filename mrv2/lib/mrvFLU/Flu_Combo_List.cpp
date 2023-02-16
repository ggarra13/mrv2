// $Id: Flu_Combo_List.cpp,v 1.5 2004/03/24 02:49:13 jbryan Exp $

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

#include "mrvFLU/Flu_Combo_List.h"

#include <FL/Fl.H>
#include <FL/fl_draw.H>
#include <FL/math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Flu_Combo_List ::Flu_Combo_List(int X, int Y, int W, int H, const char *l)
    : Flu_Combo_Box(X, Y, W, H, l), list(0, 0, 0, 0) {
  list.box(FL_FLAT_BOX);
  list.textcolor(FL_BLACK);
  list.callback(_cb, this);
  set_combo_widget(&list);
}

Flu_Combo_List ::~Flu_Combo_List() {}

void Flu_Combo_List ::cb() {
  if (list.value())
    selected(list.text(list.value()));
  else
    _value(value());
}

void Flu_Combo_List ::_hilight(int event, int x, int y) {
  if (list.scrollbar.visible()) {
    if (x > list.x() && y > list.y() &&
        x < (list.x() + list.w() - list.scrollbar.w()) &&
        y < (list.y() + list.h()))
      list.handle(FL_DRAG);
  } else {
    if (x > list.x() && y > list.y() && x < (list.x() + list.w()) &&
        y < (list.y() + list.h()))
      list.handle(FL_DRAG);
  }
}

bool Flu_Combo_List ::_value(const char *v) {
  // see if 'v' is in the list, and if so, make it the current selection
  for (int i = 1; i <= list.size(); i++) {
    if (strcmp(list.text(i), v) == 0) {
      list.value(i);
      return true;
    }
  }
  return false;
}

const char *Flu_Combo_List ::_next() {
  int v = list.value();
  if (v < list.size())
    list.value(v + 1);
  list.middleline(list.value());
  return list.text(list.value());
}

const char *Flu_Combo_List ::_previous() {
  int v = list.value();
  if (v > 1)
    list.value(v - 1);
  list.middleline(list.value());
  return list.text(list.value());
}
