#
# "$Id: sudoku.py 536 2020-10-30 15:20:32Z andreasheld $"
#
# Sudoku game using pyFLTK, the Python bindings
# for the Fast Light Tool Kit (FLTK).
# Port of the game by Michael Sweet
# Copyright 2005-2006 by Michael Sweet.
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
import sys, time, random
import pdb

# default sizes
GROUP_SIZE = 160
CELL_SIZE = 50
CELL_OFFSET = 5
MENU_OFFSET = 25


# Sudoku cell class
class SudokuCell(Fl_Widget):
    readonly_ = 0
    #test_value_ = [0]*8
    test_value_ = []
    value_ = 0
    parent_ = None
    
    def __init__(self, X, Y, W, H):
        Fl_Widget.__init__(self, X, Y, W, H)
        self.test_value_ = [0]*8

    def readonly(self, arg = None):
        if arg == None:
            return self.readonly_
        else:
            self.readonly_ = arg

    def value(self, arg = None):
        if arg == None:
            return self.value_
        else:
            self.value_ = arg

    def test_value(self, pos, value = -1):
        if value == -1:
            return self.test_value_[pos]
        else:
            self.test_value_[pos] = value
            self.redraw()

    def draw(self):
        align = [FL_ALIGN_TOP_LEFT,
                 FL_ALIGN_TOP,
                 FL_ALIGN_TOP_RIGHT,
                 FL_ALIGN_RIGHT,
                 FL_ALIGN_BOTTOM_RIGHT,
                 FL_ALIGN_BOTTOM,
                 FL_ALIGN_BOTTOM_LEFT,
                 FL_ALIGN_LEFT]

        # draw the cell ...
        if self.readonly():
            fl_draw_box(FL_UP_BOX, self.x(), self.y(), self.w(), self.h(), self.color())
        else:
            fl_draw_box(FL_DOWN_BOX, self.x(), self.y(), self.w(), self.h(), self.color())
        # draw the cell background ...
        if Fl.focus() == self:
            c = fl_color_average(FL_SELECTION_COLOR, self.color(), 0.5)
            fl_color(c)
            fl_rectf(self.x()+4, self.y()+4, self.w()-8, self.h()-8)
            fl_color(fl_contrast(self.labelcolor(), c))
        else:
            fl_color(self.labelcolor())

        # draw the cell value ...
        s = str(self.value())
        if self.value() > 0:
            fl_font(FL_HELVETICA_BOLD, self.h()-10)
            fl_draw(s, self.x(), self.y(), self.w(), self.h(), FL_ALIGN_CENTER)

        fl_font(FL_HELVETICA_BOLD, int(self.h()/5))

        for i in range(8):
            if self.test_value_[i] != 0:
                s = str(self.test_value_[i])
                fl_draw(s, self.x()+5, self.y()+5, self.w()-10, self.h()-10, align[i])

    # handle events
    def handle(self, event):
        if event == FL_FOCUS:
            Fl.focus(self)
            self.redraw()
            return 1
        elif event == FL_UNFOCUS:
            self.redraw()
            return 1
        elif event == FL_PUSH:
            if Fl.event_inside(self):
                if Fl.event_clicks():
                    if self.value():
                        if self.value() < 9:
                            self.value(self.value()+1)
                        else:
                            self.value(1)
                    else:
                        self.value(self.parent_.next_value(self))
                Fl.focus(self)
                self.redraw()
                return 1
        elif event == FL_KEYDOWN:
            if Fl.event_state() & FL_CTRL:
                pass
            else:
                key = Fl.event_key()-ord('0')
                if key < 0 or key > 9:
                    key = Fl.event_key()-FL_KP-ord('0')
                if key >= 0 and key <= 9:
                    if self.readonly():
                        fl_beep(FL_BEEP_ERROR)
                        return 1
                    if Fl.event_state() & (FL_SHIFT | FL_CAPS_LOCK):
                        status = 0
                        for i in range(8):
                            if self.test_value_[i] == key:
                                self.test_value_[i] = 0
                                break
                        else:
                            for i in range(8):
                                if self.test_value_[i] == 0:
                                    self.test_value_[i] = key
                                    break
                            else:
                                for i in range(7):
                                    self.test_value_[i] = self.test_value_[i+1]
                                self.test_value_[7] = key
                        self.redraw()
                    else:
                        self.value(key)
                        self.do_callback()
                return 1
        return Fl_Widget.handle(self, event)
                        


# Sudoku window class
class Sudoku(Fl_Window):
    grid_groups = [[]]*3
    grid_cells = [[]]*9
    grid_values = [[]]*9
    seed = 0.0
    help_dialog_ = None
    
    def __init__(self):
        Fl_Window.__init__(self, GROUP_SIZE*3, GROUP_SIZE*3+MENU_OFFSET, "pySudoku")
        
        items = [[ "&Game", 0, 0, 0, FL_SUBMENU ],
                 [ "&New Game", FL_COMMAND | ord('n'), self.new_cb, self, FL_MENU_DIVIDER ],
                 [ "&Check Game", FL_COMMAND | ord('c'), self.check_cb, 0, 0 ],
                 [ "&Restart Game", FL_COMMAND | ord('r'), self.restart_cb, 0, 0 ],
                 [ "&Solve Game", FL_COMMAND | ord('s'), self.solve_cb, 0, FL_MENU_DIVIDER ],
                 [ "&Quit", FL_COMMAND | ord('q'), self.close_cb, 0, 0 ],
                 [ None, 0 ],
                 [ "&Difficulty", 0, 0, 0, FL_SUBMENU ],
                 [ "&Easy", 0, self.diff_cb, "0", FL_MENU_RADIO ],
                 [ "&Medium", 0, self.diff_cb, "1", FL_MENU_RADIO ],
                 [ "&Hard", 0, self.diff_cb, "2", FL_MENU_RADIO ],
                 [ "&Impossible", 0, self.diff_cb, "3", FL_MENU_RADIO ],
                 [ None, 0 ],
                 [ "&Help", 0, 0, 0, FL_SUBMENU ],
                 [ "&About Sudoku", FL_F + 1, self.help_cb, 0, 0 ],
                 [ None, 0 ],
                 [ None, 0 ]
                 ]

        #pdb.set_trace()
        self.prefs = Fl_Preferences(Fl_Preferences.USER, "fltk.org", "pysudoku")
        (status, self.difficulty) = self.prefs.get("difficulty", 0)
        if self.difficulty < 0 or self.difficulty > 3:
            self.difficulty = 0

        items[8+self.difficulty][4] = items[8+self.difficulty][4] | FL_MENU_VALUE
        new_items = []
        for item in items:
            new_items.append(tuple(item))

        self.menubar = Fl_Menu_Bar(0, 0, 3*GROUP_SIZE, 25)
        self.menubar.menu(tuple(new_items))

        # create the grids ...
        self.grid = Fl_Group(0, MENU_OFFSET, 3*GROUP_SIZE, 3*GROUP_SIZE)
        
        for i in range(3):
            for j in range(3):
                g = Fl_Group(j*GROUP_SIZE, i*GROUP_SIZE+MENU_OFFSET, GROUP_SIZE, GROUP_SIZE)
                g.box(FL_BORDER_BOX)
                if i==1 ^ j==1:
                    g.color(FL_DARK3)
                else:
                    g.color(FL_DARK2)
                g.end()

                self.grid_groups[i].append(g)

        
        for i in range(9):
            self.grid_cells[i] = []
            for j in range(9):
                cell = SudokuCell(int(j * CELL_SIZE + CELL_OFFSET +
                                  (j / 3) * (GROUP_SIZE - 3 * CELL_SIZE)),
                                  int(i * CELL_SIZE+CELL_OFFSET+MENU_OFFSET +
                                  (i / 3) * (GROUP_SIZE - 3 * CELL_SIZE)),
                                  CELL_SIZE, CELL_SIZE)
                cell.parent_ = self
                cell.callback(self.reset_cb)
                cell.readonly(0)
                self.grid_cells[i].append(cell)

        # catch window close events...
        self.callback(self.close_cb, 0)

        # make the window resizable ...
        self.resizable(self.grid)
        self.size_range(3*GROUP_SIZE, 3 * GROUP_SIZE + MENU_OFFSET, 0, 0, 5, 5, 1)

        # restore the previous window dimensions ...
        X = -1
        Y = -1
        W = 3*GROUP_SIZE
        H = 3*GROUP_SIZE+MENU_OFFSET
        (status, X) = self.prefs.get("x", -1)
        if status:
            (status, Y) = self.prefs.get("y", -1)
            (status, W) = self.prefs.get("w", 3 * GROUP_SIZE)
            (status, H) = self.prefs.get("h", 3 * GROUP_SIZE+MENU_OFFSET)

            self.resize(X, Y, W, H)

        self.set_title()

    # check for a solution to the game
    def check_cb(self, widget, data):
        self.check_game()

    # check if the user has correctly solved the game
    def check_game(self, highlight = 1):
        empty = False
        correct = True

        # check the game for right/wrong answers
        for i in range(9):
            for j in range(9):
                cell = self.grid_cells[i][j]
                if cell.readonly() == 1:
                    continue
                val = cell.value()
                if val == 0:
                    empty = True
                else:
                    loop_done = 0
                    for k in range(9):
                        if (i != k and self.grid_cells[k][j].value() == val) or (j != k and self.grid_cells[i][k].value() == val):
                            break
                    else:
                        loop_done = 1
                    if loop_done == 0:
                        if highlight == 1:
                            cell.color(FL_YELLOW)
                            cell.redraw()

                        correct = False
                    elif highlight == 1:
                        cell.color(FL_LIGHT3)
                        cell.redraw()

        # check subgrids for duplicate numbers
        for i in range(0,9,3):
            for j in range(0,9,3):
                for ii in range(3):
                    for jj in range(3):
                        cell = self.grid_cells[i+ii][j+jj]
                        val = cell.value()

                        if cell.readonly() == 1 or val == 0:
                            continue

                        break_loop = 0
                        for iii in range(3):
                            for jjj in range(3):
                                if not ii == iii and not jj == jjj and self.grid_cells[i+iii][j+jjj].value() == val:
                                    break_loop = 1
                                    break
                            if break_loop == 1:
                                break
                        if break_loop == 1:
                            if highlight == 1:
                                cell.color(FL_YELLOW)
                                cell.redraw()
                            correct = False
        if not empty and correct:
            # success
            for i in range(9):
                for j in range(9):
                    cell = self.grid_cells[i][j]
                    cell.color(FL_GREEN)
                    cell.redraw()
                    cell.readonly(1)
                    

    # close the window, saving the game first...
    def close_cb(self, widget, data):
        self.prefs = None
        self.hide()

    # set the level of difficulty
    def diff_cb(self, widget, d):
        diff = ord(d)-ord('0')
        #pdb.set_trace()
        if diff != self.difficulty:
            self.difficulty = diff
            self.new_game(self.seed)
            self.set_title()
            self.redraw()

            self.prefs.set("difficulty", diff)

    # show the online help
    def help_cb(self, widget, data):
        if (self.help_dialog_ == None) :
            self.help_dialog_ = Fl_Help_Dialog()

            self.help_dialog_.value(
	"<HTML>\n"
	"<HEAD>\n"
	"<TITLE>Sudoku Help</TITLE>\n"
	"</HEAD>\n"
	"<BODY BGCOLOR='#ffffff'>\n"

	"<H2>About the Game</H2>\n"

	"<P>Sudoku (pronounced soo-dough-coo with the emphasis on the\n"
        "first syllable) is a simple number-based puzzle/game played on a\n"
	"9x9 grid that is divided into 3x3 subgrids. The goal is to enter\n"
	"a number from 1 to 9 in each cell so that each number appears\n"
	"only once in each column and row. In addition, each 3x3 subgrid\n"
	"may only contain one of each number.</P>\n"

	"<P>This version of the puzzle is Copyright 2005 by Michael R Sweet</P>\n"

	"<H2>How to Play the Game</H2>\n"

	"<P>At the start of a new game, Sudoku fills in a random selection\n"
	"of cells for you - the number of cells depends on the difficulty\n"
	"level you use. Click in any of the empty cells or use the arrow\n"
	"keys to highlight individual cells and press a number from 1 to 9\n"
	"to fill in the cell. To clear a cell, press 0, Delete, or\n"
	"Backspace. When you have successfully completed all subgrids, the\n"
	"entire puzzle is highlighted in green until you start a new\n"
	"game.</P>\n"

	"<P>As you work to complete the puzzle, you can display possible\n"
	"solutions inside each cell by holding the Shift key and pressing\n"
	"each number in turn. Repeat the process to remove individual\n"
	"numbers, or press a number without the Shift key to replace them\n"
	"with the actual number to use.</P>\n"
	"</BODY>\n"
        )
            

        self.help_dialog_.show()

    # load the game from saved preferences
    def load_game(self):
        # Load the current values and state of each grid...
        for i in range(9):
            if len(self.grid_values[i]) != 9:
                self.grid_values[i] = [0]*9
            else:
                for j in range(9):
                    self.grid_values[i][j] = 0

        solved = True

        for i in range(9):
            for j in range(9):
                cell = self.grid_cells[i][j]
                name = f"value{i}.{j}"
                val = 0
                (status, val) = self.prefs.get(name, 0)
                if status == 0:
                    self.grid_values[0][0] = 0
                    break
                self.grid_values[i][j] = val
                
                name = f"state{i}.{j}"
                (status, val) = self.prefs.get(name, 0)
                cell.readonly(val)
                if val != 0:
                    cell.color(FL_GRAY)
                else:
                    cell.color(FL_LIGHT3)
                    solved = False

                for k in range(8):
                    name = f"test{k}{i}.{j}"
                    (status, val) = self.prefs.get(name, 0)
                    cell.test_value(k, val)

        # If we didn't load any values or the last game was solved, then
        # create a new game automatically...
        if solved or self.grid_values[0][0] == 0:
            self.new_game(0)
        else:
            self.check_game(False);

    # create a new game ...
    def new_cb(self, widget, data):
        s = data
        
        if s.grid_cells[0][0].color() != FL_GREEN:
            if not fl_choice("Are you sure you want to change the difficulty level and "
                   "discard the current game?", "Keep Current Game", "Start New Game", None):
                return
        s.new_game(time.time())
        s.redraw()

    # create a new game ...
    def new_game(self, seed):
        # Generate a new (valid) Sudoku grid...
        self.seed = seed
        random.seed(seed)
        
        ValidPuzzle = 0
        while ValidPuzzle == 0:
            ValidPuzzle = 1
            for i in range(9):
                self.grid_values[i] = []
                for j in range(9):
                    self.grid_values[i].append(0)

            
            # loop over all cells
            for i in range(0,9,3):
                for j in range(0,9,3):
                    # get all possible positions
                    pos = []
                    for ii in range(3):
                        for jj in range(3):
                            pos.append([ii,jj])

                    # for all possible numbers
                    for t in range(1,10):
                        for count in range(30):
                            p1 = random.randint(0, len(pos)-1)
                            k = i+pos[p1][0]
                            m = j+pos[p1][1]
                            if self.grid_values[k][m] == 0:
                                t1 = []
                                t2 = []
                                if m > 0:
                                    t1 = self.grid_values[k][:m]
                                if k > 0:
                                    t2 = [r[m] for r in self.grid_values][:k]
                                if t not in t1 and t not in t2:
                                    self.grid_values[k][m] = t
                                    done = 1
                                    del pos[p1]
                                    break
                        else:
                            ValidPuzzle = 0
                            break
                    if ValidPuzzle == 0:
                        break
                if ValidPuzzle == 0:
                    break

        # Start by making all cells editable
        cell = None
        for i in range(9):
            for j in range(9):
                cell = self.grid_cells[i][j]
                
                cell.value(0)
                cell.readonly(0)
                cell.color(FL_LIGHT3)

        # Show N cells...
        count = 10 * (5 - self.difficulty)

        numbers = list(range(10)[1:])

        while count > 0:
            for i in range(20):
                k = random.randint(0,8)
                m = random.randint(0,8)
                t =numbers[k]
                numbers[k] = numbers[m]
                numbers[m] = t

            i = -1
            while count > 0 and i < 8:
                i += 1
                t = numbers[i]
                j = -1
                while count > 0 and j < 8:
                    j += 1

                    cell = self.grid_cells[i][j]
                    if self.grid_values[i][j] == t and cell.readonly() == 0:
                        cell.value(self.grid_values[i][j])
                        cell.readonly(1)
                        cell.color(FL_GRAY)

                        count -= 1

                        break



    def next_value(self, c):
        # find the cell
        xpos = -1
        ypos = -1
        for i in range(9):
            for j in range(9):
                if self.grid_cells[i][j] == c:
                    xpos = i
                    ypos = j
                    break
            if xpos > -1:
                break

        if xpos < 0:
            return 1

        i -= i % 3
        j -= j % 3

        #numbers = map(0, range(9))
        numbers = [0]*9

        for k in range(3):
            for m in range(3):
                c = self.grid_cells[i+k][j+m]
                if c.value():
                    numbers[c.value()-1] = 1

        for i in range(9):
            if not numbers[i]:
                return i+1

        return 1

    # reset widget color to gray
    def reset_cb(self, widget):
        widget.color(FL_LIGHT3)
        widget.redraw()

        self.check_game(0)

    # resize the window
    def resize(self, X, Y, W, H):
        # resize the window
        Fl_Window.resize(self, X, Y, W, H)

        # save the new window geometry
        self.prefs.set("x", X);
        self.prefs.set("y", Y);
        self.prefs.set("width", W);
        self.prefs.set("height", H);


    def restart_cb(self, widget, data):
        Solved = True;

        for i in range(9):
            for j in range(9):
                cell = self.grid_cells[i][j]

                if cell.readonly() == 0:
                    Solved = False
                    v = cell.value()
                    cell.value(0)
                    cell.color(FL_LIGHT3)

        if Solved:
            self.new_game(self.seed)

    def save_game(self):
        # Save the current values and state of each grid...
        for i in range(9):
            for j in range(9):
                cell = self.grid_cells[i][j]
                name = f"value{i}.{j}"
                self.prefs.set(name, self.grid_values[i][j])

                name = f"state{i}.{j}"
                self.prefs.set(name, cell.value())

                name = f"readonly{i}.{j}"
                self.prefs.set(name, cell.readonly())

                for k in range(8):
                    name = f"test{k}{i}.{j}"
                    self.prefs.set(name, cell.test_value(k))

    def set_title(self):
        titles = ["pySudoku - Easy",
                  "pySudoku - Medium",
                  "pySudoku - Hard",
                  "pySudoku - Impossible"]
        self.label(titles[self.difficulty])

    # solve the puzzle
    def solve_cb(self, widget, data):
        self.solve_game()

    def solve_game(self):
        try:
            for i in range(9):
                for j in range(9):
                    cell = self.grid_cells[i][j]

                    cell.value(self.grid_values[i][j])
                    cell.readonly(1)
                    cell.color(FL_GRAY)
                    cell.redraw()
            self.redraw()
        except IndexError:
            print("IndexError")
            # we reach here if the puzzle has not been initialized yet
            pass

# main entry
if __name__=='__main__':
    
    s = Sudoku()

    # show the game
    s.show(sys.argv)

    # load the previous game
    s.load_game()

    # run
    Fl.run()
    
    
