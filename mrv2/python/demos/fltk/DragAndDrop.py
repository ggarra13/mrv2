#
# "$Id: DragAndDrop.py 536 2020-10-30 15:20:32Z andreasheld $"
#
# Drag and drop test program for pyFLTK the Python bindings
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


class DNDWidget(Fl_Widget):
    text_string = ""
    
    def __init__(self, x, y, w, h, label):
        Fl_Widget.__init__(self, x, y, w, h, label)

    def draw(self):
        fl_rect(self.x(), self.y(), self.w(), self.h(), FL_RED)

    def handle(self, event):
        if event == FL_DND_ENTER:
            print("FL_DND_ENTER ", self.text_string) 
            return 1
        elif event == FL_DND_LEAVE:
            print("FL_DND_LEAVE ", self.text_string)
            return 1
        elif event == FL_DND_DRAG:
            print(f"FL_DND_DRAG {self.text_string} ({Fl.event_x()}, {Fl.event_y()})")
            return 1
        elif event == FL_DND_RELEASE:
            print(f"FL_DND_RELEASE {self.text_string} ({Fl.event_x()}, {Fl.event_y()})")
            return 1
        elif event == FL_PUSH:
            print("PREPUSH ", self.text_string)
            Fl.copy("text", 5, 0)
            Fl.dnd()
            print("POSTPUSH ", self.text_string)
            return 1
        elif event == FL_PASTE:
            cl = Fl.event_text()
            ln = Fl.event_length()
            print("PASTE ", self.text_string)
            print(f"  text = {cl}, length = {ln}")
            return 1
        else:
            return 0

    def text(self, t):
        self.text_string = t
        return 
        
    
                  
            
if __name__=='__main__':
    window = Fl_Window(50, 50, 200, 200)
    w1 = DNDWidget(5, 5, 90, 90, "DND")
    w1.text("A")
    w2 = DNDWidget(105, 105, 90, 90, "DND")
    w2.text("B")
    window.end()

    window.show()

    Fl.run()
