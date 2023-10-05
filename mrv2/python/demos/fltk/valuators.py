#
# "$Id: valuators.py 531 2019-12-27 12:15:45Z andreasheld $"
#
# Valuators test program for pyFLTK the Python bindings
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


def theCallback(ptr):
    print(ptr.value())


def main():

    o_1_0 = Fl_Window(465, 113, 567, 506, "Valuator classes, showing values for type()")

    o_2_0 = Fl_Box(10, 10, 280, 210, "Fl_Slider")
    o_2_0.label('Fl_Slider')
    o_2_0.align(17)
    o_2_0.labelfont(1)
    o_2_0.box(FL_ENGRAVED_BOX)

    o_2_1 = Fl_Slider(30, 45, 20, 145, "")
    o_2_1.label('')
    o_2_1.labelsize(8)
    o_2_1.callback(theCallback)
    o_2_1.align(1)
    o_2_1.selection_color(1)

    o_2_2 = Fl_Fill_Slider(70, 55, 20, 145, "FL_VERT_FILL_SLIDER")
    o_2_2.label('FL_VERT_FILL_SLIDER')
    o_2_2.labelsize(8)
    o_2_2.callback(theCallback)
    o_2_2.selection_color(1)

    o_2_3 = Fl_Nice_Slider(105, 45, 20, 145, "FL_VERT_NICE_SLIDER")
    o_2_3.label('FL_VERT_NICE_SLIDER')
    o_2_3.labelsize(8)
    o_2_3.color(10)
    o_2_3.callback(theCallback)
    o_2_3.align(1)
    o_2_3.selection_color(1)
    o_2_3.box(FL_FLAT_BOX)

    o_2_4 = Fl_Box(10, 230, 280, 205, "Fl_Value_Slider")
    o_2_4.label('Fl_Value_Slider')
    o_2_4.align(17)
    o_2_4.labelfont(1)
    o_2_4.box(FL_ENGRAVED_BOX)

    o_2_5 = Fl_Value_Slider(30, 260, 30, 145)
    o_2_5.callback(theCallback)
    o_2_5.align(1)
    o_2_5.labelsize(8)
    o_2_5.selection_color(1)

    o_2_6 = Fl_Value_Slider(70, 275, 30, 140, "FL_VERT_FILL_SLIDER")
    o_2_6.type(2)
    o_2_6.label('FL_VERT_FILL_SLIDER')
    o_2_6.labelsize(8)
    o_2_6.callback(theCallback)
    o_2_6.selection_color(1)

    o_2_7 = Fl_Value_Slider(110, 260, 20, 145, "FL_VERT_NICE_SLIDER")
    o_2_7.type(4)
    o_2_7.label('FL_VERT_NICE_SLIDER')
    o_2_7.labelsize(8)
    o_2_7.color(10)
    o_2_7.callback(theCallback)
    o_2_7.align(1)
    o_2_7.selection_color(1)
    o_2_7.box(FL_FLAT_BOX)

    o_2_8 = Fl_Hor_Slider(140, 80, 130, 20, "FL_HORIZONTAL")
    o_2_8.label('FL_HORIZONTAL')
    o_2_8.labelsize(8)
    o_2_8.callback(theCallback)
    o_2_8.selection_color(1)

    o_2_9 = Fl_Hor_Fill_Slider(140, 120, 130, 20, "FL_HOR_FILL_SLIDER")
    o_2_9.label('FL_HOR_FILL_SLIDER')
    o_2_9.labelsize(8)
    o_2_9.callback(theCallback)
    o_2_9.selection_color(1)

    o_2_10 = Fl_Hor_Nice_Slider(140, 160, 130, 20, "FL_HOR_NICE_SLIDER")
    o_2_10.label('FL_HOR_NICE_SLIDER')
    o_2_10.color(10)
    o_2_10.labelsize(8)
    o_2_10.callback(theCallback)
    o_2_10.selection_color(1)
    o_2_10.box(FL_FLAT_BOX)

    o_2_11 = Fl_Hor_Value_Slider(140, 290, 130, 20, "FL_HOR_SLIDER")
    o_2_11.label('FL_HOR_SLIDER')
    o_2_11.labelsize(8)
    o_2_11.callback(theCallback)
    o_2_11.selection_color(1)

    o_2_12 = Fl_Value_Slider(140, 330, 130, 20, "FL_HOR_FILL_SLIDER")
    o_2_12.type(3)
    o_2_12.label('FL_HOR_FILL_SLIDER')
    o_2_12.labelsize(8)
    o_2_12.callback(theCallback)
    o_2_12.selection_color(1)

    o_2_13 = Fl_Box(430, 10, 125, 120, "Fl_Adjuster")
    o_2_13.label('Fl_Adjuster')
    o_2_13.align(17)
    o_2_13.labelfont(1)
    o_2_13.box(FL_ENGRAVED_BOX)

    o_2_14 = Fl_Value_Slider(140, 370, 130, 20, "FL_HOR_NICE_SLIDER")
    o_2_14.type(5)
    o_2_14.label('FL_HOR_NICE_SLIDER')
    o_2_14.color(10)
    o_2_14.labelsize(8)
    o_2_14.callback(theCallback)
    o_2_14.selection_color(1)
    o_2_14.box(FL_FLAT_BOX)

    o_2_15 = Fl_Adjuster(440, 60, 75, 25, "w()>h()")
    o_2_15.label('w()>h()')
    o_2_15.callback(theCallback)
    o_2_15.labelsize(8)

    o_2_16 = Fl_Adjuster(520, 35, 25, 75, "w()<h()")
    o_2_16.label('w()<h()')
    o_2_16.callback(theCallback)
    o_2_16.labelsize(8)

    o_2_17 = Fl_Box(345, 135, 210, 115, "Fl_Counter")
    o_2_17.label('Fl_Counter')
    o_2_17.align(17)
    o_2_17.labelfont(1)
    o_2_17.box(FL_ENGRAVED_BOX)

    o_2_18 = Fl_Counter(360, 160, 180, 30, "")
    o_2_18.label('')
    o_2_18.callback(theCallback)
    o_2_18.labelsize(8)

    o_2_19 = Fl_Simple_Counter(360, 205, 180, 30, "FL_SIMPLE_COUNTER")
    o_2_19.label('FL_SIMPLE_COUNTER')
    o_2_19.callback(theCallback)
    o_2_19.labelsize(8)

    o_2_20 = Fl_Box(300, 260, 255, 105, "Fl_Dial")
    o_2_20.label('Fl_Dial')
    o_2_20.align(17)
    o_2_20.labelfont(1)
    o_2_20.box(FL_ENGRAVED_BOX)

    o_2_21 = Fl_Dial(315, 280, 65, 65, "")
    o_2_21.label('')
    o_2_21.color(10)
    o_2_21.labelsize(8)
    o_2_21.callback(theCallback)
    o_2_21.selection_color(1)

    o_2_22 = Fl_Line_Dial(395, 280, 65, 65, "FL_LINE_DIAL")
    o_2_22.label('FL_LINE_DIAL')
    o_2_22.labelsize(8)
    o_2_22.color(10)
    o_2_22.callback(theCallback)
    o_2_22.selection_color(1)

    o_2_23 = Fl_Fill_Dial(475, 280, 65, 65, "FL_FILL_DIAL")
    o_2_23.label('FL_FILL_DIAL')
    o_2_23.labelsize(8)
    o_2_23.color(10)
    o_2_23.callback(theCallback)
    o_2_23.selection_color(1)

    o_2_24 = Fl_Box(300, 375, 145, 120, "Fl_Roller")
    o_2_24.label('Fl_Roller')
    o_2_24.align(17)
    o_2_24.labelfont(1)
    o_2_24.box(FL_ENGRAVED_BOX)

    o_2_25 = Fl_Roller(315, 390, 20, 95, "")
    o_2_25.label('')
    o_2_25.callback(theCallback)
    o_2_25.labelsize(8)

    o_2_26 = Fl_Roller(340, 430, 90, 20, "FL_HORIZONTAL")
    o_2_26.type(1)
    o_2_26.label('FL_HORIZONTAL')
    o_2_26.callback(theCallback)
    o_2_26.labelsize(8)

    o_2_27 = Fl_Box(10, 445, 140, 50, "Fl_Value_Input")
    o_2_27.label('Fl_Value_Input')
    o_2_27.align(17)
    o_2_27.labelfont(1)
    o_2_27.box(FL_ENGRAVED_BOX)

    o_2_28 = Fl_Box(455, 375, 100, 120, "Some widgets have color(FL_GREEN) and color2(FL_RED) to show the areas these effect.")
    o_2_28.label('Some widgets have color(FL_GREEN) and color2(FL_RED) to show the areas these effect.')
    o_2_28.color(0)
    o_2_28.labelsize(10)
    o_2_28.align(128)
    o_2_28.selection_color(0)
    o_2_28.box(FL_BORDER_FRAME)

    o_2_29 = Fl_Box(155, 445, 135, 50, "Fl_Value_Output")
    o_2_29.label('Fl_Value_Output')
    o_2_29.align(17)
    o_2_29.labelfont(1)
    o_2_29.box(FL_ENGRAVED_BOX)

    o_2_30 = Fl_Value_Input(30, 460, 110, 30, "")
    o_2_30.label('')
    o_2_30.labelsize(8)
    o_2_30.callback(theCallback)
    o_2_30.step(0.1)

    o_2_31 = Fl_Value_Output(170, 460, 110, 30, "")
    o_2_31.label('')
    o_2_31.labelsize(8)
    o_2_31.callback(theCallback)
    o_2_31.step(0.1)

    o_2_32 = Fl_Box(295, 10, 130, 120, "   Fl_Scrollbar")
    o_2_32.label('   Fl_Scrollbar')
    o_2_32.align(21)
    o_2_32.labelfont(1)
    o_2_32.box(FL_ENGRAVED_BOX)

    o_2_33 = Fl_Scrollbar(395, 20, 20, 105, "")
    o_2_33.label('')
    o_2_33.labelsize(8)
    o_2_33.callback(theCallback)
    o_2_33.align(1)

    o_2_34 = Fl_Scrollbar(300, 65, 95, 20, "FL_HORIZONTAL")
    o_2_34.type(1)
    o_2_34.label('FL_HORIZONTAL')
    o_2_34.labelsize(8)
    o_2_34.callback(theCallback)
    o_1_0.label('Valuator classes, showing values for type()')
    o_1_0.color(43)
    o_1_0.selection_color(43)
    o_1_0.end()

    return o_1_0



if __name__=='__main__':
    import sys
    window = main()
    window.show()
    
