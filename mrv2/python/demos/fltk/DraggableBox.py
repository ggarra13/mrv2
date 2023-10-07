#
# "$Id: DraggableBox.py 493 2012-02-14 21:40:41Z andreasheld $"
#
# Draggable boxes in a scrollable window. After the demo program
# by G. Ercolano, http://seriss.com/people/erco/fltk
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

# Demonstrate user-movable boxes in a scroll region
# erco@netcom.com 08/06/02

# XPM
cat_xpm = [
"50 50 4 1",
"  c black",
"o c #ff9900",
"@ c #ffffff",
"# c None",
"##################################################",
"##################################################",
"##################################################",
"##################################################",
"##################################################",
"##################################################",
"##################################################",
"##################################################",
"##################################################",
"###      ##############################       ####",
"### ooooo  ###########################  ooooo ####",
"### oo  oo  #########################  oo  oo ####",
"### oo   oo  #######################  oo   oo ####",
"### oo    oo  #####################  oo    oo ####",
"### oo     oo  ###################  oo     oo ####",
"### oo      oo                     oo      oo ####",
"### oo       oo  ooooooooooooooo  oo       oo ####",
"### oo        ooooooooooooooooooooo        oo ####",
"### oo     ooooooooooooooooooooooooooo    ooo ####",
"#### oo   ooooooo ooooooooooooo ooooooo   oo #####",
"####  oo oooooooo ooooooooooooo oooooooo oo  #####",
"##### oo oooooooo ooooooooooooo oooooooo oo ######",
"#####  o ooooooooooooooooooooooooooooooo o  ######",
"###### ooooooooooooooooooooooooooooooooooo #######",
"##### ooooooooo     ooooooooo     ooooooooo ######",
"##### oooooooo  @@@  ooooooo  @@@  oooooooo ######",
"##### oooooooo @@@@@ ooooooo @@@@@ oooooooo ######",
"##### oooooooo @@@@@ ooooooo @@@@@ oooooooo ######",
"##### oooooooo  @@@  ooooooo  @@@  oooooooo ######",
"##### ooooooooo     ooooooooo     ooooooooo ######",
"###### oooooooooooooo       oooooooooooooo #######",
"###### oooooooo@@@@@@@     @@@@@@@oooooooo #######",
"###### ooooooo@@@@@@@@@   @@@@@@@@@ooooooo #######",
"####### ooooo@@@@@@@@@@@ @@@@@@@@@@@ooooo ########",
"######### oo@@@@@@@@@@@@ @@@@@@@@@@@@oo ##########",
"########## o@@@@@@ @@@@@ @@@@@ @@@@@@o ###########",
"########### @@@@@@@     @     @@@@@@@ ############",
"############  @@@@@@@@@@@@@@@@@@@@@  #############",
"##############  @@@@@@@@@@@@@@@@@  ###############", 
"################    @@@@@@@@@    #################",
"####################         #####################",
"##################################################",
"##################################################",
"##################################################",
"##################################################",
"##################################################",
"##################################################",
"##################################################",
"##################################################",
"##################################################"
]

G_win    = None
G_scroll = None
G_cat    = Fl_Pixmap(cat_xpm)
offset = [0,0]

# A 'MOVABLE' BOX
class Box(Fl_Box):
    def __init__(self, X, Y, W, H, L):
        Fl_Box.__init__(self, X, Y, W, H, L)
        self.image(G_cat)
        self.box(FL_UP_BOX)
        self.color(FL_GRAY)
    def __init__(self, X, Y):
        Fl_Box.__init__(self, X, Y, 80, 50)
        self.image(G_cat)
        self.box(FL_UP_BOX)
        self.color(FL_GRAY)

    def handle(self, e):
        global offset
        if e == FL_PUSH:
            # save where user clicked for dragging
            offset[0] = self.x() - Fl.event_x()
            offset[1] = self.y() - Fl.event_y()
            return 1
        elif e == FL_RELEASE:
            return 1
        elif e == FL_DRAG:
            # handle dragging
            self.position(offset[0]+Fl.event_x(), offset[1]+Fl.event_y())
            G_win.redraw()
            return 1
        elif e == FL_ENTER:
            return 1
        elif e == FL_MOVE:
            print("FL_MOVE")
            return 0
        return 0
                          

# MAIN
if __name__=='__main__':
    G_win = Fl_Double_Window(720,486)
    G_scroll = Fl_Scroll(10,10,720-20,486-20)
    G_scroll.box(FL_FLAT_BOX)
    G_scroll.color(fl_rgb_color(46))
    G_scroll.begin()
    # CREATE NEW BOXES ON THE SCROLLABLE 'DESK'
    l = []
    x = 200
    while x <= 500:
        y = 100
        while y < 370:
            l.append(Box(x,y))
            y += 70
        x += 100

    G_scroll.end()
    G_win.resizable(G_win)
    G_win.show()

