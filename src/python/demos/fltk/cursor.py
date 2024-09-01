# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.

#
# "$Id: cursor.py 35 2003-09-29 21:39:48Z andreasheld $"
#
# Cursor test program for pyFLTK the Python bindings
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


fg = FL_BLACK
bg = FL_WHITE
cursor = FL_CURSOR_DEFAULT

#Fl_Hor_Value_Slider *cursor_slider;
cursor_slider = ''

def choice_cb(ptr, value):
  global cursor
  global fg
  global bg
  cursor = value
  cursor_slider.value(cursor)
  fl_cursor(cursor, fg, bg)



def setcursor(slider):
  global cursor
  global fg
  global bg
  #slider = castWidget2Slider(ptr)
  cursor = int(slider.value())
  fl_cursor(cursor, fg, bg)

def setfg(slider):
  global cursor
  global fg
  global bg
  #slider = castWidget2Slider(ptr)
  fg = int(slider.value())
  fl_cursor(cursor, fg, bg)

def setbg(slider):
  global cursor
  global fg
  global bg
  #slider = castWidget2Slider(ptr)
  bg = int(slider.value())
  fl_cursor(cursor, fg, bg)


# draw the label without any ^C or \nnn conversions:
class CharBox(Fl_Box):
  def __init__(self, X, Y, W, H, L):
    #Super(X,Y,W,H,L)
    Fl_Box.__init__(self, X,Y,W,H,L)
  def draw():
    fl_font(FL_FREE_FONT,14)
    fl_draw(label(), x()+w()/2, y()+h()/2)
 


window = Fl_Window(400,300)

choice = Fl_Choice(80,100,200,25,"Cursor:")
#choice.menu(choices)
choice.copy(
(( ("FL_CURSOR_DEFAULT",0,choice_cb,FL_CURSOR_DEFAULT),
  ("FL_CURSOR_ARROW",0,choice_cb,FL_CURSOR_ARROW),
  ("FL_CURSOR_CROSS",0,choice_cb,FL_CURSOR_CROSS),
  ("FL_CURSOR_WAIT",0,choice_cb,FL_CURSOR_WAIT),
  ("FL_CURSOR_INSERT",0,choice_cb,FL_CURSOR_INSERT),
  ("FL_CURSOR_HAND",0,choice_cb,FL_CURSOR_HAND),
  ("FL_CURSOR_HELP",0,choice_cb,FL_CURSOR_HELP),
  ("FL_CURSOR_MOVE",0,choice_cb,FL_CURSOR_MOVE),
  ("FL_CURSOR_NS",0,choice_cb,FL_CURSOR_NS),
  ("FL_CURSOR_WE",0,choice_cb,FL_CURSOR_WE),
  ("FL_CURSOR_NWSE",0,choice_cb,FL_CURSOR_NWSE),
  ("FL_CURSOR_NESW",0,choice_cb,FL_CURSOR_NESW),
  ("FL_CURSOR_NONE",0,choice_cb,FL_CURSOR_NONE),
  (None,) )  ))
choice.callback(choice_cb)
#choice.when(FL_WHEN_RELEASE|FL_WHEN_NOT_CHANGED)

slider1 = Fl_Hor_Value_Slider(80,180,310,30,"Cursor:")
cursor_slider = slider1
slider1.align(FL_ALIGN_LEFT)
slider1.step(1)
slider1.precision(0)
slider1.bounds(0,100)
slider1.value(0)
slider1.callback(setcursor)
slider1.value(cursor);

slider2 = Fl_Hor_Value_Slider(80,220,310,30,"fgcolor:")
slider2.align(FL_ALIGN_LEFT)
slider2.step(1)
slider2.precision(0)
slider2.bounds(0,255)
slider2.value(0)
slider2.callback(setfg)
slider2.value(fg)

slider3 = Fl_Hor_Value_Slider(80,260,310,30,"bgcolor:")
slider3.align(FL_ALIGN_LEFT)
slider3.step(1)
slider3.precision(0)
slider3.bounds(0,255)
slider3.value(0)
slider3.callback(setbg)
slider3.value(bg)

window.resizable(window)
window.end()
window.show()