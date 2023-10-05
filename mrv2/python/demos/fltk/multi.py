#
# "$Id: multi.py 28 2003-07-16 20:00:27Z andreasheld $"
#
# Multiline input test program for pyFLTK the Python bindings
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

def btn_cb(ptr):
	import sys
	sys.exit(0)

window = Fl_Window(100, 100, 200, 220, "Inputs")

input = Fl_Multiline_Input(10, 10, 180, 90)

button = Fl_Button(80, 110, 40, 20, "Exit")
button.callback(btn_cb)

input.value("Here is a\nMultiline input.\nBelow are the Secret\nInt, and Float\ninput widgets.")

input2 = Fl_Secret_Input(10, 140, 180, 20)
input2.value("secret")

input3 = Fl_Int_Input(10, 165, 180, 20)
input3.value("12345")

input4 = Fl_Float_Input(10, 190, 180, 20)
input4.value("12.44")

window.end()
window.show()

Fl.run()

