// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <string>

#include <FL/Fl_Browser.H>

namespace mrv
{

    class CMedia;

    class OCIOBrowser : public Fl_Browser
    {
    public:
        enum Type {
            kInputColorSpace,
            kDisplay,
            kView,
            kDisplay_And_View,
            kNone,
        };

    public:
        OCIOBrowser(int x, int y, int w, int h, const char* l = 0);
        ~OCIOBrowser();

        const std::string& get_selection() { return _sel; }
        void set_selection(const std::string& n) { _sel = n; }
        void set_type(Type type)
        {
            _type = type;
            fill();
        }

        int handle(int event);

    protected:
        void fill();
        void fill_view();
        void fill_display();
        void fill_display_and_view();
        void fill_input_color_space();

    protected:
        Type _type;
        std::string _sel;
    };

} // namespace mrv
