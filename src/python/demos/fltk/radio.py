# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.

#
# "$Id: radio.py 531 2019-12-27 12:15:45Z andreasheld $"
#
# Radio buttons test program for pyFLTK the Python bindings
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


def main():

    o_1_0 = Fl_Window(447, 255, 369, 214)

    o_2_0 = Fl_Button(20, 10, 160, 30, "Fl_Button")
    o_2_0.label('Fl_Button')

    o_2_1 = Fl_Return_Button(20, 50, 160, 30, "Fl_Return_Button")
    o_2_1.label('Fl_Return_Button')

    o_2_2 = Fl_Light_Button(20, 90, 160, 30, "Fl_Light_Button")
    o_2_2.label('Fl_Light_Button')

    o_2_3 = Fl_Check_Button(20, 130, 160, 30, "Fl_Check_Button")
    o_2_3.down_box(FL_DOWN_BOX)
    o_2_3.label('Fl_Check_Button')

    o_2_4 = Fl_Round_Button(20, 170, 160, 30, "Fl_Round_Button")
    o_2_4.down_box(fl_define_FL_ROUND_UP_BOX()+1)
    o_2_4.label('Fl_Round_Button')

    o_2_5 = Fl_Group(190, 10, 70, 120)

# warning: following obj has type class
# option not handled by code generator:
# Radio
# Please email this comment and the following
# 2 lines to pyfltk@egroups.com
    o_3_0 = Fl_Round_Button(190, 10, 70, 30, "radio")
# XXX unknown typedef Fl_Round_Button Radio
    o_3_0.down_box(fl_define_FL_ROUND_UP_BOX()+1)
    o_3_0.label('radio')

# warning: following obj has type class
# option not handled by code generator:
# Radio
# Please email this comment and the following
# 2 lines to pyfltk@egroups.com
    o_3_1 = Fl_Round_Button(190, 40, 70, 30, "radio")
# XXX unknown typedef Fl_Round_Button Radio
    o_3_1.down_box(fl_define_FL_ROUND_UP_BOX()+1)
    o_3_1.label('radio')

# warning: following obj has type class
# option not handled by code generator:
# Radio
# Please email this comment and the following
# 2 lines to pyfltk@egroups.com
    o_3_2 = Fl_Round_Button(190, 70, 70, 30, "radio")
# XXX unknown typedef Fl_Round_Button Radio
    o_3_2.down_box(fl_define_FL_ROUND_UP_BOX()+1)
    o_3_2.label('radio')

# warning: following obj has type class
# option not handled by code generator:
# Radio
# Please email this comment and the following
# 2 lines to pyfltk@egroups.com
    o_3_3 = Fl_Round_Button(190, 100, 70, 30, "radio")
# XXX unknown typedef Fl_Round_Button Radio
    o_3_3.down_box(fl_define_FL_ROUND_UP_BOX()+1)
    o_3_3.label('radio')
    o_2_5.box(FL_THIN_UP_FRAME)
    o_2_5.end()

    o_2_6 = Fl_Group(270, 10, 90, 115)

    o_3_0 = Fl_Button(280, 20, 20, 20, "radio")
    o_3_0.type(102)
    o_3_0.selection_color(1)
    o_3_0.align(8)
    o_3_0.label('radio')

    o_3_1 = Fl_Button(280, 45, 20, 20, "radio")
    o_3_1.type(102)
    o_3_1.selection_color(1)
    o_3_1.align(8)
    o_3_1.label('radio')

    o_3_2 = Fl_Button(280, 70, 20, 20, "radio")
    o_3_2.type(102)
    o_3_2.selection_color(1)
    o_3_2.align(8)
    o_3_2.label('radio')

    o_3_3 = Fl_Button(280, 95, 20, 20, "radio")
    o_3_3.type(102)
    o_3_3.selection_color(1)
    o_3_3.align(8)
    o_3_3.label('radio')
    o_2_6.box(FL_THIN_UP_BOX)
    o_2_6.end()
    o_1_0.end()

    return o_1_0



if __name__=='__main__':
    import sys
    window = main()
    window.show()
    
