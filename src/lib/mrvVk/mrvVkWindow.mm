
#include <FL/platform.H>

#import <Cocoa/Cocoa.h>

#include "mrvVk/mrvVkWindow.h"

namespace mrv
{
  void VkWindow::set_window_transparency(double alpha)
  {
    [fl_xid(this) setAlphaValue:alpha];	
  }
}
