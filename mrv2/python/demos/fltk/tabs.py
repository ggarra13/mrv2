#
# "$Id: tabs.py 531 2019-12-27 12:15:45Z andreasheld $"
#
# Tabs test program for pyFLTK the Python bindings
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

# global object names
foo_window = None      # type 'Fl_Window' from '()'


def main():
    global foo_window

    foo_window = Fl_Window(733, 489, 321, 324)

    o_2_0 = Fl_Tabs(10, 10, 300, 200)

    o_3_0 = Fl_Group(10, 30, 300, 180, "Label1")

    o_4_0 = Fl_Input(60, 50, 240, 40, "input:")
    o_4_0.label('input:')

    o_4_1 = Fl_Input(60, 90, 240, 30, "input2:")
    o_4_1.label('input2:')

    o_4_2 = Fl_Input(60, 120, 240, 80, "input3:")
    o_4_2.label('input3:')
    o_3_0.selection_color(1)
    o_3_0.resizable(o_3_0.this)
    o_3_0.label('Label1')
    o_3_0.end()

    o_3_1 = Fl_Group(10, 30, 300, 180, "tab2")

    o_4_0 = Fl_Button(20, 60, 100, 30, "button1")
    o_4_0.label('button1')

    o_4_1 = Fl_Input(140, 100, 100, 30, "input in box2")
    o_4_1.label('input in box2')

    o_4_2 = Fl_Button(30, 140, 260, 30, "This is stuff inside the Fl_Group tab2")
    o_4_2.label("This is stuff inside the Fl_Group tab2")

    o_4_3 = Fl_Button(30, 170, 260, 30, "Test event blocking by modal window")
    o_4_3.label('Test event blocking by modal window')
    o_3_1.selection_color(2)
    o_3_1.label('tab2')
    o_3_1.end()

    o_3_2 = Fl_Group(10, 30, 300, 180, "tab3")

    o_4_0 = Fl_Button(20, 60, 60, 80, "button2")
    o_4_0.label('button2')

    o_4_1 = Fl_Button(80, 60, 60, 80, "button")
    o_4_1.label('button')

    o_4_2 = Fl_Button(140, 60, 60, 80, "button")
    o_4_2.label('button')
    o_3_2.selection_color(3)
    o_3_2.label('tab3')
    o_3_2.end()

    o_3_3 = Fl_Group(10, 30, 300, 180, "tab4")

    o_4_0 = Fl_Button(20, 50, 60, 110, "button2")
    o_4_0.label('button2')

    o_4_1 = Fl_Button(80, 50, 60, 110, "button")
    o_4_1.label('button')

    o_4_2 = Fl_Button(140, 50, 60, 110, "button")
    o_4_2.label('button')
    o_3_3.selection_color(5)
    o_3_3.labelfont(2)
    o_3_3.label('tab4')
    o_3_3.end()

    o_3_4 = Fl_Group(10, 30, 300, 180, "     tab5      ")

    o_4_0 = Fl_Button(20, 80, 60, 80, "button2")
    o_4_0.label('button2')

    o_4_1 = Fl_Button(90, 90, 60, 80, "button")
    o_4_1.label('button')

    o_4_2 = Fl_Clock(160, 50, 100, 100, "Make sure this clock does not use processor time when this tab is hidden or window is iconized")
    o_4_2.selection_color(0)
    o_4_2.align(130)
    o_4_2.labelsize(10)
    o_4_2.box(fl_define_FL_OVAL_BOX()+1)
    o_4_2.labelfont(8)
    o_4_2.color(238)
    o_4_2.label('Make sure this clock does not use processor time when this tab is hidden or window is iconized')
    o_3_4.labeltype(fl_define_FL_ENGRAVED_LABEL())
    o_3_4.label('     tab5      ')
    o_3_4.end()
    o_2_0.selection_color(7)
    o_2_0.resizable(o_2_0.this)
    o_2_0.end()

    o_2_1 = Fl_Input(60, 220, 130, 30, "inputA:")
    o_2_1.label('inputA:')

    o_2_2 = Fl_Input(60, 250, 250, 30, "inputB:")
    o_2_2.label('inputB:')

    o_2_3 = Fl_Button(180, 290, 60, 30, "cancel")
    o_2_3.label('cancel')

    o_2_4 = Fl_Return_Button(250, 290, 60, 30, "OK")
    o_2_4.label('OK')
    foo_window.resizable(foo_window.this)
    foo_window.end()

    return foo_window



if __name__=='__main__':
    import sys
    window = main()
    window.show(len(sys.argv), sys.argv)
    Fl.run()
