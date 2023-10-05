# "$Id: subwindow.py 535 2020-10-30 10:43:46Z andreasheld $"
#
# Subwindows test program for pyFLTK the Python bindings
# for the Fast Light Tool Kit (FLTK).
# Test to make sure nested windows work.
# Events should be reported for enter/exit and all mouse operations
# Buttons and pop-up menu should work, indicating that mouse positions
# are being correctly translated.
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
import sys, math


class EnterExit(Fl_Box):
	def __init__(self, x, y, w, h, l):
		Fl_Box.__init__(self,FL_BORDER_BOX,x,y,w,h,l)
		print(self)

#	def draw(self):
#		#Fl_Box.draw(self)
#		pass

	def handle(self, e):
		print("event = ", e)
		if e == FL_ENTER:
			try:
				print("FL_ENTER")
				self.color(FL_RED)
				self.redraw()
			except:
				print('uncaught!', sys.exc_info()[0], sys.exc_info()[1])
			return 1
		elif e == FL_LEAVE:
			try:
				print("FL_LEAVE")
				self.color(FL_GRAY)
				self.redraw()
			except:
				print('uncaught!', sys.exc_info()[0], sys.exc_info()[1])
			return 1
		else:
			return 0



class testwindow(Fl_Window):
	cx = 0
	cy = 0
	key = '0'
	def __init__(self, b, x, y, w, h, l):
		Fl_Window.__init__(self,x,y,w, h,l)
		self.box(b)
		key = 0

	def draw(self):
		print(self.label(), " :draw")
		Fl_Window.draw(self)
		if self.key:
			fl_draw(self.key, 1, self.cx, self.cy)

	def handle(self, e):
		if e != FL_MOVE:
			print(self.label())
		if Fl_Window.handle(self, e):
			return 1;
		if e == FL_FOCUS:
			return 1
		if e == FL_PUSH:
			Fl.focus(self.this)
			return 1
		if e == FL_KEYDOWN and Fl.event_text()[0]:
			self.key = Fl.event_text()[0]
			self.cx = Fl.event_x()
			self.y = Fl.event_y()
			self.redraw()
			return 1
		return 0



popup = None

bigmess = "this|is|only|a test"

window = testwindow(FL_UP_BOX,0,0,400,400,"outer")

tn1 = Fl_Toggle_Button(310,310,80,80,"&outer")

ee1 = EnterExit(10,310,80,80,"enterexit")

fi1 = Fl_Input(150,310,150,25,"input:")

mb1 = Fl_Menu_Button(5,150,80,25,"menu&1")
#setMenu(mb1, ( ("this",), ("is",), ("only",), ("a test",), (None,)))
mb1.menu(( ("this",), ("is",), ("only",), ("a test",), (None,)))
#mb1.add(bigmess)
subwindow = testwindow(FL_DOWN_BOX,100,100,200,200,"inner")
#subwindow = Fl_Window(FL_DOWN_BOX,100,100,200,200,"inner")
tn2 = Fl_Toggle_Button(310,310,80,80,"&outer")
subwindow.add(tn2)

ee2 = EnterExit(10,310,80,80,"enterexit")
subwindow.add(ee2)

fi2 = Fl_Input(150,310,150,25,"input:")
subwindow.add(fi2)

mb2 = Fl_Menu_Button(5,150,80,25,"menu&2")
#mb2.add(bigmess)
#setMenu(mb2, ( ("this",), ("is",), ("only",), ("a test",), (None,)))
mb2.menu(( ("this",), ("is",), ("only",), ("a test",), (None,)))
subwindow.end()
subwindow.resizable(subwindow)
window.resizable(subwindow)
window.add(subwindow)

fb = Fl_Box(FL_NO_BOX,0,0,400,100,
	     "A child Fl_Window with children of it's own may "
	     "be useful for imbedding controls into a GL or display "
	     "that needs a different visual.  There are bugs with the "
	     "origins being different between drawing and events, "
	     "which I hope I have solved."
	     )
fb.align(FL_ALIGN_WRAP)

popup = Fl_Menu_Button(0,0,400,400)
popup.type(Fl_Menu_Button.POPUP3)
#setMenu(popup, ( ("This",), ("is",), ("a popup",), ("menu",), ("this",), ("is",), ("only",), ("a test",), (None,)))
popup.menu(( ("This",), ("is",), ("a popup",), ("menu",), ("this",), ("is",), ("only",), ("a test",), (None,)))
#popup.add("This|is|a popup|menu")
#popup.add(bigmess)
window.add(fb)
window.add(popup)
window.end()

#window.show(len(sys.argv), sys.argv)
window.show()

print("rec = ", sys.getrecursionlimit())
#sys.setrecursionlimit(2000)

Fl.run()
