# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.

#
# "$Id: preferences.py 495 2013-03-30 09:39:45Z andreasheld $"
#
# Preferences test program for pyFLTK the Python bindings
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

import sys, time
from fltk14 import *

myWindow = None
wAmPm = None

def closeWindowCB(widget):
    global myWindow
    myWindow.hide()
    myWindow = None

def saveAndCloseCB(widget):
    global myWindow
    writePrefs()
    myWindow.hide()    
    myWindow = None
    
def readPrefs():
    global wAmPm
    app = Fl_Preferences(Fl_Preferences.USER, "fltk.org", "test/preferences")
    #app = Fl_Preferences("C:/", "fltk.org", "test/preferences")

    #path = ""
    (status, path) = app.getUserdataPath(128)
    print("Path = ", path)

    bed = Fl_Preferences(app, "Bed")
    buffer = ""
    (status, buffer) = bed.get("alarm", "8:00", 79)
    wAlarm.value(buffer)

    (status, intValue) = bed.get("ampm", 0)
    wAmPm.value(intValue)

    (status, intValue) = bed.get("wear", 1)
    wWear.value(intValue)

    (status, side) = bed.get("side", 2)
    if side == 1:
        wLeft.value(1)
    elif side == 2:
        wRight.value(1)

    (status, tasks) = bed.get("taskFlags", 0x05)
    if tasks & 0x01:
        wShower.value( 1 )
    if tasks & 0x02:
        wShave.value( 1 )
    if tasks & 0x04:
        wBrush.value( 1 )

    eat = Fl_Preferences(app, "Breakfast")
    (status, intValue) = eat.get("drink", 1)
    wDrink.value(intValue)

    (status, boolValue) = eat.get("wMilk", 0 )
    wMilk.value(boolValue)

    (status, intValue) = eat.get("bread", 0 )
    wBread.value(intValue)

    (status, boolValue) = eat.get("wButter", 1 )
    wButter.value(boolValue)

    (status, intValue) = eat.get( "nEggs", 2 )
    buffer = str(intValue)
    wEggs.value(buffer)

    (status, doubleValue) = eat.get( "minutes", 3.2 )
    wMinutes.value(doubleValue)

    flexBuffer = ""
    (status, flexBuffer) = eat.get( "newspaper", "NY Tymes", 80)
    wPaper.value(flexBuffer);

    # destruct children first
    eat = None
    bed = None
    #app = None
    

def writePrefs():
    app = Fl_Preferences(Fl_Preferences.USER, "fltk.org", "test/preferences")
    #app = Fl_Preferences("C:/", "fltk.org", "test/preferences")
    bed = Fl_Preferences(app, "Bed" )
    
    bed.set( "alarm", wAlarm.value() )
    bed.set( "ampm", wAmPm.value() )
    bed.set( "wear", wWear.value() )

    side = 0
    if wLeft.value() != 0:
        side = 1
    if wRight.value() != 0:
        side = 2
    bed.set("side", side)

    tasks = 0;
    if wShower.value() != 0:
        tasks |= 0x01
    if wShave.value() != 0:
        tasks |= 0x02
    if wBrush.value() != 0:
        tasks |= 0x04
    bed.set("taskFlags", tasks)
    eat = Fl_Preferences(app, "Breakfast")
    eat.set("drink", wDrink.value() )
    eat.set("wMilk", wMilk.value() )
    eat.set("bread", wBread.value() )
    eat.set("wButter", wButter.value() )
    eat.set("nEggs", wEggs.value() )
    eat.set("minutes", wMinutes.value() )
    eat.set("newspaper", wPaper.value() )

    # destruct children first
    bed = None
    eat = None

    # app.flush()


menu_wAmPm = (("a.m.", 0,  0, 0, 0, 0, 0, 14, 56),
 ("p.m.", 0,  0, 0, 0, 0, 0, 14, 56),
 (None,))

menu_wWear = (("shoes", 0,  0, 0, 0, 0, 0, 14, 56),
 ("sandals", 0,  0, 0, 0, 0, 0, 14, 56),
 ("flip flops", 0,  0, 0, 0, 0, 0, 14, 56),
 ("bare foot", 0,  0, 0, 0, 0, 0, 14, 56),
              (None, ))

menu_wDrink = (("coffee", 0,  0, 0, 0, 0, 0, 14, 56),
 ("tea", 0,  0, 0, 0, 0, 0, 14, 56),
 ("juice", 0,  0, 0, 0, 0, 0, 14, 56),
 (None, ))

menu_wBread = (("wheat", 0,  0, 0, 0, 0, 0, 14, 56),
 ("white", 0,  0, 0, 0, 0, 0, 14, 56),
 ("rye", 0,  0, 0, 0, 0, 0, 14, 56),
 ("sour doh", 0,  0, 0, 0, 0, 0, 14, 56),
 (None, ))

myWindow = Fl_Double_Window(298, 311, "My Preferences")
#myWindow = Fl_Window(298, 311, "My Preferences")
myWindow.callback(closeWindowCB)
b1 = Fl_Button(210, 275, 75, 25, "Cancel")
b1.callback(closeWindowCB)
b2 = Fl_Button(125, 275, 75, 25, "OK")
b2.callback(saveAndCloseCB)

grp1 = Fl_Group(20, 30, 115, 225, "get Up:")
grp1.box(FL_ENGRAVED_BOX)
grp1.align(FL_ALIGN_TOP_LEFT)
wAlarm = Fl_Input(25, 55, 45, 20, "Alarm at:")
wAlarm.align(FL_ALIGN_TOP_LEFT)
wAmPm = Fl_Choice(75, 55, 55, 20)
wAmPm.down_box(FL_BORDER_BOX)
wAmPm.menu(menu_wAmPm)
wWear = Fl_Choice(25, 100, 105, 20, "Wear:")
wWear.down_box(FL_BORDER_BOX)
wWear.align(FL_ALIGN_TOP_LEFT)
wWear.menu(menu_wWear)
wLeft = Fl_Round_Button(35, 120, 95, 25, "left side")
wLeft.down_box(FL_ROUND_DOWN_BOX)
wRight = Fl_Round_Button(35, 140, 95, 25, "right side")
wRight.down_box(FL_ROUND_DOWN_BOX)

b3 = Fl_Box(38, 160, 95, 20, "of the bed")
wShower = Fl_Check_Button(25, 180, 105, 25, "shower")
wShower.down_box(FL_DOWN_BOX)
wShave = Fl_Check_Button(25, 200, 105, 25, "shave")
wShave.down_box(FL_DOWN_BOX);
wBrush = Fl_Check_Button(25, 220, 105, 25, "brush teeth")
wBrush.down_box(FL_DOWN_BOX);
grp1.end()

grp2 = Fl_Group(160, 30, 115, 225, "Breakfast::")
grp2.box(FL_ENGRAVED_FRAME)
grp2.align(FL_ALIGN_TOP_LEFT)
wDrink = Fl_Choice(165, 50, 105, 20, "Drink:")
wDrink.down_box(FL_BORDER_BOX)
wDrink.align(FL_ALIGN_TOP_LEFT)
wDrink.menu(menu_wDrink)
wMilk = Fl_Check_Button(170, 70, 100, 25, "with milk")
wMilk.down_box(FL_DOWN_BOX)
wBread = Fl_Choice(165, 110, 105, 20, "Bread:")
wBread.down_box(FL_BORDER_BOX)
wBread.align(FL_ALIGN_TOP_LEFT)
wBread.menu(menu_wBread)
wButter = Fl_Check_Button(170, 130, 100, 25, "with butter")
wButter.down_box(FL_DOWN_BOX)
wEggs = Fl_Input(165, 163, 30, 20, "eggs")
wEggs.type(2)
wEggs.align(FL_ALIGN_RIGHT)
wMinutes = Fl_Value_Slider(175, 185, 70, 20, "min.")
wMinutes.type(1)
wMinutes.minimum(2)
wMinutes.maximum(6)
wMinutes.value(3.1)
wMinutes.align(FL_ALIGN_RIGHT)
wPaper = Fl_Input(165, 225, 105, 20, "Newspaper:")
wPaper.align(FL_ALIGN_TOP_LEFT)
grp2.end();

myWindow.end()
readPrefs()
myWindow.show()

