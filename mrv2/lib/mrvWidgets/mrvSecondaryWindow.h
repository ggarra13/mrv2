// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <mrvFl/mrvTimelinePlayer.h>

class ViewerUI;

namespace mrv
{
    class MainWindow;
    class Viewport;

    //! Secondary window.
    class SecondaryWindow
    {

    public:
        SecondaryWindow(ViewerUI*);
        ~SecondaryWindow();

        //! Save the settings for the vindow (if it is visible or not)
        void save() const;

        //! Get the main window
        MainWindow* window() const;

        //! Get the viewport.
        Viewport* viewport() const;

    private:
        TLRENDER_PRIVATE();
    };
} // namespace mrv
