# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.

#
# "$Id: clock.py 103 2005-06-24 13:55:26Z andreasheld $"
#
# Clock test program for pyFLTK the Python bindings
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

window = Fl_Double_Window(220,220,"Fl_Clock")

c1 = Fl_Clock(0,0,220,220,"test")
window.resizable(c1);
window.end();
window2 = Fl_Double_Window(220,220,"Fl_Round_Clock")

c2 = Fl_Round_Clock(0,0,220,220)
window2.resizable(c2);
window2.end();
#  // my machine had a clock* Xresource set for another program, so
#  // I don't want the class to be "clock":
#  window.xclass("Fl_Clock")
#  window2.xclass("Fl_Clock")
window.show()
window2.show()

