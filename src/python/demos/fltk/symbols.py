# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.

#
# "$Id: symbols.py 495 2013-03-30 09:39:45Z andreasheld $"
#
# Symbols test program for pyFLTK the Python bindings
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

N=0
W=60
H=60
ROWS=5
COLS=5
window=None

def bt(name):
  global N
  x = N%COLS
  y = int(N/COLS)
  N = N+1
  x = x*W+10
  y = y*H+10
  b = Fl_Box(x,y,W-20,H-20,name)
  b.box(FL_UP_BOX)
  b.label(name)
  b.labeltype(FL_NORMAL_LABEL)
  b.labelcolor(FL_DARK3)
  return b
  
window = Fl_Window(COLS*W,ROWS*H+20)

b1 = bt("@->")
b2 = bt("@>")
b3 = bt("@>>")
b4 = bt("@>|")
b5 = bt("@>[]")
b6 = bt("@|>")
b7 = bt("@<-")
b8 = bt("@<")
b9 = bt("@<<")
b10 = bt("@|<")
b11 = bt("@[]<")
b12 = bt("@<|")
b13 = bt("@<->")
b14 = bt("@-->")
b15 = bt("@+")
b16 = bt("@->|")
b17 = bt("@||")
b18 = bt("@arrow")
b19 = bt("@returnarrow")
b20 = bt("@square")
b21 = bt("@circle")
b22 = bt("@line")
b23 = bt("@menu")
b24 = bt("@UpArrow")
b25 = bt("@DnArrow")

window.resizable(window.this)
window.show()


