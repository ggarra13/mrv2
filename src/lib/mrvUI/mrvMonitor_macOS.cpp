// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include "mrvMonitor_macOS.h"

#include <stdexcept>
#include <vector>

namespace mrv
{
    namespace monitor
    {
        // Get the monitor name given its FLTK screen index.
        // (numMonitors is unused; kept so existing callers still compile.)
        std::string getName(int monitorIndex, int /*numMonitors*/)
        {
            CGDisplayCount displayCount = 0;
            CGGetActiveDisplayList(0, nullptr, &displayCount);   // query count only

            if (displayCount == 0)
                throw std::runtime_error("No monitors connected");

            if (monitorIndex < 0 || monitorIndex >= (int)displayCount)
                throw std::runtime_error("Invalid monitor index");

            // No more fixed 32-entry array.
            std::vector<CGDirectDisplayID> displays(displayCount);
            CGGetActiveDisplayList(displayCount, displays.data(), &displayCount);

            // NOTE: the active-display order is not guaranteed to match
            // FLTK's / NSScreen's order.  If the names ever look swapped,
            // switch this to an [NSScreen screens] index in the .mm (as
            // get_hdr_capabilities() now does).
            std::string out;
            if (!getDisplayNameForDispID(displays[monitorIndex], out))
                throw std::runtime_error("Could not get monitor name");

            return out;
        }

    } // namespace monitor
} // namespace mrv
