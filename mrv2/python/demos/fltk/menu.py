#
# "$Id: menu.py 536 2020-10-30 15:20:32Z andreasheld $"
#
# Menu test program for pyFLTK the Python bindings
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


def onFileQuit(ptr, data):
    #menuItem = Fl_Menu_ItemPtr(ptr)
    menuItem = ptr
    print(f'onFileQuit({str(menuItem)}, "{str(data)}")')
    import sys  # code
    sys.exit(0)  # code
    

def main():

    o_1_0 = Fl_Window(810, 23, 100, 100)

    o_2_0 = Fl_Menu_Bar(0, 0, 105, 25)

    userData = "User callback data"
    #setMenu( o_2_0, (
    o_2_0.menu((
        ("&File", 0, 0, 0, 64), 
            ("&Quit", 0, onFileQuit, userData, 0), 
            (None, ),
        (None, ) ))

    o_1_0.end()

    return o_1_0



if __name__=='__main__':
    import sys
    window = main()
    window.show(len(sys.argv), sys.argv)
    Fl.run()
