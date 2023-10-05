# timeout bug test program for pyFLTK the Python bindings
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

'''
Hi all,
I'm new to this mailing list, but we've been using Fltk for about 15 years at my work and pyFltk to a lesser extent and more recently (for 5+ years). We started using pyFltk for some more serious development very recently and have found a few small bugs. Today, I'm sending over a patch for the remove_timeout behavior, to make it more compliant with standard Fltk behavior. Hopefully we've got this right here, but please let me know if we did something wrong.

We've also attached a test program (test_timeout.py) that demonstrates the bugs this patch attempts to fix.

This program tests pyFltk for compliance with Fltk remove_timeout behavior.
pyFltk 1.3.3 and previous have bugs in remove_timeout when:
	timeout callback is a member function of a class 
		FIX: function comparison must not be a C-pointer comparison but rather a python comparison
	data pointer in C ("argp") is ignored for pyFltk remove_timeout
    	  FIX: do Python comparison to check that passed data matches
      remove_timeout in Fltk 1.3.3 docs says that it removes all matching timeouts, not just the last one
    	  FIX: keep looping after finding first match
      ALSO FIXED: DECREF is in wrong place

This program sets up 4 different timeouts with 3 different data values passed to a member function timeout callback. Data = (1,30) is used twice. 
remove_timeout is called on data=(1,30) and both timeouts should be removed according to the Fltk docs.
Running with pyFltk 1.3.3, none of the timeouts is removed. With the supplied patch, the two (1,30) timeouts are removed.
'''

import time

import fltk
print('fltk.__file__ = {!r}'.format( fltk.__file__ ))

win = fltk.Fl_Window( 640, 480, 'Timeout test' )
win.show()

class TestClass( object ):
	def __init__( self ):
		self.x = 5
		self.y = 1
		
	def timeout_cb( self, data ):
		print('timeout called: x={}, y={}, data = {!r}'.format( self.x, self.y, data ))
		fltk.Fl.repeat_timeout( 1, self.timeout_cb, data )
		
tc = TestClass()	
fltk.Fl.add_timeout( 1, tc.timeout_cb, ( 1, 31 ) )
fltk.Fl.add_timeout( 1, tc.timeout_cb, ( 1, 30 ) )
fltk.Fl.add_timeout( 1, tc.timeout_cb, ( 1, 30 ) )
fltk.Fl.add_timeout( 1, tc.timeout_cb, ( 1, 29 ) )

print('waiting....')

startTime = time.time()
while time.time() < startTime+5:
	fltk.Fl.wait( 5 - ( time.time() - startTime ) )

print('removing timeout... (1, 30)')
fltk.Fl.remove_timeout( tc.timeout_cb, ( 1, 30 ) )
fltk.Fl.run()
