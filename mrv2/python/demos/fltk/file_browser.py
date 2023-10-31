# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.

#
# "$Id: file_browser.py 495 2013-03-30 09:39:45Z andreasheld $"
#
# File chooser test program for pyFLTK the Python bindings
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

fb = None
directory = "./"


def SortCB(widget):
    method = widget.value()
    print("Sorting: ", method)
    
    if method == "FL_NUMERICSORT":
        fb.load(directory, FL_NUMERICSORT)
    elif method == "FL_ALPHASORT":
        fb.load(directory, FL_ALPHASORT)
    elif method == "FL_CASENUMERICSORT":
        fb.load(directory, FL_CASENUMERICSORT)
    elif method == "FL_CASEALPHASORT":
        fb.load(directory, FL_CASEALPHASORT)
    else:
        print("Wrong sorting method!")


if __name__=='__main__':
    win = Fl_Window(100, 100, 300, 400, "File Browser")
    fb = Fl_File_Browser(50,50,200,200)
    fb.load(directory, FL_NUMERICSORT)
    ic = Fl_Input_Choice(50, 280, 200, 25, "Sorting:")
    ic.align(FL_ALIGN_TOP | FL_ALIGN_LEFT)
    ic.add("FL_NUMERICSORT")
    ic.add("FL_ALPHASORT")
    ic.add("FL_CASENUMERICSORT")
    ic.add("FL_CASEALPHASORT")
    ic.value(0)
    ic.callback(SortCB)

    win.show()

