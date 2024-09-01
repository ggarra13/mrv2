# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.

#
# "$Id: iconize.py 35 2003-09-29 21:39:48Z andreasheld $"
#
# Iconize test program for pyFLTK the Python bindings
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


def iconize_cb(ptr, win):
	win.iconize()

def show_cb(ptr, win):
	win.show()

def hide_cb(ptr, win):
	win.hide()

def window_cb(ptr):
  	sys.exit(0)




mainw = Fl_Window(200,200)
mainw.end()
mainw.show()

control = Fl_Window(120,120)

hide_button = Fl_Button(0,0,120,30,"hide()")
hide_button.callback(hide_cb, mainw);

iconize_button = Fl_Button(0,30,120,30,"iconize()")
iconize_button.callback(iconize_cb, mainw)

show_button = Fl_Button(0,60,120,30,"show()")
show_button.callback(show_cb, mainw)

show_button2 = Fl_Button(0,90,120,30,"show this")
show_button2.callback(show_cb, control)

#  Fl_Box box(FL_NO_BOX,0,60,120,30,"Also try running\nwith -i switch");

control.end()
control.show()
control.callback(window_cb)


