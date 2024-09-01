// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <string>

#include <FL/Fl_Text_Editor.H>

class Fl_Window;
class Fl_Input;
class Fl_Button;
class Fl_Return_Button;

namespace mrv
{

    class PythonEditor : public Fl_Text_Editor
    {
    public:
        PythonEditor(int X, int Y, int W, int H, const char* L = 0);

        int handle(int e) override;

        inline const std::string& code() const { return m_code; }
        inline const std::string& eval() const { return m_eval; }
        inline const std::string& variable() const { return m_variable; }

        void split_code();

        static int kf_enter(int c, Fl_Text_Editor* e);
        static int kf_kp_enter(int c, Fl_Text_Editor* e);
        static int kf_tab(int c, Fl_Text_Editor* e);
        static int kf_backspace(int c, Fl_Text_Editor* e);
        static int kf_delete(int c, Fl_Text_Editor* e);
        static void style_parse(const char* text, char* style, int length);
        static void style_parse(Fl_Text_Editor* e);

        Fl_Window* replace_dlg;
        Fl_Input* replace_find;
        Fl_Input* replace_with;
        Fl_Button* replace_all;
        Fl_Return_Button* replace_next;
        Fl_Button* replace_cancel;
        char search[256]; // Search text
    protected:
        int skipParenthesis(int start, int numParenthesis = 1);

        std::string m_code, m_eval, m_variable;
    };

} // namespace mrv
