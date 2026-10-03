// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <mrvFl/mrvTimelinePlayer.h>

#include "mrvWidgets/mrvBackend.h"

class Fl_Group;

namespace mrv
{
    class MainWindow;

    //! Secondary window.
    class SecondaryWindow
    {

    public:
        SecondaryWindow(ViewerUI*);
        ~SecondaryWindow();

        //! Save the settings for the window (if it is visible or not)
        void save() const;

        //! Get the main window
        MainWindow* window() const;

        //! Get the group that holds the viewport.
        Fl_Group* group() const;

        //! Get the viewport.
        MyViewport* viewport() const;

    private:
        TLRENDER_PRIVATE();
    };
} // namespace mrv
