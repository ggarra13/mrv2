#include <FL/Enumerations.H>

#if defined(_WIN32) || defined(_WIN64)
#include "winsock2.h"
#endif

#include "mrvFl/mrvActionWindow.h"
#include "mrViewer.h"

namespace mrv {

GLViewport* ActionWindow::view() const {
    return uiMain->uiView;
}

int ActionWindow::handle( int event )
{
    int ok = 0;
    if ( event == FL_KEYBOARD && view() )
        ok = view()->handle( event );
    if ( !ok ) ok = Fl_Window::handle( event );
    return ok;
}

}
