// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <cstring> // for strcpy

#include "mrvCore/mrvHome.h"

#include "mrvPanels/mrvPanelsCallbacks.h"

#include "mrvWidgets/mrvLogDisplay.h"

#include "mrvFl/mrvPreferences.h"
#include "mrvFl/mrvIO.h"

#include "mrvCore/mrvOS.h"

namespace mrv
{

    namespace trace
    {

        std::fstream logbuffer::out;
        bool logbuffer::_debug = true;

        logbuffer::~logbuffer()
        {
            if (out.is_open())
                out.close();
        };

        void logbuffer::open_log_panel()
        {
            if (!Preferences::ui)
                return;

            if (LogDisplay::prefs == LogDisplay::kDockOnError)
            {
                logs_panel_cb(NULL, Preferences::ui);
                logsPanel->dock();
            }
            else if (LogDisplay::prefs == LogDisplay::kWindowOnError)
            {
                logs_panel_cb(NULL, Preferences::ui);
                logsPanel->undock();
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

            if (_debug && out.is_open())
                out << c << std::flush;

            print(c);

            free(c);

            // reset iterator to first position & unfreeze
            seekoff(0, std::ios::beg);
            return 0;
        }

        void logbuffer::open_stream()
        {
            if (!out.is_open())
            {
                std::string file = mrv::homepath();
                file += "/.filmaura/errorlog.txt";
                out.open(file.c_str(), std::ios_base::out);
            }
            if (out.is_open())
            {
                out << "DEBUG LOG" << std::endl
                    << "=========" << std::endl
                    << std::endl;
            }
        }

        void errorbuffer::print(const char* c)
        {
            std::cerr << c;
            open_log_panel();
            uiLogDisplay->error(c);
        }

        void warnbuffer::print(const char* c)
        {
            std::cerr << c;
            uiLogDisplay->warning(c);
        }

        void infobuffer::print(const char* c)
        {
            std::cout << c;
            uiLogDisplay->info(c);
        }

        infostream info;
        warnstream warn;
        errorstream error;
    } // namespace trace

} // namespace mrv
