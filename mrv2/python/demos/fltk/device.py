#
# "$Id: device.py 456 2009-09-25 13:35:05Z andreasheld $"
#
# Test program for pyFLTK the Python bindings
# for the Fast Light Tool Kit (FLTK).
#
# FLTK copyright 1998-1999 by Bill Spitzak and others.
# pyFLTK copyright 2011 by Andreas Held and others.
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

porsche_xpm=[
"64 64 4 1",
" 	c #background",
".	c #000000000000",
"X	c #ffd100",
"o	c #FFFF00000000",
"                                                                ",
"                   ..........................                   ",
"              .....................................             ",
"        ............XXXXXXXXXXXXXXXXXXXXXXXX............        ",
"        ......XXXXXXX...XX...XXXXXXXX...XXXXXXXXXX......        ",
"        ..XXXXXXXXXX..X..XX..XXXX.XXXX..XXXXXXXXXXXXXX..        ",
"        ..XXXXXXXXXX..X..XX..XXX..XXXX..X...XXXXXXXXXX..        ",
"        ..XXXXXXXXXX..XXXXX..XX.....XX..XX.XXXXXXXXXXX..        ",
"        ..XXXXXXXXX.....XXX..XXX..XXXX..X.XXXXXXXXXXXX..        ",
"        ..XXXXXXXXXX..XXXXX..XXX..XXXX....XXXXXXXXXXXX..        ",
"        ..XXXXXXXXXX..XXXXX..XXX..XXXX..X..XXXXXXXXXXX..        ",
"        ..XXXXXXXXXX..XXXXX..XXX..X.XX..XX..XXXXXXXXXX..        ",
"        ..XXXXXXXXX....XXX....XXX..XX....XX..XXXXXXXXX..        ",
"        ..XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX..        ",
"        ..XXXXXXXXX..........................XXXXXXXXX..        ",
"        ..XXX.......XXXXXXXXXXX...................XXXX..        ",
"        ......XX.XXX.XXX..XXXXX.........................        ",
"        ..XXXXX.XXX.XXX.XXXX.XX.........................        ",
"        ..XXXX.XXX.XX.......XXX.........................        ",
"        ..XXXX.......XXXXXX..XX..ooooooooooooooooooooo..        ",
"        ..X.....XXXXXXXXXXXXXXX..ooooooooooooooooooooo..        ",
"        ..X...XXXXXXXXXXXXXXXXX..ooooooooooooooooooooo..        ",
"        ..X..XXXXXXX.XX.XXXXXXX..ooooooooooooooooooooo..        ",
"        ..XXXXX.XXX.XX.XXXXXXXX..ooooooooooooooooooooo..        ",
"        ..XXXX.XXX.XX.XX................................        ",
"        ..XXXX.X.........X....X.X.X.....................        ",
"        ..XXXX...XXXXXXX.X..X...X.X.X.X.................        ",
"        ..X....XXXXXXXXXX.X...X.X.X.....................        ",
"        ..X...XXXXXXXXXX.XXXXXXXXXXXXXX.................        ",
"        ..X..XXXXXX.XX.X.XXX...XXXXXXXX.................        ",
"        ..XXXXX.XX.XX.XX.XX.....XXXXXXX.oooooooooooooo..        ",
"        ..XXXX.XX.XX.XX..XX.X...XXXXX.X.oooooooooooooo..        ",
"        ..XXXX.X.......X.XXXX...XXXX..X.oooooooooooooo..        ",
"        ..X......XXXXXX..XXXX...XXXX..X.oooooooooooooo..        ",
"        ..X...XXXXXXXXXX.XXX.....XXX.XX.oooooooooooooo..        ",
"        ..X..XXXXXXXXXXX.X...........XX.oooooooooooooo..        ",
"        .................X.X.........XX.................        ",
"        .................X.X.XXXX....XX.XXXXXXXXXXXXXX..        ",
"        .................XXX.XXXXX.X.XX.XXX.XX.XXXXXXX..        ",
"         ................XXXX.XXX..X..X.XX.XX.XXX.XXX..         ",
"         ................XXXXXXXX.XX.XX.X.XX.XXX.XXXX..         ",
"         .................XXXXXX.XX.XX.X..........XXX..         ",
"          ..oooooooooooooo.XXXXXXXXXX....XXXXXXXX..X..          ",
"          ..ooooooooooooooo.XXXXXXXX....XXXXXXXXXXXX..          ",
"           ..ooooooooooooooo........XXXXXXX.XX.XXXX..           ",
"           ..oooooooooooooooooo..XXXXX.XXX.XX.XX.XX..           ",
"            ..ooooooooooooooooo..XXXX.XXX.XX.XX.XX..            ",
"            ..ooooooooooooooooo..XXX.XX........XXX..            ",
"             ....................XXX....XXXXXX..X..             ",
"              ...................XX...XXXXXXXXXXX.              ",
"              ...................X...XXXXXXXXXXX..              ",
"               ..................X..XXXX.XXXXXX..               ",
"                .................XXX.XX.XX.XXX..                ",
"                 ................XX.XX.XX.XXX..                 ",
"                  ..ooooooooooo..XX.......XX..                  ",
"                   ..oooooooooo..X...XXXX.X..                   ",
"                    ..ooooooooo..X..XXXXXX..                    ",
"                     ...ooooooo..X..XXXX...                     ",
"                      ....ooooo..XXXXX....                      ",
"                        ....ooo..XXX....                        ",
"                          ....o..X....                          ",
"                            ........                            ",
"                              ....                              ",
"                                                                "]


sorceress_width = 75
sorceress_height = 75
sorceress_bits = [
   0xfc, 0x7e, 0x40, 0x20, 0x90, 0x00, 0x07, 0x80, 0x23, 0x00, 0x00, 0xc6,
   0xc1, 0x41, 0x98, 0xb8, 0x01, 0x07, 0x66, 0x00, 0x15, 0x9f, 0x03, 0x47,
   0x8c, 0xc6, 0xdc, 0x7b, 0xcc, 0x00, 0xb0, 0x71, 0x0e, 0x4d, 0x06, 0x66,
   0x73, 0x8e, 0x8f, 0x01, 0x18, 0xc4, 0x39, 0x4b, 0x02, 0x23, 0x0c, 0x04,
   0x1e, 0x03, 0x0c, 0x08, 0xc7, 0xef, 0x08, 0x30, 0x06, 0x07, 0x1c, 0x02,
   0x06, 0x30, 0x18, 0xae, 0xc8, 0x98, 0x3f, 0x78, 0x20, 0x06, 0x02, 0x20,
   0x60, 0xa0, 0xc4, 0x1d, 0xc0, 0xff, 0x41, 0x04, 0xfa, 0x63, 0x80, 0xa1,
   0xa4, 0x3d, 0x00, 0x84, 0xbf, 0x04, 0x0f, 0x06, 0xfc, 0xa1, 0x34, 0x6b,
   0x01, 0x1c, 0xc9, 0x05, 0x06, 0xc7, 0x06, 0xbe, 0x11, 0x1e, 0x43, 0x30,
   0x91, 0x05, 0xc3, 0x61, 0x02, 0x30, 0x1b, 0x30, 0xcc, 0x20, 0x11, 0x00,
   0xc1, 0x3c, 0x03, 0x20, 0x0a, 0x00, 0xe8, 0x60, 0x21, 0x00, 0x61, 0x1b,
   0xc1, 0x63, 0x08, 0xf0, 0xc6, 0xc7, 0x21, 0x03, 0xf8, 0x08, 0xe1, 0xcf,
   0x0a, 0xfc, 0x4d, 0x99, 0x43, 0x07, 0x3c, 0x0c, 0xf1, 0x9f, 0x0b, 0xfc,
   0x5b, 0x81, 0x47, 0x02, 0x16, 0x04, 0x31, 0x1c, 0x0b, 0x1f, 0x17, 0x89,
   0x4d, 0x06, 0x1a, 0x04, 0x31, 0x38, 0x02, 0x07, 0x56, 0x89, 0x49, 0x04,
   0x0b, 0x04, 0xb1, 0x72, 0x82, 0xa1, 0x54, 0x9a, 0x49, 0x04, 0x1d, 0x66,
   0x50, 0xe7, 0xc2, 0xf0, 0x54, 0x9a, 0x58, 0x04, 0x0d, 0x62, 0xc1, 0x1f,
   0x44, 0xfc, 0x51, 0x90, 0x90, 0x04, 0x86, 0x63, 0xe0, 0x74, 0x04, 0xef,
   0x31, 0x1a, 0x91, 0x00, 0x02, 0xe2, 0xc1, 0xfd, 0x84, 0xf9, 0x30, 0x0a,
   0x91, 0x00, 0x82, 0xa9, 0xc0, 0xb9, 0x84, 0xf9, 0x31, 0x16, 0x81, 0x00,
   0x42, 0xa9, 0xdb, 0x7f, 0x0c, 0xff, 0x1c, 0x16, 0x11, 0x00, 0x02, 0x28,
   0x0b, 0x07, 0x08, 0x60, 0x1c, 0x02, 0x91, 0x00, 0x46, 0x29, 0x0e, 0x00,
   0x00, 0x00, 0x10, 0x16, 0x11, 0x02, 0x06, 0x29, 0x04, 0x00, 0x00, 0x00,
   0x10, 0x16, 0x91, 0x06, 0xa6, 0x2a, 0x04, 0x00, 0x00, 0x00, 0x18, 0x24,
   0x91, 0x04, 0x86, 0x2a, 0x04, 0x00, 0x00, 0x00, 0x18, 0x27, 0x93, 0x04,
   0x96, 0x4a, 0x04, 0x00, 0x00, 0x00, 0x04, 0x02, 0x91, 0x04, 0x86, 0x4a,
   0x0c, 0x00, 0x00, 0x00, 0x1e, 0x23, 0x93, 0x04, 0x56, 0x88, 0x08, 0x00,
   0x00, 0x00, 0x90, 0x21, 0x93, 0x04, 0x52, 0x0a, 0x09, 0x80, 0x01, 0x00,
   0xd0, 0x21, 0x95, 0x04, 0x57, 0x0a, 0x0f, 0x80, 0x27, 0x00, 0xd8, 0x20,
   0x9d, 0x04, 0x5d, 0x08, 0x1c, 0x80, 0x67, 0x00, 0xe4, 0x01, 0x85, 0x04,
   0x79, 0x8a, 0x3f, 0x00, 0x00, 0x00, 0xf4, 0x11, 0x85, 0x06, 0x39, 0x08,
   0x7d, 0x00, 0x00, 0x18, 0xb7, 0x10, 0x81, 0x03, 0x29, 0x12, 0xcb, 0x00,
   0x7e, 0x30, 0x28, 0x00, 0x85, 0x03, 0x29, 0x10, 0xbe, 0x81, 0xff, 0x27,
   0x0c, 0x10, 0x85, 0x03, 0x29, 0x32, 0xfa, 0xc1, 0xff, 0x27, 0x94, 0x11,
   0x85, 0x03, 0x28, 0x20, 0x6c, 0xe1, 0xff, 0x07, 0x0c, 0x01, 0x85, 0x01,
   0x28, 0x62, 0x5c, 0xe3, 0x8f, 0x03, 0x4e, 0x91, 0x80, 0x05, 0x39, 0x40,
   0xf4, 0xc2, 0xff, 0x00, 0x9f, 0x91, 0x84, 0x05, 0x31, 0xc6, 0xe8, 0x07,
   0x7f, 0x80, 0xcd, 0x00, 0xc4, 0x04, 0x31, 0x06, 0xc9, 0x0e, 0x00, 0xc0,
   0x48, 0x88, 0xe0, 0x04, 0x79, 0x04, 0xdb, 0x12, 0x00, 0x30, 0x0c, 0xc8,
   0xe4, 0x04, 0x6d, 0x06, 0xb6, 0x23, 0x00, 0x18, 0x1c, 0xc0, 0x84, 0x04,
   0x25, 0x0c, 0xff, 0xc2, 0x00, 0x4e, 0x06, 0xb0, 0x80, 0x04, 0x3f, 0x8a,
   0xb3, 0x83, 0xff, 0xc3, 0x03, 0x91, 0x84, 0x04, 0x2e, 0xd8, 0x0f, 0x3f,
   0x00, 0x00, 0x5f, 0x83, 0x84, 0x04, 0x2a, 0x70, 0xfd, 0x7f, 0x00, 0x00,
   0xc8, 0xc0, 0x84, 0x04, 0x4b, 0xe2, 0x2f, 0x01, 0x00, 0x08, 0x58, 0x60,
   0x80, 0x04, 0x5b, 0x82, 0xff, 0x01, 0x00, 0x08, 0xd0, 0xa0, 0x84, 0x04,
   0x72, 0x80, 0xe5, 0x00, 0x00, 0x08, 0xd2, 0x20, 0x44, 0x04, 0xca, 0x02,
   0xff, 0x00, 0x00, 0x08, 0xde, 0xa0, 0x44, 0x04, 0x82, 0x02, 0x6d, 0x00,
   0x00, 0x08, 0xf6, 0xb0, 0x40, 0x02, 0x82, 0x07, 0x3f, 0x00, 0x00, 0x08,
   0x44, 0x58, 0x44, 0x02, 0x93, 0x3f, 0x1f, 0x00, 0x00, 0x30, 0x88, 0x4f,
   0x44, 0x03, 0x83, 0x23, 0x3e, 0x00, 0x00, 0x00, 0x18, 0x60, 0xe0, 0x07,
   0xe3, 0x0f, 0xfe, 0x00, 0x00, 0x00, 0x70, 0x70, 0xe4, 0x07, 0xc7, 0x1b,
   0xfe, 0x01, 0x00, 0x00, 0xe0, 0x3c, 0xe4, 0x07, 0xc7, 0xe3, 0xfe, 0x1f,
   0x00, 0x00, 0xff, 0x1f, 0xfc, 0x07, 0xc7, 0x03, 0xf8, 0x33, 0x00, 0xc0,
   0xf0, 0x07, 0xff, 0x07, 0x87, 0x02, 0xfc, 0x43, 0x00, 0x60, 0xf0, 0xff,
   0xff, 0x07, 0x8f, 0x06, 0xbe, 0x87, 0x00, 0x30, 0xf8, 0xff, 0xff, 0x07,
   0x8f, 0x14, 0x9c, 0x8f, 0x00, 0x00, 0xfc, 0xff, 0xff, 0x07, 0x9f, 0x8d,
   0x8a, 0x0f, 0x00, 0x00, 0xfe, 0xff, 0xff, 0x07, 0xbf, 0x0b, 0x80, 0x1f,
   0x00, 0x00, 0xff, 0xff, 0xff, 0x07, 0x7f, 0x3a, 0x80, 0x3f, 0x00, 0x80,
   0xff, 0xff, 0xff, 0x07, 0xff, 0x20, 0xc0, 0x3f, 0x00, 0x80, 0xff, 0xff,
   0xff, 0x07, 0xff, 0x01, 0xe0, 0x7f, 0x00, 0xc0, 0xff, 0xff, 0xff, 0x07,
   0xff, 0x0f, 0xf8, 0xff, 0x40, 0xe0, 0xff, 0xff, 0xff, 0x07, 0xff, 0xff,
   0xff, 0xff, 0x40, 0xf0, 0xff, 0xff, 0xff, 0x07, 0xff, 0xff, 0xff, 0xff,
   0x41, 0xf0, 0xff, 0xff, 0xff, 0x07 ]


class MyWidget(Fl_Box):
    def __init__(self, x, y):
        Fl_Box.__init__(self, x,y,100,100, "Clipping and rect(f):\nYellow rect.framed\nby B-Y-G-R rect. 1 p.\nthick. Your printer may \nrender very thin lines\nsurrounding \"X\"")
        self.align(FL_ALIGN_TOP)
        self.labelsize(10)

    def draw(self):
        Fl_Box.draw(self)
        fl_color(FL_RED)
        fl_rectf(self.x()+5,self.y()+5,self.w()-10,self.h()-10)
        fl_push_clip(self.x()+6,self.y()+6,self.w()-12,self.h()-12)
        fl_color(FL_DARK_GREEN)
        fl_rectf(self.x()+5,self.y()+5,self.w()-10,self.h()-10)
        fl_pop_clip()
        fl_color(FL_YELLOW)
        fl_rectf(self.x()+7,self.y()+7,self.w()-14,self.h()-14)
        fl_color(FL_BLUE)
        
        fl_rect(self.x()+8,self.y()+8,self.w()-16,self.h()-16)
        fl_push_clip(self.x()+25,self.y()+25,self.w()-50, self.h()-50)
        fl_color(FL_BLACK)
        fl_rect(self.x()+24,self.y()+24,self.w()-48,self.h()-48)
        fl_line(self.x()+27,self.y()+27,self.x()+self.w()-27,self.y()+self.h()-27)
        fl_line(self.x()+27,self.y()+self.h()-27,self.x()+self.w()-27,self.y()+27)
        fl_pop_clip()



class MyWidget2(Fl_Box):
    def __init__(self, x, y):
        Fl_Box.__init__(self,x,y,100,100, "Integer primitives")
        self.labelsize(10)
        self.align(FL_ALIGN_TOP)
        
    def draw(self):
        Fl_Box.draw(self)
        d = self.y()+5
        while d < 48+self.y():
            fl_xyline(self.x()+5,d,self.x()+48)
            d = d+2

        fl_push_clip(self.x()+52,self.y()+5,45,43)
        d = self.y()+5
        while d < 150+self.y():
            fl_line(self.x()+52,d,self.x()+92,d-40)
            d = d+3

        fl_pop_clip()
    
        fl_line_style(FL_DASH)
        fl_xyline(self.x()+5,self.y()+55,self.x()+48)
        fl_line_style(FL_DOT)
        fl_xyline(self.x()+5,self.y()+58,self.x()+48)
        fl_line_style(FL_DASHDOT)
        fl_xyline(self.x()+5,self.y()+61,self.x()+48)
        fl_line_style(FL_DASHDOTDOT)
        fl_xyline(self.x()+5,self.y()+64,self.x()+48)
        fl_line_style(0,0,"\7\3\7\2")
        fl_xyline(self.x()+5,self.y()+67,self.x()+48)
      
        fl_line_style(0)

        fl_line(self.x()+5,self.y()+72,self.x()+25,self.y()+95)
        fl_line(self.x()+8,self.y()+72,self.x()+28,self.y()+95,self.x()+31,self.y()+72)
        
        fl_color(FL_YELLOW)
        fl_polygon(self.x()+11, self.y()+72,self.x()+27,self.y()+91,self.x()+29,self.y()+72)
        fl_color(FL_RED)
        fl_loop(self.x()+11, self.y()+72,self.x()+27,self.y()+91,self.x()+29,self.y()+72)

        fl_color(FL_BLUE)
        fl_line_style(FL_SOLID, 6)
        fl_loop(self.x()+31,self.y()+12,self.x()+47,self.y()+31,self.x()+49,self.y()+12)
        fl_line_style(0)

        fl_color(200,0,200)
        fl_polygon(self.x()+35,self.y()+72,self.x()+33,self.y()+95,self.x()+48,self.y()+95,self.x()+43,self.y()+72)
        fl_color(FL_GREEN)
        fl_loop(self.x()+35,self.y()+72,self.x()+33,self.y()+95,self.x()+48,self.y()+95,self.x()+43,self.y()+72)

        fl_color(FL_BLUE)
        fl_yxline(self.x()+65,self.y()+63,self.y()+66)
        fl_color(FL_GREEN)
        fl_yxline(self.x()+66,self.y()+66,self.y()+63)

        fl_color(FL_BLUE)
        fl_rect(self.x()+80,self.y()+55,5,5)
        fl_color(FL_YELLOW)
        fl_rectf(self.x()+81,self.y()+56,3,3)
        fl_color(FL_BLACK)
        fl_point(self.x()+82,self.y()+57)

        fl_color(FL_BLUE)
        fl_rect(self.x()+56, self.y()+79, 24, 17)
        fl_color(FL_CYAN)
        fl_rectf(self.x()+57, self.y()+80, 22 , 15 )
        fl_color(FL_RED)
        fl_arc(self.x()+57, self.y()+80, 22 ,15 ,40, 270)
        fl_color(FL_YELLOW)
        fl_pie(self.x()+58, self.y()+81, 20 ,13 ,40, 270)

        fl_line_style(0)


        fl_color(FL_BLACK)
        fl_point(self.x()+58,self.y()+58)
        fl_color(FL_RED)
        fl_yxline(self.x()+59,self.y()+58,self.y()+59)
        fl_color(FL_GREEN)
        fl_yxline(self.x()+60,self.y()+59,self.y()+58)
        fl_color(FL_BLACK)
        fl_xyline(self.x()+61,self.y()+58,self.x()+62)
        fl_color(FL_RED)
        fl_xyline(self.x()+62,self.y()+59,self.x()+61)

        fl_color(FL_GREEN)
        fl_yxline(self.x()+57,self.y()+58,self.y()+59,self.x()+58)
        fl_color(FL_BLUE)
        fl_xyline(self.x()+58,self.y()+60,self.x()+56,self.y()+58)
        fl_color(FL_RED)
        fl_xyline(self.x()+58,self.y()+61,self.x()+56,self.y()+63)
        fl_color(FL_GREEN)
        fl_yxline(self.x()+57,self.y()+63,self.y()+62,self.x()+58)
    
        fl_color(FL_BLUE)
        fl_line(self.x()+58,self.y()+63, self.x()+60, self.y()+65)
        fl_color(FL_BLACK)
        fl_line(self.x()+61,self.y()+65, self.x()+59, self.y()+63)

        fl_color(FL_BLACK)


class MyWidget3(Fl_Box):
    def __init__(self, x, y):
        Fl_Box.__init__(self, x,y,100,100, "Sub-pixel drawing of\nlines 1.63 points apart\nOn the screen you\ncan see aliasing, the\nprinter should render\nthem properly")
        self.labelsize(10)
        self.align(FL_ALIGN_TOP)
        
    def draw(self):
        Fl_Box.draw(self)
        fl_push_clip(self.x()+5,self.y()+5,45,43)
        d = self.y()+5.0
        while d < 95+self.y():
            fl_begin_line()
            fl_vertex(self.x()+5,d)
            fl_vertex(self.x()+48,d)
            fl_end_line()
            d = d+1.63

        fl_pop_clip()

        fl_push_clip(self.x()+52,self.y()+5,45,43)
        d = self.y()+5.0
        while d < 150+self.y():
            fl_begin_line()
            fl_vertex(self.x()+52,d)
            fl_vertex(self.x()+92,d-43)
            fl_end_line()
            d = d+2.3052

        fl_pop_clip()


class MyWidget4(Fl_Box):
    def __init__(self, x, y):
        Fl_Box.__init__(self, x,y,150,150, "Line styles")
        self.labelsize(10)
        self.align(FL_ALIGN_TOP)

    def draw(self):
        Fl_Box.draw(self)
        fl_push_matrix()
        fl_translate(self.x(),self.y())
        fl_scale(.75,.75)
        
        fl_line_style(FL_SOLID , 5)
        fl_begin_line()
        fl_vertex(10, 160)
        fl_vertex(40, 160)
        fl_vertex(40, 190)
        fl_end_line()
        fl_line_style(0)
        
        
        fl_color(FL_RED)
        fl_line_style(FL_SOLID | FL_CAP_FLAT |FL_JOIN_MITER , 5)
        fl_begin_line()
        fl_vertex(10, 150)
        fl_vertex(50, 150)
        fl_vertex(50, 190)
        fl_end_line()
        fl_line_style(0)
        
        fl_color(FL_GREEN)
        fl_line_style(FL_SOLID | FL_CAP_ROUND |FL_JOIN_ROUND , 5)
        fl_begin_line()
        fl_vertex(10, 140)
        fl_vertex(60, 140)
        fl_vertex(60, 190)
        fl_end_line()
        fl_line_style(0)
        
        fl_color(FL_BLUE)
        fl_line_style(FL_SOLID | FL_CAP_SQUARE |FL_JOIN_BEVEL , 5)
        fl_begin_line()
        fl_vertex(10, 130)
        fl_vertex(70, 130)
        fl_vertex(70, 190)
        fl_end_line()
        fl_line_style(0)
        
        fl_color(FL_BLACK)
        fl_line_style(FL_DASH , 5)
        fl_begin_line()
        fl_vertex(10, 120)
        fl_vertex(80, 120)
        fl_vertex(80, 190)
        fl_end_line()
        fl_line_style(0)
        
        fl_color(FL_RED)
        fl_line_style(FL_DASH |FL_CAP_FLAT , 5)
        fl_begin_line()
        fl_vertex(10, 110)
        fl_vertex(90, 110)
        fl_vertex(90, 190)
        fl_end_line()
        fl_line_style(0)
        
        fl_color(FL_GREEN)
        fl_line_style(FL_DASH |FL_CAP_ROUND , 5)
        fl_begin_line()
        fl_vertex(10, 100)
        fl_vertex(100, 100)
        fl_vertex(100, 190)
        fl_end_line()
        fl_line_style(0)
        
        fl_color(FL_BLUE)
        fl_line_style(FL_DASH |FL_CAP_SQUARE , 5)
        fl_begin_line()
        fl_vertex(10, 90)
        fl_vertex(110, 90)
        fl_vertex(110, 190)
        fl_end_line()
        fl_line_style(0)
        
        fl_color(FL_BLACK)
        fl_line_style(FL_DOT, 5)
        fl_begin_line()
        fl_vertex(10, 80)
        fl_vertex(120, 80)
        fl_vertex(120, 190)
        fl_end_line()
        fl_line_style(0)
        
        fl_color(FL_RED)
        fl_line_style(FL_DOT | FL_CAP_FLAT, 5)
        fl_begin_line()
        fl_vertex(10, 70)
        fl_vertex(130, 70)
        fl_vertex(130, 190)
        fl_end_line()
        fl_line_style(0)
        
        fl_color(FL_GREEN)
        fl_line_style(FL_DOT | FL_CAP_ROUND, 5)
        fl_begin_line()
        fl_vertex(10, 60)
        fl_vertex(140, 60)
        fl_vertex(140, 190)
        fl_end_line()
        fl_line_style(0)
        
        fl_color(FL_BLUE)
        fl_line_style(FL_DOT | FL_CAP_SQUARE, 5)
        fl_begin_line()
        fl_vertex(10, 50)
        fl_vertex(150, 50)
        fl_vertex(150, 190)
        fl_end_line()
        fl_line_style(0)
        
        fl_color(FL_BLACK)
        fl_line_style(FL_DASHDOT |FL_CAP_ROUND |FL_JOIN_ROUND , 5)
        fl_begin_line()
        fl_vertex(10, 40)
        fl_vertex(160, 40)
        fl_vertex(160, 190)
        fl_end_line()
        fl_line_style(0)
        
        fl_color(FL_RED)
        fl_line_style(FL_DASHDOTDOT |FL_CAP_SQUARE |FL_JOIN_BEVEL , 5)
        fl_begin_line()
        fl_vertex(10, 30)
        fl_vertex(170, 30)
        fl_vertex(170, 190)
        fl_end_line()
        fl_line_style(0)
        
        fl_color(FL_GREEN)
        fl_line_style(FL_DASHDOTDOT |FL_CAP_ROUND |FL_JOIN_ROUND , 5)
        fl_begin_line()
        fl_vertex(10, 20)
        fl_vertex(180, 20)
        fl_vertex(180, 190)
        fl_end_line()
        fl_line_style(0)
        
        fl_color(FL_BLUE)
        fl_line_style(0, 5, "\12\3\4\2\2\1")
        fl_begin_line()
        fl_vertex(10, 10)
        fl_vertex(190, 10)
        fl_vertex(190, 190)
        
        fl_end_line()
        fl_line_style(0)
        fl_pop_matrix()  
        
        fl_color(FL_BLACK)



class MyWidget5(Fl_Box):
    def __init__(self, x, y):
        Fl_Box.__init__(self,x,y,230,250, "Complex (double) drawings:\nBlue ellipse may not be\ncorrectly transformed\ndue to non-orthogonal\ntransformation")
        self.labelsize(10)
        self.align(FL_ALIGN_TOP)

    def draw(self):
        Fl_Box.draw(self)
        fl_push_matrix()
        
        fl_translate(self.x(),self.y())
        fl_push_matrix()
        fl_mult_matrix(1,3,0,1,0,-20)
        fl_color(FL_GREEN)
        fl_begin_polygon()
        fl_vertex(10,10)
        fl_vertex(100,-80)
        fl_vertex(100,-190)
        fl_end_polygon()
        
        fl_color(FL_RED)
        fl_line_style(FL_DASHDOT, 7)
        fl_begin_loop()
        
        fl_vertex(10,10)
        fl_vertex(100,-80)
        fl_vertex(100,-190)
        fl_end_loop()
        fl_line_style(0)
        
        fl_color(FL_BLUE)
        fl_line_style(FL_SOLID, 3)
        fl_begin_loop()
        fl_circle(60,-50,30)
        fl_end_loop()
        fl_line_style(0)
        
        fl_pop_matrix()
        fl_scale(1.8,1)
        
        fl_color(FL_YELLOW)
        fl_begin_polygon()
        fl_arc(30,90,20,-45,200)
        fl_end_polygon()
        
        fl_color(FL_BLACK)
        fl_line_style(FL_DASH, 3)
        fl_begin_line()
        fl_arc(30,90,20,-45,200)
        fl_end_line()
        fl_line_style(0)
        
        fl_translate(15,0)
        fl_scale(1.5,3)
        fl_begin_complex_polygon()
        fl_vertex(30,70)
        fl_arc(45,55,10,200,90)
        fl_arc(55,45,8,-170,20)
        fl_vertex(60,40)
        fl_vertex(30,20)
        fl_vertex(40,5)
        fl_vertex(60,25)
        fl_curve(35,30,30,53,0,35,65,65)
        fl_gap()
        fl_vertex(50,25)
        fl_vertex(40,10)
        fl_vertex(35,20)
        fl_end_complex_polygon()
        
        fl_pop_matrix()

image = []
width = 80
height = 80

def make_image():
    p = image
    index = 0

    y = 0
    while y < height:
        dy = float(y)/(height-1)
        x = 0
        while x < width:
            dx = float(x)/(width-1)
            p.append(int(255*((1.0-dx)*(1.0-dy))))
            index = index+1
            p.append(int(255*((1.0-dx)*dy)))
            index = index+1
            p.append(int(255*(dx*dy)))
            index = index+1
            dx = dx-0.5
            dy = dy-0.5
            alpha = int(255*math.sqrt(dx*dx+dy*dy))
            if alpha < 255:
                p.append(alpha)
            else:
                p.append(255)
            index = index+1
            dy = dy+0.5
            x = x+1
        y = y+1

    return None


def print_cb(widget, w):
    p = Fl_Printer()
    if (p.start_job(1) == 0):
        p.start_page()
        p.print_widget(w)
        p.end_page()
        p.end_job()


class My_Button(Fl_Button):
    def __init__(self, x, y, w, h, label = 0):
        Fl_Button.__init__(self, x,y,w,h,label)
        
    def draw(self):
        if (self.type() == FL_HIDDEN_BUTTON):
            return
        col = self.color()
        if self.value() != 0:
            col = self.selection_color()
        boxtype = fl_down(self.box())
        if self.down_box():
            boxtype = self.down_box()
        #self.draw_box(boxtype, col)
        
        fl_color(FL_WHITE)
        fl_line_style(FL_SOLID,5)
        fl_line(self.x()+15,self.y()+10,self.x()+self.w()-15,self.y()+self.h()-23)
        fl_line(self.x()+self.w()-15,self.y()+10,self.x()+15,self.y()+self.h()-23)
        fl_line_style(0)
        self.draw_label()


if __name__=='__main__':
    window = Fl_Double_Window(500,560,"Graphics test")


    c2 = Fl_Group(3, 43, 494, 514 )

    w1 = MyWidget(10,140)
    w2 = MyWidget2(110,80)
    w3 = MyWidget3(220,140)
    w4 = MyWidget4(330,70)
    w5 = MyWidget5(140,270)

    make_image()
    rgb = Fl_RGB_Image(image, width, height, 4)
    b_rgb = My_Button(10,245,100,100,"RGB with alpha")
    b_rgb.image(rgb)

    b_pixmap = My_Button(10,345,100,100,"Pixmap")
    pixmap = Fl_Pixmap(porsche_xpm)
    b_pixmap.image(pixmap)

    b_bitmap = My_Button(10,445,100,100,"Bitmap")
    b_bitmap.labelcolor(FL_GREEN)
    b_bitmap.image(Fl_Bitmap(sorceress_bits,sorceress_width,sorceress_height))

    clock = Fl_Clock(360,230,120,120)
    ret = Fl_Return_Button (360, 360, 120,30, "Return")
    ret.deactivate()
    but1 = Fl_Button(360, 390, 30, 30, "@->|")
    but1.labelcolor(FL_DARK3)
    but2 = Fl_Button(390, 390, 30, 30, "@UpArrow")
    but2.labelcolor(FL_DARK3)
    but3 = Fl_Button(420, 390, 30, 30, "@DnArrow")
    but3.labelcolor(FL_DARK3)
    but4 = Fl_Button(450, 390, 30, 30, "@+")
    but4.labelcolor(FL_DARK3)
    but5 = Fl_Button(360, 425, 120, 30, "Hello, World")
    but5.labelfont(FL_BOLD|FL_ITALIC)
    but5.labeltype(FL_SHADOW_LABEL)
    but5.box(FL_ROUND_UP_BOX)


    but6 = Fl_Button(360, 460, 120, 30, "Plastic")
    but6.box(FL_PLASTIC_UP_BOX)

    o1 = Fl_Group(360, 495, 120, 40)
    o1.box(FL_UP_BOX)
    o2 = Fl_Group(365, 500, 110, 30)
    o2.box(FL_THIN_UP_FRAME)
    rnd_btn = Fl_Round_Button(365, 500, 40, 30, "rad")
    rnd_btn.value(1)
    chk_btn = Fl_Check_Button(410, 500, 60, 30, "check")
    chk_btn.value(1)
    o2.end()
    o1.end()
    o1.deactivate()
    

    tx = Fl_Box(120,492,230,50,"Background is not printed because\nencapsulating group, which we are\n printing, has not set the box type")
    tx.box(FL_SHADOW_BOX)
    tx.labelsize(12)

    c2.end()
    b4 = Fl_Button(10,5, 150, 25, "Print")
    b4.callback(print_cb,c2)

    window.end()
    window.show()

