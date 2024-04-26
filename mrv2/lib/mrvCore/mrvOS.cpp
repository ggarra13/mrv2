// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#ifndef _WIN32
#    include <unistd.h>
#endif

#include <fstream>
#include <string>
#include <algorithm>

#include <tlCore/OS.h>

#include <FL/Fl.H>

#include "mrvCore/mrvI8N.h"

namespace mrv
{
    namespace os
    {
        std::string getDesktop()
        {
            std::string out = "Unknown";
            const char* env = fl_getenv("XDG_SESSION_DESKTOP");
            if (env && strlen(env) > 0)
            {
                out = env;
            }
            else
            {
                env = fl_getenv("DESKTOP_SESSION");
                if (env && strlen(env) > 0)
                {
                    out = env;
                }
            }
            out = _("Desktop: ") + out;
            return out;
        }

        std::string getVersion()
        {
            tl::os::SystemInfo info = tl::os::getSystemInfo();
            std::string os_version;
#ifdef __linux__
            std::ifstream os_release("/etc/os-release");
            std::string line;

            if (os_release.is_open())
            {
                while (std::getline(os_release, line))
                {
                    if (line.find("PRETTY_NAME=") != std::string::npos)
                    {
                        os_version = line.substr(line.find("=") + 1);
                        // Remove quotes from the version string
                        os_version.erase(
                            std::remove(
                                os_version.begin(), os_version.end(), '"'),
                            os_version.end());
                        break;
                    }
                }
                os_release.close();
            }
            if (os_version.empty())
                os_version = info.name;
#else
            os_version = info.name;
#endif

            // Output also ocmpile type
            std::string compile = "Debug Compile";
#ifdef NDEBUG
#    ifdef MRV2_RelWithDebInfo
            compile = "RelWithDebInfo";
#    else
            compile = "Release";
#    endif
#endif
            os_version = _("Running on: ") + os_version + " " + compile;
            return os_version;
        }

        bool runningInTerminal()
        {
#ifdef _WIN32
            char* term = fl_getenv("TERM");
            if (!term)
                return false;
            return true;
#else
            return (isatty(fileno(stdout)));
#endif
        }
    } // namespace os

} // namespace mrv
