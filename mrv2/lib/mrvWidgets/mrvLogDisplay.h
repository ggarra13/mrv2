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

    public:
        LogDisplay(int x, int y, int w, int h, const char* l = 0);
        ~LogDisplay();

        void clear();

        void setMaxLines(unsigned lines) { maxLines = lines; }
        void trim();

        void print(const char* x, const char style);

        void info(const char* x);
        void warning(const char* x);
        void error(const char* x);

    protected:
        std::thread::id main_thread;
        unsigned maxLines;
    };

    extern LogDisplay* uiLogDisplay;

} // namespace mrv
