#
# "$Id: simple_table.py 107 2005-08-17 10:25:30Z andreasheld $"
#
# Simple Fl_Table test program for pyFLTK the Python bindings
# for the Fast Light Tool Kit (FLTK).
#
# FLTK copyright 1998-1999 by Bill Spitzak and others.
# Fl_Table copyright 2003 by G. Ercolano
# pyFLTK copyright 2003-2006 by Andreas Held and others.
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
import random


class WidgetTable(Fl_Table_Row):
    data = []
    def __init__(self, x, y, w, h, l):
        Fl_Table_Row.__init__(self, x, y, w, h, l)
        self.end()
        

    def draw_cell(self, context, R, C, X, Y, W, H):
        if context==self.CONTEXT_STARTPAGE:
            fl_font(FL_HELVETICA, 12) # font used by all headers
            return None
        elif context==self.CONTEXT_CELL:
            s = f"{self.data[R][C]}"
            fl_push_clip(X,Y,W,H)
            fl_draw_box(FL_THIN_UP_BOX, X, Y, W, H, self.row_header_color())
            fl_color(FL_BLACK)
            fl_draw(s, X, Y, W, H, FL_ALIGN_CENTER)
            fl_pop_clip()
            return None
        else:
            return None

    def SetSize(self, newrows, newcols):
        self.rows(newrows)
        self.cols(newcols)
        for r in range(newrows):
            l = []
            for c in range(newcols):
                l.append(random.randint(0, 1000))
            self.data.append(l)
        self.redraw()
        self.end()

    def getValue(self, row, col):
        return self.data[row][col]

    def setValue(self, row, col, value):
        self.data[row][col] = value
        self.redraw()
                    


if __name__=='__main__':
    win1 = Fl_Window(842, 292, "widgettable")
    table = WidgetTable(20, 20, win1.w()-40, win1.h()-40, "widgettable")
    table.SetSize(10, 10)
    win1.end()
    win1.show()

    print("Some values: ")
    print("  0:0:", table.getValue(0,0))
    print("  5:5:", table.getValue(5,5))

    table.setValue(5,5,13)

    print("  5:5:", table.getValue(5,5))

    
