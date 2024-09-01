
#import <Cocoa/Cocoa.h>
#import <OpenGL/OpenGL.h>

#include "mrvGL/mrvGLWindow.h"

namespace mrv
{
  void GLWindow::make_current()
  {
    [(NSOpenGLContext*)this->context() makeCurrentContext];
  }
}
