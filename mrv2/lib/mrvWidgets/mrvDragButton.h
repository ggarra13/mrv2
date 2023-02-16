// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <FL/Fl_Box.H>

namespace mrv {

class DragButton : public Fl_Box {
private:
  int x1, y1;     // click posn., used for dragging and docking checks
  int xoff, yoff; // origin used for dragging calcs
  int was_docked; // used in handle to note that we have just undocked

protected:
  // override handle method to catch drag/dock operations
  int handle(int event) override;

public:
  // basic constructor
  DragButton(int x, int y, int w, int h, const char *l = 0);
};

} // namespace mrv
