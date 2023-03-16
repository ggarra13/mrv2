// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <FL/Fl_Tile.H>
#include <FL/Fl_Flex.H>
#include <FL/Fl_Menu_Bar.H>
#include <FL/Fl_Sys_Menu_Bar.H>
#include <FL/Fl_Text_Editor.H>
#include <FL/Fl_Box.H>
#include <FL/fl_ask.H>

#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>
namespace fs = std::filesystem;

#include <pybind11/embed.h>
namespace py = pybind11;

#include <tlCore/StringFormat.h>

#include "mrvCore/mrvHome.h"

#include "mrvWidgets/mrvFunctional.h"
#include "mrvWidgets/mrvLogDisplay.h"
#include "mrvWidgets/mrvPythonEditor.h"

#include "mrvFl/mrvFileRequester.h"
#include "mrvFl/mrvIO.h"

#include "mrvPanels/mrvPanelsCallbacks.h"

#include "mrViewer.h"

namespace
{
    const char* kModule = "pypanel";
}

namespace mrv
{
    static Fl_Text_Display::Style_Table_Entry kCodeStyles[] = {
        // Style table
        {FL_BLACK, FL_COURIER, FL_NORMAL_SIZE},             // A - Plain
        {FL_DARK_GREEN, FL_COURIER_ITALIC, FL_NORMAL_SIZE}, // B - Line comments
        {FL_DARK_GREEN, FL_COURIER_ITALIC,
         FL_NORMAL_SIZE},                               // C - Block comments
        {FL_BLUE, FL_COURIER, FL_NORMAL_SIZE},          // D - Strings
        {FL_DARK_RED, FL_COURIER, FL_NORMAL_SIZE},      // E - Directives
        {FL_DARK_RED, FL_COURIER_BOLD, FL_NORMAL_SIZE}, // F - Types
        {FL_BLUE, FL_COURIER_BOLD, FL_NORMAL_SIZE}      // G - Keywords
    };

    // Style table
    static Fl_Text_Display::Style_Table_Entry kLogStyles[] = {
        // FONT COLOR       FONT FACE   SIZE  ATTR
        // --------------- ------------ ---- ------
        {FL_BLACK, FL_HELVETICA, 14, 0},       // A - Info
        {FL_DARK_YELLOW, FL_HELVETICA, 14, 0}, // B - Warning
        {FL_RED, FL_HELVETICA, 14, 0}          // C - Error
    };

    //! We keep this global so the content won't be erased when the user
    //! closes the Python Panel
    static Fl_Text_Buffer* textBuffer = nullptr;
    static Fl_Text_Buffer* styleBuffer = nullptr;

    struct PythonPanel::Private
    {
        PythonEditor* pythonEditor = nullptr;
        LogDisplay* outputDisplay = nullptr;
    };

    //! Class used to redirect stdout and stderr to two python strings
    class PyStdErrOutStreamRedirect
    {
        py::object _stdout;
        py::object _stderr;
        py::object _stdout_buffer;
        py::object _stderr_buffer;

    public:
        PyStdErrOutStreamRedirect()
        {
            auto sysm = py::module::import("sys");
            _stdout = sysm.attr("stdout");
            _stderr = sysm.attr("stderr");
            auto stringio = py::module::import("io").attr("StringIO");
            _stdout_buffer =
                stringio(); // Other filelike object can be used here as well,
                            // such as objects created by pybind11
            _stderr_buffer = stringio();
            sysm.attr("stdout") = _stdout_buffer;
            sysm.attr("stderr") = _stderr_buffer;
        }
        std::string stdoutString()
        {
            _stdout_buffer.attr("seek")(0);
            return py::str(_stdout_buffer.attr("read")());
        }
        std::string stderrString()
        {
            _stderr_buffer.attr("seek")(0);
            return py::str(_stderr_buffer.attr("read")());
        }
        ~PyStdErrOutStreamRedirect()
        {
            auto sysm = py::module::import("sys");
            sysm.attr("stdout") = _stdout;
            sysm.attr("stderr") = _stderr;
        }
    };

    void PythonPanel::style_update_cb(
        int pos,                 // I - Position of update
        int nInserted,           // I - Number of inserted chars
        int nDeleted,            // I - Number of deleted chars
        int nRestyled,           // I - Number of restyled chars
        const char* deletedText, // I - Text that was deleted
        void* cbArg)             // I - Callback data
    {
        PythonPanel* p = static_cast< PythonPanel* >(cbArg);
        p->style_update(
            pos, nInserted, nDeleted, nRestyled, deletedText, nullptr);
    }

    //!
    //! 'style_update()' - Update the style buffer...
    //!
    void PythonPanel::style_update(
        int pos,                 // I - Position of update
        int nInserted,           // I - Number of inserted chars
        int nDeleted,            // I - Number of deleted chars
        int nRestyled,           // I - Number of restyled chars
        const char* deletedText, // I - Text that was deleted
        void* cbArg)             // I - Callback data
    {
        int start,  // Start of text
            end;    // End of text
        char last,  // Last style on line
            *style, // Style data
            *text;  // Text data

        // If this is just a selection change, just unselect the style buffer...
        if (nInserted == 0 && nDeleted == 0)
        {
            styleBuffer->unselect();
            return;
        }

        // Track changes in the text buffer...
        if (nInserted > 0)
        {
            // Insert characters into the style buffer...
            style = new char[nInserted + 1];
            memset(style, 'A', nInserted);
            style[nInserted] = '\0';

            styleBuffer->replace(pos, pos + nDeleted, style);
            delete[] style;
        }
        else
        {
            // Just delete characters in the style buffer...
            styleBuffer->remove(pos, pos + nDeleted);
        }

        // Select the area that was just updated to avoid unnecessary
        // callbacks...
        styleBuffer->select(pos, pos + nInserted - nDeleted);

        // Re-parse the changed region; we do this by parsing from the
        // beginning of the line of the changed region to the end of
        // the line of the changed region...  Then we check the last
        // style character and keep updating if we have a multi-line
        // comment character...
        start = textBuffer->line_start(pos);
        end = textBuffer->line_end(pos + nInserted - nDeleted);
        text = textBuffer->text_range(start, end);
        style = styleBuffer->text_range(start, end);
        last = style[end - start - 1];

        PythonEditor::style_parse(text, style, end - start);

        styleBuffer->replace(start, end, style);
        _r->pythonEditor->redisplay_range(start, end);

        if (last != style[end - start - 1])
        {
            // The last character on the line changed styles, so reparse the
            // remainder of the buffer...
            free(text);
            free(style);

            end = textBuffer->length();
            text = textBuffer->text_range(start, end);
            style = styleBuffer->text_range(start, end);

            PythonEditor::style_parse(text, style, end - start);

            styleBuffer->replace(start, end, style);
            _r->pythonEditor->redisplay_range(start, end);
        }

        free(text);
        free(style);
    }

    PythonPanel::PythonPanel(ViewerUI* ui) :
        _r(new Private),
        PanelWidget(ui)
    {

        if (!textBuffer)
        {
            styleBuffer = new Fl_Text_Buffer;
            textBuffer = new Fl_Text_Buffer;
        }

        add_group("Python");

        // Fl_SVG_Image* svg = load_svg("Python.svg");
        // g->image(svg);

        g->callback(
            [](Fl_Widget* w, void* d)
            {
                ViewerUI* ui = static_cast< ViewerUI* >(d);
                delete pythonPanel;
                pythonPanel = nullptr;
                ui->uiMain->fill_menu(ui->uiMenuBar);
            },
            ui);
    }

    PythonPanel::~PythonPanel()
    {
        textBuffer->remove_modify_callback(style_update_cb, this);
    }

    void PythonPanel::create_menu()
    {
        TLRENDER_P();

        Fl_Menu_Bar* menu;

#if __APPLE__
        if (p.ui->uiPrefs->uiPrefsMacOSMenus->value())
        {
            menu = p.ui->uiMenuBar;
        }
        else
        {
            menu = new Fl_Menu_Bar(g->x(), g->y() + 20, g->w(), 14);
        }
#else
        menu = new Fl_Menu_Bar(g->x(), g->y() + 20, g->w(), 14);
#endif
        create_menu(menu);
    }

    void PythonPanel::create_menu(Fl_Menu_* menu)
    {
        menu->clear();
        menu->add(
            _("Python/Open File"), 0, (Fl_Callback*)open_python_file_cb, this,
            FL_MENU_DIVIDER);
        menu->add(
            _("Python/Save Code"), 0, (Fl_Callback*)save_python_file_cb, this);
        menu->add(_("Clear/Output"), 0, (Fl_Callback*)clear_output_cb, this);
        menu->add(_("Clear/Editor"), 0, (Fl_Callback*)clear_editor_cb, this);
        menu->add(
            _("Editor/Run Code"), 0, (Fl_Callback*)run_code_cb, this,
            FL_MENU_DIVIDER);
        menu->add(
            _("Editor/Toggle Line Numbers"), 0,
            (Fl_Callback*)toggle_line_numbers_cb, this, FL_MENU_TOGGLE);
        menu->menu_end();
        Fl_Menu_Bar* bar = dynamic_cast< Fl_Menu_Bar* >(menu);
        if (bar)
            bar->update();
    }

    void PythonPanel::add_controls()
    {
        TLRENDER_P();

        g->clear();

        g->begin();

        create_menu();

        int H = p.ui->uiViewGroup->h() - 20;
        int Y = 34;
        int M = (H - Y) / 2;

        Fl_Tile* tile = new Fl_Tile(g->x(), g->y() + Y, g->w(), H - Y);
        tile->labeltype(FL_NO_LABEL);

        int dx = 20, dy = dx; // border width of resizable() - see below
        Fl_Box r(
            tile->x() + dx, tile->y() + dy, tile->w() - 2 * dx,
            tile->h() - 2 * dy);
        tile->resizable(r);

        _r->outputDisplay = new LogDisplay(g->x(), g->y() + Y, g->w(), M);
        _r->outputDisplay->box(FL_DOWN_BOX);
        DBG;

        H -= (M + Y);

        PythonEditor* e;
        _r->pythonEditor = e =
            new PythonEditor(g->x(), g->y() + M + Y, g->w(), H);
        e->box(FL_DOWN_BOX);
        e->textfont(FL_COURIER);
        e->textcolor(FL_BLACK);
        e->textsize(14);
        DBG;
        Fl_Text_Buffer* oldBuffer = e->buffer();
        if (oldBuffer != textBuffer)
            delete oldBuffer;
        DBG;
        e->buffer(textBuffer);
        oldBuffer = e->style_buffer();
        if (oldBuffer != styleBuffer)
            delete oldBuffer;
        DBG;
        e->highlight_data(
            styleBuffer, kCodeStyles,
            sizeof(kCodeStyles) / sizeof(kCodeStyles[0]), 'A', 0, 0);
        textBuffer->add_modify_callback(style_update_cb, this);
        if (textBuffer->length() == 0)
        {
            textBuffer->append(R"PYTHON(
import mrv2
from mrv2 import cmd, math, imaging, media, timeline

)PYTHON");
        }

        tile->end();

        Fl_Flex* flex = new Fl_Flex(g->x(), g->y() + 120 + H, g->w(), 20);
        flex->type(Fl_Flex::HORIZONTAL);
        flex->begin();
        Fl_Button* b;
        auto bW = new Widget< Fl_Button >(
            g->x(), g->y() + 120 + H, 120, 20, _("Run"));
        b = bW;
        b->tooltip(_("Run the code"));
        bW->callback([=](auto o) { run_code(); });

        bW = new Widget< Fl_Button >(
            g->x(), g->y() + 120 + H, 120, 20, _("Output"));
        b = bW;
        b->tooltip(_("Clear the output window"));
        bW->callback([=](auto o) { clear_output(); });

        bW = new Widget< Fl_Button >(
            g->x(), g->y() + 120 + H, 120, 14, _("Editor"));
        b = bW;
        b->tooltip(_("Clear the editor window"));
        bW->callback([=](auto o) { clear_editor(); });

        flex->end();

        g->end();

        Fl_Scroll* s = g->get_scroll();
        s->resizable(g->get_pack());
    }

    void PythonPanel::run_code()
    {
        PythonEditor* e = _r->pythonEditor;
        e->split_code();

        std::string code = e->code();
        std::string eval = e->eval();
        std::string var = e->variable();
        _r->outputDisplay->warning(code.c_str());
        if (!eval.empty() && var != eval)
        {
            eval += '\n';
            _r->outputDisplay->warning(eval.c_str());
        }
        try
        {
            PyStdErrOutStreamRedirect pyRedirect;
            py::exec(code);
            if (!eval.empty())
            {
                py::object result = py::eval(eval);
                py::print(result);
            }
            const std::string& out = pyRedirect.stdoutString();
            if (!out.empty())
            {
                _r->outputDisplay->info(out.c_str());
            }
            const std::string err = pyRedirect.stderrString();
            if (!err.empty())
            {
                _r->outputDisplay->error(out.c_str());
            }
        }
        catch (const std::exception& e)
        {
            _r->outputDisplay->error(e.what());
            _r->outputDisplay->error("\n");
        }
    }

    void PythonPanel::open_python_file(const std::string& file)
    {
        std::ifstream is(file);
        std::stringstream s;
        s << is.rdbuf();
        clear_editor();
        Fl_Text_Buffer* buffer = _r->pythonEditor->buffer();
        buffer->append(s.str().c_str());
    }

    void PythonPanel::save_python_file(std::string file)
    {
        if (file.substr(file.size() - 3, file.size()) != ".py")
            file += ".py";

        if (fs::exists(file))
        {
            std::string msg =
                tl::string::Format(_("Python file {0} already "
                                     "exists.  Do you want to overwrite it?"))
                    .arg(file);
            int ok = fl_choice(msg.c_str(), _("No"), _("Yes"), NULL, NULL);
            if (!ok)
                return;
        }

        std::ofstream ofs(file);
        if (!ofs.is_open())
        {
            fl_alert("%s", _("Failed to open the file for writing."));
            return;
        }
        Fl_Text_Buffer* buffer = _r->pythonEditor->buffer();
        char* text = buffer->text();
        ofs << text;
        free(text);
        if (ofs.fail())
        {
            fl_alert("%s", _("Failed to write to the file."));
            return;
        }
        if (ofs.bad())
        {
            fl_alert("%s", _("The stream is in an unrecoverable error state."));
            return;
        }
        ofs.close();
    }

    void PythonPanel::clear_output()
    {
        _r->outputDisplay->clear();
    }

    void PythonPanel::clear_editor()
    {
        Fl_Text_Buffer* buffer = _r->pythonEditor->buffer();
        buffer->remove(0, buffer->length());
    }

    void PythonPanel::toggle_line_numbers(Fl_Menu_* m)
    {
        const Fl_Menu_Item* i = m->mvalue();
        PythonEditor* e = _r->pythonEditor;
        if (i->value())
        {
            e->linenumber_width(75);
            e->linenumber_size(e->textsize());
        }
        else
        {
            e->linenumber_width(0);
        }
        e->redraw();
    }

    void PythonPanel::run_code_cb(Fl_Menu_*, PythonPanel* o)
    {
        o->run_code();
    }

    void PythonPanel::save_python_file_cb(Fl_Menu_*, PythonPanel* o)
    {
        std::string file =
            mrv::save_python_file(mrv::rootpath().c_str(), Preferences::ui);
        if (file.empty())
            return;
        o->save_python_file(file);
    }

    void PythonPanel::open_python_file_cb(Fl_Menu_*, PythonPanel* o)
    {
        std::string file =
            mrv::open_python_file(mrv::pythonpath().c_str(), Preferences::ui);
        if (file.empty())
            return;
        o->open_python_file(file);
    }

    void PythonPanel::clear_output_cb(Fl_Menu_*, PythonPanel* o)
    {
        o->clear_output();
    }

    void PythonPanel::clear_editor_cb(Fl_Menu_*, PythonPanel* o)
    {
        o->clear_editor();
    }

    void PythonPanel::toggle_line_numbers_cb(Fl_Menu_* m, PythonPanel* o)
    {
        o->toggle_line_numbers(m);
    }

} // namespace mrv
