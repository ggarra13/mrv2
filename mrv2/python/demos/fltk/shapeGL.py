#
# "$Id: shapeGL.py 495 2013-03-30 09:39:45Z andreasheld $"
#
# OpenGL test program for pyFLTK the Python bindings
# for the Fast Light Tool Kit (FLTK).
#
# FLTK copyright 1998-1999 by Bill Spitzak and others.
# pyFLTK copyright 2003 by Andreas Held.
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
import sys, math, time

#sleep(5)

try:
    class ShapeWindow(Fl_Gl_Window):

        def __init__(self, x, y, w, h, l):
            Fl_Gl_Window.__init__(self, x,y,w,h,l)
            self.sides = 3

        def draw(self):
            #// draw an amazing graphic:
            glClear(0x00004000) #GL_COLOR_BUFFER_BIT)
            glColor3f(0.5,0.6,0.7)
            glBegin(9) #GL_POLYGON)
            for i in range(int(self.sides)):
                ang = i*2.0*3.141/self.sides
                x = math.cos(ang)
                y = math.sin(ang)
                glVertex3f( x, y, 0.0)
            glEnd()

#	def handle(self, event):
#		if event == FL_PUSH:
#			print "Mouse pushed!"
#			return 1
#		else:
#			return 0


        def sides_cb(self, slider):
            self.sides = slider.value()
            self.redraw()

except NameError:
        fl_message("This demo requires OpenGL.  Reconfigure and rebuild with OpenGl enabled.")
        window.hide()
 
window = Fl_Window(10, 10, 300, 330, sys.argv[0])

sw = ShapeWindow(10,10,280,280, "Shape Window")
window.resizable(sw)

slider = Fl_Hor_Slider(50, 295, window.w()-60, 30, "Sides")
slider.align(FL_ALIGN_LEFT)
slider.callback(sw.sides_cb)
slider.value(float(sw.sides))
slider.step(1)
slider.bounds(3, 40)

window.end()
window.show()
sw.show()


