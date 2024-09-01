// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.


#include <iostream>
#include <sstream>
#include <string>

#ifdef _WIN32
#    include "mrvUI/mrvMonitor_win32.cpp"
#endif

#ifdef __linux__
#    include "mrvUI/mrvMonitor_linux.cpp"
#endif

#ifdef __APPLE__
#    include "mrvUI/mrvMonitor_macOS.cpp"
#endif

#include "mrvCore/mrvI8N.h"

#undef Status
#undef None
#include "mrvFl/mrvIO.h"

#include "mrvUI/mrvDesktop.h"

namespace
{
    const char* kModule = "monitor";
}

namespace mrv
{
    namespace desktop
    {

        std::string monitorName(int monitorIndex)
        {
            std::string out;
            try
            {
                out = getMonitorName(monitorIndex);
            }
            catch (const std::exception& e)
            {
                LOG_INFO(e.what());
            }

            if (out.empty())
            {
                // Unknown OS, or could not retrieve monitor name.
                // Just return Monitor #
                out = _("Monitor ") + std::to_string(monitorIndex +1) + ":";
            }

            return out;
        }
    } // namespace desktop
} // namespace mrv
