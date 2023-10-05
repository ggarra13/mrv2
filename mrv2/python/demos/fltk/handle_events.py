#
# "$Id: handle_events.py 536 2020-10-30 15:20:32Z andreasheld $"
#
# Event test program for pyFLTK the Python bindings
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


highlight = 0

class EventHandler(Fl_Widget):
    def __init__(self, X, Y, W, H, L):
        Fl_Widget.__init__(self, X, Y, W, H, L)
        
    def handle(self, event):
        global highlight
	#print "Alt event: ", Fl.event_alt()
        if event == FL_PUSH:
            print("FL_PUSH")
            if Fl.event_clicks():
                print("Double click")
            else:
                if Fl.event_button1():
                    print("Button1")
                elif Fl.event_button2():
                    print("Button2")
                elif Fl.event_button3():
                    print("Button3")
            highlight = 1
            self.redraw()
            return 1
        elif event == FL_DRAG:
            print("FL_DRAG")
            t = Fl.event_inside(self);
            if t != highlight:
                highlight = t
                self.redraw()
            return 1
        elif event == FL_RELEASE:
            print("FL_RELEASE")
            print(f"Button = {Fl.event_button()}")
            if highlight != 0:
                highlight = 0
                self.redraw()
                self.do_callback()
                # never do anything after a callback, as the callback
                # may delete the widget!
            return 1
        
        elif event == FL_SHORTCUT:
            print("FL_SHORTCUT")
            if Fl.event_key() == 'x':
                self.do_callback()
                return 1
            return 0
        elif event == FL_FOCUS:
            Fl.focus(self)
            self.redraw()
            return 1
        elif event == FL_UNFOCUS:
            self.redraw()
            return 1
        elif event == FL_KEYDOWN:
            print("Last key pressed = ", Fl.event_key())
            return 1
        elif event == FL_MOVE:
            print("FL_MOVE")
            return 1
        else:
            return 0

    def draw(self):
        fl_color(FL_RED)
        fl_rectf(10,10,self.w(),self.h())
        return None

    def resize(self, X, Y, W, H):
        print(f"Resizing: {X}, {Y}, {W}, {H}\n")
        Fl_Widget.resize(self, X, Y, W, H)
        


window = Fl_Window(300, 300)
d = EventHandler(10,10,280,280,"test")
window.resizable(d)

window.end()
window.show()
