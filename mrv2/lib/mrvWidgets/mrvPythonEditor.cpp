// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <cstring>
#include <iostream>

#include "mrvWidgets/mrvPythonEditor.h"

#include "mrvPanels/mrvPanelsCallbacks.h"

#include "mrViewer.h"

namespace mrv
{
    namespace
    {
        extern "C"
        {
            //
            // Compare two keywords
            //
            int compare_keywords(const void* a, const void* b)
            {
                return (strcmp(*((const char**)a), *((const char**)b)));
            }
        }

        const char* kKeywords[] = {
            "False",  "None",     "True",  "and",    "as",       "assert",
            "async",  "await",    "break", "class",  "continue", "def",
            "del",    "elif",     "else",  "except", "finally",  "for",
            "from",   "global",   "if",    "import", "in",       "is",
            "lambda", "nonlocal", "not",   "or",     "pass",     "raise",
            "return", "try",      "while", "with",   "yield",
        };

        const char* kSpecial[] = {
            // List of special tokens
            "args",
            "kwargs",
            "self",
        };

        const char* kFunctions[] = {
            // List of known Python functions...
            "abs",      "all",        "any",         "ascii",
            "bin",      "bool",       "bytearray",   "bytes",
            "callable", "chr",        "classmethod", "compile",
            "complex",  "delattr",    "dict",        "dir",
            "divmod",   "enumerate",  "eval",        "exec",
            "filter",   "float",      "format",      "frozenset",
            "getattr",  "globals",    "hasattr",     "hash",
            "help",     "hex",        "id",          "input",
            "int",      "isinstance", "issubclass",  "iter",
            "len",      "list",       "locals",      "map",
            "max",      "memoryview", "min",         "next",
            "object",   "oct",        "open",        "ord",
            "pow",      "print",      "property",    "range",
            "repr",     "reversed",   "round",       "set",
            "setattr",  "slice",      "sorted",      "staticmethod",
            "str",      "sum",        "super",       "tuple",
            "type",     "vars",       "zip",
        };
    } // namespace

    static void kill_selection(Fl_Text_Editor* e)
    {
        if (e->buffer()->selected())
        {
            e->insert_position(e->buffer()->primary_selection()->start());
            e->buffer()->remove_selection();
        }
    }

    PythonEditor::PythonEditor(int X, int Y, int W, int H, const char* L) :
        Fl_Text_Editor(X, Y, W, H, L)
    {
        tab_nav(0);
        cursor_style(HEAVY_CURSOR);
        cursor_color(FL_RED);
        linenumber_bgcolor(FL_RED);
        linenumber_fgcolor(0xffffffff);
        remove_key_binding(FL_Enter, FL_TEXT_EDITOR_ANY_STATE);
        add_key_binding(FL_Enter, FL_TEXT_EDITOR_ANY_STATE, (Key_Func)kf_enter);
        remove_key_binding(FL_KP_Enter, FL_TEXT_EDITOR_ANY_STATE);
        add_key_binding(
            FL_KP_Enter, FL_TEXT_EDITOR_ANY_STATE, (Key_Func)kf_kp_enter);
        remove_key_binding(FL_Tab, FL_TEXT_EDITOR_ANY_STATE);
        add_key_binding(FL_Tab, FL_TEXT_EDITOR_ANY_STATE, (Key_Func)kf_tab);
        remove_key_binding(FL_BackSpace, FL_TEXT_EDITOR_ANY_STATE);
        add_key_binding(
            FL_BackSpace, FL_TEXT_EDITOR_ANY_STATE, (Key_Func)kf_backspace);
        add_key_binding(
            FL_Delete, FL_TEXT_EDITOR_ANY_STATE, (Key_Func)kf_delete);
    }

    int PythonEditor::kf_enter(int key, Fl_Text_Editor* e)
    {
        Fl_Text_Buffer* b = e->buffer();
        int found;
        int pos = e->insert_position();
        int start = b->line_start(pos);

        int tab = start;
        found = b->search_backward(pos, "    ", &tab);
        if (found)
        {
            bool empty = true;
            for (int i = start; i < pos; ++i)
            {
                unsigned c = b->char_at(i);
                if (c != ' ')
                {
                    tab = i;
                    empty = false;
                    break;
                }
            }
            if (empty)
                found = false;
        }

        int end = b->line_end(tab);
        int prev = b->prev_char(end);
        unsigned c = b->char_at(prev);

        // Search for colon that add a 4 space tabulation
        if (c == ':')
        {
            tab += 4;
            found = true;
        }

        if (found && tab > 0)
        {
            b->insert(pos, "\n");

            int len = tab - start;
            for (int i = 0; i < len; ++i)
                b->insert(pos + 1, " ");
            e->insert_position(pos + len + 1);
        }
        else
        {
            Fl_Text_Editor::kf_enter(key, e);
        }
        return 1;
    }

    void PythonEditor::split_code()
    {
        m_code.clear();
        m_eval.clear();
        m_variable.clear();

        Fl_Text_Buffer* b = buffer();
        char* text = b->selection_text();
        int start = 0;
        int line_start = 0;
        int end = b->length() - 1;
        if (!text || strlen(text) == 0)
        {
            text = b->text();
            line_start = b->line_start(end);
        }
        else
        {
            Fl_Text_Selection* s = b->primary_selection();
            start = s->start();
            end = s->end();
        }

        unsigned end_char = b->char_at(end);
        while (end_char == ' ' || end_char == '\n' || end_char == '\0')
        {
            end = b->prev_char(end);
            end_char = b->char_at(end);
        }

        line_start = b->line_start(end);
        if (line_start == end)
        {
            end = b->prev_char(end);
            line_start = b->line_start(end);
        }

        int pos = end;
        int found = b->search_forward(line_start, "=", &pos);

        if (b->char_at(line_start) != ' ')
        {
            end = b->next_char(end);
            if (found && pos < end)
            {
                int prev = b->prev_char(pos);
                while (b->char_at(prev) == ' ')
                    prev = b->prev_char(prev);
                prev = b->next_char(prev);
                char* tmp = b->text_range(line_start, prev);
                m_variable = tmp;
                free(tmp);
                int next = b->next_char(pos);
                while (b->char_at(next) == ' ')
                    next = b->next_char(next);
                m_eval = m_variable;
                tmp = b->text_range(start, end);
                m_code = tmp;
                free(tmp);
            }
            else
            {
                char* tmp = b->text_range(line_start, end);
                m_eval = tmp;
                free(tmp);
                int prev = b->prev_char(line_start);
                tmp = b->text_range(start, prev);
                m_code = tmp;
                free(tmp);

                // py::eval cannot handle these commands
                if (m_eval.substr(0, 4) == "from" ||
                    m_eval.substr(0, 6) == "import" ||
                    m_eval.substr(0, 5) == "print" ||
                    m_eval.substr(0, 5) == "help(" ||
                    m_eval.substr(0, 1) == "#")
                {
                    m_code += "\n";
                    m_code += m_eval;
                    m_eval.clear();
                }
                m_variable.clear();
            }
        }
        else
        {
            m_code = text;
            m_eval.clear();
            m_variable.clear();
        }

        if (!m_code.empty())
            m_code += "\n";
        // std::cerr << "CODE=!" << m_code << "!" << std::endl;
        // std::cerr << "EVAL=!" << m_eval << "!" << std::endl;
        // std::cerr << "VAR =!" << m_variable << "!" << std::endl;

        free(text);
    }

    int PythonEditor::kf_kp_enter(int key, Fl_Text_Editor* e)
    {
        if (pythonPanel)
        {
            pythonPanel->run_code();
        }
        return 1;
    }

    int PythonEditor::kf_tab(int key, Fl_Text_Editor* e)
    {
        bool parse = false;
        Fl_Text_Buffer* b = e->buffer();
        Fl_Text_Selection* s = b->primary_selection();
        if (!s || !s->selected())
        {
            int ins = e->insert_position();
            int pos = ins;
            int end = b->line_end(pos);
            if (pos == end)
            {
                ins = b->line_start(ins);
            }
            b->insert(ins, "    ");
            e->insert_position(pos + 4);
        }
        else
        {
            int start = s->start();
            int end = s->end();
            for (int i = start; i <= end;)
            {
                int pos = b->line_start(i);
                b->insert(pos, "    ");
                i = b->line_end(i);
                i = b->next_char(i);
                end += 4;
            }
        }

        return 1;
    }

    int PythonEditor::handle(int e)
    {
        int r = Fl_Text_Editor::handle(e);
        if (e == FL_ENTER)
        {
            if (Preferences::ui->uiPrefs->uiPrefsMacOSMenus->value() &&
                pythonPanel)
                pythonPanel->create_menu();
            r = 1;
        }
        else if (e == FL_PUSH && Fl::event_button3())
        {
            Fl_Group::current(0);
            Fl_Menu_Button* popupMenu = new Fl_Menu_Button(0, 0, 0, 0);

            popupMenu->type(Fl_Menu_Button::POPUP3);

            pythonPanel->create_menu(popupMenu);
            popupMenu->popup();

            popupMenu = nullptr;
        }
        return r;
    }

    void PythonEditor::style_parse(Fl_Text_Editor* e)
    {
        Fl_Text_Buffer* buffer = e->buffer();
        Fl_Text_Buffer* styleBuffer = e->style_buffer();
        char* text = buffer->text();
        char* styles = styleBuffer->text();
        int len = buffer->length();
        int stylen = styleBuffer->length();
        if (len != stylen)
        {
            free(styles);
            char* new_styles = (char*)malloc(len + 1);
            new_styles[len] = 0;
            if (len < stylen)
                stylen = len;
            memcpy(new_styles, styles, stylen);
            styles = new_styles;
        }
        style_parse(text, styles, len);
        styleBuffer->replace(0, len, styles);
        free(text);
        free(styles);
    }

    //
    // Delete text forwards
    //
    int PythonEditor::kf_delete(int key, Fl_Text_Editor* e)
    {
        Fl_Text_Buffer* b = e->buffer();
        if (!b->selected())
        {
            int p1 = e->insert_position();
            int p2 = b->next_char(p1);
            unsigned c1 = b->char_at(p1);
            unsigned c2 = b->char_at(p2);

            // Check if we have 4 spaces
            int line_start = b->line_start(p2);
            int pos;

            // If we find 4 spaces, select them
            int found = b->search_backward(p1, "    ", &pos);
            if (found && c1 == ' ' && c2 == ' ' && pos >= line_start)
            {
                p1 = pos;
                p2 = p1 + 4;
            }

            b->select(p1, p2);
        }

        kill_selection(e);
        e->show_insert_position();
        e->set_changed();
        if (e->when() & FL_WHEN_CHANGED)
            e->do_callback(FL_REASON_CHANGED);
        return 1;
    }

    //
    // Remove text backwards
    //
    int PythonEditor::kf_backspace(int key, Fl_Text_Editor* e)
    {
        Fl_Text_Buffer* b = e->buffer();
        int found = 0;
        if (!b->selected() && e->move_left())
        {
            int p1 = e->insert_position();
            int p2 = b->next_char(p1);
            unsigned c1 = b->char_at(p1);
            unsigned c2 = b->char_at(p2);

            // Check if we have 4 spaces
            int line_start = b->line_start(p2);
            int line_end = b->line_end(p2);
            int pos;

            // Find the last 4 spaces, and select them
            found = b->search_backward(p2, "    ", &pos);
            if (found && pos >= line_start && pos == p2 - 4)
            {
                p1 = pos;
                p2 = p1 + 4;
            }

            // If not, we do a single space removal
            b->select(p1, p2);
        }
        kill_selection(e);
        e->show_insert_position();
        e->set_changed();
        if (e->when() & FL_WHEN_CHANGED)
            e->do_callback(FL_REASON_CHANGED);
        return 1;
    }

    // Style letters:
    //
    // A - Plain
    // B - Line comments
    // C - Block comments
    // D - Strings
    // E - Directives
    // F - Types
    // G - Keywords
    // H - Functions

    //!
    //! 'style_parse()' - Parse text and produce style data.
    //!
    void PythonEditor::style_parse(const char* text, char* style, int length)
    {
        char current;
        int last;
        char buf[255], *bufptr = nullptr;
        const char* temp;
        for (current = *style, last = 0; length > 0; length--, text++)
        {
            if (current == 'A')
            {
                // Check for directives, comments, strings, and keywords...
                if (*text == '#')
                {
                    current = 'B';
                }
                else if (*text == '\'')
                {
                    current = 'E';
                }
                else if (strncmp(text, "\"\"\"", 3) == 0)
                {
                    current = 'C';
                    *style++ = current;
                    *style++ = current;
                    text += 2;
                    length -= 2;
                }
                else if (
                    (strncmp(text, "\\\"", 2) == 0) ||
                    (strncmp(text, "\\\'", 2) == 0))
                {
                    // Quoted quote...
                    *style++ = current;
                    *style++ = current;
                    text++;
                    length--;
                    continue;
                }
                else if (*text == '\"')
                {
                    current = 'D';
                }
                else if (!last && isalpha(*text))
                {
                    char start = *(text - 1);

                    // Might be a function...
                    for (temp = text, bufptr = buf;
                         isalpha(*temp) && bufptr < (buf + sizeof(buf) - 1);
                         *bufptr++ = *temp++)
                        ;

                    if (*temp == '(' && start == ' ')
                    {
                        *bufptr = '\0';

                        bufptr = buf;

                        if (bsearch(
                                &bufptr, kFunctions,
                                sizeof(kFunctions) / sizeof(kFunctions[0]),
                                sizeof(kFunctions[0]), compare_keywords))
                        {
                            while (text < temp)
                            {
                                *style++ = 'H';
                                text++;
                                length--;
                            }

                            text--;
                            length++;
                            last = 1;
                            continue;
                        }
                    }

                    // Might be a keyword...
                    for (temp = text, bufptr = buf;
                         isalpha(*temp) && bufptr < (buf + sizeof(buf) - 1);
                         *bufptr++ = *temp++)
                        ;

                    if (*temp == '\0' || *temp == '\n' || *temp == ' ')
                    {
                        *bufptr = '\0';

                        bufptr = buf;

                        if (bsearch(
                                &bufptr, kSpecial,
                                sizeof(kSpecial) / sizeof(kSpecial[0]),
                                sizeof(kSpecial[0]), compare_keywords))
                        {
                            while (text < temp)
                            {
                                *style++ = 'F';
                                text++;
                                length--;
                            }

                            text--;
                            length++;
                            last = 1;
                            continue;
                        }
                        else if (bsearch(
                                     &bufptr, kKeywords,
                                     sizeof(kKeywords) / sizeof(kKeywords[0]),
                                     sizeof(kKeywords[0]), compare_keywords))
                        {
                            while (text < temp)
                            {
                                *style++ = 'G';
                                text++;
                                length--;
                            }
                            text--;
                            length++;
                            last = 1;
                            continue;
                        }
                    }
                }
            }
            else if (current == 'C' && strncmp(text, "\"\"\"", 3) == 0)
            {
                // Close a """ comment...
                *style++ = current;
                *style++ = current;
                *style++ = current;
                text += 3;
                length -= 3;
                current = 'A';
                continue;
            }
            else if (current == 'D')
            {
                // Continuing in string...
                if (strncmp(text, "\\\"", 2) == 0)
                {
                    // Quoted end quote...
                    *style++ = current;
                    *style++ = current;
                    text++;
                    length--;
                    continue;
                }
                else if (*text == '\"')
                {
                    // End quote...
                    *style++ = current;
                    current = 'A';
                    continue;
                }
            }
            else if (current == 'E')
            {
                if (*text == '\'')
                {
                    // End quote...
                    *style++ = current;
                    current = 'A';
                    continue;
                }
            }
            else if (current == 'G')
            {
                if (*text == ' ' || *text == '_' || *text == '\n')
                    current = 'A';
            }
            else if (current == 'H')
            {
                if (*text == '(' || *text == ')' || *text == ' ' ||
                    *text == '\n')
                    current = 'A';
            }

            *style++ = current;

            last = isalpha(*text) || *text == '.';

            if (*text == '\n')
            {
                // Reset column and possibly reset the style
                if (current == 'B')
                    current = 'A';
            }
        }
    }

} // namespace mrv
