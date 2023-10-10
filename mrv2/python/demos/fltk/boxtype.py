#
# "$Id: boxtype.py 515 2016-12-29 09:48:42Z andreasheld $"
#
# Boxtype test program for pyFLTK the Python bindings
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

Nt = 0
W =  200
H = 50
ROWS = 19

window = None
buttons = []

def bt(name, type, square=0):
    global Nt
    x = Nt % 4
    y = int(Nt / 4)
    Nt = Nt + 1
    x = x*W+10
    y = y*H+10
    temp = W-20
    if (square):
        temp = H - 20
    b = Fl_Box(x,y,temp,H-20,name)
    b.box(type)
    b.labelsize(11)
    if (square):
        b.align(FL_ALIGN_RIGHT)
    buttons.append(b)


window = Fl_Single_Window(0,0,4*W,ROWS*H)
#window = Fl_Window(5*W, ROWS*H)
window.box(FL_FLAT_BOX)
window.color(12) #light blue
bt("FL_NO_BOX",FL_NO_BOX)
bt("FL_FLAT_BOX",FL_FLAT_BOX)
Nt = Nt + 2 # go to start of next row to line up boxes & frames
bt("FL_UP_BOX",FL_UP_BOX)
bt("FL_DOWN_BOX",FL_DOWN_BOX)
bt("FL_UP_FRAME",FL_UP_FRAME)
bt("FL_DOWN_FRAME",FL_DOWN_FRAME)
bt("FL_THIN_UP_BOX",FL_THIN_UP_BOX)
bt("FL_THIN_DOWN_BOX",FL_THIN_DOWN_BOX)
bt("FL_THIN_UP_FRAME",FL_THIN_UP_FRAME)
bt("FL_THIN_DOWN_FRAME",FL_THIN_DOWN_FRAME)
bt("FL_ENGRAVED_BOX",FL_ENGRAVED_BOX)
bt("FL_EMBOSSED_BOX",FL_EMBOSSED_BOX)
bt("FL_ENGRAVED_FRAME",FL_ENGRAVED_FRAME)
bt("FL_EMBOSSED_FRAME",FL_EMBOSSED_FRAME)
bt("FL_BORDER_BOX",FL_BORDER_BOX)
bt("FL_SHADOW_BOX",FL_SHADOW_BOX)
bt("FL_BORDER_FRAME",FL_BORDER_FRAME)
bt("FL_SHADOW_FRAME",FL_SHADOW_FRAME)
bt("FL_ROUNDED_BOX",FL_ROUNDED_BOX)
bt("FL_RSHADOW_BOX",FL_RSHADOW_BOX)
bt("FL_ROUNDED_FRAME",FL_ROUNDED_FRAME)
bt("FL_RFLAT_BOX",FL_RFLAT_BOX)
bt("FL_OVAL_BOX",FL_OVAL_BOX)
bt("FL_OSHADOW_BOX",FL_OSHADOW_BOX)
bt("FL_OVAL_FRAME",FL_OVAL_FRAME)
bt("FL_OFLAT_BOX",FL_OFLAT_BOX)
bt("FL_ROUND_UP_BOX",FL_ROUND_UP_BOX)
bt("FL_ROUND_DOWN_BOX",FL_ROUND_DOWN_BOX)
bt("FL_DIAMOND_UP_BOX",FL_DIAMOND_UP_BOX)
bt("FL_DIAMOND_DOWN_BOX",FL_DIAMOND_DOWN_BOX)

bt("FL_PLASTIC_UP_BOX",FL_PLASTIC_UP_BOX)
bt("FL_PLASTIC_DOWN_BOX",FL_PLASTIC_DOWN_BOX)
bt("FL_PLASTIC_UP_FRAME",FL_PLASTIC_UP_FRAME)
bt("FL_PLASTIC_DOWN_FRAME",FL_PLASTIC_DOWN_FRAME)
bt("FL_PLASTIC_THIN_UP_BOX",FL_PLASTIC_THIN_UP_BOX)
bt("FL_PLASTIC_THIN_DOWN_BOX",FL_PLASTIC_THIN_DOWN_BOX)
Nt = Nt+2
bt("FL_PLASTIC_ROUND_UP_BOX",FL_PLASTIC_ROUND_UP_BOX)
bt("FL_PLASTIC_ROUND_DOWN_BOX",FL_PLASTIC_ROUND_DOWN_BOX)
Nt = Nt+2

bt("FL_GTK_UP_BOX",FL_GTK_UP_BOX)
bt("FL_GTK_DOWN_BOX",FL_GTK_DOWN_BOX)
bt("FL_GTK_UP_FRAME",FL_GTK_UP_FRAME)
bt("FL_GTK_DOWN_FRAME",FL_GTK_DOWN_FRAME)
bt("FL_GTK_THIN_UP_BOX",FL_GTK_THIN_UP_BOX)
bt("FL_GTK_THIN_DOWN_BOX",FL_GTK_THIN_DOWN_BOX)
bt("FL_GTK_THIN_UP_FRAME",FL_GTK_THIN_UP_FRAME)
bt("FL_GTK_THIN_DOWN_FRAME",FL_GTK_THIN_DOWN_FRAME)
bt("FL_GTK_ROUND_UP_BOX",FL_GTK_ROUND_UP_BOX)
bt("FL_GTK_ROUND_DOWN_BOX",FL_GTK_ROUND_DOWN_BOX)
Nt = Nt+2
bt("FL_GLEAM_UP_BOX",FL_GLEAM_UP_BOX)
bt("FL_GLEAM_DOWN_BOX",FL_GLEAM_DOWN_BOX)
bt("FL_GLEAM_UP_FRAME",FL_GLEAM_UP_FRAME)
bt("FL_GLEAM_DOWN_FRAME",FL_GLEAM_DOWN_FRAME)
bt("FL_GLEAM_THIN_UP_BOX",FL_GLEAM_THIN_UP_BOX)
bt("FL_GLEAM_THIN_DOWN_BOX",FL_GLEAM_THIN_DOWN_BOX)
bt("FL_GLEAM_ROUND_UP_BOX",FL_GLEAM_ROUND_UP_BOX)
bt("FL_GLEAM_ROUND_DOWN_BOX",FL_GLEAM_ROUND_DOWN_BOX)

bt("FL_OXY_UP_BOX",FL_OXY_UP_BOX)
bt("FL_OXY_DOWN_BOX",FL_OXY_DOWN_BOX)
bt("FL_OXY_UP_FRAME",FL_OXY_UP_FRAME)
bt("FL_OXY_DOWN_FRAME",FL_OXY_DOWN_FRAME)
bt("FL_OXY_THIN_UP_BOX",FL_OXY_THIN_UP_BOX)
bt("FL_OXY_THIN_DOWN_BOX",FL_OXY_THIN_DOWN_BOX)
bt("FL_OXY_ROUND_UP_BOX",FL_OXY_ROUND_UP_BOX)
bt("FL_OXY_ROUND_DOWN_BOX",FL_OXY_ROUND_DOWN_BOX)
bt("FL_OXY_BUTTON_UP_BOX",FL_OXY_BUTTON_UP_BOX)
bt("FL_OXY_BUTTON_DOWN_BOX",FL_OXY_BUTTON_DOWN_BOX)

window.end()

window.resizable(window)

window.show()

