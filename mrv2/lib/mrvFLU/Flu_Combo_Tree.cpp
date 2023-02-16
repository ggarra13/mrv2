// $Id: Flu_Combo_Tree.cpp,v 1.5 2004/08/02 14:18:16 jbryan Exp $

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

#include "mrvFLU/Flu_Combo_Tree.h"

#include <FL/Fl.H>
#include <FL/Fl_Rect.H>
#include <FL/fl_draw.H>
#include <FL/math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <iostream>

Flu_Combo_Tree ::Flu_Combo_Tree(int X, int Y, int W, int H, const char *l)
    : Flu_Combo_Box(X, Y, W, H, l), tree(0, 0, 0, 0) {
  tree.callback(_cb, this);
  tree.when(FL_WHEN_RELEASE);
  tree.showroot(false);
  tree.showcollapse(false);
  tree.label("/"); // this does not work
  tree.item_labelfgcolor(fl_contrast(FL_WHITE, tree.item_labelbgcolor()));
  set_combo_widget(&tree);
}

Flu_Combo_Tree ::~Flu_Combo_Tree() {}

void Flu_Combo_Tree ::cb() {
  if (tree.callback_reason() == FL_TREE_REASON_SELECTED) {
    char path[2048];
    tree.item_pathname(path + 1, 2047, tree.callback_item());
    path[0] = '/';
    value(path);
  }
}

void Flu_Combo_Tree ::_hilight(int event, int x, int y) {
  Fl_Tree_Item *i;
  for (i = tree.first(); i; i = tree.next_item(i)) {
    if (i->is_root())
      continue;
    if (i->event_on_label(tree.prefs())) {
      tree.deselect_all();
      tree.select(i, 0);
      break;
    }
  }

  if (event == FL_PUSH && i) {
    std::string path = i->label();
    selected(path.c_str());
  }
}

bool Flu_Combo_Tree ::_value(const char *v) {
  // see if 'v' is in the tree, and if so, make it the current selection
  Fl_Tree_Item *n = tree.find_item(v);
  if (n) {
    tree.deselect_all();
    tree.select(n, 0);
    return true;
  }
  return false;
}

const char *Flu_Combo_Tree ::_next() {
  Fl_Tree_Item *n = tree.first_selected_item();
  if (n) {
    Fl_Tree_Item *n2 = tree.next_selected_item(n);
    if (n2) {
      n->select(false);
      n2->select(true);
      const char *path = n2->label();
      return (strlen(path) ? path : NULL);
    }
  }
  return NULL;
}

const char *Flu_Combo_Tree ::_previous() {
  Fl_Tree_Item *n = tree.last_selected_item();
  if (n) {
    Fl_Tree_Item *n2 = tree.next_selected_item(n, FL_Up);
    if (n2) {
      if (n2->is_root() && !tree.showroot())
        return NULL;
      n->select(false);
      n2->select(true);
      const char *path = n2->label();
      return (strlen(path) ? path : NULL);
    }
  }
  return NULL;
}
