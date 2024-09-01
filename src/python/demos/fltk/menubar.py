# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.

#
# "$Id: menubar.py 536 2020-10-30 15:20:32Z andreasheld $"
#
# Menubar test program for pyFLTK the Python bindings
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

hugemenu = []

def window_cb(win):
	print("window callback called!")
	win.hide()

def test_cb(menu):
	m = menu.mvalue()
	if (not m):
		print("menu.mvalue()=NULL")
	else:
		if (m.shortcut()):
			print(f"{m.label()} - {fl_shortcut_label(m.shortcut())}")
		else:
			print(m.label())

def quit_cb(ptr, data):
	sys.exit(0)

# hack alert:
# nm - create Fl_Menu_Item from initializer list
def nm(list):
	m = Fl_Menu_Item()
	try:
		if not list[0]:
			m.text = Null
		else:
			m.text = list[0]
		m.shortcut_ =  list[1]
		m.callback_ = list[2]
		m.user_data = list[3]
		m.flags = list[4]
	except:
		pass
	return m


pulldown = []
pulldown.append(nm(("Red",	FL_ALT+ord('r'))))
pulldown.append(nm(("Green",	FL_ALT+ord('g'))))
pulldown.append(nm(("Blue",	FL_ALT+ord('b'))))
pulldown.append(nm(("Strange",	FL_ALT+ord('s'))))
pulldown.append(nm(("&Charm",	FL_ALT+ord('c'))))
pulldown.append(nm(("Truth",	FL_ALT+ord('t'))))
pulldown.append(nm(("Beauty",	FL_ALT+ord('b'))))
pulldown.append(nm((0,)))



WIDTH=600

menus = [0,0,0,0]

# turn MicroSoft style on/off
def button_cb(btn):
	if (btn.value() == 0):
		for i in range(4):
			menus[i].down_box(FL_FLAT_BOX) 
			menus[i].selection_color(137) 
			menus[i].textfont(FL_HELVETICA) 
	else:
		for i in range(4):
			menus[i].down_box(FL_NO_BOX) 
			menus[i].selection_color(FL_WHITE) 
			menus[i].textfont(FL_BOLD|FL_ITALIC) 
	menus[0].parent().redraw()

for i in range(0, 99, 1):
	hugemenu.append( nm(f"item {i}") )

window = Fl_Window(0,0, WIDTH, 400)

menubar = Fl_Menu_Bar(0,0,WIDTH,30)


pulldown= (  ("Red",	FL_ALT+ord('r')),
	     ("Green",	FL_ALT+ord('g')),
	     ("Blue",	FL_ALT+ord('b')),
	     ("Strange",FL_ALT+ord('s')),
	     ("&Charm",	FL_ALT+ord('c')),
	     ("Truth",	FL_ALT+ord('t')),
	     ("Beauty",	FL_ALT+ord('b')),
	     (None,))
 
menutable =  ( ("foo",0,0,0,FL_MENU_INACTIVE),
	("&File",0,0,0,FL_SUBMENU),
	("&Open", FL_ALT+ord('o'), 0, 0, FL_MENU_INACTIVE),
	("&Close", 0, 0),
	("&Quit", FL_ALT+ord('q'), quit_cb, 0, FL_MENU_DIVIDER),
	("shortcut",ord('a')),
	("shortcut",FL_SHIFT+ord('a')),
	("shortcut",FL_CTRL+ord('a')),
	("shortcut",FL_CTRL+FL_SHIFT+ord('a')),
	("shortcut",FL_ALT+ord('a')),
	("shortcut",FL_ALT+FL_SHIFT+ord('a')),
	("shortcut",FL_ALT+FL_CTRL+ord('a')),
	("shortcut",FL_ALT+FL_SHIFT+FL_CTRL+ord('a'), 0,0, FL_MENU_DIVIDER),
	("shortcut",ord('\r')),
	("shortcut",FL_CTRL+FL_Enter, 0,0, FL_MENU_DIVIDER),
	("shortcut",FL_F+1),
	("shortcut",FL_SHIFT+FL_F+1),
	("shortcut",FL_CTRL+FL_F+1),
	("shortcut",FL_SHIFT+FL_CTRL+FL_F+1),
	("shortcut",FL_ALT+FL_F+1),
	("shortcut",FL_ALT+FL_SHIFT+FL_F+1),
	("shortcut",FL_ALT+FL_CTRL+FL_F+1),
	("shortcut",FL_ALT+FL_SHIFT+FL_CTRL+FL_F+1, 0,0, FL_MENU_DIVIDER),
	("&Submenus", FL_ALT+ord('S'), 0, "Submenu1", FL_SUBMENU),
	("A very long menu item", 0),
	("&submenu",FL_CTRL+ord('S'), 0, "submenu2", FL_SUBMENU),
	("item 1", 0),
	("item 2", 0),
	("item 3", 0),
	("item 4", 0),
	(None, 0),
	("after submenu", 0),
	(None, 0),
	(None, 0),
	("&Edit",0,0,0,FL_SUBMENU),
	("Undo", FL_ALT+ord('z'), 0),
	("Redo", FL_ALT+ord('r'), 0, 0, FL_MENU_DIVIDER),
	("Cut", FL_ALT+ord('x'), 0),
	("Copy", FL_ALT+ord('c'), 0),
	("Paste", FL_ALT+ord('v'), 0),
	("Inactive",FL_ALT+ord('d'), 0, 0, FL_MENU_INACTIVE),
	("Clear", 0, 0, 0, FL_MENU_DIVIDER),
	("Invisible",FL_ALT+ord('e'), 0, 0, FL_MENU_INVISIBLE),
	("Preferences",0, 0),
	("Size", 0, 0),
	(None, 0),
	("&Checkbox",0,0,0,FL_SUBMENU),
	("&Alpha", FL_F+2, 0, 1, FL_MENU_TOGGLE),
	("&Beta", 0, 0, 2, FL_MENU_TOGGLE),
	("&Gamma", 0, 0, 3, FL_MENU_TOGGLE),
	("&Delta", 0, 0, 4, FL_MENU_TOGGLE|FL_MENU_VALUE),
	("&Epsilon",0, 0, 5, FL_MENU_TOGGLE),
	("&Pi", 0, 0, 6, FL_MENU_TOGGLE),
	("&Mu", 0, 0, 7, FL_MENU_TOGGLE|FL_MENU_DIVIDER),
	("Red", 0, 0, 1, FL_MENU_TOGGLE, 0, 0, 0, 1),
	("Black", 0, 0, 1, FL_MENU_TOGGLE|FL_MENU_DIVIDER),
	("00", 0, 0, 1, FL_MENU_TOGGLE),
	("000", 0, 0, 1, FL_MENU_TOGGLE),
	(None, 0),
	("&Radio",0,0,0,FL_SUBMENU),
	("&Alpha", 0, 0, 1, FL_MENU_RADIO),
	("&Beta", 0, 0, 2, FL_MENU_RADIO),
	("&Gamma", 0, 0, 3, FL_MENU_RADIO),
	("&Delta", 0, 0, 4, FL_MENU_RADIO|FL_MENU_VALUE),
	("&Epsilon",0, 0, 5, FL_MENU_RADIO),
	("&Pi", 0, 0, 6, FL_MENU_RADIO),
	("&Mu", 0, 0, 7, FL_MENU_RADIO|FL_MENU_DIVIDER),
	("Red", 0, 0, 1, FL_MENU_RADIO),
	("Black", 0, 0, 1, FL_MENU_RADIO|FL_MENU_DIVIDER),
	("00", 0, 0, 1, FL_MENU_RADIO),
	("000", 0, 0, 1, FL_MENU_RADIO),
	(None, 0),
	("&Font",0,0,0,FL_SUBMENU),
	("Normal", 0, 0, 0, 0, 0, 0, 14),
	("Bold", 0, 0, 0, 0, 0, FL_BOLD, 14),
	("Italic", 0, 0, 0, 0, 0, FL_ITALIC, 14),
	("BoldItalic",0,0,0, 0, 0, FL_BOLD+FL_ITALIC, 14),
	("Small", 0, 0, 0, 0, 0, FL_BOLD+FL_ITALIC, 10),
	("Emboss", 0, 0, 0, 0, FL_EMBOSSED_LABEL),
	("Engrave", 0, 0, 0, 0, FL_ENGRAVED_LABEL),
	("Shadow", 0, 0, 0, 0, FL_SHADOW_LABEL),
	("@->", 0, 0, 0, 0, FL_SYMBOL_LABEL),
	(None, 0),
	("E&mpty",0,0,0,FL_SUBMENU),
	(None, 0),
	("&Inactive", 0, 0, 0, FL_MENU_INACTIVE|FL_SUBMENU),
	("A very long menu item", 0),
	("A very long menu item", 0),
	(None, 0),
	("Invisible",0, 0, 0, FL_MENU_INVISIBLE|FL_SUBMENU),
	("A very long menu item", 0),
	("A very long menu item", 0),
	(None, 0),
#	("&Huge", 0, 0, hugemenu, FL_SUBMENU_POINTER),
	("button",0, 0, 0, FL_MENU_TOGGLE),
	(None,) )  

menubar.copy(menutable)
menubar.callback(test_cb)
menus[0] = menubar

mb1 = Fl_Menu_Button (50,100,120,25,"&menubutton")
mb1.text = "&menubutton"
mb1.copy(pulldown)
mb1.callback(test_cb)
menus[1] = mb1

ch = Fl_Choice (250,100,80,25,"&choice:")
ch.copy(pulldown)
ch.callback(test_cb)
menus[2] = ch

ic = Fl_Input_Choice(400,100,80,25,"input")
ic.add("item1")
ic.add("item2")
ic.add("item3")


mb = Fl_Menu_Button (0,0,WIDTH,400,"&popup")
mb.type(Fl_Menu_Button.POPUP3)
mb.copy(menutable)
mb.callback(test_cb)
menus[3] = mb

sp = Fl_Spinner(100, 200, 80, 25, "spinner")
sp.format("%.1f")
sp.maximum(113)
sp.minimum(101)
sp.step(0.5)
sp.value(101.5)
print("Min/Max = ", sp.minimum(), sp.maximum())

b = Fl_Box(200,200,200,100,"Press right button\nfor a pop-up menu")
t = Fl_Toggle_Button(250,50,150,25,"MicroSoft Style")
t.callback(button_cb)




window.resizable(mb)
window.size_range(300,400,0,400)
window.end()

window.show()


