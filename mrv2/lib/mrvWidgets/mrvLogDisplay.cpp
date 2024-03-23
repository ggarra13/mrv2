// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <cstdio>
#include <cstring>
#include <cstdlib>

#include <stdexcept>

#include <FL/Fl_Window.H>
#include <FL/Fl_Text_Buffer.H>
#include <FL/Fl_Text_Display.H>
#include <FL/Fl.H>

#include "mrvCore/mrvHome.h"
#include "mrvWidgets/mrvLogDisplay.h"
#include "mrViewer.h"

namespace mrv
{
    LogDisplay* uiLogDisplay = nullptr;

    // Style table
    static Fl_Text_Display::Style_Table_Entry kLogStyles[] = {
        // FONT COLOR       FONT FACE   SIZE  ATTR
        // --------------- ------------ ---- ------
        {FL_BLACK, FL_HELVETICA, 14, 0},       // A - Info
        {FL_DARK_YELLOW, FL_HELVETICA, 14, 0}, // B - Output
        {0x80400000, FL_HELVETICA, 14, 0},     // C - Warning (Orange)
        {FL_RED, FL_HELVETICA, 14, 0}          // D - Error
    };

    static const int kMaxLines = 300;

    LogDisplay::ShowPreferences LogDisplay::prefs = LogDisplay::kDockOnError;
    LogDisplay::ShowPreferences LogDisplay::ffmpegPrefs = LogDisplay::kNoRaise;

    class LogData
    {
    public:
        LogData(LogDisplay* l, const char* msg, const char s) :
            log(l),
            message(strdup(msg))
        {
            size_t t = strlen(msg);
            style = (char*)malloc(t + 1);
            memset(style, s, t);
            style[t] = 0;
        }

        ~LogData()
        {
            free(message);
            free(style);
        }

    public:
        LogDisplay* log;
        char* message;
        char* style;
    };

    static void log_callback(void* v)
    {
        LogData* d = (LogData*)v;

        LogDisplay* log = d->log;
        log->style_buffer()->append(d->style);

        Fl_Text_Buffer* buffer = log->buffer();
        buffer->append(d->message);
        log->scroll(buffer->length(), 0);
        log->trim();

        delete d;
    }

    LogDisplay::LogDisplay(int x, int y, int w, int h, const char* l) :
        Fl_Text_Display(x, y, w, h, l),
        maxLines(kMaxLines)
    {
        color(FL_GRAY0);

        scrollbar_align(FL_ALIGN_BOTTOM | FL_ALIGN_RIGHT);

        wrap_mode(WRAP_AT_BOUNDS, 80);

        Fl_Text_Buffer* buf = new Fl_Text_Buffer();
        buffer(buf);
        Fl_Text_Buffer* style = new Fl_Text_Buffer();
        highlight_data(style, kLogStyles, 4, 'A', 0, 0);

        main_thread = std::this_thread::get_id();
    }

    LogDisplay::~LogDisplay()
    {
        delete mBuffer;
        mBuffer = NULL;
        delete mStyleBuffer;
        mStyleBuffer = NULL;
    }

    void LogDisplay::clear()
    {
        mStyleBuffer->text("");
        mBuffer->text("");
        redraw();
    }

    void LogDisplay::trim()
    {
        if (maxLines == 0)
            return;

        int lines = mBuffer->count_lines(0, mBuffer->length());
        if (lines < maxLines)
            return;
        int last_line = lines - maxLines;
        int endByte = mBuffer->skip_lines(0, last_line);
        mStyleBuffer->remove(0, endByte);
        mBuffer->remove(0, endByte);
        redraw();
    }

    inline void LogDisplay::print(const char* x, const char style)
    {
        if (App::ui)
        {
            if (style == 'D')
                App::ui->uiStatusBar->error(x);
            else if (style == 'C')
                App::ui->uiStatusBar->warning(x);
        }

        LogData* data = new LogData(this, x, style);
        if (main_thread != std::this_thread::get_id())
        {
            Fl::awake((Fl_Awake_Handler)log_callback, data);
        }
        else
        {
            style_buffer()->append(data->style);
            buffer()->append(data->message);
            scroll(buffer()->length(), 0);
            delete data;
            trim();
            redraw();
        }
    }

    void LogDisplay::info(const char* x)
    {
        print(x, 'A');
    }

    void LogDisplay::output(const char* x)
    {
        print(x, 'B');
    }

    void LogDisplay::warning(const char* x)
    {
        print(x, 'C');
    }

    void LogDisplay::error(const char* x)
    {
        print(x, 'D');
    }

    int LogDisplay::handle(int e)
    {
        if (e == FL_PUSH && Fl::event_button3() && buffer()->selected())
        {
            Fl_Group::current(0);

            Fl_Menu_Button menu(0, 0, 0, 0);
            menu.type(Fl_Menu_Button::POPUP3);
            menu.clear();
            menu.add(
                _("Edit/&Copy"), FL_CTRL + 'c', (Fl_Callback*)kf_copy, this);
            menu.menu_end();

            menu.popup();
            return 1;
        }
        int ret = Fl_Text_Display::handle(e);
        return ret;
    }

    int LogDisplay::copy_text()
    {
        if (!buffer()->selected())
            return 1;
        const char* copy = buffer()->selection_text();
        if (*copy)
        {
            Fl::copy(copy, (int)strlen(copy), 1);
        }
        free((void*)copy);
        return 1;
    }

    int LogDisplay::kf_copy(int c, LogDisplay* e)
    {
        e->copy_text();
        return 1;
    }
} // namespace mrv
