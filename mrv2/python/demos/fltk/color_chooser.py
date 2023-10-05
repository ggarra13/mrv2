# Color chooser test program for the Python port of
# the Fast Light Tool Kit (pyFLTK).
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

width = 75
height = 75
image = []

def make_image():
    global image
    for y in range(0, height, 1):
        Y = (1.0 *y)/(height-1)
        for x in range(0, width, 1):
            X = (1.0 * x)/(width-1)
            image.append(int((255*((1-X)*(1-Y))))) # red in upper-left
            image.append(int((255*((1-X)*Y)))) #	// green in lower-left
            image.append(int((255*(X*Y)))) #	// blue in lower-right

class Pens(Fl_Box):
    def __init__(self, X, Y, W, H, L):
        Fl_Box.__init__(self, X,Y,W,H,L)
	
    def draw(self):
        #// use every color in the gray ramp:
        i = 0
        while i < 3*8:
            fl_color(FL_GRAY_RAMP+i)
            fl_line(self.x()+i, self.y(), self.x()+i, self.y()+self.h())
            i += 1

c = FL_GRAY
fullcolor_cell = FL_FREE_COLOR

def cb1(ptr, box):
   global c
   c = fl_show_colormap(c)
   box.color(c)
   parent = box.parent()
   parent.redraw()

def cb2(ptr, box):
   global c
   r = 0
   g = 0
   b = 0
   (r, g, b) = Fl.get_color(c)
   (status, r, g, b) = fl_color_chooser("New color:", r,g,b)
   if status == 0:
      return None
   c = fullcolor_cell
   Fl.set_color(fullcolor_cell, r, g, b)
   box.color(fullcolor_cell)
   parent = box.parent()
   parent.redraw()


Fl.set_color(fullcolor_cell,145,159,170)
window = Fl_Window(400,400)

box  = Fl_Box(50,50,300,300)
box.box(FL_THIN_DOWN_BOX)
c = fullcolor_cell
box.color(c)

b1 = Fl_Button(130,120,140,30,"fl_show_colormap")
b1.callback(cb1, box) # b1.callback(cb1,&box)

b2 = Fl_Button(130,160,140,30,"fl_choose_color")
b2.callback(cb2,box)

image_box = Fl_Box(140,200,120,120,"")
make_image()

#i = Fl_Image(image, width, height)
i = Fl_RGB_Image(image, width, height)
i.label(image_box)

b = Fl_Box(140,335,120,0,"Example of fl_draw_image()")

p = Pens(80,200,3*8,120,"lines")
p.align(FL_ALIGN_TOP)
p.redraw()

window.end()
window.show(len(sys.argv), sys.argv)
Fl.run()

