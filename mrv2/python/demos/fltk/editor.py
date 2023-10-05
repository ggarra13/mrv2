#!/usr/bin/env python
# $Id: editor.py 198 2006-02-16 07:22:28Z andreasheld $
# Author: Anderas Held
# Purpose: test text editor
# Created: Thu Jan 22 14:06:24 EST 2004
#
#
# Text Editor test program for pyFLTK the Python bindings
# for the Fast Light Tool Kit (FLTK).
#
# FLTK copyright 1998-1999 by Bill Spitzak and others.
# pyFLTK copyright 2003-2006 by Andreas Held and others.
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
import string, os.path, sys
from keyword import iskeyword

changed = False
filename = ""
title = ""
textbuf = None
editor = None
window = None

# Syntax highlighting stuff...
stylebuf = None

styletable = [ # Style table
    [ FL_BLACK,      FL_COURIER,        14 ], # A - Plain
    [ FL_DARK_GREEN, FL_COURIER_ITALIC, 14 ], # B - Line comments
    [ FL_DARK_GREEN, FL_COURIER_ITALIC, 14 ], # C - Block comments
    [ FL_BLUE,       FL_COURIER,        14 ], # D - Strings
    [ FL_DARK_RED,   FL_COURIER,        14 ], # E - Directives
    [ FL_DARK_RED,   FL_COURIER_BOLD,   14 ], # F - Types
    [ FL_BLUE,       FL_COURIER_BOLD,   14 ]  # G - Keywords
]

# 'compare_keywords()' - Compare two keywords...

def compare_keywords(a, b):
    return String.compare(a,b)

#
# 'style_parse()' - Parse text and produce style data.
#
def style_parse(text, style, length):
    # Style letters:
    #
    # A - Plain
    # B - Line comments
    # C - Block comments
    # D - Strings
    # E - Directives
    # F - Types
    # G - Keywords
    col = 0
    new_style = ""
    isComment = False
    isString = False

    pos = 0
    last = False
    while pos < length:
        current = style[pos]
        if isComment:
            current = 'B'
            if text[pos] == '\n':
                isComment = False
        elif isString:
            current = 'D'
            if text[pos] == '"' or text[pos] == "'":
                isString = False
        elif current in ['B', 'F', 'G']:
            current = 'A'

        if current == 'A':
            # Check for directives, comments, strings, and keywords...
            if text[pos] == '#':
                current = 'B'
                isComment = True
            elif text[pos] == '"' or text[pos] == "'":
                current = 'D'
                isString = True
            elif not last and text[pos].islower() or text[pos] == '_':
                pos1 = pos
                while pos1 < length:
                    if text[pos1].islower() or text[pos1] == '_':
                        pos1 += 1
                    else:
                        break
                
                kw = text[pos:pos1]
                if iskeyword(kw):
                    new_style = new_style+'F'*(pos1-pos)
                    current = 'F'
                    if pos1 >= length:
                        pos1 -= 1
                    pos = pos1
                    last = True
        
        last = text[pos].isalnum() or text[pos] == '_' or text[pos] == '.'
        if text[pos] == '\n':
            col = 0
            if current == 'B' or current == 'E':
                current = 'A'
        new_style = new_style+current
        pos += 1
    #print "new style: ", new_style
    return  new_style

def style_init():
    global stylebuf
    style = "A"*(textbuf.length())
    text = textbuf.text()
    if stylebuf == None:
        stylebuf = Fl_Text_Buffer(textbuf.length())
    style = style_parse(text, style, textbuf.length())
    stylebuf.text(style)

def style_unfinished_cb(arg1, arg2):
    #print "style_unfinished_cb"
    pass

#
# 'style_update()' - Update the style buffer...
#
def style_update(pos, nInserted, nDeleted, nRestyled, deletedText, e):
    global stylebuf, editor

    if nInserted == 0 and nDeleted == 0:
        stylebuf.unselect()
    else:
        if nInserted > 0:
            style = 'A'*nInserted
            stylebuf.replace(pos, pos+nDeleted, style)
        else:
            # Just delete characters in the style buffer...
            stylebuf.remove(pos, pos+nDeleted)

        # Select the area that was just updated to avoid unnecessary
        # callbacks...
        stylebuf.select(pos, pos + nInserted - nDeleted)
        # Re-parse the changed region; we do this by parsing from the
        # beginning of the previous line of the changed region to the end of
        # the line of the changed region...  Then we check the last
        # style character and keep updating if we have a multi-line
        # comment character...
        start = textbuf.line_start(pos)
        end   = textbuf.line_end(pos + nInserted)
        text  = textbuf.text_range(start, end)
        style = stylebuf.text_range(start, end)
        if start==end:
            last = 0
        else:
            last  = style[end - start - 1]

        style = style_parse(text, style, end - start)
        stylebuf.replace(start, end, style)
        editor.editor.redisplay_range(start, end)
        if start == end or last != style[end - start - 1]:
            # Either the user deleted some text, or the last character 
            # on the line changed styles, so reparse the
            # remainder of the buffer...
            end   = textbuf.length()
            text  = textbuf.text_range(start, end)
            style = stylebuf.text_range(start, end)

            style_parse(text, style, end - start)

            stylebuf.replace(start, end, style)
            editor.editor.redisplay_range(start, end)
    

class EditorWindow(Fl_Double_Window):
    search = ""
    def __init__(self, w, h, label):
        Fl_Double_Window.__init__(self, w, h, label)
        self.replace_dlg = Fl_Window(300, 105, "Replace")
        self.replace_find = Fl_Input(80, 10, 210, 25, "Find:")
        self.replace_find.align(FL_ALIGN_LEFT)

        self.replace_with = Fl_Input(80, 40, 210, 25, "Replace:")
        self.replace_with.align(FL_ALIGN_LEFT)

        self.replace_all = Fl_Button(10, 70, 90, 25, "Replace All")
        self.replace_all.callback(replall_cb, self)

        self.replace_next = Fl_Return_Button(105, 70, 120, 25, "Replace Next")
        self.replace_next.callback(replace2_cb, self)

        self.replace_cancel = Fl_Button(230, 70, 60, 25, "Cancel")
        self.replace_cancel.callback(replcan_cb, self)
        self.replace_dlg.end()
        self.replace_dlg.set_non_modal()
        self.editor = 0

def check_save():
    global changed
    if not changed:
        return

    r = fl_choice("The current file has not been saved.\n"
                    "Would you like to save it now?",
                    "Cancel", "Save", "Don't Save")

    if r == 1:
        save_cb(None)
        return not changed

    if r == 2:
        return 1
    else:
        return 0

loading = False

def load_file(newfile, ipos):
    global changed, loading, filename
    loading = True
    if ipos != -1:
        insert = 1
        changed = True
    else:
        insert = 0
        changed = False
    if insert == 0:
        filename = ""
        r = textbuf.loadfile(newfile)
    else:
        r = textbuf.insertfile(newfile, ipos)
    if r != 0:
        fl_alert(f"Error reading from file {newfile}.")
    else:
        if insert == 0:
            filename = newfile
    loading = False
    textbuf.call_modify_callbacks()

def save_file(newfile):
    global changed, filename
    if textbuf.savefile(newfile) != 0:
        fl_alert(f"Error writing to file {newfile}.")
    else:
        filename = newfile
    changed = False
    textbuf.call_modify_callbacks()

def copy_cb(widget, editor):
    Fl_Text_Editor.kf_copy(0, editor.editor)

def cut_cb(widget, editor):
    Fl_Text_Editor.kf_cut(0, editor.editor)

def delete_cb(widget):
    global textbuf
    textbuf.remove_selection()

def find_cb(widget, editor):
    val = fl_input("Search String:", editor.search)
    if val != None:
        # User entered a string - go find it!
        editor.search = val
        find2_cb(widget)

def find2_cb(widget):
    if editor.search[0] == 0:
        # Search string is blank; get a new one...
        find_cb(widget, editor)
        return

    pos = editor.editor.insert_position();
    (found, pos) = textbuf.search_forward(pos, editor.search);
    if found!= 0:
        # Found a match; select and update the position...
        textbuf.select(pos, pos+len(editor.search))
        editor.editor.insert_position(pos+len(editor.search))
        editor.editor.show_insert_position()
    else:
        fl_alert(f"No occurrences of {editor.search} found!")

def set_title(win):
    global filename, title
    if len(filename) == 0:
        title = "Untitled"
    else:
        title = os.path.basename(filename)
    if changed:
        title = title+" (modified)"
    win.label(title)

def changed_cb(i1, nInserted, nDeleted, i2, c1, editor):
    global changed, loading

    if (nInserted != 0 or nDeleted != 0) and loading == False:
        changed = True
    set_title(editor);
    if loading:
        editor.editor.show_insert_position()

def new_cb(widget):
    global filename, changed
    if check_save() == 0:
        return
    filename = ""
    textbuf.select(0, textbuf.length())
    textbuf.remove_selection()
    changed = False
    textbuf.call_modify_callbacks()

def open_cb(widget):
    global filename
    if check_save() == 0:
        return
    newfile = fl_file_chooser("Open File?", "*", filename)
    if newfile != None:
        load_file(newfile, -1)

def insert_cb(widget, editor):
    global filename
    newfile = fl_file_chooser("Open File?", "*", filename)
    if newfile != None:
        load_file(newfile, editor.editor.insert_position())

def paste_cb(widget, editor):
    Fl_Text_Editor.kf_paste(0, editor.editor)

num_windows = 0

def close_cb(window, data):
    global num_windows
    if num_windows == 1 and check_save() == 0:
        return
    window.hide()
    textbuf.remove_modify_callback(changed_cb, window, window)
    num_windows -= 1;
    if num_windows == 0:
        sys.exit(0)

def quit_cb(widget, data):
    global changed
    if changed and check_save() == 0:
        return
    sys.exit(0)

def replace_cb(widget, editor):
    editor.replace_dlg.show()

def replace2_cb(widget, editor):
    find = editor.replace_find.value()
    replace = editor.replace_with.value()

    if len(find) == 0:
        editor.replace_dlg.show()
        return

    editor.replace_dlg.hide()

    pos = editor.editor.insert_position()
    (found, pos) = textbuf.search_forward(pos, find)

    if found != 0:
        # Found a match; update the position and replace text...
        textbuf.select(pos, pos+len(find))
        textbuf.remove_selection()
        textbuf.insert(pos, replace)
        textbuf.select(pos, pos+len(replace))
        editor.editor.insert_position(pos+len(replace))
        editor.editor.show_insert_position()
    else:
        fl_alert(f"No occurrences of {find} found!")

def replall_cb(widget, editor):
    find = editor.replace_find.value()
    replace = editor.replace_with.value()

    if len(find) == 0:
        editor.replace_dlg.show()
        return

    editor.replace_dlg.hide()
    editor.editor.insert_position(0)
    times = 0

    found = 1
    while found != 0:
        pos = editor.editor.insert_position()
        (found, pos) = textbuf.search_forward(pos, find)
        
        if found != 0:
            # Found a match; update the position and replace text...
            textbuf.select(pos, pos+len(find))
            textbuf.remove_selection()
            textbuf.insert(pos, replace)
            editor.editor.insert_position(pos+len(replace))
            editor.editor.show_insert_position()
            times += 1

    if times > 0:
        fl_message(f"Replaced {times} occurrences.")
    else:
        fl_alert(f"No occurrences of {find} found!")

def replcan_cb(widget, editor):
    editor.replace_dlg.hide()

def save_cb(widget):
    global filename
    if len(filename) == 0:
        # No filename - get one!
        saveas_cb(widget, 0)
        return
    else:
        save_file(filename)

def saveas_cb(widget, data):
    global filename
    newfile = fl_file_chooser("Save File As?", "*", filename)
    if newfile != None:
        save_file(newfile)

def view_cb(widget, data):
    win = new_view()
    win.show()

def new_view():
    global num_windows, editor, stylebuf, styletable
    w = EditorWindow(660, 400, title)
    editor = w
    w.begin()
    m = Fl_Menu_Bar(0, 0, 660, 30);
    menuitems = (( "&File",              0, 0, 0, FL_SUBMENU ),
                 ( "&New File",        0, new_cb ),
                 ( "&Open File...",    FL_CTRL + ord('o'), open_cb ),
                 ( "&Insert File...",  FL_CTRL + ord('i'), insert_cb, editor, FL_MENU_DIVIDER ),
                 ( "&Save File",       FL_CTRL + ord('s'), save_cb ),
                 ( "Save File &As...", FL_CTRL + FL_SHIFT + ord('s'), saveas_cb, 0, FL_MENU_DIVIDER ),
                 ( "New &View", FL_ALT + ord('v'),view_cb, 0 ),
                 ( "&Close View", FL_CTRL + ord('w'), close_cb, window, FL_MENU_DIVIDER ),
    ( "E&xit", FL_CTRL + ord('q'), quit_cb, 0 ),
                 ( None, 0 ),

                 ( "&Edit", 0, 0, 0, FL_SUBMENU ),
                 ( "Cu&t",        FL_CTRL + ord('x'), cut_cb, editor ),
                 ( "&Copy",       FL_CTRL + ord('c'), copy_cb, editor ),
                 ( "&Paste",      FL_CTRL + ord('v'), paste_cb, editor ),
                 ( "&Delete",     0, delete_cb ),
                 ( None, 0 ),

                 ( "&Search", 0, 0, 0, FL_SUBMENU ),
                 ( "&Find...",       FL_CTRL + ord('f'), find_cb, editor ),
                 ( "F&ind Again",    FL_CTRL + ord('g'), find2_cb ),
                 ( "&Replace...",    FL_CTRL + ord('r'), replace_cb, editor ),
                 ( "Re&place Again", FL_CTRL + ord('t'), replace2_cb, editor ),
                 ( None, 0 ),

                 ( None, 0 )
                 )
    m.copy(menuitems)

    w.editor = Fl_Text_Editor(0, 30, 660, 370)
    w.editor.buffer(textbuf)
    w.editor.highlight_data(stylebuf, styletable,
                              len(styletable),
#                            'A', None, None)
			      'A', style_unfinished_cb, w.editor)
    w.editor.textfont(FL_COURIER)
    w.end()
    w.resizable(w.editor)
    w.callback(close_cb, w)
    textbuf.add_modify_callback(style_update, w.editor)
    textbuf.add_modify_callback(changed_cb, w)
    textbuf.call_modify_callbacks()
    num_windows += 1
    return w

# main

textbuf = Fl_Text_Buffer()
style_init()

window = new_view()
window.show()
