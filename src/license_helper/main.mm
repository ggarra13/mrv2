#import <Cocoa/Cocoa.h>

#include <FL/platform.H>
#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>

#import <Cocoa/Cocoa.h>

#include "mrvMainWindow.h"


void MainWindow::always_on_top()
{
    FLWindow* nswin = fl_xid(this);
    [nswin setLevel:NSFloatingWindowLevel];
} // above_all function
