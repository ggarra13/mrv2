# Standard dialog test program for the Python port of
# the Fast Light Tool Kit (pyFLTK).
#
# Demonstrates how to use readqueue to see if a button has been
# pushed, and to see if a window has been closed, thus avoiding
# the need to define callbacks.
#
# This also demonstrates how to trap attempts by the user to
# close the last window by overriding Fl::exit
#
# FLTk is Copyright 1998-2003 by Bill Spitzak and others.
# pyFLTK copyright 2003 by Andreas Held and others.
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Library General Public
# License, version 2.0 as published by the Free Software Foundation.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Library General Public License for more details.
#
# You should have received a copy of the GNU Library General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
# USA.
#
# Please report all bugs and problems to "pyfltk-user@lists.sourceforge.net".
#


from fltk14 import *
import sys

buffer = None

def rename_me(o):
    global buffer
    buffer = o.label()
    input = fl_input("Input:", buffer)
    if input != None:
       buffer = input
    o.label(buffer)
    o.redraw()

def window_callback(ptr):
    #status = fl_ask("Are you sure you want to quit?")
    status = fl_choice("Are you sure you want to quit?", "No", "Yes", None)
    if status == 1:
       return sys.exit(0)


# main function
buffer = "test text"

window = Fl_Window(200, 55)

#b = Fl_Return_Button(20, 10, 160, 35, buffer)
b = Fl_Return_Button(20, 10, 160, 35, "test text")
b.callback(rename_me)
window.add(b)
window.resizable(b)

window.callback(window_callback)

window.show()




