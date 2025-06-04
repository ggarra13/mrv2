// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <iostream>

#include <FL/platform.H>

#ifdef FLTK_USE_X11
#    include <GL/glx.h>
#endif

#ifdef FLTK_USE_WAYLAND

#    include <wayland-client.h>

// Since we cannot include glx.h and egl together, we define some constants
// and extern EGL functions here.
extern "C"
{
    typedef unsigned int EGLBoolean;
    typedef void* EGLDisplay;
    typedef void* EGLSurface;
    typedef void* EGLContext;

    extern EGLContext eglGetCurrentContext();
    extern EGLBoolean eglMakeCurrent(
        EGLDisplay display, EGLSurface draw, EGLSurface read,
        EGLContext context);
}
#endif

#include "mrvGL/mrvGLWindow.h"

namespace
{
    const char* kModule = "gl";
}

namespace mrv
{
    namespace opengl
    {

        GLWindow::GLWindow(int X, int Y, int W, int H, const char* L) :
            Fl_Gl_Window(X, Y, W, H, L)
        {
        }

        GLWindow::GLWindow(int W, int H, const char* L) :
            Fl_Gl_Window(W, H, L)
        {
        }

#ifndef __APPLE__
        void GLWindow::make_current()
        {
            GLContext ctx = this->context();
            if (!ctx)
                return Fl_Gl_Window::make_current();

#    ifdef _WIN32
            HGLRC cur_hglrc = wglGetCurrentContext();
            HGLRC hglrc = fl_win32_glcontext(ctx);
            if (hglrc == cur_hglrc)
                return;

            HWND hwnd = fl_win32_xid(this);
            HDC hdc = fl_GetDC(hwnd);
            wglMakeCurrent(hdc, hglrc);
#    endif

#    ifdef __linux__
#        ifdef FLTK_USE_WAYLAND
            auto wldpy = fl_wl_display();
            if (wldpy)
            {
                auto eglctx = fl_wl_glcontext(ctx);
                EGLContext currentContext = eglGetCurrentContext();
                if (currentContext == eglctx)
                    return;

                auto win = fl_wl_xid(this);

                auto surface = fl_wl_surface(win);
                eglMakeCurrent(wldpy, surface, surface, eglctx);
            }
#        endif
#        ifdef FLTK_USE_X11
            auto dpy = fl_x11_display();
            if (dpy)
            {
                // Get the current OpenGL context
                auto currentContext = fl_x11_glcontext(ctx);
                if (currentContext == ctx)
                    return;

                auto win = fl_x11_xid(this);
                glXMakeCurrent(dpy, win, (GLXContext)ctx);
            }
#        endif
#    endif
        }
#endif

        void GLWindow::show()
        {
            Fl_Gl_Window::show();

#ifdef FLTK_USE_WAYLAND
            // Not sure if this is needed
            if (fl_wl_display())
            {
                wl_surface_set_opaque_region(fl_wl_surface(fl_wl_xid(this)), NULL);
            }
#endif
        }

    } // namespace opengl

} // namespace mrv
