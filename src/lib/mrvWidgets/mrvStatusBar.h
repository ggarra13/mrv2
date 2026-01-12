// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <iostream>

#include <FL/Fl_Group.H>
#include <FL/Fl.H>

namespace mrv
{
    class StatusBar : public Fl_Group
    {
        float seconds_ = 8.0F;
        Fl_Color labelcolor_;
        Fl_Color color_;

        static void clear_cb(StatusBar* o);
        static void all_ok_cb(StatusBar* o);

    public:
        StatusBar(int X, int Y, int W, int H, const char* L = 0);

        //! Handle event (for opening Log panel when clicked on it)
        int handle(int event) override;
        
        void timeout(float seconds);

        //! Save the current color scheme of the widget.  Used at creation
        //! mainly.
        void save_colors();

        //! Restore the original color scheme of the widget.
        void restore_colors();

        //! Clear the status bar and restore the colors
        void clear();

        //! Set up the default status message.
        void default_message();

        //! Restore the default status message.
        void restore_message();

        //! Store a message in the status bar (a FLTK label)
        void error(const char* msg);

        //! Store a warning message in the status nar (a FLTK label)
        void warning(const char* msg);
    };
} // namespace mrv
