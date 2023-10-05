#! /usr/bin/env python

#
# "$Id: tree.py 536 2020-10-30 15:20:32Z andreasheld $"
#
# Tree widget test program for pyFLTK the Python bindings
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


"""
Tree test program.
"""

import traceback, sys
if sys.version > '3':
    long = int

from fltk14 import *

G_cb_counter = 0
but = None
grp = None

def reason_as_name(reason):
    if reason == FL_TREE_REASON_NONE:
        return "none"
    elif reason == FL_TREE_REASON_SELECTED:
        return "selected"
    elif reason == FL_TREE_REASON_DESELECTED:
        return "deselected"
    elif reason == FL_TREE_REASON_OPENED:
        return "opened"
    elif reason == FL_TREE_REASON_CLOSED:
        return "closed"
    else:
        return "???"

def Button_CB(w):
    print(f"'{w.label()}' button pushed\n")

def RebuildTree(tree):
    global but, grp
    # REBUILD THE TREE TO MAKE CURRENT "DEFAULT" PREFS TAKE EFFECT
    tree.clear()
    tree.add("Aaa")
    tree.add("Bbb")
    tree.add("Ccc")
    tree.add("Ddd")
    tree.add("Bbb/child-01")
    tree.add("Bbb/child-01/111")
    tree.add("Bbb/child-01/222")
    tree.add("Bbb/child-01/333")
    tree.add("Bbb/child-02")
    tree.add("Bbb/child-03")
    tree.add("Bbb/child-04")
  
    # Assign an FLTK widget to one of the items
    i = tree.find_item("Bbb/child-03")
    if i != None:
        if but == None: # only do this once at program startup
            tree.begin()
            but = Fl_Button(1,1,140,1,"ccc button") # we control w() only
            but.labelsize(10)
            but.callback(Button_CB)
        i.widget(but)
        tree.end()

  
    # Assign an FLTK group to one of the items with widgets
    i = tree.find_item("Bbb/child-04")
    if i != None:
        if grp == None: # only do this once at program startup
            tree.begin()
            grp = Fl_Group(100,100,140,18) # build group.. tree handles position
            grp.color(FL_WHITE);
            grp.begin()
            abut = Fl_Button(grp.x()+0 ,grp.y()+2,65,15,"D1")
            abut.labelsize(10)
            abut.callback(Button_CB)
            bbut = Fl_Button(grp.x()+75,grp.y()+2,65,15,"D2")
            bbut.labelsize(10)
            bbut.callback(Button_CB)
            grp.end()
            grp.resizable(grp)
            tree.end()

        i.widget(grp);
  
    # Add an 'Ascending' node, and create it sorted
    tree.sortorder(FL_TREE_SORT_NONE)
    tree.add("Ascending").close()
    tree.sortorder(FL_TREE_SORT_ASCENDING)
    tree.add("Ascending/Zzz")
    tree.add("Ascending/Xxx")
    tree.add("Ascending/Aaa")
    tree.add("Ascending/Bbb")
    tree.add("Ascending/Yyy")
    tree.add("Ascending/Ccc")
  
    # Add a 'Descending' node, and create it sorted
    tree.sortorder(FL_TREE_SORT_NONE)
    tree.add("Descending").close()
    tree.sortorder(FL_TREE_SORT_DESCENDING)
    tree.add("Descending/Zzz")
    tree.add("Descending/Xxx")
    tree.add("Descending/Aaa")
    tree.add("Descending/Bbb")
    tree.add("Descending/Yyy")
    tree.add("Descending/Ccc")
  
    # Add 500 items in numerical order
    tree.sortorder(FL_TREE_SORT_NONE);
    for t in range(500):
        s = f"500 Items/item {t:04d}"
        tree.add(s)

    tree.close("500 Items")	# close the 500 items by default
  
    tree.redraw()



  

def cb_tree(tree, data):
    global G_cb_counter
    # Increment callback counter whenever tree callback is invoked
    G_cb_counter = G_cb_counter+1
    
    item = tree.callback_item()
    if item:
        print(f"TREE CALLBACK: label='{item.label()}' userdata={data} reason={reason_as_name(tree.callback_reason())}\n")
    else:
        print(f"TREE CALLBACK: reason={reason_as_name(tree.callback_reason())} item=(no item -- probably multiple items were changed at once)\n")


def cb_margintop_slider(margintop_slider, tree):
    val = int(margintop_slider.value())
    tree.margintop(val)
    tree.redraw()

def cb_marginleft_slider(marginleft_slider, tree):
    val = int(marginleft_slider.value())
    tree.marginleft(val)
    tree.redraw()

def cb_openchild_marginbottom_slider(openchild_marginbottom_slider, tree):
    val = int(openchild_marginbottom_slider.value())
    tree.openchild_marginbottom(val)
    tree.redraw()

def cb_labelsize_slider(labelsize_slider, tree):
    size = int(labelsize_slider.value())
    count = 0

    item = tree.first()
    while item != None:
        if item.is_selected():
            item.labelsize(size)
            count = count+1
        item = tree.next(item)

    if count == 0:
        item = tree.first()
        while item != None:
            item.labelsize(size)
            item = tree.next(item)

    tree.redraw()

def cb_connectorwidth_slider(connectorwidth_slider, tree):
    val = int(connectorwidth_slider.value())
    tree.connectorwidth(val)
    #tree.redraw()

L_folderpixmap = None
L_documentpixmap = None

def cb_usericon_radio(usericon_radio, tree):
    global L_folderpixmap, L_documentpixmap
    L_folder_xpm = [
    "11 11 3 1",
    ".  c None",
    "x  c #d8d833",
    "@  c #808011",
    "...........",
    ".....@@@@..",
    "....@xxxx@.",
    "@@@@@xxxx@@",
    "@xxxxxxxxx@",
    "@xxxxxxxxx@",
    "@xxxxxxxxx@",
    "@xxxxxxxxx@",
    "@xxxxxxxxx@",
    "@xxxxxxxxx@",
    "@@@@@@@@@@@"]
    L_folderpixmap = Fl_Pixmap(L_folder_xpm)

    L_document_xpm = [
    "11 11 3 1",
    ".  c None",
    "x  c #d8d8f8",
    "@  c #202060",
    ".@@@@@@@@@.",
    ".@xxxxxxx@.",
    ".@xxxxxxx@.",
    ".@xxxxxxx@.",
    ".@xxxxxxx@.",
    ".@xxxxxxx@.",
    ".@xxxxxxx@.",
    ".@xxxxxxx@.",
    ".@xxxxxxx@.",
    ".@xxxxxxx@.",
    ".@@@@@@@@@."]
    L_documentpixmap = Fl_Pixmap(L_document_xpm)

    i = None
    
    if usericon_radio.value() != 0.0:
        tree.usericon(L_folderpixmap)
        i = tree.find_item("Bbb/bgb/111")
        if i != None:
            i.usericon(L_documentpixmap)
        i = tree.find_item("Bbb/bgb/222")
        if i != None:
            i.usericon(L_documentpixmap)
        i = tree.find_item("Bbb/bgb/333")
        if i != None:
            i.usericon(L_documentpixmap)
    else:
        tree.usericon(None)
        i = tree.find_item("Bbb/bgb/111")
        if i != None:
            i.usericon(0)
        i = tree.find_item("Bbb/bgb/222")
        if i != None:
            i.usericon(0)
        i = tree.find_item("Bbb/bgb/333")
        if i != None:
            i.usericon(0)

def cb_showroot_radio(showroot_radio, datatree):
    onoff = int(showroot_radio.value())
    tree.showroot(onoff)


def cb_visiblefocus_checkbox(visiblefocus_checkbox, tree):
    onoff = int(visiblefocus_checkbox.value())
    tree.visible_focus(onoff)

L_openpixmap = None
L_closepixmap = None

def cb_collapseicons_chooser(collapseicons_chooser, tree):
    global L_openpixmap, L_closepixmap
    L_open_xpm = [
    "11 11 2 1",
    ".  c None",
    "@  c #000000",
    "...@.......",
    "...@@......",
    "...@@@.....",
    "...@@@@....",
    "...@@@@@...",
    "...@@@@@@..",
    "...@@@@@...",
    "...@@@@....",
    "...@@@.....",
    "...@@......",
    "...@......."]

    L_openpixmap = Fl_Pixmap(L_open_xpm)

    L_close_xpm = [
    "11 11 2 1",
    ".  c None",
    "@  c #000000",
    "...........",
    "...........",
    "...........",
    "...........",
    "...........",
    "@@@@@@@@@@@",
    ".@@@@@@@@@.",
    "..@@@@@@@..",
    "...@@@@@...",
    "....@@@....",
    ".....@....."]
    L_closepixmap = Fl_Pixmap (L_close_xpm)

    if collapseicons_chooser.value() == 0:
        tree.showcollapse(1)
        tree.openicon(None)
        tree.closeicon(None)
    elif collapseicons_chooser.value() == 1:
        tree.showcollapse(1)
        tree.openicon(L_openpixmap)
        tree.closeicon(L_closepixmap)
    elif collapseicons_chooser.value() == 2:
        tree.showcollapse(0)

menu_collapseicons_chooser = (("Normal", 0,  0, 0, 0, FL_NORMAL_LABEL, 0, 11, 0),
 ("Custom", 0,  0, 0, 0, FL_NORMAL_LABEL, 0, 11, 0),
 ("Off", 0,  0, 0, 0, FL_NORMAL_LABEL, 0, 11, 0),
 (None,))

def cb_connectorstyle_chooser(connectorstyle_chooser, tree):
    # CHANGE COLLAPSESTYLE
    if connectorstyle_chooser.value() == 0:
        tree.connectorstyle(FL_TREE_CONNECTOR_NONE)
    elif connectorstyle_chooser.value() == 1:
        tree.connectorstyle(FL_TREE_CONNECTOR_DOTTED)
    elif connectorstyle_chooser.value() == 2:
        tree.connectorstyle(FL_TREE_CONNECTOR_SOLID)

menu_connectorstyle_chooser = (("None", 0,  0, 0, 0, FL_NORMAL_LABEL, 0, 11, 0),
 ("Dotted", 0,  0, 0, 0, FL_NORMAL_LABEL, 0, 11, 0),
 ("Solid", 0,  0, 0, 0, FL_NORMAL_LABEL, 0, 11, 0),
 (None,))


def cb_labelcolor_chooser(labelcolor_chooser, tree):
    # Set color..
    c = 0x00000000
    if labelcolor_chooser.value() == 0:
        c = 0x00000000 # black
    elif labelcolor_chooser.value() == 1:
        c = 0xd0000000 # red
    elif labelcolor_chooser.value() == 2:
        c = 0x00a00000 # green
    elif labelcolor_chooser.value() == 3:
        c = 0x0000a000 # blue
    else:
        c = 0x00000000 # black
        
    # DO SELECTED ITEMS
    count = 0
    item = tree.first()
    while item != None:
        if item.is_selected():
            item.labelcolor(c)
            count = count + 1
        item = tree.next(item)
    # NO ITEMS SELECTED? DO ALL
    if count == 0:
        item = tree.first()
        while item != None:
            item.labelcolor(c)
            item = tree.next(item)
    tree.redraw()

menu_labelcolor_chooser = (
 ("Black", 0,  0, 0, 0, FL_NORMAL_LABEL, 0, 11, 0),
 ("Red", 0,  0, 0, 0, FL_NORMAL_LABEL, 0, 11, 0),
 ("Green", 0,  0, 0, 0, FL_NORMAL_LABEL, 0, 11, 0),
 ("Blue", 0,  0, 0, 0, FL_NORMAL_LABEL, 0, 11, 0),
 (None,))

def cb_selectmode_chooser(selectmode_chooser, tree):
    # Set selection mode
    if selectmode_chooser.value() == 0:
        tree.selectmode(FL_TREE_SELECT_NONE)
    elif selectmode_chooser.value() == 1:
        tree.selectmode(FL_TREE_SELECT_SINGLE)
    elif selectmode_chooser.value() == 2:
        tree.selectmode(FL_TREE_SELECT_MULTI)
    else:
        tree.selectmode(FL_TREE_SELECT_SINGLE)

menu_selectmode_chooser = (
 ("None", 0,  0, 0, 0, FL_NORMAL_LABEL, 0, 11, 0),
 ("Single", 0,  0, 0, 0, FL_NORMAL_LABEL, 0, 11, 0),
 ("Multi", 0,  0, 0, 0, FL_NORMAL_LABEL, 0, 11, 0),
 (None,))

def cb_whenmode_chooser(whenmode_chooser, tree):
    # Set when mode
    if  whenmode_chooser.value() == 0:
        tree.when(FL_WHEN_RELEASE)
    elif  whenmode_chooser.value() == 1:
        tree.when(FL_WHEN_CHANGED)
    elif  whenmode_chooser.value() == 2:
        tree.when(FL_WHEN_NEVER)
    else:
        tree.when(FL_WHEN_RELEASE)

menu_whenmode_chooser = (
 ("Changed", 0,  0, 0, 0, FL_NORMAL_LABEL, 0, 11, 0),
 ("Released", 0,  0, 0, 0, FL_NORMAL_LABEL, 0, 11, 0),
 ("Never", 0,  0, 0, 0, FL_NORMAL_LABEL, 0, 11, 0),
 (None,))


def cb_Show(show_btn, tree):
    item = tree.next_selected_item()
    print(item)
    tree.show_item(item)

def cb_Top(top_btn, tree):
    item = tree.next_selected_item()
    print(item)
    tree.show_item_top(item)


def cb_Mid(mid_btn, tree):
    item = tree.next_selected_item()
    tree.show_item_middle(item)


def cb_Bot(bot_btn, tree):
    item = tree.next_selected_item()
    tree.show_item_bottom(item)

def cb_selectall_button(selectall_button, tree):
    tree.select_all(None)
    tree.redraw()


def cb_deselectall_button(deselectall_button, tree):
    tree.deselect_all(None)
    tree.redraw()

def cb_bbbselect_toggle(bbbselect_toggle, tree):
    # Toggle select of just the Bbb item (not children)
    bbb = tree.find_item("/Bbb")
    if bbb == None:
        fl_alert("FAIL: Couldn't find item '/Bbb'???")
        return

    onoff = bbbselect_toggle.value()
    if onoff != 0:
        tree.select(bbb)	# select /Bbb
    else:
        tree.deselect(bbb)	# deselect /Bbb

def cb_bbbselect2_toggle(bbbselect2_toggle, tree):
    # Toggle select of just the Bbb item and its immediate children
    bbb = tree.find_item("/Bbb")
    if bbb == None:
        fl_alert("FAIL: Couldn't find item '/Bbb'???")
        return

    onoff = bbbselect2_toggle.value()
    if onoff != 0:
        tree.select_all(bbb)	# select /Bbb
    else:
        tree.deselect_all(bbb)	# deselect /Bbb

def cb_bbbchild02select_toggle(bbbchild02select_toggle, tree):
    # Toggle select of just the /Bbb/child-02 item
    onoff = bbbchild02select_toggle.value()
    if onoff != 0:
        tree.select("/Bbb/child-02")
    else:
        tree.deselect("/Bbb/child-02")

def cb_rootselect_toggle(rootselect_toggle, tree):
    # Toggle select of ROOT item and its children
    item = tree.find_item("/ROOT")
    if item == None:
        fl_alert("FAIL: Couldn't find item '/ROOT'???")
        return

    onoff = rootselect_toggle.value()
    if onoff != 0:
        tree.select(item)	# select /ROOT and its children
    else:
        tree.deselect(item)	# deselect /ROOT and its children

def cb_rootselect2_toggle(rootselect2_toggle, tree):
    # Toggle select of ROOT item and its children
    item = tree.find_item("/ROOT")
    if item == None:
        fl_alert("FAIL: Couldn't find item '/ROOT'???")
        return

    onoff = rootselect_toggle.value()
    if onoff != 0:
        tree.select_all(item)	# select /ROOT and its children
    else:
        tree.deselect_all(item)	# deselect /ROOT and its children


def cb_deactivate_toggle(deactivate_toggle, tree):
    onoff = deactivate_toggle.value()
    if onoff != 0:
        onoff = 1

    count = 0
    item=tree.first()
    while item != None:
        if item.is_selected():
            item.activate(onoff)
            count = count+1
        item = tree.next(item)

    if count == 0:
        item=tree.first()
        while item != None:
            item.activate(onoff)
            item = tree.next(item)
    tree.redraw()

def cb_bold_toggle(bold_toggle, tree):
    face = FL_HELVETICA
    if bold_toggle.value() != 0:
        face = FL_HELVETICA_BOLD

    # DO SELECTED ITEMS
    count = 0
    item=tree.first()
    while item != None:
        if item.is_selected():
            item.labelfont(face)
            count = count+1
        item = tree.next(item)

    # NO ITEMS SELECTED? DO ALL
    if count == 0:
        item=tree.first()
        while item != None:
            item.labelfont(face)
            item = tree.next(item)

    tree.redraw()


def cb_loaddb_button(loaddb_button, tree):
    filename = fl_file_chooser("Select a Preferences style Database", "Preferences(*.prefs)", "")
    if filename and len(filename) > 0:
        tree.clear()
        prefs = Fl_Preferences(filename, int(0), int(0))
        tree.load(prefs)
        tree.redraw()

def cb_insertabove_button(insertabove_button, tree):
    item=tree.first()
    while item != None:
        if item.is_selected():
            tree.insert_above(item, "AaaAaa")
            tree.insert_above(item, "BbbBbb")
            tree.insert_above(item, "CccCcc")
        item = next(item)
    tree.redraw()

def cb_rebuildtree_button(rebuildtree_button, tree):
    RebuildTree(tree)

def cb_showpathname_button(showpathname_button, tree):
    item = tree.first_selected_item()
    if item == None:
        fl_message("No item was selected")
        return

    pathname = ""
    ret_val =  tree.item_pathname(pathname, len(pathname), item)
    if ret_val == 0:
        msg = item.label()
        if msg == None or msg == "":
            msg = "???"
        fl_message("Pathname for '%s' is: \"%s\"", msg, pathname)
    elif ret_val == -1:
        fl_message("item_pathname() returned -1 (NOT FOUND)")
    elif ret_val == -2:
        fl_message("item_pathname() returned -2 (STRING TOO LONG)")

def cb_showselected_button(showselected_button, tree):
    print("--- SELECTED ITEMS")
    item = tree.first_selected_item()
    while item != None:
        msg = item.label()
        if msg == None or msg == "":
            msg = "???"
        print(f"\t{msg}\n")
        item = tree.next_selected_item(item)

def cb_clearselected_button(clearselected_button, tree):
    item=tree.first()
    while item != None:
        if item.is_selected():
            if tree.remove(item) == -1:
                break
            item = tree.first()
        else:
            item = next(item)
    tree.redraw()

def cb_clearall_button(clearall_button, tree):
    tree.clear()
    tree.redraw()


def cb_testcallbackflag_button(testcallbackflag_button, tree):
    global G_cb_counter
    root = tree.root()
    print("--- Checking docallback off\n")

    # "OFF" TEST

    # open/close: Make sure these methods don't trigger cb
    G_cb_counter = 0
    tree.close(root, 0)
    if G_cb_counter != 0:
        fl_alert("FAILED 'OFF' TEST\n close(item) triggered cb!")
    G_cb_counter = 0
    tree.open(root, 0)
    if G_cb_counter != 0:
        fl_alert("FAILED 'OFF' TEST\n open(item) triggered cb!")
    G_cb_counter = 0
    tree.open_toggle(root, 0)
    if G_cb_counter != 0:
        fl_alert("FAILED 'OFF' TEST\n open_toggle(item) triggered cb!")
    G_cb_counter = 0
    tree.open("ROOT", 0)
    if G_cb_counter != 0:
        fl_alert("FAILED 'OFF' TEST\n open(path) triggered cb!")
    G_cb_counter = 0
    tree.close("ROOT", 0)
    if G_cb_counter != 0:
        fl_alert("FAILED 'OFF' TEST\n close(path) triggered cb!")
    tree.open(root,0)	# leave root open

    # select/deselect: Make sure these methods don't trigger cb
    G_cb_counter = 0
    tree.select(root, 0)
    if G_cb_counter != 0:
        fl_alert("FAILED 'OFF' TEST\n select(item) triggered cb!")
    G_cb_counter = 0
    tree.deselect(root, 0)
    if G_cb_counter != 0:
        fl_alert("FAILED 'OFF' TEST\n deselect(item) triggered cb!")
    G_cb_counter = 0
    tree.select_toggle(root, 0);
    if G_cb_counter != 0:
        fl_alert("FAILED 'OFF' TEST\n select_toggle(item) triggered cb!")
    G_cb_counter = 0
    tree.deselect("ROOT", 0)
    if G_cb_counter != 0:
        fl_alert("FAILED 'OFF' TEST\n deselect(path) triggered cb!")
    G_cb_counter = 0
    tree.select("ROOT", 0);
    if G_cb_counter != 0:
        fl_alert("FAILED 'OFF' TEST\n select(path) triggered cb!")
    tree.deselect("ROOT") # leave deselected

    # "ON" TEST

    # open/close: Make sure these methods don't trigger cb
    G_cb_counter = 0
    tree.close(root, 1)
    if G_cb_counter == 0:
        fl_alert("FAILED 'ON' TEST\n close(item) cb wasn't triggered!")
    G_cb_counter = 0
    tree.open(root, 1)
    if G_cb_counter == 0:
        fl_alert("FAILED 'ON' TEST\n open(item) cb wasn't triggered!")
    G_cb_counter = 0
    tree.open_toggle(root, 1);
    if G_cb_counter == 0:
        fl_alert("FAILED 'ON' TEST\n open_toggle(item) cb wasn't triggered!")
    G_cb_counter = 0
    tree.open(root, 1)
    if G_cb_counter == 0:
        fl_alert("FAILED 'ON' TEST\n open(item)[2] cb wasn't triggered!")
    G_cb_counter = 0
    tree.close(root, 1)
    if G_cb_counter == 0:
        fl_alert("FAILED 'ON' TEST\n close(item)[2] cb wasn't triggered!")
    G_cb_counter = 0
    tree.open("ROOT", 1);
    if G_cb_counter == 0:
        fl_alert("FAILED 'ON' TEST\n open(path) cb wasn't triggered!")
    G_cb_counter = 0
    tree.close("ROOT", 1)
    if G_cb_counter == 0:
        fl_alert("FAILED 'ON' TEST\n close(path) cb wasn't triggered!")
    tree.open(root,0)	# leave root open

    # select/deselect: Make sure these methods don't trigger cb
    G_cb_counter = 0
    tree.select(root, 1)
    if G_cb_counter == 0:
        fl_alert("FAILED 'ON' TEST\n select(item) cb wasn't triggered!")
    G_cb_counter = 0
    tree.deselect(root, 1)
    if G_cb_counter == 0:
        fl_alert("FAILED 'ON' TEST\n deselect(item) cb wasn't triggered!")
    G_cb_counter = 0
    tree.select_toggle(root, 1)
    if G_cb_counter == 0:
        fl_alert("FAILED 'ON' TEST\n select_toggle(item) cb wasn't triggered!")
    G_cb_counter = 0
    tree.deselect("ROOT", 1)
    if G_cb_counter == 0:
        fl_alert("FAILED 'ON' TEST\n deselect(path) cb wasn't triggered!")
    G_cb_counter = 0
    tree.select("ROOT", 1)
    if G_cb_counter == 0:
        fl_alert("FAILED 'ON' TEST\n select(path) cb wasn't triggered!")
    tree.deselect("ROOT")  # leave deselected

    # "default" TEST (should be same as 'on'

    # open/close: Make sure these methods don't trigger cb
    G_cb_counter = 0
    tree.close(root)
    if G_cb_counter == 0:
        fl_alert("FAILED 'DEFAULT' TEST: close(item) cb wasn't triggered!")
    G_cb_counter = 0
    tree.open(root)
    if G_cb_counter == 0:
        fl_alert("FAILED 'DEFAULT' TEST: open(item) cb wasn't triggered!")
    G_cb_counter = 0
    tree.open_toggle(root)
    if G_cb_counter == 0:
        fl_alert("FAILED 'DEFAULT' TEST: open_toggle(item) cb wasn't triggered!")
    G_cb_counter = 0
    tree.open("ROOT")
    if G_cb_counter == 0:
        fl_alert("FAILED 'DEFAULT' TEST: open(path) cb wasn't triggered!")
    G_cb_counter = 0
    tree.close("ROOT")
    if G_cb_counter == 0:
        fl_alert("FAILED 'DEFAULT' TEST: close(path) cb wasn't triggered!")
    tree.open(root,0)	# leave root open

    # select/deselect: Make sure these methods don't trigger cb
    G_cb_counter = 0
    tree.select(root)
    if G_cb_counter == 0:
        fl_alert("FAILED 'DEFAULT' TEST\n select(item) cb wasn't triggered!")
    G_cb_counter = 0
    tree.deselect(root)
    if G_cb_counter == 0:
        fl_alert("FAILED 'DEFAULT' TEST\n deselect(item) cb wasn't triggered!")
    G_cb_counter = 0
    tree.select_toggle(root)
    if G_cb_counter == 0:
        fl_alert("FAILED 'DEFAULT' TEST\n select_toggle(item) cb wasn't triggered!")
    G_cb_counter = 0
    tree.deselect("ROOT")
    if G_cb_counter == 0:
        fl_alert("FAILED 'DEFAULT' TEST\n deselect(path) cb wasn't triggered!")
    G_cb_counter = 0
    tree.select("ROOT")
    if G_cb_counter == 0:
        fl_alert("FAILED 'DEFAULT' TEST\n select(path) cb wasn't triggered!")
    tree.deselect("ROOT") # leave deselected

    fl_alert("TEST COMPLETED\n If you didn't see any error dialogs, test PASSED.")
    
if __name__ == '__main__':
    window = Fl_Double_Window(580, 695, "tree")
    
    # tree
    tree = Fl_Tree(15, 15, 550, 390)
    tree.box(FL_DOWN_BOX)
    tree.color(55)
    tree.selection_color(FL_BACKGROUND_COLOR)
    tree.labeltype(FL_NORMAL_LABEL)
    tree.labelfont(0)
    tree.labelsize(14)
    tree.labelcolor(FL_FOREGROUND_COLOR)
    tree.callback(cb_tree, 1234)
    tree.align(FL_ALIGN_TOP)
    tree.when(FL_WHEN_RELEASE)
    tree.end()

    # margintop_slider
    margintop_slider = Fl_Value_Slider(190, 414, 240, 16, "margintop()")
    margintop_slider.tooltip("Changes the top margin for the tree widget")
    margintop_slider.type(1)
    margintop_slider.labelsize(12)
    margintop_slider.step(0.01)
    margintop_slider.textsize(12)
    margintop_slider.callback(cb_margintop_slider, tree)
    margintop_slider.align(FL_ALIGN_LEFT)
    margintop_slider.value(tree.margintop())
    margintop_slider.range(0.0, 100.0)
    margintop_slider.step(1.0)
    margintop_slider.color(46)
    margintop_slider.selection_color(FL_RED)

    # marginleft_slider
    marginleft_slider = Fl_Value_Slider(190, 434, 240, 16, "marginleft()")
    marginleft_slider.tooltip("Changes the left margin for the tree widget")
    marginleft_slider.type(1)
    marginleft_slider.labelsize(12)
    marginleft_slider.step(0.01)
    marginleft_slider.textsize(12)
    marginleft_slider.callback(cb_marginleft_slider, tree)
    marginleft_slider.align(FL_ALIGN_LEFT)
    marginleft_slider.value(tree.marginleft())
    marginleft_slider.range(0.0, 100.0)
    marginleft_slider.step(1.0)
    marginleft_slider.color(46)
    marginleft_slider.selection_color(FL_RED)

    # openchild_marginbottom_slider
    openchild_marginbottom_slider = Fl_Value_Slider(190, 454, 240, 16, "openchild_marginbottom()")
    openchild_marginbottom_slider.tooltip("Changes the vertical space below an open child tree")
    openchild_marginbottom_slider.type(1)
    openchild_marginbottom_slider.labelsize(12)
    openchild_marginbottom_slider.step(0.01)
    openchild_marginbottom_slider.textsize(12)
    openchild_marginbottom_slider.callback(cb_openchild_marginbottom_slider, tree)
    openchild_marginbottom_slider.align(FL_ALIGN_LEFT)
    openchild_marginbottom_slider.value(tree.openchild_marginbottom())
    openchild_marginbottom_slider.range(0.0, 100.0)
    openchild_marginbottom_slider.step(1.0)
    openchild_marginbottom_slider.color(46)
    openchild_marginbottom_slider.selection_color(FL_RED)

    # labelsize_slider
    labelsize_slider = Fl_Value_Slider(190, 474, 240, 16, "Text size")
    labelsize_slider.tooltip("Changes the font size of the selected items\nIf none selected, all are changed")
    labelsize_slider.type(1)
    labelsize_slider.labelsize(12)
    labelsize_slider.step(0.01)
    labelsize_slider.textsize(12)
    labelsize_slider.callback(cb_labelsize_slider, tree)
    labelsize_slider.align(FL_ALIGN_LEFT)
    labelsize_slider.value(tree.labelsize())
    labelsize_slider.range(5.0, 200.0)
    labelsize_slider.step(1.0)
    labelsize_slider.color(46)
    labelsize_slider.selection_color(FL_RED)

    # connectorwidth_slider
    connectorwidth_slider = Fl_Value_Slider(190, 494, 240, 16, "Connector width")
    connectorwidth_slider.tooltip("Tests Fl_Tree::connectorwidth()")
    connectorwidth_slider.type(1)
    connectorwidth_slider.labelsize(12)
    connectorwidth_slider.step(0.01)
    connectorwidth_slider.textsize(12)
    connectorwidth_slider.callback(cb_connectorwidth_slider, tree)
    connectorwidth_slider.align(FL_ALIGN_LEFT)
    connectorwidth_slider.value(tree.connectorwidth())
    connectorwidth_slider.range(1.0, 100.0)
    connectorwidth_slider.step(1.0)
    connectorwidth_slider.color(46)
    connectorwidth_slider.selection_color(FL_RED)

    # usericon_radio
    usericon_radio =  Fl_Check_Button(90, 525, 130, 16, "Enable user icons?")
    usericon_radio.tooltip("Tests Fl_Tree_Item::usericon()")
    usericon_radio.down_box(FL_DOWN_BOX)
    usericon_radio.labelsize(11)
    usericon_radio.callback(cb_usericon_radio, tree)

    # showroot_radio
    showroot_radio = Fl_Check_Button(90, 542, 130, 16, "Show root?")
    showroot_radio.tooltip("Tests Fl_Tree_Item::usericon()")
    showroot_radio.down_box(FL_DOWN_BOX)
    showroot_radio.labelsize(11)
    showroot_radio.callback(cb_showroot_radio, tree)
    onoff = tree.showroot()
    showroot_radio.value(onoff)

    # visiblefocus_checkbox
    visiblefocus_checkbox = Fl_Check_Button(90, 559, 130, 16, "Visible focus?")
    visiblefocus_checkbox.tooltip("Toggles the tree\'s visible_focus()\nThis toggles the visible \'focus box\'")
    visiblefocus_checkbox.down_box(FL_DOWN_BOX)
    visiblefocus_checkbox.labelsize(11)
    visiblefocus_checkbox.callback(cb_visiblefocus_checkbox, tree)
    onoff = tree.visible_focus()
    visiblefocus_checkbox.value(onoff)

    # collapseicons_chooser
    collapseicons_chooser = Fl_Choice(115, 589, 110, 16, "Collapse icons")
    collapseicons_chooser.tooltip("Tests Fl_Tree::openicon() and Fl_Tree::closeicon()")
    collapseicons_chooser.down_box(FL_BORDER_BOX)
    collapseicons_chooser.labelsize(11)
    collapseicons_chooser.textsize(11)
    collapseicons_chooser.callback(cb_collapseicons_chooser, tree)
    collapseicons_chooser.menu(menu_collapseicons_chooser)

    # connectorstyle_chooser
    connectorstyle_chooser = Fl_Choice(115, 609, 110, 16, "Line style")
    connectorstyle_chooser.tooltip("Tests connectorstyle() bit flags")
    connectorstyle_chooser.down_box(FL_BORDER_BOX)
    connectorstyle_chooser.labelsize(11)
    connectorstyle_chooser.textsize(11)
    connectorstyle_chooser.callback(cb_connectorstyle_chooser, tree)
    connectorstyle_chooser.menu(menu_connectorstyle_chooser)
    if tree.connectorstyle() == FL_TREE_CONNECTOR_NONE:
        connectorstyle_chooser.value(0)
    elif  tree.connectorstyle() == FL_TREE_CONNECTOR_DOTTED:
        connectorstyle_chooser.value(1)
    elif  tree.connectorstyle() == FL_TREE_CONNECTOR_SOLID:
        connectorstyle_chooser.value(2)

    # labelcolor_chooser
    labelcolor_chooser = Fl_Choice(115, 629, 110, 16, "Item Text Color")
    labelcolor_chooser.tooltip("Changes the label color for the selected items\nIf no items selected, all are\changed")
    labelcolor_chooser.down_box(FL_BORDER_BOX)
    labelcolor_chooser.labelsize(11)
    labelcolor_chooser.textsize(11)
    labelcolor_chooser.callback(cb_labelcolor_chooser, tree)
    labelcolor_chooser.menu(menu_labelcolor_chooser)
    
    # selectmode_chooser
    selectmode_chooser = Fl_Choice(115, 649, 110, 16, "Selection Mode")
    selectmode_chooser.tooltip("Sets how Fl_Tree handles mouse selection of tree items")
    selectmode_chooser.down_box(FL_BORDER_BOX)
    selectmode_chooser.labelsize(11)
    selectmode_chooser.textsize(11)
    selectmode_chooser.callback(cb_selectmode_chooser, tree)
    selectmode_chooser.menu(menu_selectmode_chooser)
    selectmode_chooser.value(1)
    cb_selectmode_chooser(selectmode_chooser, tree)

    # whenmode_chooser
    whenmode_chooser = Fl_Choice(115, 669, 110, 16, "When")
    whenmode_chooser.tooltip("Sets when() the tree\'s callback is invoked")
    whenmode_chooser.down_box(FL_BORDER_BOX)
    whenmode_chooser.labelsize(11)
    whenmode_chooser.textsize(11)
    whenmode_chooser.callback(cb_whenmode_chooser, tree)
    whenmode_chooser.menu(menu_whenmode_chooser)
    whenmode_chooser.value(1)
    cb_whenmode_chooser(whenmode_chooser, tree)

    # showitem_box
    showitem_box = Fl_Box(480, 425, 70, 82, "show_item()\n")
    showitem_box.box(FL_GTK_DOWN_BOX)
    showitem_box.color(FL_DARK1)
    showitem_box.labelsize(11)
    showitem_box.align(FL_ALIGN_TOP)

    show_btn = Fl_Button(495, 434, 40, 17, "Show")
    show_btn.tooltip("Tests show_item() with no position specified.\nMakes the selected item visibl\e IF it is off-screen.\nNo change made if it is not off-screen.")
    show_btn.labelsize(11)
    show_btn.callback(cb_Show, tree)

    top_btn = Fl_Button(495, 451, 40, 16, "Top")
    top_btn.tooltip("Test show_item_top().\nScrolls selected item to the top of the display\n(only\ works if scrollbar showing)\nTo use:\n1) open \'500 items\'\n2) select item 0\010\n3) Hit Top/Mid/Bot")
    top_btn.labelsize(11)
    top_btn.callback(cb_Top, tree)

    mid_btn = Fl_Button(495, 467, 40, 16, "Mid")
    mid_btn.tooltip("Tests show_item_middle().\nScrolls the selected item to the middle of the dis\play\nTo use:\n  1) open \'500 items\'\n  2) select \'item 0010\'\n  3) Hit To\p/Mid/Bot")
    mid_btn.labelsize(11)
    mid_btn.callback(cb_Mid, tree)

    bot_btn = Fl_Button(495, 483, 40, 16, "Bot")
    bot_btn.tooltip("Tests show_item_bottom().\nScrolls the selected item to the bottom of the dis\play\nTo use:\n  1) open \'500 items\'\n  2) select \'item 0010\'\n  3) Hit To\p/Mid/Bot")
    bot_btn.labelsize(11)
    bot_btn.callback(cb_Bot, tree)

    docallback_box = Fl_Box(245, 527, 320, 77, "Selection State Changes")
    docallback_box.box(FL_GTK_DOWN_BOX)
    docallback_box.color(FL_DARK1)
    docallback_box.labelsize(12)
    docallback_box.align(FL_ALIGN_TOP)

    selectall_button = Fl_Button(260, 539, 75, 16, "Select All")
    selectall_button.tooltip("Selects all items in the tree")
    selectall_button.labelsize(9)
    selectall_button.callback(cb_selectall_button, tree)

    deselectall_button = Fl_Button(260, 559, 75, 16, "Deselect All")
    deselectall_button.tooltip("Deselects all items in the tree")
    deselectall_button.labelsize(9)
    deselectall_button.callback(cb_deselectall_button, tree)

    bbbselect_toggle = Fl_Light_Button(350, 540, 95, 15, " Select Bbb")
    bbbselect_toggle.tooltip("Toggle selection of just the /Bbb item\n(Not children)")
    bbbselect_toggle.selection_color(1)
    bbbselect_toggle.labelsize(9)
    bbbselect_toggle.callback(cb_bbbselect_toggle, tree)

    bbbselect2_toggle = Fl_Light_Button(350, 560, 95, 15, " Select Bbb+")
    bbbselect2_toggle.tooltip("Toggle selection of the /Bbb item and its children")
    bbbselect2_toggle.selection_color(1)
    bbbselect2_toggle.labelsize(9)
    bbbselect2_toggle.callback(cb_bbbselect2_toggle, tree)

    bbbchild02select_toggle = Fl_Light_Button(350, 579, 95, 16, " Toggle child-02")
    bbbchild02select_toggle.tooltip("Toggle the single item /Bbb/child-02")
    bbbchild02select_toggle.selection_color(1)
    bbbchild02select_toggle.labelsize(9)
    bbbchild02select_toggle.callback(cb_bbbchild02select_toggle, tree)
    
    rootselect_toggle = Fl_Light_Button(460, 540, 90, 15, "Select ROOT")
    rootselect_toggle.tooltip("Toggle selection of the ROOT item")
    rootselect_toggle.selection_color(1)
    rootselect_toggle.labelsize(9)
    rootselect_toggle.callback(cb_rootselect_toggle, tree)

    rootselect2_toggle = Fl_Light_Button(460, 560, 90, 15, "Select ROOT+")
    rootselect2_toggle.tooltip("Toggle selection of the ROOT item and all children")
    rootselect2_toggle.selection_color(1)
    rootselect2_toggle.labelsize(9)
    rootselect2_toggle.callback(cb_rootselect2_toggle, tree)

    deactivate_toggle = Fl_Light_Button(280, 634, 90, 16, " Deactivate")
    deactivate_toggle.tooltip("Toggle the deactivation state of the selected items.\nIf none are selected, a\ll are set.")
    deactivate_toggle.selection_color(1)
    deactivate_toggle.labelsize(9)
    deactivate_toggle.callback(cb_deactivate_toggle, tree)

    bold_toggle = Fl_Light_Button(280, 654, 90, 16, " Bold Font")
    bold_toggle.tooltip("Toggles bold font for selected items\nIf nothing selected, all are changed")
    bold_toggle.selection_color(1)
    bold_toggle.labelsize(9)
    bold_toggle.callback(cb_bold_toggle, tree)

    loaddb_button = Fl_Button(380, 614, 90, 16, "Load Database...")
    loaddb_button.tooltip("Load the contents of an Fl_Preferences database into the tree view")
    loaddb_button.labelsize(9)
    loaddb_button.callback(cb_loaddb_button, tree)

    insertabove_button = Fl_Button(380, 634, 90, 16, "Insert Above")
    insertabove_button.tooltip("Inserts three items above the selected items")
    insertabove_button.labelsize(9)
    insertabove_button.callback(cb_insertabove_button, tree)

    rebuildtree_button = Fl_Button(380, 654, 90, 16, "Rebuild Tree")
    rebuildtree_button.tooltip("Rebuilds the tree with defaults")
    rebuildtree_button.labelsize(9)
    rebuildtree_button.callback(cb_rebuildtree_button, tree)

    showpathname_button = Fl_Button(380, 674, 90, 16, "Show Pathname")
    showpathname_button.tooltip("Show the pathname for the selected item. Tests the Fl_Tree::item_pathname() m\ethod.")
    showpathname_button.labelsize(8)
    showpathname_button.callback(cb_showpathname_button, tree)

    showselected_button = Fl_Button(475, 614, 90, 16, "Show Selected")
    showselected_button.tooltip("Clears the selected items")
    showselected_button.labelsize(9)
    showselected_button.callback(cb_showselected_button, tree)

    clearselected_button = Fl_Button(475, 634, 90, 16, "Remove Selected")
    clearselected_button.tooltip("Removes the selected items")
    clearselected_button.labelsize(9)
    clearselected_button.callback(cb_clearselected_button, tree)

    clearall_button = Fl_Button(475, 654, 90, 16, "Clear All")
    clearall_button.tooltip("Clears all items\nTests Fl_Tree::clear()")
    clearall_button.labelsize(9)
    clearall_button.callback(cb_clearall_button, tree)

    testcallbackflag_button = Fl_Button(475, 674, 90, 16, "Test Callback Flag")
    testcallbackflag_button.tooltip("Test the \'docallback\' argument can disable callbacks.")
    testcallbackflag_button.labelsize(8)
    testcallbackflag_button.callback(cb_testcallbackflag_button, tree)
    window.end() 

    tree.root_label("ROOT")
    RebuildTree(tree)

    window.resizable(window)
    window.size_range(window.w(), window.h(), 0, 0)

    if tree.when() == FL_WHEN_CHANGED:
        whenmode_chooser.value(0)
    elif tree.when() == FL_WHEN_RELEASE:
        whenmode_chooser.value(1)
    elif tree.when() == FL_WHEN_NEVER:
        whenmode_chooser.value(2)
    
    window.show()

    
