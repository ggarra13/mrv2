# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.

#
# "$Id: pixmap.py 247 2006-08-22 05:54:01Z andreasheld $"
#
# Pixmap test program for pyFLTK the Python bindings
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

porsche_xpm=[
"64 64 4 1",
" 	c #background",
".	c #000000000000",
"X	c #ffd100",
"o	c #FFFF00000000",
"                                                                ",
"                   ..........................                   ",
"              .....................................             ",
"        ............XXXXXXXXXXXXXXXXXXXXXXXX............        ",
"        ......XXXXXXX...XX...XXXXXXXX...XXXXXXXXXX......        ",
"        ..XXXXXXXXXX..X..XX..XXXX.XXXX..XXXXXXXXXXXXXX..        ",
"        ..XXXXXXXXXX..X..XX..XXX..XXXX..X...XXXXXXXXXX..        ",
"        ..XXXXXXXXXX..XXXXX..XX.....XX..XX.XXXXXXXXXXX..        ",
"        ..XXXXXXXXX.....XXX..XXX..XXXX..X.XXXXXXXXXXXX..        ",
"        ..XXXXXXXXXX..XXXXX..XXX..XXXX....XXXXXXXXXXXX..        ",
"        ..XXXXXXXXXX..XXXXX..XXX..XXXX..X..XXXXXXXXXXX..        ",
"        ..XXXXXXXXXX..XXXXX..XXX..X.XX..XX..XXXXXXXXXX..        ",
"        ..XXXXXXXXX....XXX....XXX..XX....XX..XXXXXXXXX..        ",
"        ..XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX..        ",
"        ..XXXXXXXXX..........................XXXXXXXXX..        ",
"        ..XXX.......XXXXXXXXXXX...................XXXX..        ",
"        ......XX.XXX.XXX..XXXXX.........................        ",
"        ..XXXXX.XXX.XXX.XXXX.XX.........................        ",
"        ..XXXX.XXX.XX.......XXX.........................        ",
"        ..XXXX.......XXXXXX..XX..ooooooooooooooooooooo..        ",
"        ..X.....XXXXXXXXXXXXXXX..ooooooooooooooooooooo..        ",
"        ..X...XXXXXXXXXXXXXXXXX..ooooooooooooooooooooo..        ",
"        ..X..XXXXXXX.XX.XXXXXXX..ooooooooooooooooooooo..        ",
"        ..XXXXX.XXX.XX.XXXXXXXX..ooooooooooooooooooooo..        ",
"        ..XXXX.XXX.XX.XX................................        ",
"        ..XXXX.X.........X....X.X.X.....................        ",
"        ..XXXX...XXXXXXX.X..X...X.X.X.X.................        ",
"        ..X....XXXXXXXXXX.X...X.X.X.....................        ",
"        ..X...XXXXXXXXXX.XXXXXXXXXXXXXX.................        ",
"        ..X..XXXXXX.XX.X.XXX...XXXXXXXX.................        ",
"        ..XXXXX.XX.XX.XX.XX.....XXXXXXX.oooooooooooooo..        ",
"        ..XXXX.XX.XX.XX..XX.X...XXXXX.X.oooooooooooooo..        ",
"        ..XXXX.X.......X.XXXX...XXXX..X.oooooooooooooo..        ",
"        ..X......XXXXXX..XXXX...XXXX..X.oooooooooooooo..        ",
"        ..X...XXXXXXXXXX.XXX.....XXX.XX.oooooooooooooo..        ",
"        ..X..XXXXXXXXXXX.X...........XX.oooooooooooooo..        ",
"        .................X.X.........XX.................        ",
"        .................X.X.XXXX....XX.XXXXXXXXXXXXXX..        ",
"        .................XXX.XXXXX.X.XX.XXX.XX.XXXXXXX..        ",
"         ................XXXX.XXX..X..X.XX.XX.XXX.XXX..         ",
"         ................XXXXXXXX.XX.XX.X.XX.XXX.XXXX..         ",
"         .................XXXXXX.XX.XX.X..........XXX..         ",
"          ..oooooooooooooo.XXXXXXXXXX....XXXXXXXX..X..          ",
"          ..ooooooooooooooo.XXXXXXXX....XXXXXXXXXXXX..          ",
"           ..ooooooooooooooo........XXXXXXX.XX.XXXX..           ",
"           ..oooooooooooooooooo..XXXXX.XXX.XX.XX.XX..           ",
"            ..ooooooooooooooooo..XXXX.XXX.XX.XX.XX..            ",
"            ..ooooooooooooooooo..XXX.XX........XXX..            ",
"             ....................XXX....XXXXXX..X..             ",
"              ...................XX...XXXXXXXXXXX.              ",
"              ...................X...XXXXXXXXXXX..              ",
"               ..................X..XXXX.XXXXXX..               ",
"                .................XXX.XX.XX.XXX..                ",
"                 ................XX.XX.XX.XXX..                 ",
"                  ..ooooooooooo..XX.......XX..                  ",
"                   ..oooooooooo..X...XXXX.X..                   ",
"                    ..ooooooooo..X..XXXXXX..                    ",
"                     ...ooooooo..X..XXXX...                     ",
"                      ....ooooo..XXXXX....                      ",
"                        ....ooo..XXX....                        ",
"                          ....o..X....                          ",
"                            ........                            ",
"                              ....                              ",
"                                                                "]

fl_pixmap = Fl_Pixmap(porsche_xpm)


leftb = None
rightb = None
topb = None
bottomb = None
insideb = None
b = None
w = None

def button_cb(ptr):
  i = 0
  if leftb.value():
    i = i + FL_ALIGN_LEFT
  if rightb.value():
    i = i + FL_ALIGN_RIGHT
  if topb.value():
    i = i + FL_ALIGN_TOP
  if bottomb.value():
    i = i + FL_ALIGN_BOTTOM
  if insideb.value():
    i = i + FL_ALIGN_INSIDE
  b.align(i)
  w.redraw()


w = window = Fl_Window(100, 100, 400,400) 

b = Fl_Button(140,160,120,120,"")
b.image(fl_pixmap)

leftb = Fl_Toggle_Button(50,75,50,25,"left")
leftb.callback(button_cb)

rightb = Fl_Toggle_Button(100,75,50,25,"right")
rightb.callback(button_cb)

topb = Fl_Toggle_Button(150,75,50,25,"top")
topb.callback(button_cb)

bottomb = Fl_Toggle_Button(200,75,50,25,"bottom")
bottomb.callback(button_cb)

insideb = Fl_Toggle_Button(250,75,50,25,"inside")
insideb.callback(button_cb)

window.resizable(window.this)
window.end()
window.show()

