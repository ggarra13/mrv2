// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <cstring> // for strcpy

#include "mrvCore/mrvHome.h"

#include "mrvPanels/mrvPanelsCallbacks.h"

#include "mrvWidgets/mrvLogDisplay.h"

#include "mrvFl/mrvIO.h"

#include "mrvFLU/Flu_File_Chooser.h"

#include "mrvCore/mrvOS.h"

#include "mrvApp/App.h"

namespace mrv
{
    namespace
    {
        static void open_log_panel_cb(ViewerUI* ui)
        {
            if (!logsPanel)
                logs_panel_cb(NULL, ui);
            if (!logsPanel->is_panel())
                logsPanel->dock();
        }

        static void open_log_window_cb(ViewerUI* ui)
        {
            if (!logsPanel)
                logs_panel_cb(NULL, ui);
            if (logsPanel->is_panel())
                logsPanel->undock();
        }
    } // namespace

    namespace trace
    {

        std::fstream logbuffer::out;
        std::thread::id logbuffer::mainThread;

        logbuffer::logbuffer() :
            string_stream()
        {
            str().reserve(1024);
            mainThread = std::this_thread::get_id();
        };

        logbuffer::~logbuffer()
        {
            if (out.is_open())
                out.close();
        };

        void logbuffer::open_log_panel()
        {
            if (!App::ui)
                return;

            if (Flu_File_Chooser::window)
                return;

            bool local = std::this_thread::get_id() == mainThread;

            if (LogDisplay::prefs == LogDisplay::kDockOnError)
            {
                if (local)
                    open_log_panel_cb(App::ui);
                else
                    Fl::awake((Fl_Awake_Handler)open_log_panel_cb, App::ui);
            }
            else if (LogDisplay::prefs == LogDisplay::kWindowOnError)
            {
                if (local)
                    open_log_window_cb(App::ui);
                else
                    Fl::awake((Fl_Awake_Handler)open_log_window_cb, App::ui);
            }
        }

        int logbuffer::sync()
        {
            if (!pbase())
                return 0;

            // make sure to null terminate the string
            sputc('\0');

            // freeze and call the virtual print method
            char* c = strdup(str().c_str());
            if (!c)
                return 1;

            print(c);

            free(c);

            // reset iterator to first position & unfreeze
            seekoff(0, std::ios::beg);
            return 0;
        }

        void errorbuffer::print(const char* c)
        {
            std::cerr << c;
            open_log_panel();
            if (uiLogDisplay)
                uiLogDisplay->error(c);
        }

        void warnbuffer::print(const char* c)
        {
            std::cerr << c;
            if (uiLogDisplay)
                uiLogDisplay->warning(c);
        }

        void infobuffer::print(const char* c)
        {
            std::cout << c << std::flush;
            if (uiLogDisplay)
                uiLogDisplay->info(c);
        }

        infostream info;
        warnstream warn;
        errorstream error;
    } // namespace trace

} // namespace mrv
