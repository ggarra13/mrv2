#
# "$Id: sliders.py 164 2006-01-17 09:56:47Z andreasheld $"
#
# Slider test program for pyFLTK the Python bindings
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

window = Fl_Window(100, 100, 600, 470)

def cb_OK(ptr):
	sys.exit(0)


horizPack = Fl_Pack( 0, 0, 600, 200)
horizPack.type(FL_HORIZONTAL)
horizPack.spacing(80)
horizPack.children = []
vsh = 100
vsw = 30
horizPack.children.append(Fl_Fill_Slider( 0, 0, vsw, vsh, "Fl_Fill_Slider"))
horizPack.children.append(Fl_Nice_Slider( 0, 0, vsw, vsh,"Fl_Nice_Slider"))
horizPack.children.append(Fl_Slider( 0, 0, vsw, vsh,"Fl_Slider"))
horizPack.children.append(Fl_Value_Slider( 0, 0, vsw, vsh,"Fl_Value_Slider"))
horizPack.end()

vertPack = Fl_Pack( 0, 250, 600, 200)
vertPack.type(FL_VERTICAL)
vertPack.spacing(30)
vertPack.children = []
vertPack.children.append(Fl_Hor_Fill_Slider( 10, 10, 10, 20, "Fl_Hor_Fill_Slider"))
vertPack.children.append(Fl_Hor_Nice_Slider( 10, 10, 10, 20,"Fl_Hor_Nice_Slider"))
vertPack.children.append(Fl_Hor_Slider( 10, 10, 10, 20,"Fl_Hor_Slider"))
vertPack.children.append(Fl_Hor_Value_Slider( 10, 10, 10, 30,"Fl_Hor_Value_Slider"))
vertPack.end()

def hcb(slider):
	v = slider.value()
	for child in vertPack.children + horizPack.children:
		child.value(v)	

for child in vertPack.children + horizPack.children:
	child.callback(hcb)


okButton = Fl_Return_Button( 520, 30, 60, 30, "OK")
okButton.callback(cb_OK)

window.end()
window.show()









