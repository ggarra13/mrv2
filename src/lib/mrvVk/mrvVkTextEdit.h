// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <string>

#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Multiline_Output.H>

namespace mrv
{
    class TextEdit : public Fl_Double_Window
    {
        Fl_Button editButton;

    public:
        TextEdit(int W, int H, const char* L = 0);

        void add(const std::string&);

        Fl_Multiline_Output textOutput;
        Fl_Choice textAnnotations;
    };
} // namespace mrv
