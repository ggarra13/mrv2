// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include "hdr/mrvIO.h"

#include "mrvCore/mrvHome.h"

#include <cstring> // for strcpy
#include <regex>


namespace mrv
{


    namespace trace
    {

        std::ofstream logbuffer::out;
        std::thread::id logbuffer::mainThread;

        logbuffer::logbuffer() :
            string_stream()
        {
            str().reserve(1024);
            mainThread = std::this_thread::get_id();
            open_file();
        };

        logbuffer::~logbuffer()
        {
            if (out.is_open())
                out.close();
        };

        void logbuffer::open_file()
        {
            if (out.is_open())
                return;

            out.open(tmppath() + "/mrv2.debug.log");
            if (!out.is_open())
            {
                std::cerr << tmppath() + "/mrv2.debug.log failed!" << std::endl;
            }
        }

        void logbuffer::open_ffmpeg_log_panel()
        {
        }

        void logbuffer::open_log_panel()
        {
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
            if (oldMsg == c)
                return;

            oldMsg = c;

            std::cerr << c;

            if (out.is_open())
            {
                out << c << std::flush;
            }
        }

        void warnbuffer::print(const char* c)
        {
            if (oldMsg == c)
                return;

            oldMsg = c;
            
            if (out.is_open())
            {
                out << c << std::flush;
            }

            std::cerr << c;
        }

        void infobuffer::print(const char* c)
        {
            std::cout << c << std::flush;

            if (out.is_open())
            {
                out << c << std::flush;
            }
        }

        infostream info;
        warnstream warn;
        errorstream error;
    } // namespace trace

} // namespace mrv
