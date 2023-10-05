#!/usr/bin/env python
# $Id: TextEditor.py 536 2020-10-30 15:20:32Z andreasheld $
# Author: Chris Green <cmg@dok.org>
# Purpose: test text editor
# Created: Thu Jan 22 14:06:24 EST 2004
#
# This has a goal of http://www.fltk.org/documentation.php/doc-1.1/editor.html#editor
#
# Text Editor test program for pyFLTK the Python bindings
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


import sys
from fltk14 import *

def onFileQuit(ptr, data):
    #menuItem = Fl_Menu_ItemPtr(ptr)
    menuItem = ptr
    print(f'onFileQuit({str(menuItem)}, "{str(data)}")')
    import sys  # code
    sys.exit(0)  # code


def onFileSave(ptr , data):
    #menuItem = Fl_Menu_ItemPtr(ptr)
    menuItem = ptr
    print(f'onFileSave({str(menuItem)}, "{str(data)}")')

    textEditor = data
    print(textEditor.buffer().text())

def tb_changed_cb(pos, nInserted, nDeleted, nRestyled, deleted_text, ptr):
    print("in callback: ", pos, nDeleted)

def main():

    o_1_0 = Fl_Window(810, 23, 200, 200)
    textEditor = Fl_Text_Editor(0,100, 100, 100)

    # create a 1000 byte long text buffer
    textBuffer = Fl_Text_Buffer(1000)
    textBuffer.text("Wee")

    # "hello world!"
    textBuffer.add_modify_callback(tb_changed_cb, textEditor);
    #textBuffer.call_modify_callbacks();

    # place the buffer inside the editor
    textEditor.buffer(textBuffer)

    o_2_0 = Fl_Menu_Bar(0, 0, 105, 25)

    userData = "User callback data"

    o_2_0.menu((
        ("&File", 0, 0, 0, 64), 
        ("&Quit", 0, onFileQuit, userData, 0),
        ("&Save", 0, onFileSave, textEditor, 0), 
        (None, ),
        (None, ) ))

    o_1_0.end()

    return o_1_0



if __name__=='__main__':
    window = main()
    window.show(len(sys.argv), sys.argv)
    Fl.run()
