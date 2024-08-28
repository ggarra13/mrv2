
#include "mrvMonitor_macOS.h"

namespace mrv
{
    namespace desktop
    {
        // Get the monitor name given its FLTK screen index
        std::string getMonitorName(int monitorIndex)
        {
            std::string out;

            CGDisplayCount displayCount;
            CGDirectDisplayID displays[32];
            CGGetActiveDisplayList(32, displays, &displayCount);

            if (displayCount <= 0)
                throw std::runtime_error("No monitors connected");

            if (monitorIndex < 0 || monitorIndex >= displayCount)
                throw std::runtime_error("Invalid monitor index");

            if (!getDisplayNameForDispID(displays[monitorIndex], out))
                throw std::runtime_error("Could not get monitor name");

            return out;
        }
    } // namespace desktop
} // namespace mrv
