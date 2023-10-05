#
# "$Id: arc.py 535 2020-10-30 10:43:46Z andreasheld $"
#
# Arc drawing program for pyFLTK the Python bindings
# for the Fast Light Tool Kit (FLTK).
# Illustrates the use of the Fl_Widget base class.
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

# Arc drawing test program for the Fast Light Tool Kit (FLTK).

from fltk14 import *
import sys
if sys.version > '3':
    long = int

args = [140, 140, 50, 0, 360, 0]
name = ["X", "Y", "R", "start", "end", "rotate"]
d = None

class Drawing(Fl_Widget):

    def __init__(self, X, Y, W, H, L):
        Fl_Widget.__init__(self, X, Y, W, H, L)

    def draw(self):
        global args
        try:
            xpos = self.x()
            ypos = self.y()
            w = self.w()
            h = self.h()
            fl_push_clip(xpos,ypos,w,h)
            fl_color(FL_DARK3)
            fl_rectf(xpos,ypos,w,h)
            fl_push_matrix()
            if args[5]:
                fl_translate(xpos+w/2.0, ypos+h/2.0)
                fl_rotate(args[5])
                fl_translate(-(xpos+w/2.0), -(ypos+h/2.0))
            fl_color(FL_WHITE)
            fl_translate(xpos,ypos)
            fl_begin_complex_polygon()
            fl_arc(args[0],args[1],args[2],args[3],args[4])
            fl_gap()
            fl_arc(140,140,20,0,-360)
            fl_end_complex_polygon()
            fl_color(FL_RED)
            fl_begin_line()
            fl_arc(args[0],args[1],args[2],args[3],args[4])
            fl_end_line()
            fl_pop_matrix()
            fl_pop_clip()
        except:
            print('uncaught!', sys.exc_info()[0], sys.exc_info()[1])
        return None

    

def slider_cb(ptr, v):
    global d
    args[int(v)] = ptr.value()
    d.redraw()



window = Fl_Window(300,500)
d = Drawing(10,10,280,280, "test")  
y = 300
n = 0
s = [None,None,None,None,None,None]
while n < 6:
    s[n] = Fl_Hor_Value_Slider(50,y,240,25,name[n])
    y = y+25
    if n<3:
        s[n].minimum(0)
        s[n].maximum(300)
    else:
        if n==5:
            s[n].minimum(0)
            s[n].maximum(360)
        else:
            s[n].minimum(-360)
            s[n].maximum(360)
    s[n].step(1)
    s[n].value(args[n])
    s[n].align(FL_ALIGN_LEFT)
    s[n].callback(slider_cb, n)
    n = n+1


window.end()
#window.show(len(sys.argv),sys.argv)
#window.show()
window.show(sys.argv)

d.redraw()
Fl.run()


