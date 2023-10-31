# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.

#
# "$Id: buttons.py 28 2003-07-16 20:00:27Z andreasheld $"
#
# Buttons test program for pyFLTK the Python bindings
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


window = Fl_Window(320,130)
b1 = Fl_Button(10, 10, 130, 30, "Fl_Button")
b1.tooltip("This is a Tooltip.")
b2 = Fl_Return_Button(150, 10, 160, 30, "Fl_Return_Button")
b3 = Fl_Repeat_Button(10,50,130,30,"Fl_Repeat_Button")
b4 = Fl_Light_Button(10,90,130,30,"Fl_Light_Button")
b5 = Fl_Round_Button(150,50,160,30,"Fl_Round_Button")
b6 = Fl_Check_Button(150,90,160,30,"Fl_Check_Button")
window.end()
window.show()

