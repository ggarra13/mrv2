# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.

#
# "$Id: browser_cols.py 531 2019-12-27 12:15:45Z andreasheld $"
#
# Browser columns test program for pyFLTK, the Python bindings
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

print("""
This is a quick hack to check setting and getting browser data 
""")

from fltk14 import *


# global object names
aBrowser = None	  # type 'Browser' from '()'


def onOK(btn):
        window = btn.parent()
        window.hide()
        window = None


def main():
	global aBrowser

	win = Fl_Window(394, 309, 245, 133, "check_browser.py")

	aBrowser = Fl_Browser(5, 5, 240, 75)
	aBrowser.end()
	aBrowser.column_widths((150,150))
	#aBrowser.format_char("@-")

	btn = Fl_Return_Button(160, 90, 70, 30, "OK")
	btn.label('OK')
	btn.callback(onOK)
	win.label('check_browser.py')
	win.end()
	aBrowser.add("Guiness\tline 1", "line 1" )  # code
	aBrowser.add("Bud\tline 2", "line 2")  # code
	aBrowser.add("Coors\tline 3", "line 3")
	aBrowser.add("rocky mountain\tline 4", "line 4")  # code
	aBrowser.add("Grimbergen\tline 5", "line 5")  # code
	aBrowser.add("Burning River\tline 6", "line 6")  # code
	aBrowser.add("Little Kings\tline 7", "line 7")  # code

	return win



if __name__=='__main__':
	import sys
	window = main()
	window.show()
	d="data for 1"
	aBrowser.data(1, d)
	aBrowser.data(2, 123)
	print("data(1):", aBrowser.data(1))
	print("data(2):", aBrowser.data(2))
	print("data(3):", aBrowser.data(3))

