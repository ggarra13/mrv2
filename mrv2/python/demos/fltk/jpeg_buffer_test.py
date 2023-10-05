# Fl_JPEG_Image from memory buffer
# test program for pyFLTK the Python bindings
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

# cat image from https://commons.wikimedia.org/wiki/File:Arthur,_the_cat.jpg

from fltk14 import *
from PIL import Image
import io

def img_resize(fname,width):
    '''resizes any image type using high quality PIL library'''
    img = Image.open(fname) #opens all image formats supported by PIL
    w,h = img.size
    height = int(width*h/w)  #correct aspect ratio
    img = img.resize((width, height), Image.BICUBIC) #high quality resizing
    mem = io.BytesIO()  #byte stream memory object
    img.save(mem, format="JPEG") #converts image type to JPEG byte stream
    return Fl_JPEG_Image(None, mem.getbuffer())

pic = img_resize('cat.jpg', 300) #resizes to 300 pixels width
win = Fl_Window(pic.w(), pic.h(), 'PIL resizing')
win.begin()
box = Fl_Box(0, 0, pic.w(), pic.h())
win.end()

box.image(pic)

win.show()
Fl.run()


