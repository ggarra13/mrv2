#import <Cocoa/Cocoa.h>

#include <FL/platform.H>
#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>

#import <Cocoa/Cocoa.h>

#include "mrvMainWindow.h"

namespace mrv {

void MainWindow::always_on_top( int t )
{
    on_top = t;
    FLWindow* nswin = fl_xid(this);
    if ( t )
       [nswin setLevel:NSFloatingWindowLevel];
    else
       [nswin setLevel:NSNormalWindowLevel];
} // above_all function

void MainWindow::set_window_transparency(Fl_Window *w, double alpha)
{
    [fl_xid(w) setAlphaValue:alpha];
}

}   // namespace mrv
