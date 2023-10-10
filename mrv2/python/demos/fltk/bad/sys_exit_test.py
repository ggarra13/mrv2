#!/usr/bin/python3
# "$Id: demos.py 531 2019-12-27 12:15:45Z andreasheld $"
#
# Test program for pyFLTK the Python bindings
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
# Please report all bugs and problems to "pyfltk14-user@lists.sourceforge.net".
#


import sys, os
import fltk14

class mybutton(fltk14.Fl_Button):
    def __init__(self, x,y,w,h,label):
        super().__init__(x,y,w,h, label)
        self.callback(self.foo)
        self.this.disown()

    def foo(self, wid):
        print("Button pressed: exit!")
        sys.exit()

class subwindow(fltk14.Fl_Gl_Window):
    global window
    def __init__(self, x,y,w,h):
        super().__init__(x,y,w,h, "")

    def draw(self):
        pass

    def handle(self, event):
        if event == fltk14.FL_PUSH:
            print("Push event detected: exit!")
            sys.exit()

            
        return super().handle(event)

window = fltk14.Fl_Double_Window(0, 0, 800, 600, "xxx")

sw = subwindow(0,0,800,500)
window.resizable(sw)

mb = mybutton(350,520,100,60, "Exit")

window.end()
window.show()

fltk14.

