# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.

#
# "$Id: adjuster.py 35 2003-09-29 21:39:48Z andreasheld $"
#
# Adjuster test program for pyFLTK the Python bindings
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
import time

label = ""

def adjcb(ptr, widget):
	global label # need to keep this global to avoid garbage collection
	label = widget.label()
	ret,label = ptr.format(label)
	widget.label(label)
	widget.redraw()
	return None

window = Fl_Window(320,100)
buf1 = '0.0000'
b1 = Fl_Box(FL_DOWN_BOX,20,30,80,25,buf1)
b1.color(FL_WHITE)
a1 = Fl_Adjuster(20+80,30,3*25,25)
a1.callback(adjcb,b1)

buf2 = '0.0000'
b2 = Fl_Box(FL_DOWN_BOX,20+80+4*25,30,80,25,buf2)
b2.color(FL_WHITE)
a2 = Fl_Adjuster(b2.x()+b2.w(),10,25,3*25)
a2.callback(adjcb,b2)

window.resizable(window)
window.end()
window.show()


