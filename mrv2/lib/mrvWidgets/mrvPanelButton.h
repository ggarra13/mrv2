// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <FL/Fl_Button.H>

namespace mrv
{
    // Simple class to turn cursor to arrow when entered
    class PanelButton : public Fl_Button
    {
    public:
        // basic constructor
        PanelButton(int x, int y, int w, int h, const char* l = 0);

        // override handle method to catch drag/dock operations
        int handle(int event) override;
    };

} // namespace mrv
