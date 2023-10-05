#
# "$Id: draw.py 495 2013-03-30 09:39:45Z andreasheld $"
#
# fl_draw test program for pyFLTK the Python bindings
# for the Fast Light Tool Kit (FLTK).
# Courtesy of G. Lielens!
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
from math import *
import sys
from array import *


width = 100
height = 100
image = array('B')

def make_image():
    #image = new uchar[4*width*height];
    #uchar *p = image;
  p = image
  index = 0

  y = 0
  while y < height:
    dy = float(y)/(height-1)
    x = 0
    while x < width:
      dx = float(x)/(width-1)
      #print 255*((1.0-dx)*(1.0-dy))
      p.append(int(255*((1.0-dx)*(1.0-dy))))
      index = index+1
      p.append(int(255*((1.0-dx)*dy)))
      index = index+1
      p.append(int(255*(dx*dy)))
      index = index+1
      dx = dx-0.5
      dy = dy-0.5
      alpha = int(255*sqrt(dx*dx+dy*dy))
      if alpha < 255:
        p.append(alpha)
      else:
        p.append(255)
      index = index+1
      dy = dy+0.5
      x = x+1
    y = y+1
  return None



class MyWidget(Fl_Widget):
    def __init__(self,x,y,w,h,image):
        Fl_Widget.__init__(self, 0, 0, w, h, "canvas")
        self._image=image
    def draw(self):
        w,h=self.w(),self.h() 
        fl_draw_image(self._image,int(w/2-50),int(h/2-50),width,height,4,0)
    

window = Fl_Window(400,400)
window.color(FL_WHITE)
make_image()
widget=MyWidget(0,0,400,400,image)
window.resizable(window)
window.end()
window.show(len(sys.argv), sys.argv)
 
Fl.run()

