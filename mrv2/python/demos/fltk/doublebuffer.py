# Double-buffering test program for the Python port of
# the Fast Light Tool Kit (pyFLTK).
#
# This demo shows how double buffering helps, by drawing the
# window in a particularily bad way.
#
# The single-buffered window will blink as it updates.  The
# double buffered one will not.  It will take just as long
# (or longer) to update, but often it will appear to be faster.
#
# This demo should work for both the GL and X versions of Fl,
# even though the double buffering mechanism is totally different.
#
#
# FLTk is Copyright 1998-2003 by Bill Spitzak and others.
# pyFLTK is Copyright 2003 by Andreas Held
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
import math

w01 = None
w1 = None
slider0 = None

# this purposely draws each line 10 times to be slow:
def star(w, h, n):
    fl_push_matrix()
    fl_translate(w/2, h/2)
    fl_scale(w/2, h/2)

    for i in range(n):
        val_i = 2*math.pi*i/n+.1
        j = i+1
        while j < n:
            val_j = 2*math.pi*j/n+.1
            fl_begin_line();
            fl_vertex(math.cos(val_i), math.sin(val_i))
            fl_vertex(math.cos(val_j), math.sin(val_j))
            fl_end_line()
            j = j+1

    fl_pop_matrix()


sides = [20,20]

def slider_cb(slider, v):
    sides[v] = int(slider.value())
    # redraw figure only
    slider.parent().child(0).redraw()


def bad_draw(w,h,which):
    fl_color(FL_BLACK)
    fl_rectf(0,0,w,h)
    fl_color(FL_WHITE)
    star(w,h,sides[which]);


class single_blink_window(Fl_Single_Window):
      def __init__(self, x, y, w, h, l):
          Fl_Single_Window.__init__(self, x, y, w, h, l)
          self.resizable(self)
	  
      def draw(self):
      	  bad_draw(self.w(), self.h(), 0)

#      def handle(self, event):
#          print "event = ", event
#          return 0

class double_blink_window(Fl_Double_Window):
      def __init__(self, x, y, w, h, l):
          Fl_Double_Window.__init__(self, x, y, w, h, l)
          self.resizable(self)
	  
      def draw(self):
          bad_draw(self.w(), self.h(), 1)


if __name__ == '__main__':
   if Fl.visual(FL_DOUBLE) == 0:
      print("Xdbe not supported, faking double buffer with pixmaps.")

   w01 = Fl_Window(420,465,"Fl_Single_Window")
   w01.box(FL_FLAT_BOX)
   w01.thiswon = 0
   w1 = single_blink_window(10,10,400,400,"Fl_Single_Window")
   w1.box(FL_FLAT_BOX)
   w1.color(FL_BLACK)
   w1.end()
   slider0 = Fl_Hor_Slider(30,420,360,25)
   slider0.range(2,30)
   slider0.step(1)
   slider0.value(sides[0])
   slider0.callback(slider_cb, 0)
   w01.end()

   w02 = Fl_Window(420,465,"Fl_Double_Window")
   w02.box(FL_FLAT_BOX)
   w2 = double_blink_window(10,10,400,400,"Fl_Double_Window")
   w2.box(FL_FLAT_BOX)
   w2.color(FL_BLACK)
   w2.end()
   slider1 = Fl_Hor_Slider(30,420,360,25)
   slider1.range(2,30)
   slider1.step(1)
   slider1.value(sides[0])
   slider1.callback(slider_cb, 1)
   w02.end()

   w01.show()
   w02.show()

#   w1.redraw()





