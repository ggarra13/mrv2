// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <FL/Fl_Group.H>
#include <FL/Fl_Box.H>

#include "mrvBrowser.h"
#include "mrvPopupMenu.h"

namespace mrv
{
    namespace area
    {
        class Info;
    }

    class ColorWidget;
    class ColorBrowser;

    class ColorInfo : public Fl_Group
    {
    public:
        ColorInfo(int x, int y, int w, int h, const char* l = 0);

        void main(ViewerUI* m);

        virtual int handle(int event);

        void update(const area::Info& info);

    protected:
        ColorWidget* dcol;
        Fl_Box* area;
        ColorBrowser* browser;
        mrv::PopupMenu* uiColorB;
        static ViewerUI* ui;
    };

} // namespace mrv
