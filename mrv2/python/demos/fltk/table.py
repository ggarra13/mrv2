#
# "$Id: table.py 536 2020-10-30 15:20:32Z andreasheld $"
#
# Fl_Table test program for pyFLTK the Python bindings
# for the Fast Light Tool Kit (FLTK).
#
# FLTK copyright 1998-1999 by Bill Spitzak and others.
# Fl_Table copyright 2003 by G. Ercolano
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


def button_cb(w, data):
    print(f"BUTTON: {w.label()}")

class WidgetTable(Fl_Table_Row):
    def __init__(self, x, y, w, h, l=""):
        Fl_Table_Row.__init__(self, x, y, w, h, l)
        self.col_header(1)
        self.col_resize(1)
        self.col_header_height(25)
        self.row_header(1)
        self.row_resize(1)
        self.row_header_width(80)
        self.end()

    def draw_cell(self, context, R, C, X, Y, W, H):
        s=f"{R}/{C}" # text for each cell

        if context==self.CONTEXT_STARTPAGE:
            fl_font(FL_HELVETICA, 12) # font used by all headers
            return None
        elif context==self.CONTEXT_RC_RESIZE:
            index = 0
            for r in range(self.rows()):
                for c in range(self.cols()):
                    if index >= self.children():
                        break
                    #(status, X, Y, W, H) = self.find_cell(self.CONTEXT_TABLE, r, c, X, Y, W, H)
                    (status, X, Y, W, H) = self.find_cell(self.CONTEXT_TABLE, r, c)
                    print("find_cell")
                    self.child(index).resize(X,Y,W,H)
                    index += 1
            self.init_sizes()
            return None
        elif context==self.CONTEXT_ROW_HEADER:
            s1=f"Row {R}"
            fl_push_clip(X,Y,W,H)
            fl_draw_box(FL_THIN_UP_BOX, X, Y, W, H, self.row_header_color())
            fl_color(FL_BLACK)
            fl_draw(s1, X, Y, W, H, FL_ALIGN_CENTER)
            fl_pop_clip()
            return None
        elif context==self.CONTEXT_COL_HEADER:
            s1=f"Column {C}"
            fl_push_clip(X, Y, W, H)
            fl_draw_box(FL_THIN_UP_BOX, X, Y, W, H, self.col_header_color())
            fl_color(FL_BLACK)
            fl_draw(s1, X, Y, W, H, FL_ALIGN_CENTER)
            fl_pop_clip()
            return None
        elif context==self.CONTEXT_CELL:
            return None
        else:
            return None

    def SetSize(self, newrows, newcols):
        self.rows(newrows)
        self.cols(newcols)
        self.begin()
        self.s_list = []
        for r in range(newrows):
            for c in range(newcols):
                X=0
                Y=0
                W=self.col_width(c)
                H=self.row_height(r)
                #c=0
                #(status, X, Y, W, H)=self.find_cell(self.CONTEXT_TABLE, r, c, X, Y, W, H)
                
                if c&1:
                    s=f"{r}.{c}"
                    inp = Fl_Input(X,Y,W,H)
                    inp.value(s)
                else:
                    s1=f"{r}/{c}"
                    butt = Fl_Light_Button(X,Y,W,H,s1)
                    self.s_list.append(s1)
                    butt.align(FL_ALIGN_CENTER|FL_ALIGN_INSIDE)
                    butt.callback(button_cb, 0)
                    if (r+2*c)&4:
                        butt.value(1)
                    else:
                        butt.value(0)
        self.end()
                    


class DemoTable(Fl_Table_Row):
    def __init__(self, x, y, w, h, l=""):
        Fl_Table_Row.__init__(self, x, y, w, h, l)

    def draw_cell(self, context, R, C, X, Y, W, H):
        s=f"{R}/{C}" # text for each cell

        if context==self.CONTEXT_STARTPAGE:
            fl_font(FL_HELVETICA, 16)
            return None
        elif context==self.CONTEXT_ROW_HEADER or context==self.CONTEXT_COL_HEADER:
            fl_push_clip(X,Y,W,H)
            fl_draw_box(FL_THIN_UP_BOX, X, Y, W, H, self.color())
            fl_color(FL_BLACK)
            fl_draw(s, X, Y, W, H, FL_ALIGN_CENTER)
            fl_pop_clip()
            return None
        elif context==self.CONTEXT_CELL:
            fl_push_clip(X,Y,W,H)
            # BG color
            if self.row_selected(R):
                fl_color(self.selection_color())
            else:
                fl_color(FL_WHITE)
            fl_rectf(X, Y, W, H)
            
            # TEXT
            fl_color(FL_BLACK)
            fl_draw(s, X, Y, W, H, FL_ALIGN_CENTER)

            # BORDER
            fl_color(FL_LIGHT2); 
            fl_rect(X, Y, W, H);
            fl_pop_clip()
            return None
        else:
            return None
                                   


def table_cb(o, data):
    table = data
    print(f"{table.label()} callback: row={table.callback_row()} col={table.callback_col()}, context={table.callback_context()}, event={Fl.event()}, clicks={Fl.event_clicks()}")
    
if __name__=='__main__':
    win1 = Fl_Window(940, 500, "widgettable")
    table = WidgetTable(20, 20, win1.w()-40, win1.h()-40, "widgettable")
    table.SetSize(20, 20)
    win1.end()
    win1.resizable(table)
    win1.show()

    w = 900
    h = 700
    t1x = 20
    t1y = 20
    t1w = w - 40
    t1h = int( (h-60) / 2 )
    t2x = 20
    t2y = t1y+t1h+20
    t2w = w - 40
    t2h = t1h

    win2 = Fl_Window(w, h, "testtablerow")
    table1 = DemoTable(t1x, t1y, t1w, t1h, "Table 1")
    table1.selection_color(FL_YELLOW)
    table1.when(FL_WHEN_RELEASE)	# handle table events on release
    #table1.rows(1001)
    table1.rows(51)
    table1.cols(31)
    table1.col_header(1)		# enable col header
    table1.col_resize(4)		# enable col resizing
    table1.row_header(1)		# enable row header
    table1.row_resize(4)		# enable row resizing
    table1.callback(table_cb, table1)
    table1.when(FL_WHEN_CHANGED|FL_WHEN_RELEASE)
    table1.end()

    table2 = DemoTable(t2x, t2y, t2w, t2h, "Table 2")
    table2.selection_color(FL_YELLOW)
    table2.when(FL_WHEN_RELEASE)	# handle table events on release
    #table2.rows(1001)
    table2.rows(51)
    table2.cols(31)
    table2.col_header(1)		# enable col header
    table2.col_resize(4)		# enable col resizing
    table2.row_header(1)		# enable row header
    table2.row_resize(4)		# enable row resizing
    table2.end()
    win2.end()
    win2.show()

    Fl.run()
