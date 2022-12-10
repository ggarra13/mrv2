// SPDX-License-Identifier: BSD-3-Clause
// mrv2 (mrViewer2)
// Copyright Contributors to the mrv2 Project. All rights reserved.


#include "mrvGLOffscreenContext.h"

#  include <FL/platform.H>

// mrViewer includes
#include "mrvFl/mrvIO.h"

#ifdef _WIN32
#  include <FL/Fl_GL_Window.H>
#  include <FL/Fl.H>
#  include <FL/gl.h>
#endif

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


namespace {
    const char* kModule = "glctx";
}

namespace mrv
{

    struct OffscreenContext::Private
    {
#ifdef _WIN32
        HGLRC hglrc  = nullptr;
        HDC   hdc    = nullptr;
        Fl_Gl_Window* win = nullptr;
#endif
#if defined( __APPLE__ )
        CGLContextObj contextObject = nullptr;
#endif
#if defined( FLTK_USE_X11 )
        Display*           dpy = nullptr;
        GLXPbuffer x11_pbuffer = 0;
        GLXContext x11_context = nullptr;
#endif
#if defined( FLTK_USE_WAYLAND )
        wl_display*           wld = nullptr;
        EGLDisplay    egl_display = nullptr;
        EGLSurface    egl_surface = nullptr;
        EGLContext    egl_context = nullptr;
#endif
    };


#if defined(FLTK_USE_WAYLAND)
#define CASE_STR( value ) case value: return #value;
    static const char* eglGetErrorString( EGLint error )
    {
        switch( error )
        {
            CASE_STR( EGL_SUCCESS             )
                CASE_STR( EGL_NOT_INITIALIZED     )
                CASE_STR( EGL_BAD_ACCESS          )
                CASE_STR( EGL_BAD_ALLOC           )
                CASE_STR( EGL_BAD_ATTRIBUTE       )
                CASE_STR( EGL_BAD_CONTEXT         )
                CASE_STR( EGL_BAD_CONFIG          )
                CASE_STR( EGL_BAD_CURRENT_SURFACE )
                CASE_STR( EGL_BAD_DISPLAY         )
                CASE_STR( EGL_BAD_SURFACE         )
                CASE_STR( EGL_BAD_MATCH           )
                CASE_STR( EGL_BAD_PARAMETER       )
                CASE_STR( EGL_BAD_NATIVE_PIXMAP   )
                CASE_STR( EGL_BAD_NATIVE_WINDOW   )
                CASE_STR( EGL_CONTEXT_LOST        )
        default: return "Unknown";
        }
    }
#undef CASE_STR
#endif


    OffscreenContext::OffscreenContext() :
        _p( new Private )
    {
    }



    OffscreenContext::~OffscreenContext()
    {
    }

    void OffscreenContext::init()
    {
#if defined(_WIN32)
        TLRENDER_P();

        // For Windows, we cannot rely on pBuffers, we need to create
        // a dummy GL Window, which we never show.
        p.win = new Fl_Gl_Window( 1, 1 );
        p.win->mode( FL_RGB | FL_ALPHA | FL_OPENGL3 );
        p.win->border(0);
        p.win->end();

        p.hdc = wglGetCurrentDC();
        if ( !p.hdc ) return;

        p.hglrc = wglCreateContext( p.hdc );
        if ( !p.hglrc ) return;

        // We store our newly cerated context on the dummy window, so
        // FLTK will know how to make it current and how to release it.
        p.win->context( p.hglrc, true );

        wglMakeCurrent( NULL, NULL );
#endif
    }


    void OffscreenContext::make_current()
    {
        TLRENDER_P();

        DBGM0( "make_current" );

#if defined(_WIN32)
        p.win->make_current();
        wglMakeCurrent( p.hdc, p.hglrc );
#endif

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

        CGLCreateContext(pixelFormat, 0, &p.contextObject );
        CGLDestroyPixelFormat(pixelFormat);
        CGLSetCurrentContext( p.contextObject );
#endif


#if defined(FLTK_USE_X11)
        p.dpy = fl_x11_display();
        if ( p.dpy )
        {

            int screen = XDefaultScreen( p.dpy );

            const int fbCfgAttribslist[] =
                {
                    GLX_RENDER_TYPE, GLX_RGBA_BIT,
                    GLX_DRAWABLE_TYPE, GLX_PBUFFER_BIT,
                    GLX_RED_SIZE, 1,
                    GLX_GREEN_SIZE, 1,
                    GLX_BLUE_SIZE, 1,
                    GLX_ALPHA_SIZE, 1,
                    GLX_DOUBLEBUFFER, GL_FALSE,
                    None
                };

            int nElements = 0;

            GLXFBConfig * glxfbCfg = glXChooseFBConfig( p.dpy,
                                                        screen,
                                                        fbCfgAttribslist,
                                                        & nElements );


            const int pfbCfg[] =
                {
                    GLX_PBUFFER_WIDTH, 1,
                    GLX_PBUFFER_HEIGHT, 1,
                    None
                };

            p.x11_pbuffer = glXCreatePbuffer( p.dpy,
                                              glxfbCfg[ 0 ],
                                              pfbCfg );
            if (!p.x11_pbuffer)
            {
                std::cerr << "no x11_pbuffer" << std::endl;
                return;
            }


            XVisualInfo * visInfo = glXGetVisualFromFBConfig( p.dpy,
                                                              glxfbCfg[ 0 ] );
            if ( ! visInfo )
            {
                std::cerr << "no visinfo" << std::endl;
                return;
            }


            p.x11_context = glXCreateNewContext(p.dpy, glxfbCfg[ 0 ],
                                                GLX_RGBA_TYPE,
                                                NULL, GL_TRUE);

            if ( glXMakeContextCurrent(p.dpy, p.x11_pbuffer, p.x11_pbuffer,
                                       p.x11_context) != True )
            {

                return;
            }

        }
#endif
#if defined(FLTK_USE_WAYLAND)
        p.wld = fl_wl_display();
        if (p.wld)
        {
            DBGM0( eglGetErrorString( eglGetError() ) );


            p.egl_display = eglGetDisplay((EGLNativeDisplayType) p.wld);
            if (p.egl_display == EGL_NO_DISPLAY)  return;
            DBGM0( eglGetErrorString( eglGetError() ) );

            // Wayland specific code here
            EGLint numConfigs;
            EGLint major, minor;
            EGLConfig egl_config;
            EGLint fbAttribs[] =
                {
                    EGL_SURFACE_TYPE, EGL_PBUFFER_BIT,
                    EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
                    EGL_COLOR_BUFFER_TYPE, EGL_RGB_BUFFER,
                    EGL_LUMINANCE_SIZE,  0,
                    EGL_RED_SIZE,        8,
                    EGL_GREEN_SIZE,      8,
                    EGL_BLUE_SIZE,       8,
                    EGL_ALPHA_SIZE,      8,
                    EGL_DEPTH_SIZE,      0,
                    EGL_LEVEL,           0,
                    EGL_BUFFER_SIZE,     24,
                    EGL_NONE
                };
            EGLint contextAttribs[] = { EGL_CONTEXT_CLIENT_VERSION, 2,
                EGL_NONE, EGL_NONE };


            if (eglInitialize(p.egl_display, &major, &minor) != EGL_TRUE) return;
            DBGM0( eglGetErrorString( eglGetError() ) );

            if ( (eglGetConfigs(p.egl_display, NULL, 0, &numConfigs) !=
                  EGL_TRUE) || (numConfigs == 0))
                return;
            DBGM0( eglGetErrorString( eglGetError() ) );

            eglBindAPI(EGL_OPENGL_API);
            DBGM0( eglGetErrorString( eglGetError() ) );


            if ( (eglChooseConfig(p.egl_display, fbAttribs, &egl_config, 1,
                                  &numConfigs) != EGL_TRUE) ||
                 (numConfigs != 1)) return;
            DBGM0( eglGetErrorString( eglGetError() ) );

            const int pfbCfg[] =
                {
                    EGL_WIDTH,  1,
                    EGL_HEIGHT, 1,
                    EGL_NONE
                };

            p.egl_surface = eglCreatePbufferSurface(p.egl_display, egl_config,
                                                  pfbCfg);
            DBGM0( eglGetErrorString( eglGetError() ) );
            if ( p.egl_surface == EGL_NO_SURFACE )
            {
                std::cerr << "No egl surface" << std::endl;
                return;
            }

            p.egl_context = eglCreateContext( p.egl_display, egl_config,
                                              EGL_NO_CONTEXT, NULL );
            if ( p.egl_context == EGL_NO_CONTEXT )
            {
                std::cerr << "No egl context" << std::endl;
                return;
            }

            if ( ! eglMakeCurrent( p.egl_display, p.egl_surface,
                                   p.egl_surface, p.egl_context ) )
            {
                std::cerr << "Could not make the context current" << std::endl;
                return;
            }
            DBGM0( eglGetErrorString( eglGetError() ) );

        }
#endif

    }

    void OffscreenContext::release()
    {
        TLRENDER_P();

#if defined(__APPLE__)
        CGLSetCurrentContext( nullptr );
        CGLDestroyContext( p.contextObject );
#endif

#ifdef _WIN32
        wglMakeCurrent( nullptr, nullptr );
        // We don't delete hglrc here, as FLTK will do it for us when we
        // delete p.win.
        delete p.win; p.win = nullptr;
#endif

#if defined(FLTK_USE_X11)
        if ( p.dpy )
        {
            glXMakeCurrent( p.dpy, None, NULL );
            glXDestroyContext( p.dpy, p.x11_context );
            glXDestroyPbuffer( p.dpy, p.x11_pbuffer );
        }
#endif

#if defined(FLTK_USE_WAYLAND)
        if ( p.wld )
        {
            eglMakeCurrent( p.egl_display, EGL_NO_SURFACE,
                            EGL_NO_SURFACE, EGL_NO_CONTEXT );
            eglDestroyContext( p.egl_display, p.egl_context );
            eglDestroySurface( p.egl_display, p.egl_surface );
        }
#endif
    }

}
