// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.


#include "mrViewer.h"

#include "mrvPy/PyStdErrOutRedirect.h"

#include "mrvPanels/mrvPanelsCallbacks.h"

#include "mrvApp/mrvSettingsObject.h"

#include "mrvFl/mrvFileRequester.h"
#include "mrvFl/mrvIO.h"

#include "mrvWidgets/mrvFunctional.h"
#include "mrvWidgets/mrvPythonOutput.h"
#include "mrvWidgets/mrvPythonEditor.h"

#include "mrvIcons/Python.h"

#include "mrvCore/mrvHome.h"

#include <tlCore/StringFormat.h>

#include <FL/Fl_Tile.H>
#include <FL/Fl_Menu_Bar.H>
#include <FL/Fl_Sys_Menu_Bar.H>
#include <FL/Fl_Text_Editor.H>
#include <FL/Fl_PNG_Image.H>
#include <FL/Fl_Box.H>
#include <FL/fl_ask.H>

#include <pybind11/embed.h>
namespace py = pybind11;

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <regex>
#include <sstream>
#include <string>
#include <filesystem>
namespace fs = std::filesystem;

#define PYBIND11_LINE_BUG // Line numbers reported by pybind11 can be off by one
                          // +.

namespace
{
    const char* kModule = "pypanel";

    // Regex to parse python error lines
    const std::regex kRE_PYTHON_ERROR(
        R"(\s*<?(?:([^>\(]+)>?)?(,\s+line\s+)?(?:\(?(\d+)\)))");

} // namespace

namespace
{

    bool ignorePythonFilename(const std::string& filename)
    {
        bool out = true;
        if (filename.substr(0, 6) == "frozen")
            out = false;
        return out;
    }

    std::pair<std::string, int> getFileInfoFromError(const std::string& error)
    {
        std::smatch match;
        int line_number = -1;
        if (std::regex_search(error, match, kRE_PYTHON_ERROR))
        {
            // Extract relevant information
            std::string filename = match[1].matched ? match[1].str() : "";
            if (ignorePythonFilename(filename))
            {
                line_number = match[3].matched ? std::stoi(match[3].str()) : -1;
            }
            return {filename, line_number};
        }
        else
        {
            // Handle cases where the regex doesn't match (e.g., different error
            // format)
            return {"", -1};
        }
    }

#ifdef PYBIND11_LINE_BUG
    std::string fixPyBind11LineError(const std::string& error)
    {
        std::string out = error;
        std::regex_iterator<std::string::const_iterator> rit(
            out.begin(), out.end(), kRE_PYTHON_ERROR);
        std::regex_iterator<std::string::const_iterator> rend;

        size_t offset = 0; // start offset
        while (rit != rend)
        {
            const std::smatch& match = *rit;

            // Extract information
            std::string filename = match[1].str();
            std::string linestr = match[2].matched ? match[2].str() : "";
            if (ignorePythonFilename(filename))
            {
                offset += rit->position() + rit->length();
            }
            else
            {
                int line_number =
                    match[3].matched ? std::stoi(match[3].str()) - 1 : -1;

                // Construct the replacement string with modified line number
                std::string replacement;
                if (linestr.empty())
                    replacement = "<" + filename + ">(" +
                                  std::to_string(line_number) + ")";
                else
                    replacement = "<" + filename + ">" + linestr +
                                  std::to_string(line_number) + ")";

                // Perform the replacement
                out.replace(
                    rit->position() + offset, rit->length(), replacement);
                offset += rit->position() + replacement.size();
            }

            // Update iterator to search from the end of the replaced match
            rit = std::regex_iterator<std::string::const_iterator>(
                out.begin() + offset, out.end(), kRE_PYTHON_ERROR);
        }

        return out;
    }
#endif

} // namespace

namespace mrv
{

    PythonOutput* outputDisplay = nullptr;

    namespace panel
    {

        static Fl_Text_Display::Style_Table_Entry kCodeStyles[] = {
            // Style table
            {FL_BLACK, FL_COURIER, FL_NORMAL_SIZE}, // A - Plain
            {FL_DARK_GREEN, FL_COURIER_ITALIC,
             FL_NORMAL_SIZE}, // B - Line comments
            {FL_DARK_GREEN, FL_COURIER_ITALIC,
             FL_NORMAL_SIZE},                           // C - Block comments
            {FL_BLUE, FL_COURIER, FL_NORMAL_SIZE},      // D - Strings
            {FL_DARK_RED, FL_COURIER, FL_NORMAL_SIZE},  // E - Directives
            {FL_RED, FL_COURIER_BOLD, FL_NORMAL_SIZE},  // F - Types
            {FL_BLUE, FL_COURIER_BOLD, FL_NORMAL_SIZE}, // G - Keywords
            {FL_DARK_GREEN, FL_COURIER_BOLD, FL_NORMAL_SIZE} // H - Functions
        };

        //! We keep this global so the content won't be erased when the user
        //! closes the Python Panel
        static Fl_Text_Buffer* textBuffer = nullptr;
        static Fl_Text_Buffer* styleBuffer = nullptr;

        struct PythonPanel::Private
        {
            Fl_Tile* tile = nullptr;
            PythonEditor* pythonEditor = nullptr;
            Fl_Menu_Bar* menu = nullptr;
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
                end,    // End of text
                len;    // Length of text change
            char last,  // Last style on line
                *style, // Style data
                *text;  // Text data

            // If this is just a selection change, just unselect the style
            // buffer...
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
            len = end - start;

            PythonEditor::style_parse(text, style, len);

            if (len > 0)
            {
                styleBuffer->replace(start, end, style);
                _r->pythonEditor->redisplay_range(start, end);
            }

            if (last != style[end - start - 1])
            {
                // The last character on the line changed styles, so reparse the
                // remainder of the buffer...
                free(text);
                free(style);

                end = textBuffer->length();
                text = textBuffer->text_range(start, end);
                style = styleBuffer->text_range(start, end);
                len = end - start;

                PythonEditor::style_parse(text, style, len);

                if (len > 0)
                {
                    styleBuffer->replace(start, end, style);
                    _r->pythonEditor->redisplay_range(start, end);
                }
            }

            free(text);
            free(style);
        }

        PythonPanel::PythonPanel(ViewerUI* ui) :
            _r(new Private),
            PanelWidget(ui)
        {
            add_group("Python");

            Fl_SVG_Image* svg = MRV2_LOAD_SVG(Python);
            g->bind_image(svg);

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
#ifdef __APPLE__
            TLRENDER_P();
            if (!p.ui->uiPrefs->uiPrefsMacOSMenus->value())
            {
                if (_r->menu)
                    g->remove(_r->menu);
                _r->menu = nullptr;
            }
#else
            if (_r->menu)
                g->remove(_r->menu);
            _r->menu = nullptr;
#endif
            textBuffer->remove_modify_callback(style_update_cb, this);
            _r->tile->remove(outputDisplay); // we make sure not to delete this
        }

        void PythonPanel::create_menu()
        {
            TLRENDER_P();

#ifdef __APPLE__
            if (p.ui->uiPrefs->uiPrefsMacOSMenus->value())
            {
                _r->menu = p.ui->uiMenuBar;
            }
            else
            {
                delete _r->menu;
                _r->menu = new Fl_Menu_Bar(g->x(), g->y() + 20, g->w(), 20);
            }
#else
            delete _r->menu;
            _r->menu = new Fl_Menu_Bar(g->x(), g->y() + 20, g->w(), 20);
#endif
            create_menu(_r->menu);
        }

        void PythonPanel::create_menu(Fl_Menu_* menu)
        {
            TLRENDER_P();

            menu->clear();
            menu->add(
                _("&File/&Open"), FL_COMMAND + 'o',
                (Fl_Callback*)open_python_file_cb, this, FL_MENU_DIVIDER);
            menu->add(
                _("&File/&Save"), FL_COMMAND + 's',
                (Fl_Callback*)save_python_file_cb, this);
            menu->add(
                _("&Edit/&Undo"), FL_COMMAND + 'z', 0, 0, FL_MENU_DIVIDER);
            menu->add(
                _("&Edit/Cu&t"), FL_COMMAND + 'x', (Fl_Callback*)cut_text_cb,
                this);
            menu->add(
                _("&Edit/&Copy"), FL_COMMAND + 'c', (Fl_Callback*)copy_text_cb,
                this);
            menu->add(
                _("&Edit/&Paste"), FL_COMMAND + 'p',
                (Fl_Callback*)paste_text_cb, this, FL_MENU_DIVIDER);
            menu->add(
                _("&Edit/&Delete"), 0, (Fl_Callback*)delete_cb,
                _r->pythonEditor);
            menu->add(
                _("&Edit/&Comment Selection"), FL_F + 12,
                (Fl_Callback*)comment_text_cb, this);
            menu->add(
                _("&Edit/&Uncomment Selection"), FL_F + 11,
                (Fl_Callback*)uncomment_text_cb, this);
            menu->add(
                _("&Search/&Find..."), FL_COMMAND + 'f', (Fl_Callback*)find_cb,
                _r->pythonEditor);
            menu->add(
                _("&Search/F&ind Again"), FL_COMMAND + 'g',
                (Fl_Callback*)find2_cb, _r->pythonEditor);
            menu->add(
                _("&Search/&Replace"), FL_COMMAND + 'r',
                (Fl_Callback*)replace_cb, _r->pythonEditor);
            menu->add(
                _("&Search/&Replace Again"), FL_COMMAND + 't',
                (Fl_Callback*)replace2_cb, _r->pythonEditor);

            _r->pythonEditor->replace_next->callback(
                (Fl_Callback*)replace2_cb, _r->pythonEditor);
            _r->pythonEditor->replace_all->callback(
                (Fl_Callback*)replall_cb, _r->pythonEditor);

            menu->add(
                _("Clear/&Output"), FL_COMMAND + 'k',
                (Fl_Callback*)clear_output_cb, this, FL_MENU_DIVIDER);
            menu->add(
                _("Clear/&Editor"), FL_COMMAND + 'e',
                (Fl_Callback*)clear_editor_cb, this);
            menu->add(
                _("Editor/&Run Code"), FL_KP_Enter, (Fl_Callback*)run_code_cb,
                this, FL_MENU_DIVIDER);
            menu->add(
                _("Editor/Toggle &Line Numbers"), 0,
                (Fl_Callback*)toggle_line_numbers_cb, this, FL_MENU_TOGGLE);
            menu->add(
                _("Editor/&Jump to Error"), FL_COMMAND + 'j',
                (Fl_Callback*)jump_to_error_cb, this, FL_MENU_DIVIDER);
            menu->add(
                _("Editor/&External Editor"), 0,
                (Fl_Callback*)external_editor_cb, this, FL_MENU_DIVIDER);
            menu->add(
                _("Scripts/Add To Script List"), 0,
                (Fl_Callback*)add_to_script_list_cb, this, FL_MENU_DIVIDER);

            auto settings = App::app->settings();
            for (const auto fullfile : settings->pythonScripts())
            {
                fs::path fullPath = fullfile;

                // Use the filename() member function to get only the filename
                fs::path filename = fullPath.filename();

                char buf[256];
                snprintf(buf, 256, _("Scripts/%s"), filename.string().c_str());

                menu->add(buf, 0, (Fl_Callback*)script_shortcut_cb, this);
            }

            menu->menu_end();
            Fl_Menu_Bar* bar = dynamic_cast< Fl_Menu_Bar* >(menu);
            if (bar)
                bar->update();
        }

        void PythonPanel::add_controls()
        {
            TLRENDER_P();

            // First, we create the tile and the python editor so we can
            // use it in the menu callbacks
            Fl_Group::current(0);

            int H = g->h() - 20;

            // When docked, the size will be just the dragbar.
            if (H < 400)
                H = 400;
            int Y = 20;
            int M = (H - Y) / 2;

            Fl_Tile* tile;
            _r->tile = tile = new Fl_Tile(g->x(), g->y() + Y, g->w(), H - 3);
            tile->labeltype(FL_NO_LABEL);
            tile->begin();

            int dx = 20, dy = 20; // border width of resizable() - see below
            Fl_Box r(
                tile->x() + dx, tile->y() + dy, tile->w() - 2 * dx,
                tile->h() - 2 * dy);
            r.clear_visible();
            tile->resizable(r);

            if (!textBuffer)
            {
                styleBuffer = new Fl_Text_Buffer;
                textBuffer = new Fl_Text_Buffer;
            }

            outputDisplay->resize(tile->x(), tile->y(), tile->w(), M);

            tile->add(outputDisplay);

            PythonEditor* e;
            _r->pythonEditor = e = new PythonEditor(
                tile->x(), tile->y() + M, tile->w(), tile->h() - M);
            e->box(FL_DOWN_BOX);
            e->textfont(FL_COURIER);
            e->textcolor(FL_BLACK);
            e->textsize(14);
            e->tooltip(_("Type in your python code here.  Select an area to "
                         "execute just "
                         "a portion of it.  Press Keypad Enter to run it."));
            Fl_Text_Buffer* oldBuffer = e->buffer();
            if (oldBuffer != textBuffer)
                delete oldBuffer;
            e->buffer(textBuffer);
            oldBuffer = e->style_buffer();
            if (oldBuffer != styleBuffer)
                delete oldBuffer;
            e->highlight_data(
                styleBuffer, kCodeStyles,
                sizeof(kCodeStyles) / sizeof(kCodeStyles[0]), 'A', 0, 0);
            textBuffer->add_modify_callback(style_update_cb, this);
            if (textBuffer->length() == 0)
            {
                std::string imports = R"PYTHON(
import mrv2
from mrv2 import annotations, cmd, math, image, io, media
from mrv2 import playlist, timeline, ui, )PYTHON";

#ifdef TLRENDER_USD
                imports += "usd, ";
#endif
                imports += "session, settings";
                textBuffer->append(imports.c_str());
            }

            tile->end();

            // Create the pack...
            g->clear();
            g->begin();
            create_menu();
            g->add(tile);
        }

        void PythonPanel::run_code()
        {
            PythonEditor* e = _r->pythonEditor;
            e->split_code();

            const std::string& code = e->code();
            const std::string& eval = e->eval();
            const std::string& var = e->variable();

            // Clear the output display selection
            Fl_Text_Buffer* buffer = outputDisplay->buffer();
            buffer->select(0, 0);

            outputDisplay->output(code.c_str());
            if (!eval.empty() && eval != var)
            {
                outputDisplay->output(eval.c_str());
            }
            try
            {
                py::exec(code);
                if (!eval.empty())
                {
                    py::object result = py::eval(eval);
                    py::print(result);
                }
            }
            catch (const std::exception& e)
            {
#ifdef PYBIND11_LINE_BUG
                const std::string& error = fixPyBind11LineError(e.what());
#else
                const std::string& error = e.what();
#endif
                outputDisplay->error(error.c_str());
            }
        }

        void PythonPanel::open_python_file(const std::string& file)
        {
            std::ifstream is(file);
            std::stringstream s;
            s << is.rdbuf();
            clear_editor();
            Fl_Text_Buffer* buffer = _r->pythonEditor->buffer();
            std::string nocr;
            std::string text = s.str();
            for (auto c : text)
            {
                if (c == '\r')
                    continue;
                nocr += c;
            }
            buffer->append(nocr.c_str());
        }

        void PythonPanel::save_python_file(std::string file)
        {
            if (file.substr(file.size() - 3, file.size()) != ".py")
                file += ".py";

            if (fs::exists(file))
            {
                std::string msg =
                    tl::string::Format(
                        _("Python file {0} already "
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
                fl_alert(
                    "%s", _("The stream is in an unrecoverable error state."));
                return;
            }
            ofs.close();
        }

        void PythonPanel::clear_output()
        {
            outputDisplay->clear();
        }

        void PythonPanel::clear_editor()
        {
            Fl_Text_Buffer* buffer = _r->pythonEditor->buffer();
            buffer->remove(0, buffer->length());
        }

        void PythonPanel::external_editor()
        {
            Fl_Group::current(0);
            Fl_Double_Window win(500, 100, _("Type your editor command"));
            win.begin();
            Fl_Box b(
                10, 10, win.w() - 20, 20,
                _("{0} will be replaced with the line number.  {1} with the "
                  "file name"));
            b.align(FL_ALIGN_WRAP | FL_ALIGN_INSIDE | FL_ALIGN_LEFT);
            Fl_Input input(10, 40, win.w() - 20, win.h() - 20 - 40);
            auto settings = App::app->settings();
            auto editor = settings->getValue<std::string>("Python/Editor");
            input.textcolor(FL_BLACK);
            input.value(editor.c_str());
            win.set_modal();
            win.show();
            while (win.shown())
                Fl::check();

            settings->setValue("Python/Editor", std::string(input.value()));
        }

        void PythonPanel::jump_to_error()
        {
            Fl_Text_Buffer* buffer = outputDisplay->buffer();
            Fl_Text_Selection* selection = buffer->primary_selection();

            int start = selection->start();
            int end = selection->end();

            char* text = nullptr;
            if (!selection->selected())
            {
                // Nothing selected, look in outputDisplay for last error.
                end = buffer->length();
                start = buffer->line_start(end);
                try
                {
                    while (start > 0)
                    {
                        end = buffer->line_end(start);
                        text = buffer->line_text(start);
                        if (text && strlen(text) > 0)
                        {
                            auto [file, line] = getFileInfoFromError(text);
                            if (line > 0)
                            {
                                break;
                            }
                        }
                        start = buffer->rewind_lines(end, 1);
                        free(text);
                        text = nullptr;
                    }
                }
                catch (const std::regex_error& e)
                {
                    std::string msg =
                        tl::string::Format(_("Regular expression error: {0}"))
                            .arg(e.what());
                    LOG_ERROR(msg);
                }
            }
            else
            {
                start = buffer->line_start(start);
                end = buffer->line_end(end);
            }

            buffer->select(start, end);
            text = buffer->selection_text();
            if (!text || strlen(text) == 0)
                return;

            outputDisplay->redraw();
            Fl::check();

            try
            {
                auto [file, line] = getFileInfoFromError(text);
                if (file == "string" && line > 0)
                {
                    buffer = _r->pythonEditor->buffer();

                    --line; // lines in buffer begin at 0

                    // Sanity check just in case....
                    int numLines = buffer->count_lines(0, buffer->length());
                    if (line > numLines)
                        line = numLines;

                    start = buffer->skip_lines(0, line);
                    end = buffer->line_end(start);
                    buffer->select(start, end);
                }
                else if (line > 0)
                {
                    auto settings = App::app->settings();
                    auto editor =
                        settings->getValue<std::string>("Python/Editor");
                    std::string command =
                        string::Format(editor).arg(line).arg(file);
                    LOG_STATUS(command);
                    int ret = std::system(command.c_str());
                    if (ret != 0)
                    {
                        const std::string msg =
                            tl::string::Format(
                                _("Could not open python editor: {0}"))
                                .arg(command);
                        LOG_ERROR(msg);
                    }
                }
            }
            catch (const std::regex_error& e)
            {
                std::string msg =
                    tl::string::Format(_("Regular expression error: {0}"))
                        .arg(e.what());
                LOG_ERROR(msg);
            }

            free(text);
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

        PythonOutput* PythonPanel::output()
        {
            return outputDisplay;
        }

        void PythonPanel::run_code_cb(Fl_Menu_*, PythonPanel* o)
        {
            o->run_code();
        }

        void PythonPanel::save_python_file_cb(Fl_Menu_*, PythonPanel* o)
        {
            std::string file = mrv::save_python_file(mrv::rootpath().c_str());
            if (file.empty())
                return;
            o->save_python_file(file);
        }

        void PythonPanel::open_python_file_cb(Fl_Menu_*, PythonPanel* o)
        {
            std::string file = mrv::open_python_file(mrv::pythonpath().c_str());
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

        void PythonPanel::jump_to_error_cb(Fl_Menu_* m, PythonPanel* o)
        {
            o->jump_to_error();
        }

        void PythonPanel::external_editor_cb(Fl_Menu_* m, PythonPanel* o)
        {
            o->external_editor();
        }

        void PythonPanel::cut_text()
        {
            if (Fl::belowmouse() == outputDisplay)
                copy_text();
            else
                PythonEditor::kf_cut(0, _r->pythonEditor);
        }

        void PythonPanel::copy_text()
        {
            if (Fl::belowmouse() == outputDisplay)
            {
                Fl_Text_Buffer* buffer = outputDisplay->buffer();
                if (!buffer->selected())
                    return;
                const char* copy = buffer->selection_text();
                if (*copy)
                    Fl::copy(copy, (int)strlen(copy), 2);
                free((void*)copy);
            }
            else
            {
                PythonEditor::kf_copy(0, _r->pythonEditor);
            }
        }

        void PythonPanel::paste_text()
        {
            PythonEditor::kf_paste(0, _r->pythonEditor);
        }

        void PythonPanel::comment_text()
        {
            Fl_Text_Buffer* buffer = _r->pythonEditor->buffer();
            if (!buffer->selected())
                return;
            const char* copy = buffer->selection_text();
            std::string originalText = copy;
            free((void*)copy);

            // Create a stringstream to process the input string line by line
            std::stringstream s(originalText);

            // Initialize an output string to store the result
            std::string outputText;

            // Loop through each line in the input string
            std::string line;
            while (std::getline(s, line))
            {
                // Add a '#' character to the beginning of each line
                outputText += '#' + line + '\n';
            }
            buffer->replace_selection(outputText.c_str());
        }

        void PythonPanel::uncomment_text()
        {
            Fl_Text_Buffer* buffer = _r->pythonEditor->buffer();
            if (!buffer->selected())
                return;
            const char* copy = buffer->selection_text();
            std::string originalText = copy;
            free((void*)copy);

            // Create a stringstream to process the input string line by line
            std::stringstream s(originalText);

            // Initialize an output string to store the result
            std::string outputText;

            // Loop through each line in the input string
            std::string line;
            while (std::getline(s, line))
            {
                // Remove first '#' character in each line
                size_t pos = line.find('#');

                // Check if '#' was found
                if (pos == 0)
                {
                    // Remove the first '#'
                    line = line.substr(pos + 1);
                }
                outputText += line + '\n';
            }

            buffer->replace_selection(outputText.c_str());
        }

        void PythonPanel::cut_text_cb(Fl_Menu_* m, PythonPanel* o)
        {
            o->cut_text();
        }

        void PythonPanel::copy_text_cb(Fl_Menu_* m, PythonPanel* o)
        {
            o->copy_text();
        }

        void PythonPanel::paste_text_cb(Fl_Menu_* m, PythonPanel* o)
        {
            o->paste_text();
        }

        void PythonPanel::delete_cb(Fl_Menu_* m, PythonEditor* e)
        {
            e->buffer()->remove_selection();
        }

        void PythonPanel::comment_text_cb(Fl_Menu_* m, PythonPanel* o)
        {
            o->comment_text();
        }

        void PythonPanel::uncomment_text_cb(Fl_Menu_* m, PythonPanel* o)
        {
            o->uncomment_text();
        }

        void PythonPanel::find_cb(Fl_Menu_* m, PythonEditor* e)
        {
            const char* val;

            const char* text = _("Search String:");
            val = fl_input(text, e->search, 0);
            if (val != NULL)
            {
                // User entered a string - go find it!
                strcpy(e->search, val);
                find2_cb(m, e);
            }
        }

        void PythonPanel::find2_cb(Fl_Menu_* m, PythonEditor* e)
        {
            if (e->search[0] == '\0')
            {
                // Search string is blank; get a new one...
                find_cb(m, e);
                return;
            }

            int pos = e->insert_position();
            auto textBuffer = e->buffer();
            int found = textBuffer->search_forward(pos, e->search, &pos);
            if (found)
            {
                // Found a match; select and update the position...
                textBuffer->select(pos, pos + strlen(e->search));
                e->insert_position(pos + strlen(e->search));
                e->show_insert_position();
            }
            else
            {
                fl_alert(_("No occurrences of \'%s\' found!"), e->search);
            }
        }

        void PythonPanel::replace_cb(Fl_Menu_* m, PythonEditor* e)
        {
            e->replace_dlg->show();
        }

        void PythonPanel::replall_cb(Fl_Menu_* m, PythonEditor* e)
        {
            const char* find = e->replace_find->value();
            const char* replace = e->replace_with->value();

            find = e->replace_find->value();
            if (find[0] == '\0')
            {
                // Search string is blank; get a new one...
                e->replace_dlg->show();
                return;
            }

            e->replace_dlg->hide();

            e->insert_position(0);
            int times = 0;

            auto textBuffer = e->buffer();

            // Loop through the whole string
            for (int found = 1; found;)
            {
                int pos = e->insert_position();
                found = textBuffer->search_forward(pos, find, &pos);

                if (found)
                {
                    // Found a match; update the position and replace text...
                    textBuffer->select(pos, pos + (int)strlen(find));
                    textBuffer->remove_selection();
                    textBuffer->insert(pos, replace);
                    e->insert_position(pos + (int)strlen(replace));
                    e->show_insert_position();
                    times++;
                }
            }

            if (times)
                fl_message("Replaced %d occurrences.", times);
            else
                fl_alert("No occurrences of \'%s\' found!", find);
        }

        void PythonPanel::replace2_cb(Fl_Menu_* m, PythonEditor* e)
        {
            const char* find = e->replace_find->value();
            const char* replace = e->replace_with->value();

            if (find[0] == '\0')
            {
                // Search string is blank; get a new one...
                e->replace_dlg->show();
                return;
            }

            e->replace_dlg->hide();

            int pos = e->insert_position();
            auto textBufferfer = e->buffer();
            int found = textBuffer->search_forward(pos, find, &pos);

            if (found)
            {
                // Found a match; update the position and replace text...
                textBuffer->select(pos, pos + strlen(find));
                textBuffer->remove_selection();
                textBuffer->insert(pos, replace);
                textBuffer->select(pos, pos + strlen(replace));
                e->insert_position(pos + strlen(replace));
                e->show_insert_position();
            }
            else
            {
                fl_alert("No occurrences of \'%s\' found!", find);
            }
        }

        void PythonPanel::add_to_script_list(const std::string& file)
        {
            TLRENDER_P();

            auto settings = App::app->settings();
            settings->addPythonScript(file);

            create_menu(_r->menu);
        }

        void PythonPanel::add_to_script_list_cb(Fl_Menu_* m, PythonPanel* o)
        {
            std::string file = mrv::open_python_file(mrv::pythonpath().c_str());
            if (file.empty())
                return;
            o->add_to_script_list(file);
        }

        void PythonPanel::script_shortcut(unsigned int idx)
        {
            TLRENDER_P();

            auto settings = App::app->settings();

            const std::string script = settings->pythonScript(idx);

            Fl_Text_Buffer* buffer = _r->pythonEditor->buffer();
            char* text = buffer->text();
            open_python_file(script);
            run_code();
            clear_editor();
            buffer->append(text);
            free(text);
        }

        void PythonPanel::script_shortcut_cb(Fl_Menu_* m, PythonPanel* o)
        {
            unsigned base = m->find_index(_("Scripts/Add To Script List"));
            const Fl_Menu_Item* item = m->mvalue();
            unsigned idx = m->find_index(item) - base - 1;
            o->script_shortcut(idx);
        }

    } // namespace panel

} // namespace mrv
