# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.

#
# "$Id: idle.py 536 2020-10-30 15:20:32Z andreasheld $"
#
# Idle test program for pyFLTK the Python bindings
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

button = None # forward dec

timesCalled = 0
def anIdleCallback(data):
	global timesCalled
	timesCalled = timesCalled + 1
	print(f"idle data={data} times called={timesCalled}")

docb = 0
def theCancelButtonCallback(ptr):
	global docb
	if docb:
		Fl.remove_idle(anIdleCallback, 123)
		button.label("Add Idle Callback")
		docb = 0
	else:
		Fl.add_idle(anIdleCallback, 123)
		docb = 1
		button.label("Remove Idle Callback")

		
window = Fl_Window(100, 100, 200, 90, sys.argv[0])

button = Fl_Button(9,20,180,50,"Add Idle Callback")
button.labeltype(FL_NORMAL_LABEL)
button.callback(theCancelButtonCallback)

window.end()
window.show()
