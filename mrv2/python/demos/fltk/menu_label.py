#
# "$Id: menu_label.py 536 2020-10-30 15:20:32Z andreasheld $"
#
# Menu label test program for pyFLTK the Python bindings
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


# global object names
L_document_xpm=[
  "13 11 3 1",
  "   c None",
  "x  c #d8d8f8",
  "@  c #202060",
  " @@@@@@@@@   ",
  " @xxxxxxx@   ",
  " @xxxxxxx@   ",
  " @xxxxxxx@   ",
  " @xxxxxxx@   ",
  " @xxxxxxx@   ",
  " @xxxxxxx@   ",
  " @xxxxxxx@   ",
  " @xxxxxxx@   ",
  " @xxxxxxx@   ",
  " @@@@@@@@@   "]
L_document_pixmap = Fl_Pixmap(L_document_xpm)

# Folder icon
L_folder_xpm=[
  "13 11 3 1",
  "   c None",
  "x  c #d8d833",
  "@  c #808011",
  "             ",
  "     @@@@    ",
  "    @xxxx@   ",
  "@@@@@xxxx@@  ",
  "@xxxxxxxxx@  ",
  "@xxxxxxxxx@  ",
  "@xxxxxxxxx@  ",
  "@xxxxxxxxx@  ",
  "@xxxxxxxxx@  ",
  "@xxxxxxxxx@  ",
  "@@@@@@@@@@@  "]
L_folder_pixmap = Fl_Pixmap(L_folder_xpm)

# Red "X"
L_redx_xpm=[
  "13 11 5 1",
  "   c None",
  "+  c #222222",
  "x  c #555555",
  "-  c #882222",
  "@  c #ffffff",
  "   x+++x     ",
  "  ++---++    ",
  " ++-----++   ",
  "++-@@-@@-++  ",
  "++--@@@--++  ",
  "++---@---++  ",
  "++--@@@--++  ",
  "++-@@-@@-++  ",
  " ++-----++   ",
  "  ++---++    ",
  "   x+++x     "]
L_redx_pixmap = Fl_Pixmap(L_redx_xpm)

# Handle the different menu items..
def onMenu(ptr, data):
    fl_message("'%s' would happen here"%data)

def onFileQuit(ptr, data):
    menuItem = ptr
    print(f'onFileQuit({str(menuItem)}, "{str(data)}")')
    import sys  # code
    sys.exit(0)  # code

# Add an image in front of item's text
def addItemToMenu(menu,                   # menu to add item to
              labeltext,              # label text
              shortcut,               # shortcut (e.g. FL_COMMAND+'a')
              cb,                     # callback to invoke
              userdata,               # userdata for callback
              pixmap,                 # image (if any) to add to item
              flags=0               # menu flags (e.g. FL_MENU_DIVIDER..)
              ):
  # Add a new menu item
  menu.add(labeltext, shortcut, cb, userdata, flags)
  item = menu.find_item(labeltext)
  item.add_multi_label(pixmap);



def createMenuItems(menu):
    # Add items with LABELS AND IMAGES using Fl_Multi_Label..
    addItemToMenu(menu, "File/New",  FL_COMMAND | ord('n'), onMenu, "New",  L_document_pixmap)
    addItemToMenu(menu, "File/Open", FL_COMMAND | ord('o'), onMenu, "Open", L_folder_pixmap, FL_MENU_DIVIDER)
    addItemToMenu(menu, "File/Quit", FL_COMMAND | ord('q'), onFileQuit, "Quit", L_redx_pixmap)

    # Create menu bar items with JUST LABELS
    menu.add("Edit/Copy", FL_COMMAND | ord('c'), onMenu, "Copy")
    menu.add("Edit/Paste", FL_COMMAND | ord('v'), onMenu, "Paste")

    menu.add("Images/One", 0, onMenu, "One");
    item = menu.find_item("Images/One")
    item.image(L_document_pixmap.copy())

    menu.add("Images/Two", 0, onMenu, "Two")
    item = menu.find_item("Images/Two")
    item.image(L_folder_pixmap.copy())

    menu.add("Images/Three", 0, onMenu, "Three")
    item = menu.find_item("Images/Three")
    item.image(L_redx_pixmap.copy())
    

def main():

    win = Fl_Window(0, 0, 400, 400)

    # menu bar
    mb = Fl_Menu_Bar(0, 0, win.w(), 25)
    createMenuItems(mb)

    # Right click context menu
    menubutt = Fl_Menu_Button(0, 25, win.w(), win.h() - 25);
    createMenuItems(menubutt)
    menubutt.type(Fl_Menu_Button.POPUP3)

    # Chooser menu
    choice = Fl_Choice(140, 50, 200, 25, "Choice")
    createMenuItems(choice)
    choice.value(1)

    return win



if __name__=='__main__':
    import sys
    window = main()
    window.show()