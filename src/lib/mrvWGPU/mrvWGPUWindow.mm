
#include <FL/platform.H>

#import <Cocoa/Cocoa.h>
#import <OpenGL/OpenGL.h>

#include "mrvGL/mrvGLWindow.h"

namespace mrv
{
    namespace opengl
    {
	void GLWindow::make_current()
	{
	    [(NSOpenGLContext*)this->context() makeCurrentContext];
	}
  
	void GLWindow::set_window_transparency(double alpha)
	{
	    [fl_xid(this) setAlphaValue:alpha];	
	}
    }	
}
