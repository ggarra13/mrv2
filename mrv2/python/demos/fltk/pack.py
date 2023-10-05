#
# "$Id: pack.py 495 2013-03-30 09:39:45Z andreasheld $"
#
# Pack test program for pyFLTK the Python bindings
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

w = Fl_Window(100, 100, 365, 525)

scroll = Fl_Scroll(10,10,345,285)

pack = Fl_Pack(10, 10, 345, 285)
pack.box(FL_DOWN_FRAME)


def type_cbH(ptr):
    type_cb(Fl_Pack.HORIZONTAL)

def type_cbV(ptr):
    type_cb(Fl_Pack.VERTICAL)

def type_cb(v):
    for i in range(0,pack.children(),1):
        pack.child(i).resize(0,0,25,25)
    pack.resize(scroll.x(), scroll.y(), scroll.w(), scroll.h())
    p = pack.parent()
    p.redraw()
    pack.type(v)
    pack.redraw()

def spacing_cb(ptr):
    pack.spacing(int(valueSlider.value()))
    scroll.redraw()

buttons=[]
buttons.append( Fl_Button(35, 35, 25, 25, "b1"))
buttons.append( Fl_Button(45, 45, 25, 25, "b2"))
buttons.append( Fl_Button(55, 55, 25, 25, "b3"))
buttons.append( Fl_Button(65, 65, 25, 25, "b4"))
buttons.append( Fl_Button(75, 75, 25, 25, "b5"))
buttons.append( Fl_Button(85, 85, 25, 25, "b6"))
buttons.append( Fl_Button(95, 95, 25, 25, "b7"))
buttons.append( Fl_Button(105, 105, 25, 25, "b8"))
buttons.append( Fl_Button(115, 115, 25, 25, "b9"))
buttons.append( Fl_Button(125, 125, 25, 25, "b10"))
buttons.append( Fl_Button(135, 135, 25, 25, "b11"))
buttons.append( Fl_Button(145, 145, 25, 25, "b12"))
buttons.append( Fl_Button(155, 155, 25, 25, "b13"))
buttons.append( Fl_Button(165, 165, 25, 25, "b14"))
buttons.append( Fl_Button(175, 175, 25, 25, "b15"))
buttons.append( Fl_Button(185, 185, 25, 25, "b16"))
buttons.append( Fl_Button(195, 195, 25, 25, "b17"))
buttons.append( Fl_Button(205, 205, 25, 25, "b18"))
buttons.append( Fl_Button(215, 215, 25, 25, "b19"))
buttons.append( Fl_Button(225, 225, 25, 25, "b20"))
buttons.append( Fl_Button(235, 235, 25, 25, "b21"))
buttons.append( Fl_Button(245, 245, 25, 25, "b22"))
buttons.append( Fl_Button(255, 255, 25, 25, "b23"))
buttons.append( Fl_Button(265, 265, 25, 25, "b24"))
w.end()
w.resizable(pack)

scroll.end()
o = ( Fl_Light_Button(10, 325, 175, 25, "VERTICAL"))
buttons.append(o)
o.type(FL_RADIO_BUTTON)
o.callback(type_cbH)

o = ( Fl_Light_Button(10, 350, 175, 25, "HORIZONTAL"))
buttons.append( o )
o.type(FL_RADIO_BUTTON)
o.value(1)
o.callback(type_cbV)

o = ( Fl_Value_Slider(50,375, 295,25,"spacing:"))
valueSlider = o
buttons.append( o )
o.align(FL_ALIGN_LEFT)
o.type(FL_HORIZONTAL)
o.range(0,30)
o.step(1)
o.callback(spacing_cb)

w.end()
w.show(len(sys.argv), sys.argv)
Fl.run()


