"""
//
// "$Id: browser.py 536 2020-10-30 15:20:32Z andreasheld $"
//
// Browser test program for pyFLTK the Python bindings
// for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-1999 by Bill Spitzak and others.
// pyFLTK copyright 2003 by Andreas Held and others.
//
// This library is free software you can redistribute it and/or
// modify it under the terms of the GNU Library General Public
// License, version 2.0 as published by the Free Software Foundation.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Library General Public License for more details.
//
// You should have received a copy of the GNU Library General Public
// License along with this library if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
// USA.
//
# Please report all bugs and problems to "pyfltk-user@lists.sourceforge.net".
#

/*
This is a test of how the browser draws lines.
This is a second line.
This is a third.

That was a blank line above this.

@r@_Right justify
@c@_Center justify
@_Left justify

@bBold text
@iItalic text
@b@iBold Italic
@fFixed width
@f@bBold Fixed
@f@iItalic Fixed
@f@i@bBold Italic Fixed
@lLarge
@l@bLarge bold
@sSmall
@s@bSmall bold
@s@iSmall italic
@s@i@bSmall italic bold
@uunderscore
@C1RED
@C2Green
@C4Blue

	You should try different browser types:
	Fl_Browser
	Fl_Select_Browser
	Fl_Hold_Browser
	Fl_Multi_Browser
*/
"""

from fltk14 import *
import sys

def b_cb(ptr):
	#print "callback, selection = %d, event_clicks = <not yet wrapped>"%(ptr.value())
	print(f"callback, selection = {ptr.value()}, event_clicks = {Fl.event_clicks()}")

def show_cb(ptr):
	if field.value() == '':
		fl_alert("Please enter a number in the text field\n"
			 "before clicking on the buttons.")
		return
	line = int(field.value())
	if str(ptr) == str(top):
    		browser.topline(line)
	elif str(ptr) == str(bottom):
    		browser.bottomline(line)
	elif str(ptr) == str(middle):
    		browser.middleline(line)
	else:
    		browser.make_visible(line)

MRV2_ROOT = os.path.join(sys.base_prefix, "..")
PYTHON_ROOT = os.path.join(MRV2_ROOT, "python")
DEMOS_ROOT = os.path.join(PYTHON_ROOT, "demos")
FLTK_ROOT = os.path.join(DEMOS_ROOT, "fltk")
fname = os.path.join(FLTK_ROOT, "browser.py")

window = Fl_Double_Window(400,400,fname)
#window.box(FL_NO_BOX) # because it is filled with browser

browser = Fl_Select_Browser(0,0,400,350,"")

browser.type(FL_MULTI_BROWSER)
#browser.color(44)
browser.callback(b_cb)
print("Browser font = ", browser.textfont())
print("Setting browser font to Courier")
browser.textfont(FL_COURIER)
print("Browser font size = ", browser.textsize())
print("Setting browser font size to 11")
browser.textsize(11)
print("Browser font color = ", browser.textcolor())
print("Setting browser font color to red")
browser.textcolor(FL_RED)

browser.scrollbar_right()
browser.has_scrollbar(Fl_Browser.BOTH_ALWAYS)

if not browser.load(fname):
	print("Can't load " +  fname)
else:

    browser.vposition(0)

    field = Fl_Int_Input(50,350,350,25,"Line #:")
    field.callback(show_cb)

    top = Fl_Button(0,375,100,25,"Top")
    top.callback(show_cb)

    bottom = Fl_Button(100, 375, 100, 25, "Bottom")
    bottom.callback(show_cb);
    
    middle = Fl_Button(200, 375, 100, 25, "Middle")
    middle.callback(show_cb);
    
    visible = Fl_Button(300, 375, 100, 25, "Make Vis.")
    visible.callback(show_cb);
    
    window.end()
    
    window.resizable(browser)
    window.show()
