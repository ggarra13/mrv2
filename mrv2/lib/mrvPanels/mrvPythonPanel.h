// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include "mrvPanelWidget.h"

class ViewerUI;
class Fl_Menu_;

namespace mrv
{
    class PythonPanel : public PanelWidget
    {
    public:
        PythonPanel(ViewerUI* ui);
        ~PythonPanel();

        void add_controls() override;

        void create_menu();
        void create_menu(Fl_Menu_*);

        void run_code();
        void open_python_file(const std::string& file);
        void save_python_file(std::string file);
        void clear_output();
        void clear_editor();
        void toggle_line_numbers(Fl_Menu_*);
        void cut_text();
        void copy_text();
        void paste_text();

        static void run_code_cb(Fl_Menu_*, PythonPanel* o);
        static void open_python_file_cb(Fl_Menu_*, PythonPanel* o);
        static void save_python_file_cb(Fl_Menu_*, PythonPanel* o);
        static void cut_text_cb(Fl_Menu_*, PythonPanel* o);
        static void copy_text_cb(Fl_Menu_*, PythonPanel* o);
        static void paste_text_cb(Fl_Menu_*, PythonPanel* o);
        static void clear_output_cb(Fl_Menu_*, PythonPanel* o);
        static void clear_editor_cb(Fl_Menu_*, PythonPanel* o);
        static void toggle_line_numbers_cb(Fl_Menu_*, PythonPanel* o);

    private:
        void style_update(
            int pos,                 // I - Position of update
            int nInserted,           // I - Number of inserted chars
            int nDeleted,            // I - Number of deleted chars
            int nRestyled,           // I - Number of restyled chars
            const char* deletedText, // I - Text that was deleted
            void* cbArg);            // I - Callback data
        static void style_update_cb(
            int pos,                 // I - Position of update
            int nInserted,           // I - Number of inserted chars
            int nDeleted,            // I - Number of deleted chars
            int nRestyled,           // I - Number of restyled chars
            const char* deletedText, // I - Text that was deleted
            void* cbArg);            // I - Callback data

        struct Private;
        std::unique_ptr<Private> _r;
    };

} // namespace mrv
