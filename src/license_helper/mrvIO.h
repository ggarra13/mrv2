// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include "mrvOS/mrvI8N.h"

#include <ostream>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <thread>


namespace mrv
{

    namespace trace
    {

        typedef std::basic_stringbuf<
            char, std::char_traits<char>, std::allocator<char> >
            string_stream;

        struct logbuffer : public string_stream
        {
            static std::ofstream out;
            static std::thread::id mainThread;

            logbuffer();
            virtual ~logbuffer();

            void open_file();

            void open_ffmpeg_log_panel();
            void open_log_panel();

            //! from basic_streambuf, stl function used to sync stream
            virtual int sync();

            virtual void print(const char* c) = 0;

            std::string oldMsg;
        };

        struct errorbuffer : public logbuffer
        {
            virtual void print(const char* c);
        };

        struct warnbuffer : public logbuffer
        {
            virtual void print(const char* c);
        };

        struct infobuffer : public logbuffer
        {
            virtual void print(const char* c);
        };

        struct errorstream : public std::ostream
        {
            errorstream() :
                std::ostream(new errorbuffer)
            {
                flags(std::ios::showpoint | std::ios::right | std::ios::fixed);
            };
            ~errorstream() { delete rdbuf(); };
        };

        struct warnstream : public std::ostream
        {
            warnstream() :
                std::ostream(new warnbuffer)
            {
                flags(std::ios::showpoint | std::ios::right | std::ios::fixed);
            };
            ~warnstream() { delete rdbuf(); };
        };

        struct infostream : public std::ostream
        {
            infostream() :
                std::ostream(new infobuffer)
            {
                flags(std::ios::showpoint | std::ios::right | std::ios::fixed);
            };
            ~infostream() { delete rdbuf(); };
        };

        extern infostream info;
        extern warnstream warn;
        extern errorstream error;

    } // namespace trace

} // namespace mrv

// Log an error
#define mrvLOG_ERROR(mod, msg)                                          \
    do                                                                  \
    {                                                                   \
        mrv::trace::error << _("ERROR:\t") << "[" << mod << "] " << msg; \
    } while (0)

// Log a warning
#define mrvLOG_WARNING(mod, msg)                                        \
    do                                                                  \
    {                                                                   \
        mrv::trace::warn << _("WARN:\t") << "[" << mod << "] " << msg;  \
    } while (0)

// Log if verbosity is more than 0
#define mrvLOG_INFO(mod, msg)                                           \
    do                                                                  \
    {                                                                   \
        mrv::trace::info << "       \t[" << mod << "] " << msg;         \
    } while (0)

// Log out always
#define mrvLOG_STATUS(mod, msg)                                         \
    do                                                                  \
    {                                                                   \
        mrv::trace::info << "       \t[" << mod << "] " << msg;         \
    } while (0)

#define LOG_ERROR(msg) mrvLOG_ERROR(kModule, msg << std::endl)
#define LOG_WARNING(msg) mrvLOG_WARNING(kModule, msg << std::endl)
#define LOG_INFO(msg) mrvLOG_INFO(kModule, msg << std::endl)
#define LOG_STATUS(msg) mrvLOG_STATUS(kModule, msg << std::endl)
#define LOG_DEBUG(msg)                                                         \
    mrvLOG_INFO(                                                               \
        kModule, __FUNCTION__ << "(" << __LINE__ << ") " << msg << std::endl)
