#
# "$Id: logo.py 276 2007-11-26 11:01:40Z andreasheld $"
#
# GIF_Image test program for pyFLTK the Python bindings
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
from string import *

def theCancelButtonCallback(ptr, data):
	print("type = ", type(ptr))
	print(f"theCancelButtonCallback({str(data)})")
	print("Tooltip: ", ptr.tooltip())

def createButton(x, y, w, h, label):
	b = Fl_Light_Button(x,y,w,h,label)
	b.selection_color(FL_GREEN)
	b.box(FL_NO_BOX)
	b.set()
	return b

window = Fl_Window(0, 0, 414, 310)
window.label("Python Wrappers for FLTK ")

box = Fl_Button(0,0,414,176)
img = Fl_GIF_Image("./logo1.gif")
box.image(img)
b = Fl_Button(window.w()-20-120, 190, 120, 25, "Start using it!")

b1 = createButton(20, 190, 120, 25, "Fast!")
b2 = createButton(20, 220, 120, 25, "Lightweight!")
b3 = createButton(20, 250, 120, 25, "Cross-platform!")
b4 = createButton(20, 280, 120, 25, "Easy to use!")

window.end()
window.show()

Fl.scheme("gleam")