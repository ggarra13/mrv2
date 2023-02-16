// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include "mrvWidgets/mrvChoice.h"

namespace mrv {

Choice::Choice(int x, int y, int w, int h, const char *l)
    : Fl_Choice(x, y, w, h, l) {}

unsigned Choice::children() { return size(); }

Fl_Menu_Item *Choice::child(int idx) { return (Fl_Menu_Item *)&menu()[idx]; }

} // namespace mrv
