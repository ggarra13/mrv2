#!/usr/bin/python
# -*- coding: utf-8 -*-


#
#
# UTF-8 test program for the Fast Light Tool Kit (FLTK).
#
# Copyright 1998-2009 by Bill Spitzak and others.
# pyFLTK copyright 2003 by Andreas Held and others.
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
# Please report all bugs and problems to "fltk-bugs@fltk.org".
#

from fltk14 import *
import sys, math

#
# Font chooser widget for the Fast Light Tool Kit(FLTK).
#


DEF_SIZE = 16 # default value for the font size picker

fnt_chooser_win = None
fontobj = None
sizeobj = None

fnt_cnt = None
refresh_btn = None
choose_btn = None
fix_prop = None
own_face = None

sizes = None
numsizes = None
pickedsize = DEF_SIZE
label = ""

main_win = None
thescroll = None
extra_font = None

font_count = 0
first_free = 0

label = None


class FontDisplay(Fl_Widget):
    def __init__(self, B, X, Y, W, H, L):
        Fl_Widget.__init__(self, X, Y, W, H, L)
        self.box(B)
        self.font = 0
        self.size = DEF_SIZE
        self.l = L

    def draw(self):
        global label
        fl_draw_box(self.box(), self.x(), self.y(), self.w(), self.h(), self.color())
        fl_font(self.font, self.size)
        fl_color(FL_BLACK)
        fl_draw(label, self.x()+3, self.y()+3, self.w()-6, self.h()-6, self.align())

    def test_fixed_pitch(self):
        w1 = 0
        w2 = 0
        h1 = 0
        h2 = 0
        fl_font(self.font, self.size)

        w1, h1 = fl_measure("MHMHWWMHMHMHM###WWX__--HUW", 0)
        w2, h2 = fl_measure("iiiiiiiiiiiiiiiiiiiiiiiiii", 0)

        if w1 == w2:
            return 1 # exact match - fixed pitch

	# Is the font "nearly" fixed pitch? If it is within 5%, say it is...
        f1 = w1*1.0
        f2 = w2*1.0
        delta = math.fabs(f1 - f2) * 5.0
        if delta <= f1:
            return 2 # nearly fixed pitch...
        return 0 # NOT fixed pitch



textobj = None


def size_cb(widget):
    global textobj
    
    size_idx = sizeobj.value()

    if size_idx == 0:
        return

    c = sizeobj.text(size_idx)

    # find the first numeric char
    i = 0
    while c[i] < '0' or c[i] > '9':
        i = i+1
    # convert the number string to a value
    pickedsize = int(c[i:])

    # Now set the font view to the selected size and redraw it.
    textobj.size = pickedsize
    textobj.redraw()


def font_cb(widget, param):
    global first_free, numsizes, sizes, sizeobj, textobj, fix_prop
    
    font_idx = fontobj.value() + first_free
    if font_idx == 0:
        return

    font_idx = font_idx-1
    textobj.font = font_idx
    sizeobj.clear()

    size_count = numsizes[font_idx-first_free]
    size_array = sizes[font_idx-first_free]
    if size_count == 0:
        # no preferred sizes - probably TT fonts etc...
        pass
    elif size_array[0] == 0:
        # many sizes, probably a scaleable font with preferred sizes
        j = 1
        i = 1
        while i <= 64 or i < size_array[size_count-1]:
            i = i+1
            buf = ""
            if j < size_count and i == size_array[j]:
                buf = f"@b{i}"
                j = j+1
            else:
                buf = f"{i}"
        
            sizeobj.add(buf)

        sizeobj.value(pickedsize)
    else:
        # some sizes, probably a font with a few fixed sizes available
        w = 0
        for i in range(size_count):
            # find the nearest available size to the current picked size
            if size_array[i] <= pickedsize:
                w = i

                buf = f"@b{size_array[i]}"
                sizeobj.add(buf)
            sizeobj.value(w+1)
    # force selection of nearest valid size, then redraw
    size_cb(sizeobj)

    # Now check to see if the font looks like a fixed pitch font or not...
    looks_fixed = textobj.test_fixed_pitch()
    if looks_fixed != 0:
        if looks_fixed > 1:
            fix_prop.value("near")
        else:
            fix_prop.value("fixed")

    else:
        fix_prop.value("prop")


def choose_cb(widget):
    global fontobj, first_free, sizeobj, main_win

    font_idx = fontobj.value()+first_free
    if font_idx == 0:
        print("No font chosen")
    else:
        font_idx = font_idx-1
        name, font_type = Fl.get_font_name(font_idx)
        print(f"idx {font_idx}\nUser name :{name}:")
        print(f"FLTK name :{Fl.get_font(font_idx)}:")

        Fl.set_font(extra_font, font_idx)

    size_idx = sizeobj.value()
    if size_idx == 0:
        print("No size selected")
    else:
        c = sizeobj.text(size_idx)

        # find the first numeric char
        i = 0
        while c[i] < '0' or c[i] > '9':
            i = i+1
        # convert the number string to a value
        pickedsize = int(c[i:])

    main_win.redraw()

def refresh_cb(widget):
    global main_win
    main_win.redraw()

def own_face_cb(widget):
    global first_free, fontobj, fnt_chooser_win, own_face, font_count
    
    cursor_restore = 0
    i_was = -1

    if i_was < 0:
        i_was = 1
    else:
        # record which was the topmost visible line
        i_was = fontobj.topline()
        fontobj.clear()
        # Populating the font widget can be slower than an old dog with three legs 
        # on a bad day, show a wait cursor
        fnt_chooser_win.cursor(FL_CURSOR_WAIT)
        cursor_restore = 1

    # Populate the font list with the names of the fonts found
    first_free = FL_FREE_FONT

    font_idx = first_free
    font_type = 0
    while font_idx < font_count:
        name, font_type = Fl.get_font_name(font_idx)
        font_idx = font_idx+1
        buf = ""

        if own_face.value() == 0:
            prefix=""
            if font_type&FL_BOLD:
                prefix = prefix+"@b"
            if font_type&FL_ITALIC:
                prefix = prefix+"@i"
            buf = prefix+"@."+name
        else:
            buf=f"@F{font_idx}@.{name}"
        fontobj.add(buf)

    fontobj.topline(i_was)
    if cursor_restore != 0:
        fnt_chooser_win.cursor(FL_CURSOR_DEFAULT)

def create_font_widget():
    global fontobj
    global sizeobj
    global fnt_cnt
    global fnt_chooser_win
    global refresh_btn
    global stat_bar
    global textobj, own_face, fix_prop , label
    
    fnt_chooser_win = Fl_Double_Window(380, 420, "Font Selector")

    label = "Font Sample\n"
    n = 0;
    c = ord(' ')+1
    while c < 127:
        if not c&0x1f:
            label = label+'\n'
        if chr(c)=='@':
            label = label+chr(c)
        label = label+chr(c)
        c = c+1
        
    label = label+'\n'

    c = 0xA1
    while c < 0x600:
        n = n+1
        if not n&0x1f:
            label = label+'\n'
        if sys.version > '3':
            label = label+str(chr(c).encode("utf-8"))
        else:
            label = label+unichr(c).encode("utf-8")
        #label = label+unichr(c)
        c = c+9
    #label = label+'\0'

    textobj = FontDisplay(FL_ENGRAVED_BOX, 10, 10, 360, 90, label)
    #
    #textobj = FontDisplay(FL_ENGRAVED_BOX, 10, 10, 360, 90, None)
    textobj.align(FL_ALIGN_TOP | FL_ALIGN_LEFT | FL_ALIGN_INSIDE | FL_ALIGN_CLIP);
    textobj.color(53, 3);

    fontobj = Fl_Hold_Browser(10, 110, 290, 270)
    fontobj.box(FL_ENGRAVED_BOX)
    fontobj.color(53, 3)
    fontobj.callback(font_cb, 0)
    fnt_chooser_win.resizable(fontobj)

    sizeobj = Fl_Hold_Browser(310, 110, 60, 270)
    sizeobj.box(FL_ENGRAVED_BOX)
    sizeobj.color(53, 3)
    sizeobj.callback(size_cb)

    # Create the status bar
    stat_bar = Fl_Group (10, 385, 380, 30)
    stat_bar.begin()
    
    fnt_cnt = Fl_Value_Output(10, 390, 40, 20)
    fnt_cnt.label("fonts")
    fnt_cnt.align(FL_ALIGN_RIGHT)

    fix_prop = Fl_Output(100, 390, 40, 20)
    fix_prop.color(FL_BACKGROUND_COLOR)
    fix_prop.value("prop")
    fix_prop.clear_visible_focus()
    
    own_face = Fl_Check_Button(150, 390, 40, 20, "Self")
    own_face.value(0)
    own_face.type(FL_TOGGLE_BUTTON)
    own_face.clear_visible_focus()
    own_face.callback(own_face_cb)
    own_face.tooltip("Display font names in their own face")
    
    dummy = Fl_Box(220, 390, 1, 1)
    
    choose_btn = Fl_Button(240, 385, 60, 30)
    choose_btn.label("Select")
    choose_btn.callback(choose_cb)
    
    refresh_btn = Fl_Button(310, 385, 60, 30)
    refresh_btn.label("Refresh")
    refresh_btn.callback(refresh_cb)
    
    stat_bar.end()
    stat_bar.resizable (dummy)
    
    fnt_chooser_win.end()


def make_font_chooser():
    global sizes, numsizes, font_count
    # create the widget frame
    create_font_widget()

    # Load the systems available fonts - ask for everything
    if sys.platform == 'win32':
        font_count = Fl.set_fonts("*")
    elif sys.platform == 'apple':
        font_count = Fl.set_fonts("*")
    else:
        # Load the systems available fonts - ask for everything that claims to be iso10646 compatible
        #font_count = Fl.set_fonts("-*-*-*-*-*-*-*-*-*-*-*-*-iso10646-1")
        font_count = Fl.set_fonts("*")

    # allocate space for the sizes and numsizes array, now we know how many entries it needs
    sizes = [None]*font_count
    numsizes = [0]*font_count

    # Populate the font list with the names of the fonts found
    first_free = FL_FREE_FONT;
    font_idx = first_free
    while font_idx < font_count:
        # Find out how many sizes are supported for each font face
        size_array = Fl.get_font_sizes(font_idx)
        size_count = len(size_array)
        numsizes[font_idx-first_free] = size_count
        if size_count != 0: # if the font has multiple sizes, populate the 2-D sizes array
            sizes[font_idx-first_free] = size_array

        font_idx = font_idx+1
    # end of font list filling loop

    # Call this once to get the font browser loaded up
    own_face_cb(None)

    fontobj.value(1)
    #	fontobj.textfont(261); # optional hard-coded font for testing - do not use!

    font_cb(fontobj, 0)
    
    fnt_cnt.value(font_count)
    
    return font_count



# Unicode Font display widget 

def box_cb(widget):
    global thescroll
    if widget.value() == 0:
        thescroll.box(FL_NO_BOX)
    else:
        thescroll.box(FL_DOWN_FRAME)
    thescroll.redraw()


class right_left_input(Fl_Input):
    def __init__(self, x, y, w, h):
        Fl_Input.__init__(self,x,y,w,h)

    def draw(self):
        pass
        if self.type() == FL_HIDDEN_INPUT:
            return
        b = self.box()
        if self.damage() & FL_DAMAGE_ALL:
            fl_draw_box(self.box(), self.x(), self.y(), self.w(), self.h(), self.color())
        self.drawtext(self.x()+Fl.box_dx(b)+3, self.y()+Fl.box_dy(b), self.w()-Fl.box_dw(b)-6, self.h()-Fl.box_dh(b))

    def drawtext(self, X, Y, W, H):
        fl_color(self.textcolor())
        fl_font(self.textfont(), self.textsize())
        fl_rtl_draw(self.value(), len(self.value()), X+W, Y+fl_height()-fl_descent())


def i7_cb(i7, i8):
    i = 0
    nb = "01234567"
    buf = ""

    ptr = i7.value()

    for i in range(len(ptr)):
        if ptr[i] < ' ' or ord(ptr[i]) > 126:
            buf.append('\\')
            buf.append("some unicode stuff here")
        else:
            if ptr[i] == '\\':
                buf.append('\\')
            buf.append(ptr[i])
            
    buf.append(chr(0));
    i8.value(buf)


class UCharDropBox(Fl_Output):
    def __init__(self, x, y, w, h, label):
        Fl_Output.__init__(self,x,y,w,h,label)

    def handle(self, event):
        if event == FL_DND_ENTER or event == FL_DND_DRAG or event == FL_DND_RELEASE:
            return 1
        elif event == FL_PASTE:
            t = Fl.event_text()
            n = fl_utf8decode(t, t)
            if n == 0:
                self.value("")
                return 1
            buffer = ""
            for i in range(n):
                buffer.append(t[i])
            buffer.append(' ')
            lut = "0123456789abcdef"
            for i in range(n):
                buffer.append('\\x')
                buffer.append(' some lut stuff here')
            buffer.append(chr(0))
            self.value(buffer)
            return 1
        return Fl_Output.handle(self, event)


if __name__ == '__main__':
        # If this file is saved as a UTF-8, the latin1 text in the comment 
        # below doesn't look right any more! 
        # Store the specific latin-1 byte values here... this should be equivalent to:
        latin1 = "\x41\x42\x43\x61\x62\x63\xe0\xe8\xe9\xef\xe2\xee\xf6\xfc\xe3\x31\x32\x33";
        utf8 = ""
        #l = fl_utf8froma(utf8, len(latin1)*5+1, latin1, len(latin1))

        font_count = make_font_chooser()
        extra_font = FL_TIMES_BOLD_ITALIC
        # setup the extra font
        if sys.platform == 'win32':
            Fl.set_font(extra_font, " Arial Unicode MS")
        elif sys.platform == 'darwin':
            Fl.set_font(extra_font, "Monaco")
        else:
            Fl.set_font(extra_font, "-*-*-*-*-*-*-*-*-*-*-*-*-iso10646-1")
        
        main_win = Fl_Double_Window (200 + 5*75, 400, "Unicode Display Test")
        main_win.begin()

        i1 = Fl_Input(5, 5, 190, 25)
        #utf8 = utf8+'\0'
        utf8 = latin1
        i1.value(utf8)
        scroll = Fl_Scroll(200,0,5 * 75,400)
        off = 2
        if len(sys.argv) > 1:
            off = off/16

        y = off

        while y < 0x10000/16:
            o = 0
            i = 16*y
            buf = ""
            for x in range(16):
                if sys.version > '3':
                    buf = buf+chr(i)
                else:
                    buf = buf+unichr(i)
                o = o+1
                i = i+1
            buf = buf+'\0'
            bu = f"0x{(y * 16):04X}"
            b = Fl_Input(200,(y-off)*25,60,25)
            b.value(bu)
            b = Fl_Input(260,(y-off)*25,400,25)
            b.textfont(extra_font)
            if sys.version >= '3':
                if y >= 0xd80 and y <= 0xdff:
                    # surrogate pairs not allowed
                    buf = "................"+'\0'
                b.value(buf)
            else:
                b.value(buf.encode("utf-8"))
            #b.value(buf)
            y = y+1
            
        main_win.resizable(scroll)
        scroll.end()

        thescroll = scroll

        i2 = Fl_Input(5, 35, 190, 25)
        utf8l = utf8.lower()
        i2.value(utf8l);


        i3 = Fl_Input(5, 65, 190, 25)
        utf8u = utf8l.upper()
        i3.value(utf8u)

        ltr_txt = "\\->e\xCC\x82=\xC3\xAA";
        i4 = Fl_Input(5, 90, 190, 25)
        i4.value(ltr_txt)
        i4.textfont(extra_font)

        r_to_l_txt =[ 1610, 1608, 1606, 1604, 1603, 1608, 1583, 0]

        r_l_buf = ""
        for i in r_to_l_txt:
            if sys.version >= '3':
                r_l_buf = r_l_buf+str(chr(i).encode("utf-8"))
            else:
                r_l_buf = r_l_buf+unichr(i).encode('utf-8')
        abuf = r_l_buf
        i5 = right_left_input(5, 115, 190, 50)
        i5.textfont(extra_font)
        i5.textsize(30)
        i5.value(abuf)

        i7 = Fl_Input(5, 230, 190, 25)
        i8 = Fl_Input(5, 260, 190, 25)
        i7.callback(i7_cb, i8)
        i7.textsize(20)
        i7.value(abuf)
        i7.when(FL_WHEN_CHANGED)

        r_to_l_txt1 = [1610, 0x20, 1608, 0x20, 1606, 0x20,  1604, 0x20, 1603, 0x20, 1608, 0x20, 1583, 0]

        r_l_buf = ""
        for i in r_to_l_txt1:
            if sys.version >= '3':
                r_l_buf = r_l_buf+str(chr(i).encode("utf-8"))
            else:
                r_l_buf = r_l_buf+unichr(i).encode('utf-8')
        abuf1 = r_l_buf
        i6 = right_left_input(5, 175, 190, 50)
        i6.textfont(extra_font)
        i6.textsize(30)
        i6.value(abuf1)
  
        # Now try Greg Ercolano's Japanese test sequence
        # SOME JAPANESE UTF8 TEXT
        # utfstr = "日本語というのは、とても難しい言語です。"
        utfstr = "\xe4\xbd\x95\xe3\x82\x82\xe8\xa1\x8c\xe3\x82\x8b\xe3\x80\x82"; 
        db = UCharDropBox(5, 300, 190, 30, "")
        db.textsize(16)
        db.value("unichar drop box")
		 
        o9 = Fl_Output(5, 330, 190, 45)
        o9.textfont(extra_font)
        o9.textsize(30)
        o9.value(utfstr)

        main_win.end()
        fl_set_status(0, 370, 100, 30)

    
        main_win.show()

        fnt_chooser_win.show()
    
        

