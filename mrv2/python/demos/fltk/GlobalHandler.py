# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.

#
# "$Id: GlobalHandler.py 493 2012-02-14 21:40:41Z andreasheld $"
#
# Global handler test program for pyFLTK the Python bindings
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


def EventHandler1(event):
    print("EventHandler1: ", event)
    if event == FL_SHORTCUT:
        print("Received FL_SHORTCUT", Fl.event_key())
        return 1
    else:
        return 0

def EventHandler2(event):
    print("EventHandler2: ", event)
    if event == FL_SHORTCUT:
        print("Received FL_SHORTCUT", Fl.event_key())
        return 1
    else:
        return 0

def CheckHandler1(data):
	print("CheckHandler1: ", data)

def CheckHandler2():
	print("CheckHandler2: no data")


cb1 = 0
cb2 = 0
cb3 = 0
cb4 = 0

def Handler1CB(widget):
    global cb1
    if cb1:
        widget.label("Add Handler1")
        Fl.remove_handler(EventHandler1)
        cb1 = 0
    else:
        widget.label("Remove Handler1")
        Fl.add_handler(EventHandler1)
        cb1 = 1

def Handler2CB(widget):
    global cb2
    if cb2:
        widget.label("Add Handler2")
        Fl.remove_handler(EventHandler2)
        cb2 = 0
    else:
        widget.label("Remove Handler2")
        Fl.add_handler(EventHandler2)
        cb2 = 1

def Check1CB(widget):
    global cb3
    if cb3:
        widget.label("Add Check1")
        Fl.remove_check(CheckHandler1)
        cb3 = 0
    else:
        widget.label("Remove Check1")
        Fl.add_check(CheckHandler1, "Check1")
        cb3 = 1

def Check2CB(widget):
    global cb4
    if cb4:
        widget.label("Add Check2")
        Fl.remove_check(CheckHandler2)
        cb4 = 0
    else:
        widget.label("Remove Check2")
        Fl.add_check(CheckHandler2)
        cb4 = 1

win = Fl_Window(100,150,300,200, "GloablHandler")
b1 = Fl_Button(20,50,120,30,"Add Handler1")
b1.callback(Handler1CB)
b2 = Fl_Button(160,50,120,30,"Add Handler2")
b2.callback(Handler2CB)

b3 = Fl_Button(20,100,120,30,"Add Check1")
b3.callback(Check1CB)
b4 = Fl_Button(160,100,120,30,"Add Check2")
b4.callback(Check2CB)

win.show()

    
    
