#
# "$Id: unittests.py 28 2003-07-16 20:00:27Z andreasheld $"
#
# Unit tests for pyFLTK the Python bindings
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

window = None
point_test_ix = None
line_test_ix = None
rect_test_ix = None
viewport_test_ix = None
circle_test_ix = None


def changePageCB(ptr, ixvp):
    i = 0
    ix = ixvp
    n = window.children()
    for i in range(n):
        window.child(i).hide()
    if ix >= n or ix < 0:
        ix = n-1
    window.child(ix).show()

def newButton(x, y, w, h, l, ix, tt):
    b = Fl_Button(x, y, w, h, l)
    b.tooltip(tt)
    b.callback(changePageCB, ix)
    return b

def createMenuPage():
    page = Fl_Group(0, 0, 600, 600)
    g = Fl_Group(100, 20, 460, 26, "drawing:")
    g.align(FL_ALIGN_LEFT)
    b1 = newButton(100+2, 22, 22, 22, "1", point_test_ix, "Testing pixel drawing")
    b2 = newButton(125+2, 22, 22, 22, "2", line_test_ix, "Testing fl_line")
    b3 = newButton(150+2, 22, 22, 22, "3", rect_test_ix, "Testing fl_rect")
    b4 = newButton(175+2, 22, 22, 22, "4", viewport_test_ix, "Testing viewport alignment")
    b5 = newButton(200+2, 22, 22, 22, "5", circle_test_ix, "Testing circle and oval drawing")
    g.end()
    page.end()

def beginTestPage(l):
    ix = window.children()
    g = Fl_Group(0, 0, window.w(), window.h())
    g.box(FL_FLAT_BOX)
    g.hide()
    newButton(20, 20, 20, 20, "M", -1, "Return to main menu")
    newButton(20, 40, 20, 20, "@<", ix-1, "previous test")
    newButton(20, 60, 20, 20, "@>", ix+1, "next test")
    bx = Fl_Box(60, 20, window.w()-80, 100, l)
    bx.box(FL_ENGRAVED_BOX)
    bx.align(FL_ALIGN_INSIDE|FL_ALIGN_WRAP)
    return g

# ------- test the point drawing capabilities of this implementation ----------
class PointTest(Fl_Widget):
    def __init__(self, x, y, w, h):
        Fl_Widget.__init__(self, x, y, w, h)

    def draw(self):
        a = self.x()
        b = self.y()
        fl_color(FL_BLACK)
        fl_rect(self.x(), self.y(), self.w(), self.h())
        fl_point(a+10, b+10)
        fl_point(a+20, b+20)
        fl_point(a+10, b+20)
        fl_point(a+20, b+10)
        fl_color(FL_RED)
        a = self.x()+70
        fl_point(a+10, b+10)
        fl_point(a+20, b+20)
        fl_point(a+10, b+20)
        fl_point(a+20, b+10)
        fl_color(FL_GREEN)
        a = self.x()
        b = self.y()+70
        fl_point(a+10, b+10)
        fl_point(a+20, b+20)
        fl_point(a+10, b+20)
        fl_point(a+20, b+10)
        fl_color(FL_BLUE)
        a = self.x()+70
        fl_point(a+10, b+10)
        fl_point(a+20, b+20)
        fl_point(a+10, b+20)
        fl_point(a+20, b+10)

def fl_point_test():
    global point_test_ix
    point_test_ix = window.children()
    page = beginTestPage(
            """testing the fl_point call\n
            You should see four pixels each in black, red, green and blue. 
            Make sure that pixels are not anti-aliased (blured across multiple pixels)!"""  )
    pt = PointTest(20, 140, 100, 100)
    page.end()
    return pt

#------- test the line drawing capabilities of this implementation ----------
class LineTest(Fl_Widget):
    def __init__(self, x, y, w, h):
        Fl_Widget.__init__(self, x, y, w, h)

    def draw(self):
        a = self.x()
        b = self.y()
        fl_color(FL_BLACK)
        fl_rect(a, b, self.w(), self.h())
        # testing fl_xyline(x, y, x1)
        fl_color(FL_RED)
        fl_point(a+10, b+10)
        fl_point(a+20, b+10)
        fl_color(FL_BLACK)
        fl_xyline(a+10, b+10, a+20)
        # testing fl_xyline(x, y, x1, y2)
        fl_color(FL_RED)
        fl_point(a+10, b+20)
        fl_point(a+20, b+20)
        fl_point(a+20, b+30)
        fl_color(FL_BLACK)
        fl_xyline(a+10, b+20, a+20, b+30)
        # testing fl_xyline(x, y, x1, y2, x3)
        fl_color(FL_RED)
        fl_point(a+10, b+40)
        fl_point(a+20, b+40)
        fl_point(a+20, b+50)
        fl_point(a+30, b+50)
        fl_color(FL_BLACK)
        fl_xyline(a+10, b+40, a+20, b+50, a+30)
        #+++ add testing for the fl_yxline commands!
        # testing fl_loop(x,y, x,y, x,y, x, y)
        fl_color(FL_RED)
        fl_point(a+60, b+60)
        fl_point(a+90, b+60)
        fl_point(a+60, b+90)
        fl_point(a+90, b+90)
        fl_color(FL_BLACK)
        fl_loop(a+60, b+60, a+90, b+60, a+90, b+90, a+60, b+90)

def fl_line_test():
    global line_test_ix
    line_test_ix = window.children()
    page = beginTestPage(
        """testing the integer based fl_line calls
        No red pixels should be visible. 
        If you see bright red pixels, the line drawing alignment is off, 
        or the last pixel in a line does not get drawn. 
        If you see dark red pixels, anti-aliasing must be switched off."""
        )
    lt = LineTest(20, 140, 100, 100)
    page.end()
    return lt


#------- test the line drawing capabilities of this implementation ----------
class RectTest(Fl_Widget):
    def __init__(self, x, y, w, h):
        Fl_Widget.__init__(self, x, y, w, h)
        
    def draw(self):
        a = self.x()
        b = self.y()
        fl_color(FL_BLACK)
        fl_rect(a, b, self.w(), self.h())
        # testing fl_rect() with positive size
        fl_color(FL_RED)
        fl_loop(a+10, b+10, a+40, b+10, a+40, b+40, a+10, b+40)
        fl_color(FL_GREEN)
        fl_loop(a+ 9, b+ 9, a+41, b+ 9, a+41, b+41, a+ 9, b+41)
        fl_color(FL_GREEN)
        fl_loop(a+11, b+11, a+39, b+11, a+39, b+39, a+11, b+39)
        fl_color(FL_BLACK)
        fl_rect(a+10, b+10, 31, 31)
        # testing fl_rect() with positive size
        fl_color(FL_RED)
        fl_loop(a+60, b+60, a+90, b+60, a+90, b+90, a+60, b+90)
        fl_color(FL_GREEN)
        fl_loop(a+59, b+59, a+91, b+59, a+91, b+91, a+59, b+91)
        fl_color(FL_BLACK)
        fl_rectf(a+60, b+60, 31, 31)
  

def fl_rect_test():
    global rect_test_ix
    rect_test_ix = window.children()
    page = beginTestPage(
        "testing the fl_rect call\n"
        "No red pixels should be visible. "
        "If you see bright red lines, or if parts of the green frames are hidden, "
        "the rect drawing alignment is off. "
        )
    rt = RectTest(20, 140, 100, 100)
    page.end()
    return rt


#------- test the line drawing capabilities of this implementation ----------
class ViewportTest(Fl_Widget):
    def __init__(self, x, y, w, h, p):
        Fl_Widget.__init__(self, x, y, w, h),
        self.pos = p

    def draw(self):
        if self.pos&1:
            fl_color(FL_RED)
            fl_yxline(self.x()+self.w(), self.y(), self.y()+self.h())
            fl_color(FL_GREEN)
            fl_yxline(self.x()+self.w()-1, self.y(), self.y()+self.h())
        else:
            fl_color(FL_RED)
            fl_yxline(self.x()-1, self.y(), self.y()+self.h())
            fl_color(FL_GREEN)
            fl_yxline(self.x(), self.y(), self.y()+self.h())
            
        if self.pos&2:
            fl_color(FL_RED)
            fl_xyline(self.x(), self.y()+self.h(), self.x()+self.w())
            fl_color(FL_GREEN)
            fl_xyline(self.x(), self.y()+self.h()-1, self.x()+self.w())
        else:
            fl_color(FL_RED)
            fl_xyline(self.x(), self.y()-1, self.x()+self.w())
            fl_color(FL_GREEN)
            fl_xyline(self.x(), self.y(), self.x()+self.w())
            
        fl_color(FL_BLACK)
        fl_loop(self.x()+3, self.y()+3, self.x()+self.w()-4, self.y()+3, self.x()+self.w()-4, self.y()+self.h()-4, self.x()+3, self.y()+self.h()-4)
  

def fl_viewport_test():
    global viewport_test_ix
    viewport_test_ix = window.children()
    page = beginTestPage(
        "testing viewport alignment\n"
        "Only green lines should be visible. "
        "If red lines are visible in the corners of this window, "
        "your viewport alignment and clipping is off. "
        "If there is a space between the green lines and the window border, "
        "the viewport is off, but some clipping may be working. "
        "Also, your window size may be off to begin with."
        )
    vt1 = ViewportTest(0, 0, 20, 20, 0)
    vt2 = ViewportTest(page.w()-20, 0, 20, 20, 1)
    vt3 = ViewportTest(0, page.h()-20, 20, 20, 2)
    vt4 = ViewportTest(page.w()-20,page.h()-20, 20, 20, 3)
    page.end()
    return (vt1, vt2, vt3, vt4)


#------- test the circle drawing capabilities of this implementation ----------
class CircleTest(Fl_Widget):
    def __init__(self, x, y, w, h):
        Fl_Widget.__init__(self, x, y, w, h)
        
    def draw(self):
        a = self.x()
        b = self.y()
        fl_color(FL_BLACK)
        fl_rect(a, b, 100, 100)
        # test fl_arc for full circles
        fl_color(FL_GREEN)
        fl_rect(a+ 9, b+ 9, 33, 33)
        fl_color(FL_RED)
        fl_xyline(a+24, b+10, a+27)
        fl_xyline(a+24, b+40, a+27)
        fl_yxline(a+10, b+24, b+27)
        fl_yxline(a+40, b+24, b+27)
        fl_color(FL_BLACK)
        fl_arc(a+10, b+10, 31, 31, 0.0, 360.0)
        # test fl_arc segmet 
        fl_color(FL_GREEN)
        fl_rect(a+54, b+ 4, 43, 43)
        fl_rect(a+54, b+4, 18, 18)
        fl_rect(a+79, b+29, 18, 18)
        fl_color(FL_RED)
        fl_point(a+55, b+30)
        fl_point(a+70, b+45)
        fl_point(a+80, b+5)
        fl_point(a+95, b+20)
        fl_color(FL_BLACK)
        fl_arc(a+65, b+ 5, 31, 31, -35.0, 125.0)
        # test fl_arc segmet 2
        fl_color(FL_BLACK)
        fl_arc(a+55, b+15, 31, 31, 145.0, 305.0)
        # test fl_pie for full circles
        fl_color(FL_RED)
        fl_xyline(a+24, b+60, a+27)
        fl_xyline(a+24, b+90, a+27)
        fl_yxline(a+10, b+74, b+77)
        fl_yxline(a+40, b+74, b+77)
        fl_color(FL_GREEN)
        fl_rect(a+ 9, b+59, 33, 33)
        fl_color(FL_BLACK)
        fl_pie(a+10, b+60, 31, 31, 0.0, 360.0)
        # test fl_pie segmet 1
        fl_color(FL_GREEN)
        fl_rect(a+54, b+54, 43, 43)
        fl_rect(a+54, b+54, 18, 18)
        fl_rect(a+79, b+79, 18, 18)
        fl_point(a+79, b+71)
        fl_point(a+71, b+79)
        fl_color(FL_RED)
        fl_point(a+55, b+80)
        fl_point(a+70, b+95)
        fl_point(a+80, b+55)
        fl_point(a+95, b+70)
        fl_point(a+81, b+69)
        fl_point(a+69, b+81)
        fl_color(FL_BLACK)
        fl_pie(a+65, b+55, 31, 31, -30.0, 120.0)
        # test fl_pie segmet 2
        fl_color(FL_BLACK)
        fl_pie(a+55, b+65, 31, 31, 150.0, 300.0)
        # --- oval testing (horizontal squish)
        a = self.x()+120
        b = self.y()
        fl_color(FL_BLACK)
        fl_rect(a, b, 100, 100)
        fl_color(FL_GREEN)
        fl_rect(a+19, b+9, 63, 33)
        fl_rect(a+19, b+59, 63, 33)
        fl_color(FL_BLACK)
        fl_arc(a+20, b+10, 61, 31, 0, 360)
        fl_pie(a+20, b+60, 61, 31, 0, 360)
        # --- oval testing (horizontal squish)
        a = self.x()+240
        b = self.y()
        fl_color(FL_BLACK)
        fl_rect(a, b, 100, 100)
        fl_color(FL_GREEN)
        fl_rect(a+9, b+19, 33, 63)
        fl_rect(a+59, b+19, 33, 63)  
        fl_color(FL_BLACK)
        fl_arc(a+10, b+20, 31, 61, 0, 360)
        fl_pie(a+60, b+20, 31, 61, 0, 360)
  

def fl_circle_test():
    global circle_test_ix
    circle_test_ix = window.children()
    page = beginTestPage(
        "testing int drawing of circles and ovals (fl_arc, fl_pie)\n"
        "No red lines should be visible. "
        "If you see bright red pixels, the circle drawing alignment is off. "
        "If you see dark red pixels, your syste supports anti-aliasing "
        "which should be of no concern. "
        "The green rectangles should not be touched by circle drawings."
        )
    ct = CircleTest(20, 140, 340, 100)
    page.end()
    return ct


if __name__=='__main__':
  window = Fl_Window(600, 600, "Unit Tests for FLTK")
  # --- list all tests
  pt = fl_point_test()
  lt = fl_line_test()
  rt = fl_rect_test()
  (vt1, vt2, vt3, vt4) = fl_viewport_test()
  ct = fl_circle_test()
  # --- last page is the menu
  createMenuPage()
  window.end()
  window.show(sys.argv)
  Fl.run()



  


