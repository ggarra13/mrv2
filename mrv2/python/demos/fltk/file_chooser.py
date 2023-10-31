# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.

#
# "$Id: file_chooser.py 495 2013-03-30 09:39:45Z andreasheld $"
#
# File chooser test program for pyFLTK the Python bindings
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

# globals
fc = None
filter = None
files = None
relative = ""

def close_callback(ptr):
        global window
        window.hide()
        window = None

def create_callback(ptr):
	global fc
	fc.type(fc.type() ^ Fl_File_Chooser.CREATE)

def dir_callback(ptr):
	global fc
	fc.type(fc.type() ^ Fl_File_Chooser.DIRECTORY)

def fc_callback(ptr):
	global fc
	#fc = Fl_File_Chooser(ptr)
	filename = fc.value()
	print("    filename = ", filename)

def multi_callback(ptr):
	global fc
	fc.type(fc.type() ^ Fl_File_Chooser.MULTI)

def pdf_check(name, header, headerlen):
	return None

def ps_check(name, header, headerlen):
	return None

def show_callback(ptr):
	global fc
	global filter
	global relative
	global files
	#fc.show()

	if filter.value() != None:
		#fc.filter(filter.value())
		fc.filter("*.*")

	fc.show()

	while fc.visible():
		Fl.wait()
	
	count = fc.count()
	#print "count = ", count
	#if count > 1:
	#	files.clear()
	#	i = 0
	#	if fc.value(i) != None:
	#	    (status, relative) = fl_filename_relative(relative, 1024, fc.value(i))
	#	    files.add(relative, None)	
	#	files.redraw()       
	if count > 0:
		files.clear()
		i = 1;
		while i <= count:
			if fc.value(i) != None:
			   (status,relative) = fl_filename_relative(relative, 1024, fc.value(i))
			   files.add(relative, Fl_File_Icon.find(fc.value(i),Fl_File_Icon.PLAIN))
			i = i+1
		files.redraw()


# Make the file chooser...
#Fl.scheme(0)
Fl_File_Icon.load_system_icons()
filter_string="*.*"

fc = Fl_File_Chooser(".", "*", Fl_File_Chooser.SINGLE, "Fl_File_Chooser Test");
#fc = Fl_File_Chooser("", filter_string, Fl_File_Chooser.SINGLE, "Fl_File_Chooser Test")
fc.callback(fc_callback)
fc.preview(0)

# Register the PS and PDF image types...
#Fl_Shared_Image.add_handler(pdf_check)
#Fl_Shared_Image.add_handler(ps_check)


#make the main window
window = Fl_Window(400, 200, "File Chooser Test")
filter = Fl_Input(50, 10, 310, 25, "Filter:")
filter.value(filter_string)
#filter.value("PDF Files (*.pdf)\t"
#                  "PostScript Files (*.ps)\t"
#		  "Image Files (*.{bmp,gif,jpg,png})\t"
#		  "C/C++ Source Files (*.{c,C,cc,cpp,cxx})")

b1 = Fl_Button(365, 10, 25, 25, "Test")
b1.labelcolor(FL_YELLOW);
b1.callback(show_callback);

icon   = Fl_File_Icon.find(".", Fl_File_Icon.DIRECTORY)
#print "icon = ", icon
if icon != None:
	icon.label(b1);

b2 = Fl_Light_Button(50, 45, 80, 25, "MULTI")
b2.callback(multi_callback)

b3 = Fl_Light_Button(140, 45, 90, 25, "CREATE")
b3.callback(create_callback);

b4 = Fl_Light_Button(240, 45, 115, 25, "DIRECTORY")
b4.callback(dir_callback);

files = Fl_File_Browser(50, 80, 340, 75, "Files:")
files.align(FL_ALIGN_LEFT);

b5 = Fl_Button(340, 165, 50, 25, "Close")
b5.callback(close_callback)

window.resizable(files)
window.end()
window.show()


