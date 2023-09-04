// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <FL/platform.H>

#include <stdexcept>

#include "mrvGL/mrvGLContextDebug.h"

#ifdef _WIN32
#    include <glad/gl.h>
#    include <windows.h>
#endif

#ifdef __APPLE__
#    define GL_SILENCE_DEPRECATION 1
#    include <OpenGL/OpenGL.h>
#endif

#ifdef __linux__
#    if defined(FLTK_USE_WAYLAND)
#        define USE_SIMPLE_CONFIG 1
#        include <wayland-client.h>
#        include <wayland-server.h>
#        include <wayland-client-protocol.h>
#        include <wayland-egl.h> // Wayland EGL MUST be included before EGL headers
#        include <EGL/egl.h>
#        include <EGL/eglplatform.h>
#    endif

#    if defined(FLTK_USE_X11)
#        include <GL/glx.h>
#    endif
#endif

namespace mrv
{

    void* GLContextDebug::get()
    {
#ifdef _WIN32
        return (void*)wglGetCurrentContext();
#endif
#ifdef FLTK_USE_X11
        if (fl_x11_display())
            return (void*)glXGetCurrentContext();
#endif
#ifdef FLTK_USE_WAYLAND
        if (fl_wl_display())
            return (void*)eglGetCurrentContext();
#endif
#ifdef __APPLE__
        return (void*)CGLGetCurrentContext();
#endif
        throw std::runtime_error("No Window API known");
    }

} // namespace mrv
