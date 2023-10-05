# Another forms test program for the Python port of
# the Fast Light Tool Kit (pyFLTK).
# Modified to have 2 cubes to test multiple OpenGL contexts
#
# FLTk is Copyright 1998-2003 by Bill Spitzak and others.
# pyFLTK is Copyright 2003 by Andreas Held
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Library General Public
# License, version 2.0 as published by the Free Software Foundation.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Library General Public License for more details.
#
# You should have received a copy of the GNU Library General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
# USA.
#

from fltk14 import *
import sys, time

try:
	from OpenGL.GLUT import *
	from OpenGL.GL import *
except:
	print('''
ERROR: PyOpenGL not installed properly.  
		''')
	sys.exit()


# requires OpenGL

class cube_box(Fl_Gl_Window):
      lasttime = 0.0
      size = 0.0
      speed = 0.0
      wire = 0
      def __init__(self, x, y, w, h, l):
	      Fl_Gl_Window.__init__(self, x, y, w, h, l)
	      self.lasttime = 0.0

      def draw(self):
	      self.lasttime = self.lasttime+self.speed	
	      if self.valid() == 0:
		      glLoadIdentity()
		      glViewport(0,0,self.w(),self.h())
		      glEnable(GL_DEPTH_TEST)
		      glFrustum(-1,1,-1,1,2,10000)
		      glTranslatef(0,0,-10)
		      gl_font(FL_HELVETICA_BOLD, 16 )

	      try:
		      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)
		      glPushMatrix()
          
		      glRotatef(self.lasttime*1.6,0,0,1)
		      glRotatef(self.lasttime*4.2,1,0,0)
		      glRotatef(self.lasttime*2.3,0,1,0)
		      glTranslatef(-1.0, 1.2, -1.5);
		      glScalef(self.size,self.size,self.size)
		      drawcube(self.wire)
		      glPopMatrix()
		      gl_color(FL_GRAY)
		      glDisable(GL_DEPTH_TEST)
		      if self.wire == 1:
			      gl_draw("Cube: wire", -4.5, -4.5)
		      else:
			      gl_draw("Cube: flat", -4.5, -4.5)
		      glEnable(GL_DEPTH_TEST)
	      except:
		      print("Exception: ", sys.exc_info()[0], sys.exc_info()[1])

# the cube definition
v0 = [0.0, 0.0, 0.0]
v1 = [1.0, 0.0, 0.0]
v2 = [1.0, 1.0, 0.0]
v3 = [0.0, 1.0, 0.0]
v4 = [0.0, 0.0, 1.0]
v5 = [1.0, 0.0, 1.0]
v6 = [1.0, 1.0, 1.0]
v7 = [0.0, 1.0, 1.0]


def drawcube(wire):
    # draw a colored cube
    if wire == 1:
	    glBegin(GL_LINE_LOOP)
    else:
	    glBegin(GL_POLYGON)
    glColor3ub(0,0,255);
    glVertex3fv(v0)
    glVertex3fv(v1) 
    glVertex3fv(v2) 
    glVertex3fv(v3)
    glEnd()

    if wire == 1:
	    glBegin(GL_LINE_LOOP)
    else:
	    glBegin(GL_POLYGON)
    glColor3ub(0,255,255)
    glVertex3fv(v4)
    glVertex3fv(v5)
    glVertex3fv(v6)	
    glVertex3fv(v7)
    glEnd()
    if wire == 1:
	    glBegin(GL_LINE_LOOP)
    else:
	    glBegin(GL_POLYGON)
    glColor3ub(255,0,255)
    glVertex3fv(v0)
    glVertex3fv(v1)
    glVertex3fv(v5)	
    glVertex3fv(v4)
    glEnd()
    if wire == 1:
	    glBegin(GL_LINE_LOOP)
    else:
	    glBegin(GL_POLYGON)
    glColor3ub(255,255,0)
    glVertex3fv(v2)
    glVertex3fv(v3)
    glVertex3fv(v7)
    glVertex3fv(v6)
    glEnd()
    if wire == 1:
	    glBegin(GL_LINE_LOOP)
    else:
	    glBegin(GL_POLYGON)
    glColor3ub(0,255,0)
    glVertex3fv(v0)
    glVertex3fv(v4)
    glVertex3fv(v7)
    glVertex3fv(v3)
    glEnd()
    if wire == 1:
	    glBegin(GL_LINE_LOOP)
    else:
	    glBegin(GL_POLYGON)
    glColor3ub(255,0,0)
    glVertex3fv(v1)
    glVertex3fv(v2)
    glVertex3fv(v6)
    glVertex3fv(v5)
    glEnd()	


form = None
speed = None
size = None
button = None
wire = None
flat = None
cube = None
cube2 = None

class MyBox(Fl_Group):
      def __init__(self, type, x, y, w, h, l):
            Fl_Group.__init__(self, x, y, w, h, l)
            Fl_Group.box(self, type)
            self.end()

def exit_cb(ptr):
      sys.exit(0)

def makeform(name):
    global form
    global speed
    global size
    global wire
    global flat
    global button
    global cube
    global cube2
    form = Fl_Window(510+390,390,name)
    b1 = Fl_Box(FL_DOWN_FRAME,20,20,350,350,"")
    b2 = Fl_Box(FL_DOWN_FRAME,510,20,350,350,"")
    speed = Fl_Slider(FL_VERT_SLIDER,390,90,40,220,"Speed")
    size = Fl_Slider(FL_VERT_SLIDER,450,90,40,220,"Size")
    wire = Fl_Radio_Light_Button(390,20,100,30,"Wire")
    flat = Fl_Radio_Light_Button(390,50,100,30,"Flat")	
    button = Fl_Button(390,340,100,30,"Exit")
    button.callback(exit_cb)
    cube = cube_box(23,23,344,344, "")
    cube2 = cube_box(513,23,344,344, "")
    b = Fl_Box(FL_NO_BOX,cube.x(),size.y(), cube.w(),size.h(),"")
    #b = MyBox(FL_NO_BOX,cube.x(),size.y(), cube.w(),size.h(),"")
    form.resizable(b)
    b.hide()
    form.end()


if __name__ == '__main__':
   makeform(sys.argv[0])

   speed.bounds(4,0)
   cube.speed = 1.0
   cube2.speed = 1.0
   speed.value(1.0)
   size.bounds(4,0.01)
   cube.size = 1.0
   cube2.size = 1.0
   size.value(1.0)
   flat.value(1)
   cube.wire = 0
   cube2.wire = 1
   form.label("cube");

   form.show(len(sys.argv), sys.argv)
   cube.show()
   cube2.show()

   while 1:
	   if form.visible() == 1 and speed.value() != 0:
		   if Fl.check() == 0:
			   break
	   else:
		   if Fl.wait() == 0:
			   break

	   cube.wire = wire.value()
	   cube2.wire = not wire.value()
	   cube.size = size.value()
	   cube2.size = size.value()
	   cube.speed = speed.value()
	   cube2.speed = speed.value()
	   cube.redraw()
	   cube2.redraw()
	   if Fl.readqueue() == button:
		   break

   
