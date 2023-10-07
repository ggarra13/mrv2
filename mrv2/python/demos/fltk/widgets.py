#
# "$Id: widgets.py 28 2003-07-16 20:00:27Z andreasheld $"
#
# Widgets test program for pyFLTK the Python bindings
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

window = None

def theExitCallback(ptr):
    global window
    window.hide()
    window = None

data = [ "@c Hello there.",
         "This is a demo of some of the widgets pyFLTK can do",
         "@s Unfortunately, that's about all it does...",
         "@b Because now the darn font attributes work differently"
       ]

window = Fl_Window(100, 100, 400, 200, "widgets.py")

browser = Fl_Browser(10, 10, 380, 100)
for line in data:
    browser.add(line)

browser.data(3, "Fortunately")
#print browser.get_data(3)

slider = Fl_Hor_Value_Slider(10, 120, 180, 15)

button = Fl_Button(200, 120, 190, 15, "Exit")
button.callback(theExitCallback)

output = Fl_Output(10, 150, 380, 20)
output.value("Label's don't work either. Need to typemap some more.")

window.end()
#window.resizable(window.this)
window.show()


