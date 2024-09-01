#import <Cocoa/Cocoa.h>

#include <FL/platform.H>
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


}   // namespace mrv
