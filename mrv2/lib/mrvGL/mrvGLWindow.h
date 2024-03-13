// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <FL/Fl_Gl_Window.H>

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
    };
} // namespace mrv
