#
# "$Id: delwin.py 531 2019-12-27 12:15:45Z andreasheld $"
#
# Delete window test program for pyFLTK the Python bindings
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
	_xxyzzy =	 o1_0 = Fl_Window(506, 641, 100, 100)
	o2_0 = Fl_Button(25, 25, 25, 25, "button")
	o2_0.callback(onB1)
	o1_0.end()
	return _xxyzzy

w = None
def onB1(ptr):
	global w
	if w == None:
		_xxyzzy =	 o1_0 = Fl_Window(196, 85, 100, 100, "222 open")
		o2_0 = Fl_Button(25, 25, 25, 25, "ok")
		o2_0.callback(onWin2DelButton)
		o1_0.end()
		o1_0.show()
		w = _xxyzzy
		return _xxyzzy
	else:
		return None

def onWin2DelButton(ptr):
	global w
	if w != None:
		w.thisown = 1
	w = None



def t():
	import sys
	window = main()
	window.show(len(sys.argv), sys.argv)
	for i in range(0, 100, 1):
		onB1(1)
		onWin2DelButton(1)
	onB1(1)
	Fl.run()

if __name__=='__main__':
	t()

