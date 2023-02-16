// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.


#include "mrvGLOffscreenContext.h"

#  include <FL/platform.H>

// mrViewer includes
#include "mrvFl/mrvIO.h"

#ifdef _WIN32
#  include "mrvGL/mrvThumbnailWindow.h"
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
        //! Under windows, we don't have an easy way to create an offscreen
        //! buffer.
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
#if defined(_WIN32)
        TLRENDER_P();
        delete p.win;
#endif
    }

    void OffscreenContext::create_gl_window()
	{
#if defined(_WIN32)
		TLRENDER_P();
		
		Fl_Group* old_group = Fl_Group::current();

        // For Windows, we cannot rely on pBuffers, we need to create
        // a dummy GL Window of 1x1 pixel.
        Fl_Group::current(0);  // this window is not parented to anything
        p.win = new ThumbnailWindow( 0, 0, 1, 1 );
        p.win->end();
        p.win->show();
		Fl_Group::current( old_group );
#endif
	}
	
    void OffscreenContext::init()
    {
#if defined(_WIN32)
        TLRENDER_P();
		if ( !p.win ) create_gl_window();
#endif
		
    }


    void OffscreenContext::make_current()
    {
        TLRENDER_P();

#if defined(_WIN32)
        ///! Note:  Is this is not thread safe?  Not sure
		p.win->context( nullptr, true );
        p.win->make_current();
        if ( ! p.win->context() )
        {
            LOG_ERROR( _("Could not create gl context") );
        }
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
            if  ( !glxfbCfg || nElements == 0  )
            {
                LOG_ERROR( "no GLXFBConfig" );
                return;
            }


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
                LOG_ERROR( "no x11_pbuffer" );
                return;
            }


            XVisualInfo * visInfo = glXGetVisualFromFBConfig( p.dpy,
                                                              glxfbCfg[ 0 ] );
            if ( ! visInfo )
            {
                LOG_ERROR( "no visinfo" );
                return;
            }


            p.x11_context = glXCreateNewContext(p.dpy, glxfbCfg[ 0 ],
                                                GLX_RGBA_TYPE,
                                                NULL, GL_TRUE);
            if ( ! p.x11_context )
            {
                LOG_ERROR( "No X11 context" );
                return;
            }

            if ( glXMakeContextCurrent(p.dpy, p.x11_pbuffer, p.x11_pbuffer,
                                       p.x11_context) != True )
            {
                LOG_ERROR( "Could not make GL context current" );
                return;
            }

        }
#endif
#if defined(FLTK_USE_WAYLAND)
        p.wld = fl_wl_display();
        if (p.wld)
        {
            p.egl_display = eglGetDisplay((EGLNativeDisplayType) p.wld);
            if (p.egl_display == EGL_NO_DISPLAY)
            {
                LOG_ERROR( "get display failed" );
                LOG_ERROR( eglGetErrorString( eglGetError() ) );
                return;
            }

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


            if (eglInitialize(p.egl_display, &major, &minor) != EGL_TRUE)
            {
                LOG_ERROR( "initialize failed" );
                LOG_ERROR( eglGetErrorString( eglGetError() ) );
                return;
            }

            if ( (eglGetConfigs(p.egl_display, NULL, 0, &numConfigs) !=
                  EGL_TRUE) || (numConfigs == 0))
            {
                LOG_ERROR( "get configs failed" );
                LOG_ERROR( eglGetErrorString( eglGetError() ) );
                return;
            }

            eglBindAPI(EGL_OPENGL_API);


            if ( (eglChooseConfig(p.egl_display, fbAttribs, &egl_config, 1,
                                  &numConfigs) != EGL_TRUE) ||
                 (numConfigs != 1))
            {
                LOG_ERROR( "choose config failed" );
                LOG_ERROR( eglGetErrorString( eglGetError() ) );
                return;
            }

            const int pfbCfg[] =
                {
                    EGL_WIDTH,  1,
                    EGL_HEIGHT, 1,
                    EGL_NONE
                };

            p.egl_surface = eglCreatePbufferSurface(p.egl_display, egl_config,
                                                  pfbCfg);
            if ( p.egl_surface == EGL_NO_SURFACE )
            {
                LOG_ERROR( "No egl surface" );
                LOG_ERROR( eglGetErrorString( eglGetError() ) );
                return;
            }

            p.egl_context = eglCreateContext( p.egl_display, egl_config,
                                              EGL_NO_CONTEXT, NULL );
            if ( p.egl_context == EGL_NO_CONTEXT )
            {
                LOG_ERROR( "No egl context" );
                LOG_ERROR( eglGetErrorString( eglGetError() ) );
                return;
            }

            if ( ! eglMakeCurrent( p.egl_display, p.egl_surface,
                                   p.egl_surface, p.egl_context ) )
            {
                LOG_ERROR( "Could not make the context current" );
                LOG_ERROR( eglGetErrorString( eglGetError() ) );
                return;
            }

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
        delete p.win; p.win = nullptr;
        wglMakeCurrent( nullptr, nullptr );
#endif

#if defined(FLTK_USE_X11)
        if ( p.dpy )
        {
            Bool ok = glXMakeCurrent( p.dpy, None, NULL );
            if ( ok != True )
            {
                LOG_ERROR( "Could not make the null context current" );
            }
            glXDestroyContext( p.dpy, p.x11_context );
            glXDestroyPbuffer( p.dpy, p.x11_pbuffer );
        }
#endif

#if defined(FLTK_USE_WAYLAND)
        if ( p.wld )
        {
            if ( !eglMakeCurrent( p.egl_display, EGL_NO_SURFACE,
                                  EGL_NO_SURFACE, EGL_NO_CONTEXT ) )
            {
                LOG_ERROR( "Could not make the null context current" );
                LOG_ERROR( eglGetErrorString( eglGetError() ) );
            }
            if ( eglDestroyContext( p.egl_display, p.egl_context ) != EGL_TRUE )
            {
                LOG_ERROR( "Could not destroy context" );
                LOG_ERROR( eglGetErrorString( eglGetError() ) );
            }
            if ( eglDestroySurface( p.egl_display, p.egl_surface ) != EGL_TRUE )
            {
                LOG_ERROR( "Could not destroy surface" );
                LOG_ERROR( eglGetErrorString( eglGetError() ) );
            }
        }
#endif
    }

}
