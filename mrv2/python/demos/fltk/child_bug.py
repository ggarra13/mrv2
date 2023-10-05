from fltk14 import *
#.add() .child() bug example code

win=Fl_Window(300,200)
win.begin()

win.add(Fl_Input(100, 10, 100, 40,'child 0 .add'))
inp=Fl_Input(100, 100, 100, 40,'child 1 inp')

win.end()

print(win.children()) #2 indices 0,1
print('parent win:       ', type(win)         , id(win))         #parent

print('child 0 .add:     ', type(win.child(0)), id(win.child(0)))#first input
#the next two widgets should be the same but they are not.
print('child 1 inp:      ', type(win.child(1)), id(win.child(1)))#second input
print('Fl_Input inp:     ', type(inp)         , id(inp))         #second input

win.child(0).color(FL_YELLOW) #works because .color IS a method of Fl_Widget 
win.child(1).color(FL_GREEN)

#win.child(0).value('Hello World') #fails because .value IS NOT a method of Fl_Widget
#win.child(1).value('Hello World')
inp.value('Bye') #works because .value IS a method of Fl_Input

print('notice the id of child 0 and child 1 are the same. This seems wrong.')
win.show()
Fl.run()

