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
            "False",  "None",   "True",     "and",    "as",       "assert",
            "async",  "await",  "break",    "class",  "continue", "def",
            "del",    "elif",   "else",     "except", "finally",  "for",
            "from",   "global", "if",       "import", "in",       "is",
            "lambda", "len",    "nonlocal", "not",    "or",       "pass",
            "raise",  "return", "try",      "while",  "with",     "yield",
        };
    } // namespace

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
            FL_BackSpace, FL_TEXT_EDITOR_ANY_STATE, (Key_Func)kf_delete);
    }

    int PythonEditor::kf_enter(int key, Fl_Text_Editor* e)
    {
        Fl_Text_Buffer* buffer = e->buffer();
        int pos = e->insert_position();
        int start = buffer->line_start(pos);
        int tab = start;
        int found = buffer->search_backward(pos, "    ", &tab);
        if (found)
        {
            bool empty = true;
            for (int i = start; i < pos; ++i)
            {
                unsigned c = buffer->char_at(i);
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

        if (found)
        {
            buffer->insert(pos, "\n");
            int len = tab - start;
            for (int i = 0; i < len; ++i)
                buffer->insert(pos + 1, " ");
            e->insert_position(pos + len + 1);
            style_parse(e);
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
                    m_eval.substr(0, 5) == "print")
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
        Fl_Text_Buffer* buffer = e->buffer();
        Fl_Text_Selection* s = buffer->primary_selection();
        if (!s || !s->selected())
        {
            int ins = e->insert_position();
            int pos = ins;
            int end = buffer->line_end(pos);
            if (pos == end)
            {
                ins = buffer->line_start(ins);
                parse = true;
            }
            buffer->insert(ins, "    ");
            e->insert_position(pos + 4);
        }
        else
        {
            int start = s->start();
            int end = s->end();
            for (int i = start; i <= end;)
            {
                int pos = buffer->line_start(i);
                buffer->insert(pos, "    ");
                i = buffer->line_end(i);
                i = buffer->next_char(i);
                end += 4;
            }
            parse = true;
        }

        if (parse)
        {
            style_parse(e);
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
        style_parse(text, styles, buffer->length());
        styleBuffer->replace(0, buffer->length(), styles);
        free(text);
        free(styles);
    }

    int PythonEditor::kf_delete(int key, Fl_Text_Editor* e)
    {
        Fl_Text_Buffer* buffer = e->buffer();
        Fl_Text_Buffer* styleBuffer = e->style_buffer();
        const Fl_Text_Selection* s = buffer->primary_selection();
        if (!s || !s->selected())
        {
            int start = e->insert_position();
            int prev1 = buffer->prev_char(start);
            unsigned c1 = buffer->char_at(prev1);
            int prev2 = buffer->prev_char(prev1);
            unsigned c2 = buffer->char_at(prev2);
            if (c1 == ' ' && c2 == ' ')
            {
                int pos = start;
                int found = buffer->search_backward(start, "    ", &pos);
                if (found)
                {
                    // Found a match; remove the whole 4 spaces
                    buffer->remove(pos, pos + 4);
                    return 1;
                }
            }
            buffer->remove(prev1, start);
        }
        else
        {
            int start = s->start();
            int end = s->end();
            buffer->remove(start, end);
        }
        style_parse(e);
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
            // std::cerr << *text << " = " << current << std::endl;
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
                    // Might be a keyword...
                    for (temp = text, bufptr = buf;
                         isalpha(*temp) && bufptr < (buf + sizeof(buf) - 1);
                         *bufptr++ = *temp++)
                        ;

                    if (!isalpha(*temp))
                    {
                        *bufptr = '\0';

                        bufptr = buf;

                        if (bsearch(
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
                if (*text == ' ' || *text == '\n')
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
