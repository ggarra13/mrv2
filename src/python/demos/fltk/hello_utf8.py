# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.

#!/usr/bin/python
# -*- coding: utf-8 -*-


#
# "$Id: hello.py 276 2006-11-13 11:01:40Z andreasheld $"
#
# Callback test program for pyFLTK the Python bindings
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
import sys
from string import *

def theCancelButtonCallback(ptr, data):
        pass
        print("type = ", type(ptr))
        print(f"theCancelButtonCallback({str(data)})")
        print("Tooltip: ", ptr.tooltip())

window = Fl_Window(100, 100, 200, 90)
button = Fl_Button(9,20,180,60)
utfstr = "日本語"
extra_font = FL_TIMES_BOLD_ITALIC
#Fl.set_font(extra_font, "*gothic*iso10646-1")
Fl.set_font(extra_font, "*gothic-medium-r-normal*")
Fl_Tooltip.font(extra_font)

button.labelfont(extra_font)
button.labelsize(30)
button.label(utfstr)
button.labeltype(FL_EMBOSSED_LABEL)
button.callback(theCancelButtonCallback, "'some callback data'")
button.callback(theCancelButtonCallback, "'任意のデータ'")
#button.tooltip("Press to see the callback!")
button.tooltip("データを見るため押して下さい。")

window.end()
window.show()