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
# Please report all bugs and problems to "pyfltk-user@lists.sourceforge.net".
#


import fltk

class subwindow(fltk.Fl_Gl_Window):

    def __init__(self, x,y,w,h):
        super().__init__(x,y,w,h, "")

    def handle(self, event):
        if event == fltk.FL_PUSH:
            raise Exception("exception in handle")
        return super().handle(event)

    def draw(self):
        pass



class MyButton(fltk.Fl_Button):
   def __init__(self, x,y,w,h, l):
       super().__init__(x,y,w,h, l)
       #b=fltk.Fl_Button(0,0,120,30,"raise exception")
       self.callback(self.b_cb)

   def b_cb(self, wid):
       raise Exception("exception in callback")
   
window = fltk.Fl_Double_Window(0, 0, 500, 500, "tst")

widget = subwindow(0,0,500,400)
window.resizable(widget)

mb = MyButton(200,420,100,60, "Exit")


try:
    window.end()
    window.show()
    fltk.Fl.run()
except Exception as e:
    print("Exception: ", e)
    #sys.exit()
