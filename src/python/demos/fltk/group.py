# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.

#
# "$Id: group.py 495 2013-03-30 09:39:45Z andreasheld $"
#
# Group test program for pyFLTK the Python bindings
# for the Fast Light Tool Kit (FLTK).
#
# FLTK copyright 1998-1999 by Bill Spitzak and others.
# pyFLTK copyright 2003 by Andreas Held and others.
#
# This library is free software you can redistribute it and/or
# modify it under the terms of the GNU Library General Public
# License, version 2.0 as published by the Free Software Foundation.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Library General Public License for more details.
#
# You should have received a copy of the GNU Library General Public
# License along with this library if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
# USA.
#
# Please report all bugs and problems to "pyfltk-user@lists.sourceforge.net".
#

from fltk14 import *
import sys

removed = 0
window = None
grp = None
btn = None


def btn_cb(ptr):
    global window
    global grp
    global removed
    if removed == 0:
        print("removing button")
        grp.remove(btn)
        removed = 1
    else:
        print("adding button")
        grp.add(btn)
        removed = 0
    window.redraw()

window = Fl_Window(320,130)

grp = Fl_Group(10, 10, 300,110)
b1 = Fl_Button(10, 10, 130, 30, "Add/Remove")
b1.tooltip("Pressing this button should remove the second button, pressing it again should add the other button.")
b1.callback(btn_cb)
grp.end()
window.end()
removed = 1
btn = Fl_Return_Button(150, 10, 160, 30, "Fl_Return_Button")

window.show()
