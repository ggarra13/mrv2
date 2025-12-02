%pythoncode %{
__idleCallbacks = []
def Fl_add_idle( func, data=None):
    __idleCallbacks.append( (func, data) )
    if len(__idleCallbacks) == 1:
        pyFLTK_controlIdleCallbacks(1)

def Fl_remove_idle( func, data=None):
    for cb in __idleCallbacks:
        if cb == ( func, data ):
            __idleCallbacks.remove(cb)
            break

def pyFLTK_doIdleCallbacks():
    for cb in __idleCallbacks:
        cb[0](cb[1])

pyFLTK_registerDoIdle(pyFLTK_doIdleCallbacks)


_fltk.add_idle = staticmethod(Fl_add_idle)
_fltk.remove_idle = staticmethod(Fl_remove_idle)


%}
