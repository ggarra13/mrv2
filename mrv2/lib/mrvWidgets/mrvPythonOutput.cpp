// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <FL/Fl_Menu_Button.H>

#include "mrvPanels/mrvPanelsCallbacks.h"

#include "mrvWidgets/mrvPythonOutput.h"

namespace mrv
{
    PythonOutput::PythonOutput(int X, int Y, int W, int H, const char* L) :
        LogDisplay(X, Y, W, H, L)
    {
        setMaxLines(0); // make output infinite
        box(FL_DOWN_BOX);
        wrap_mode(WRAP_NONE, 80);
    }

    int PythonOutput::handle(int e)
    {
        if (e == FL_PUSH && Fl::event_button3())
        {
            Fl_Group::current(0);
            Fl_Menu_Button* popupMenu = new Fl_Menu_Button(0, 0, 0, 0);

            popupMenu->type(Fl_Menu_Button::POPUP3);

            panel::pythonPanel->create_menu(popupMenu);
            popupMenu->popup();

            delete popupMenu;
            popupMenu = nullptr;
            return 1;
        }
        int ret = LogDisplay::handle(e);
        return ret;
    }

} // namespace mrv
