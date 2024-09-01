# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.

#
# "$Id: dials.py 531 2019-12-27 12:15:45Z andreasheld $"
#
# Dials test program for pyFLTK the Python bindings
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
aDial = None      # type 'Fl_Dial' from '()'
aLineDial = None      # type 'Fl_Line_Dial' from '()'
aFillDial = None      # type 'Fl_Fill_Dial' from '()'


def dialChanged(dial):
    aDial.value(dial.value())  # code
    aLineDial.value(dial.value())  # code
    aFillDial.value(dial.value())  # code


def main():
    global aDial
    global aLineDial
    global aFillDial

    o_1_0 = Fl_Double_Window(550, 247, 269, 116)

    aDial = Fl_Dial(20, 15, 45, 45, "Dial")
    aDial.label('Dial')
    aDial.callback(dialChanged)

    aLineDial = Fl_Line_Dial(190, 15, 45, 45, "Line Dial")
    aLineDial.label('Line Dial')
    aLineDial.callback(dialChanged)

    aFillDial = Fl_Fill_Dial(105, 15, 45, 45, "Fill Dial")
    aFillDial.label('Fill Dial')
    aFillDial.callback(dialChanged)
    aFillDial.selection_color(1)
    o_1_0.color(215)
    o_1_0.end()

    return o_1_0



if __name__=='__main__':
    import sys
    window = main()
    window.show()
