#
# "$Id: timeout.py 536 2020-10-30 15:20:32Z andreasheld $"
#
# Timeout test program for pyFLTK the Python bindings
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

numTimeoutsCalled = 1

def timeoutCallback(data):
	global numTimeoutsCalled
	print(f"timeoutCallback({str(data)})")
	Fl.repeat_timeout( 2.0, timeoutCallback, numTimeoutsCalled)
	numTimeoutsCalled = numTimeoutsCalled + 1
	if numTimeoutsCalled == 5:
		print("Removing timeoutCallbackNoData")
		Fl.remove_timeout(timeoutCallbackNoData)

def timeoutCallbackNoData():
	global numTimeoutsCalled
	print("timeoutCallbackNoData")
	Fl.repeat_timeout( 3.0, timeoutCallbackNoData)

def theCancelButtonCallback(ptr):
	sys.exit(0)
		
window = Fl_Window(100, 100, 200, 90, sys.argv[0])

button = Fl_Button(9,20,180,50,"OK")
button.labeltype(FL_NORMAL_LABEL)
button.callback(theCancelButtonCallback)

window.end()
window.show(len(sys.argv), sys.argv)

Fl.add_timeout( 2.0, timeoutCallback, "'this is the time-out data'")
Fl.add_timeout( 3.0, timeoutCallbackNoData)

Fl.run()
