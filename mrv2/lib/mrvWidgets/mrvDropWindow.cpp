// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <iostream>

#include <FL/Fl.H>

#include "mrvDropWindow.h"
#include "mrvEventHeader.h"

namespace mrv
{

    // basic fltk constructors
    DropWindow::DropWindow(int x, int y, int w, int h, const char* l) :
        Fl_Double_Window(x, y, w, h, l)
    {
        init();
    }

    DropWindow::DropWindow(int w, int h, const char* l) :
        Fl_Double_Window(w, h, l)
    {
        init();
    }

    void DropWindow::init(void)
    {
        dock      = (DockGroup*)nullptr;
        workspace = (Fl_Flex*)nullptr;
    }

    int DropWindow::handle(int evt)
    {
        int res = Fl_Double_Window::handle(evt);

        // Is this a dock_drop event?
        if ((evt == FX_DROP_EVENT) && (dock))
        {
            // Did the drop happen on us?
            // Get our co-ordinates
            int ex = x_root() + dock->x();
            int ey = y_root() + dock->y();
            int ew = dock->w();
            int eh = dock->h();
            if (ew < 5)
            {
                ew += DROP_REGION_WIDTH * 2;
                ex -= DROP_REGION_WIDTH * 2;
            }

            // get the drop event co-ordinates
            int cx = Fl::event_x_root();
            int cy = Fl::event_y_root();

            // Is the event inside the boundary of this window?
            if (visible() && (cx > ex) && (cy > ey) && (cx < (ew + ex))
                && (cy < (eh + ey)))
            {
                // std::cerr << "ACCEPTED" << std::endl;
                res = 1;
            } else
            {
                // std::cerr << "REJECTED visible? " << visible() << std::endl;
                // std::cerr << "cx > ex = " << (cx > ex) << std::endl
                //           << "cy > ey = " << (cy > ey) << std::endl
                //           << "(cx < (ew + ex)) = " << (cx < (ew + ex)) <<
                //           std::endl
                //           << "(cy < (eh + ey)) = " << (cy < (eh + ey)) <<
                //           std::endl;
                res = 0;
            }
        }
        return res;
    }

} // namespace mrv
