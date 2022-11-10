// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <mrvFl/mrvTimelinePlayer.h>

class ViewerUI;

namespace mrv
{
    class MainWindow;
    class GLViewport;
    class App;

    //! Secondary window.
    class SecondaryWindow
    {

    public:
        SecondaryWindow( ViewerUI* );
        ~SecondaryWindow();

        //! Get the main window
        MainWindow* window() const;
        
        //! Get the viewport.
        GLViewport* viewport() const;

    private:
        TLRENDER_PRIVATE();
    };
}
