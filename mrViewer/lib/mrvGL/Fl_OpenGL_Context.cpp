#include <Fl_OpenGL_Context.h>

#ifdef __APPLE__
#  define GL_SILENCE_DEPRECATION 1
#  include <OpenGL/OpenGL.h>
#endif

#if defined(FLTK_USE_WAYLAND)
#  include <wayland-client.h>
#  include <wayland-server.h>
#  include <wayland-client-protocol.h>
#  include <wayland-egl.h> // Wayland EGL MUST be included before EGL headers
#  include <EGL/egl.h>
#  include <EGL/eglplatform.h>
#endif


#if defined(FLTK_USE_X11)
#  include <X11/Xlib.h>
#  include <X11/Xutil.h>
#  include <X11/keysymdef.h>
#  include <GL/glx.h>
#endif


#if defined(_WIN32)
#  include <windows.h>
#endif


// Creates a new opengl context that gets destroyed upon exit
OpenGLContext::OpenGLContext() :
    m_destroy( true )
{

#if defined(__APPLE__)

    CGLPixelFormatAttribute pixelFormatAttributes[] = {
        kCGLPFAOpenGLProfile,
        (CGLPixelFormatAttribute) kCGLOGLPVersion_3_2_Core,
        kCGLPFAColorSize, (CGLPixelFormatAttribute) 24,
        kCGLPFAAlphaSize, (CGLPixelFormatAttribute) 8,
        kCGLPFAAccelerated,
        (CGLPixelFormatAttribute) 0
    };

    CGLPixelFormatObj pixelFormat;
    GLint numberOfPixels;
    CGLChoosePixelFormat(pixelFormatAttributes, &pixelFormat,
                         &numberOfPixels);

    CGLContextObj contextObject;
    CGLCreateContext(pixelFormat, 0, &contextObject);
    CGLDestroyPixelFormat(pixelFormat);
    CGLSetCurrentContext(contextObject);



#elif defined(_WIN32)
    // @todo check win32
    HDC hdc = wglGetCurrentDC();
    contextObject = wglCreateContext( hdc );
    wglMakeCurrent( hdc, contextObject );
#endif

#if defined(__linux__) && defined(FLTK_USE_X11)
    if ( fl_display )
    {
        int screenId;
        screenId = DefaultScreen(fl_display);
        GLint glxAttribs[] = {
            GLX_RGBA,
            GLX_RED_SIZE,       8,
            GLX_GREEN_SIZE,     8,
            GLX_BLUE_SIZE,      8,
            GLX_ALPHA_SIZE,     8,
            None
        };

        XVisualInfo* visual = glXChooseVisual(fl_display, screenId,
                                              glxAttribs);

        contextObject = glXCreateContext(fl_display, visual, NULL,
                                          GL_TRUE);
        glXMakeCurrent(fl_display, fl_xid(this), contextObject);
    }
#endif
#if defined(__linux__) && defined(FLTK_USE_WAYLAND)
    wl_display* wld = fl_wl_display();
    if (wld)
    {
        wld_window*  win  = fl_wl_xid(this);
        wl_surface* surface = fl_wl_surface(win);
        // Wayland specific code here
        EGLint numConfigs;
        EGLConfig config;
        EGLint fbAttribs[] =
            {
                EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
                EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
                EGL_RED_SIZE,        8,
                EGL_GREEN_SIZE,      8,
                EGL_BLUE_SIZE,       8,
                EGL_ALPHA_SIZE,      8,
                EGL_NONE
            };
        EGLint contextAttribs[] = { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE, EGL_NONE };

        if ( (eglGetConfigs(wld, NULL, 0, &numConfigs) != EGL_TRUE) ||
             (numConfigs == 0))
        {
            return;
        }

        if ( (eglChooseConfig(wld, fbAttribs, &config, 1, &numConfigs) !=
              EGL_TRUE) || (numConfigs != 1))
        {
            return;
        }

        contextObject = eglCreateContext( wld, config,
                                          EGL_NO_CONTEXT, contextAttribs );
        eglMakeCurrent( wld, surface, surface, contextObject );
    }
#endif

}

OpenGLContext::~OpenGLContext()
{
    if ( !m_destroy ) return;

#ifdef __APPLE__
    CGLDestroyContext(contextObject);
#if defined(__linux__)
#if defined(FLTK_USE_WAYLAND)
#endif
#if defined(FLTK_USE_X11)
#endif
#endif

#if defined(_WIN32)
    // @todo check win32
    wglMakeCurrent( nullptr, nullptr );
    wglDeleteContext( contextObject );
#endif

}
