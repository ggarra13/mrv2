#
# "$Id: glimage.py 146 2005-11-22 12:32:49Z andreasheld $"
#
# Simple gl test program which generates an RGB image from
# scratch and attempts to display it within a GL pane.
# Test program for pyFLTK the Python bindings
# for the Fast Light Tool Kit (FLTK).
# Courtesy of: David McNab
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


import sys
from fltk14 import *

class MyWindow(Fl_Window):
    """
    main window
    """
    def __init__(self, w, h):

        # parent constructor
        Fl_Window.__init__(self, w, h, sys.argv[0])

        # add GL pane
        gl = self.gl = GLPane(10, 10, w-20, h-20, "GL Window")
        self.resizable(None)

        # gui created
        self.end()

        # display this window and the GL pane
        self.show()
        gl.show()

class GLPane(Fl_Gl_Window):
    """
    GL window that hopefully will display an image
    """
    def __init__(self, x, y, w, h, l):

        Fl_Gl_Window.__init__(self, x,y,w,h,l)

        # generate an RGB image string
        print("Generating RGB image as a string")

        import numpy as np
        rgb = np.zeros((h,w,3), dtype=np.uint8)

        # increase red from left to right
        rgb[...,0] = np.linspace(0, 256, w, endpoint=False, dtype=np.uint8)

        # increase green from right to left
        rgb[...,1] = 255 - rgb[...,0]

        # increase blue from top to bottom
        rgb[...,2] = np.linspace(0, 256, h, endpoint=False, dtype=np.uint8)[..., np.newaxis]

        self.rgb = rgb.tobytes()
        print("RGB image created")

    def draw(self):
        glClear(0x00004000) #GL_COLOR_BUFFER_BIT)
        glColor3f(0.5,0.6,0.7)

        # need to set x, y to -1 for drawing to lower left-hand corner
        # 0,0 is i n the centre of the canvas
        gl_draw_image(self.rgb, -1, -1, self.w(), self.h(), 3)


def main():
    w = MyWindow(400, 300)
    
if __name__ == "__main__":
    main()


