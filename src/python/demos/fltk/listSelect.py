# ListSelect test program for the Python port of
# the Fast Light Tool Kit (pyFLTK).
#
# Demonstrates the use of the contributed ListSelect widget.
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
# Please report all bugs and problems to "pyfltk-user@lists.sourceforge.net".
#
# "$Id: listSelect.py 536 2020-10-30 15:20:32Z andreasheld $"
#
# ListSelect test program for pyFLTK the Python bindings
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

	

window = Fl_Window(100, 100, 420, 300)

ls = ListSelect( 0, 0, 420, 300, " From:", " To:");

for t in range(0, 30, 1):
	ls.getTopBrowser().add(f"Item #{t}")

window.resizable(window.this)
window.end()
window.show()

