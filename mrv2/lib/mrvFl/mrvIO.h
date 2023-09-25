// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <ostream>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <thread>

#include "mrvCore/mrvI8N.h"

namespace mrv
{

    namespace trace
    {

        typedef std::basic_stringbuf<
            char, std::char_traits<char>, std::allocator<char> >
            string_stream;

        struct logbuffer : public string_stream
        {
            static std::fstream out;
            static std::thread::id mainThread;

            logbuffer();
            virtual ~logbuffer();

            void open_log_panel();

            //! from basic_streambuf, stl function used to sync stream
            virtual int sync();

            virtual void print(const char* c) = 0;
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

#define mrvLOG_ERROR(mod, msg)                                                 \
    do                                                                         \
    {                                                                          \
        mrv::trace::error << _("ERROR: ") << "[" << mod << "] " << msg;        \
    } while (0)
#define mrvLOG_WARNING(mod, msg)                                               \
    do                                                                         \
    {                                                                          \
        mrv::trace::warn << _("WARN : ") << "[" << mod << "] " << msg;         \
    } while (0)
#define mrvLOG_INFO(mod, msg)                                                  \
    do                                                                         \
    {                                                                          \
        mrv::trace::info << _("       ") << "[" << mod << "] " << msg;         \
    } while (0)

#define LOG_ERROR(msg) mrvLOG_ERROR(kModule, msg << std::endl)
#define LOG_WARNING(msg) mrvLOG_WARNING(kModule, msg << std::endl)
#define LOG_INFO(msg) mrvLOG_INFO(kModule, msg << std::endl)
#define LOG_DEBUG(msg)                                                         \
    mrvLOG_INFO(                                                               \
        kModule, __FUNCTION__ << "(" << __LINE__ << ") " << msg << std::endl)

#if defined(NDEBUG)
#    define DBGM3(msg)
#    define DBGM2(msg)
#    define DBGM1(msg)
#    define DBGM0(msg)
#    define DBG3
#    define DBG2
#    define DBG
#else
#    include "mrvFl/mrvPreferences.h"
#    define DBGM3(msg)                                                         \
        do                                                                     \
        {                                                                      \
            if (mrv::Preferences::debug > 2)                                   \
                LOG_DEBUG(msg);                                                \
        } while (0)

#    define DBGM2(msg)                                                         \
        do                                                                     \
        {                                                                      \
            if (mrv::Preferences::debug > 1)                                   \
                LOG_DEBUG(msg);                                                \
        } while (0)

#    define DBGM1(msg)                                                         \
        do                                                                     \
        {                                                                      \
            if (mrv::Preferences::debug > 0)                                   \
                LOG_DEBUG(msg);                                                \
        } while (0)

#    define DBGM0(msg)                                                         \
        do                                                                     \
        {                                                                      \
            LOG_DEBUG(msg);                                                    \
        } while (0)

#    define DBG3                                                               \
        do                                                                     \
        {                                                                      \
            if (mrv::Preferences::debug > 2)                                   \
                LOG_DEBUG("");                                                 \
        } while (0)

#    define DBG2                                                               \
        do                                                                     \
        {                                                                      \
            if (mrv::Preferences::debug > 1)                                   \
                LOG_DEBUG("");                                                 \
        } while (0)

#    define DBG                                                                \
        do                                                                     \
        {                                                                      \
            if (mrv::Preferences::debug > 0)                                   \
                std::cerr << __FUNCTION__ << " (" << __LINE__ << ")"           \
                          << std::endl;                                        \
        } while (0)

#endif
