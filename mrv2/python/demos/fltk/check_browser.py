#
# "$Id: check_browser.py 536 2020-10-30 15:20:32Z andreasheld $"
#
# Check browser test program for pyFLTK the Python bindings
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
aCheckBrowser = None      # type 'Check_Browser' from '()'


def checkBrowserCallback(ptr):
    #cb = Fl_Check_BrowserPtr(ptr)  # code
    cb = ptr
    print(f"contains {cb.nitems()} items")
    for i in range(1, cb.nitems()+1):  # code
    	if cb.checked(i):  # code
    		print(cb.text(i)+" is checked")
    	else:  # code
    		print(cb.text(i)+" is not checked")


def onOK(ptr):
    import sys  # code
    checkBrowserCallback(aCheckBrowser)  # code
    window.hide()

def main():
    global aCheckBrowser

    o_1_0 = Fl_Window(394, 309, 245, 133, "check_browser.py")

    aCheckBrowser = Fl_Check_Browser(5, 5, 240, 75)
    aCheckBrowser.callback(checkBrowserCallback)
    aCheckBrowser.end()

    o_2_1 = Fl_Return_Button(160, 90, 70, 30, "OK")
    o_2_1.label('OK')
    o_2_1.callback(onOK)
    o_1_0.label('check_browser.py')
    o_1_0.end()
    aCheckBrowser.add("Guiness", 1)  # code
    aCheckBrowser.add("Bud")  # code
    aCheckBrowser.add("Coors")  # code
    aCheckBrowser.add("Grimbergen", 1)  # code
    aCheckBrowser.add("Burning River", 1)  # code
    aCheckBrowser.add("Little Kings")  # code

    return o_1_0



if __name__=='__main__':
    import sys
    window = main()
    window.show()