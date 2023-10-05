#
# "$Id: tile.py 172 2006-01-26 07:45:15Z andreasheld $"
#
# Tiling test program for pyFLTK the Python bindings
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

window = Fl_Window(300, 300)
window.box(FL_NO_BOX)
window.resizable(window)
tile = Fl_Tile(0,0,300,300)
box0 = Fl_Box(0,0,150,150,"0")
box0.box(FL_DOWN_BOX)
box0.color(9)
box0.labelsize(36)
box0.align(FL_ALIGN_CLIP)

w1 = Fl_Window(150,0,150,150,"1")
w1.box(FL_NO_BOX)

box1 = Fl_Box(0,0,150,150,"1\nThis is a\nchild\nwindow")
box1.box(FL_DOWN_BOX)
box1.color(19)
box1.labelsize(18)
box1.align(FL_ALIGN_CLIP)
w1.resizable(box1)
w1.end()

box2a = Fl_Box(0,150,70,150,"2a");
box2a.box(FL_DOWN_BOX);
box2a.color(12);
box2a.labelsize(36)
box2a.align(FL_ALIGN_CLIP)
box2b = Fl_Box(70,150,80,150,"2b")
box2b.box(FL_DOWN_BOX)
box2b.color(13)
box2b.labelsize(36)
box2b.align(FL_ALIGN_CLIP)

box3a = Fl_Box(150,150,150,70,"3a")
box3a.box(FL_DOWN_BOX)
box3a.color(12)
box3a.labelsize(36)
box3a.align(FL_ALIGN_CLIP)
box3b = Fl_Box(150,150+70,150,80,"3b")
box3b.box(FL_DOWN_BOX)
box3b.color(13)
box3b.labelsize(36)
box3b.align(FL_ALIGN_CLIP)

r = Fl_Box(10,0,300-10,300-10)
tile.resizable(r)

tile.end()
window.end()
window.show(len(sys.argv), sys.argv)
w1.show()
Fl.run()
