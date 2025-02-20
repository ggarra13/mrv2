#import <Cocoa/Cocoa.h>

#include <FL/platform.H>
#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>

#import <Cocoa/Cocoa.h>

#include "mrvMainWindow.h"

namespace mrv {

void MainWindow::always_on_top( int t, bool synthetic )
{
    on_top = t;
    FLWindow* nswin = fl_xid(this);
    if ( t )
       [nswin setLevel:NSFloatingWindowLevel];
    else
       [nswin setLevel:NSNormalWindowLevel];
} // above_all function

void MainWindow::set_window_transparency(double alpha)
{
    [fl_xid(this) setAlphaValue:alpha];
}

void MainWindow::setClickThrough(bool enable) {
    BOOL value = (BOOL) enable;
    [fl_xid(this) setIgnoresMouseEvents:value];
}

}   // namespace mrv
