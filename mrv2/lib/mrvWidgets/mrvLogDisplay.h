// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <thread>

#include <FL/Fl_Text_Display.H>

namespace mrv
{

    class LogDisplay : public Fl_Text_Display
    {
    public:
        enum ShowPreferences {
            kNoRaise,
            kDockOnError,
            kWindowOnError,
        };

        static ShowPreferences prefs;
        static ShowPreferences ffmpegPrefs;

    public:
        LogDisplay(int x, int y, int w, int h, const char* l = 0);
        ~LogDisplay();

        int handle(int event) override;

        void clear();

        void setMaxLines(unsigned lines) { maxLines = lines; }
        void trim();

        void print(const char* x, const char style);

        void info(const char* x);
        void output(const char* x);
        void warning(const char* x);
        void error(const char* x);

        int copy_text();

        static int kf_copy(int c, LogDisplay* e);

    protected:
        std::thread::id main_thread;
        unsigned maxLines;
    };

    extern LogDisplay* uiLogDisplay;

} // namespace mrv
