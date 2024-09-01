// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include "FL/Fl_Select_Browser.H"

class ViewerUI;

namespace mrv
{

    class TextBrowser : public Fl_Select_Browser
    {

    public:
        // CTOR
        TextBrowser(int X, int Y, int W, int H, const char* L = 0);

        int handle(int e) override;
    };

} // namespace mrv
