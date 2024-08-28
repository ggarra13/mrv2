
#include <iostream>
#include <sstream>
#include <string>

#include <vector>
#include <codecvt>
#include <windows.h>

namespace mrv
{
    namespace desktop
    {
        // Get the monitor name given its FLTK screen index
        std::string getMonitorName(int monitorIndex)
        {
            std::string out;

            std::vector<DISPLAYCONFIG_PATH_INFO> paths;
            std::vector<DISPLAYCONFIG_MODE_INFO> modes;
            UINT32 flags = QDC_ONLY_ACTIVE_PATHS;
            LONG isError = ERROR_INSUFFICIENT_BUFFER;

            UINT32 pathCount, modeCount;
            isError =
                GetDisplayConfigBufferSizes(flags, &pathCount, &modeCount);

            if (isError != ERROR_SUCCESS)
            {
                throw std::runtime_error("GetDisplayConfigBufferSizes failed");
            }

            // Allocate the path and mode arrays
            paths.resize(pathCount);
            modes.resize(modeCount);

            // Get all active paths and their modes
            isError = QueryDisplayConfig(
                flags, &pathCount, paths.data(), &modeCount, modes.data(),
                nullptr);

            // The function may have returned fewer paths/modes than estimated
            paths.resize(pathCount);
            modes.resize(modeCount);

            if (isError != ERROR_SUCCESS)
            {
                throw std::runtime_error("QueryDisplayConfig failed");
            }

            // Ensure monitorIndex is within bounds
            if (monitorIndex >= static_cast<int>(paths.size()))
            {
                throw std::runtime_error("Monitor index out of bounds");
            }

            // Find the target (monitor) friendly name
            DISPLAYCONFIG_TARGET_DEVICE_NAME targetName = {};
            targetName.header.adapterId =
                paths[monitorIndex].targetInfo.adapterId;
            targetName.header.id = paths[monitorIndex].targetInfo.id;
            targetName.header.type = DISPLAYCONFIG_DEVICE_INFO_GET_TARGET_NAME;
            targetName.header.size = sizeof(targetName);

            isError = DisplayConfigGetDeviceInfo(&targetName.header);

            if (isError != ERROR_SUCCESS)
            {
                throw std::runtime_error("DisplayConfigGetDeviceInfo failed");
            }

            std::ostringstream s;
            s << "Monitor " << (monitorIndex + 1) << ": ";
            out = s.str();

            if (targetName.flags.friendlyNameFromEdid)
            {
                std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t>
                    utf8_conv;
                out += utf8_conv.to_bytes(targetName.monitorFriendlyDeviceName);
            }
            else
            {
                DISPLAY_DEVICE displayDevice;
                displayDevice.cb = sizeof(DISPLAY_DEVICE);

                if (EnumDisplayDevices(NULL, monitorIndex, &displayDevice, 0))
                {
                    out = displayDevice.DeviceString;
                }
            }

            return out;
        }

    } // namespace desktop
} // namespace mrv
