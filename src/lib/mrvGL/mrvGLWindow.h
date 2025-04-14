// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <FL/Fl_Gl_Window.H>

#include "mrvOptions/mrvEnums.h"

namespace mrv
{

    //
    // This class implements coomon OpenGL functionality
    //
    class GLWindow : public Fl_Gl_Window
    {

    public:
        GLWindow(int X, int Y, int W, int H, const char* L = 0);
        GLWindow(int W, int H, const char* L = 0);

        void make_current();
        void show() FL_OVERRIDE;

#ifdef __APPLE__
    protected:
        void set_window_transparency(double alpha);
#endif
    };
} // namespace mrv
