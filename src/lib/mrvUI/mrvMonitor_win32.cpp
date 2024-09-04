// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.



#include <iostream>
#include <sstream>
#include <string>

#include <vector>
#include <codecvt>
#include <windows.h>

#include "mrvUI/mrvDesktop.h"


namespace
{
    
    std::string
    getOutputTechnology(DISPLAYCONFIG_VIDEO_OUTPUT_TECHNOLOGY tech)
    {
        switch (tech)
        {
        case DISPLAYCONFIG_OUTPUT_TECHNOLOGY_HD15:
            return "HD15";
        case DISPLAYCONFIG_OUTPUT_TECHNOLOGY_SVIDEO:
            return "SVIDEO";
        case DISPLAYCONFIG_OUTPUT_TECHNOLOGY_COMPOSITE_VIDEO:
            return "COMPOSITE";
        case DISPLAYCONFIG_OUTPUT_TECHNOLOGY_COMPONENT_VIDEO:
            return "COMPONENT";
        case DISPLAYCONFIG_OUTPUT_TECHNOLOGY_DVI:
            return "DVI";
        case DISPLAYCONFIG_OUTPUT_TECHNOLOGY_HDMI:
            return "HDMI";
        case DISPLAYCONFIG_OUTPUT_TECHNOLOGY_LVDS:
            return "LVDS";
        case DISPLAYCONFIG_OUTPUT_TECHNOLOGY_D_JPN:
            return "D_JPN";
        case DISPLAYCONFIG_OUTPUT_TECHNOLOGY_SDI:
            return "SDI";
        case DISPLAYCONFIG_OUTPUT_TECHNOLOGY_DISPLAYPORT_EXTERNAL:
            return "DISPLAYPORT_EXTERNAL";
        case DISPLAYCONFIG_OUTPUT_TECHNOLOGY_DISPLAYPORT_EMBEDDED:
            return "DISPLAYPORT_EMBEDDED";
        case DISPLAYCONFIG_OUTPUT_TECHNOLOGY_UDI_EXTERNAL:
            return "UDI_EXTERNAL";
        case DISPLAYCONFIG_OUTPUT_TECHNOLOGY_UDI_EMBEDDED:
            return "UDI_EMBEDDED";
        case DISPLAYCONFIG_OUTPUT_TECHNOLOGY_SDTVDONGLE:
            return "SDTVDONGLE";
        case DISPLAYCONFIG_OUTPUT_TECHNOLOGY_MIRACAST:
            return "MIRACAST";
        case DISPLAYCONFIG_OUTPUT_TECHNOLOGY_INDIRECT_WIRED:
            return "INDIRECT_WIRED";
        case DISPLAYCONFIG_OUTPUT_TECHNOLOGY_INDIRECT_VIRTUAL:
            return "INDIRECT_VIRTUAL";
        case DISPLAYCONFIG_OUTPUT_TECHNOLOGY_INTERNAL:
            return "INTERNAL";
        default:
            return "OTHER";
        }
    }
}

namespace mrv
{
    namespace monitor
    {
        // Get the monitor name given its FLTK screen index
        std::string getName(int monitorIndex)
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

            // Get path for monitor
            const auto& path = paths[monitorIndex];
            
            // Find the target (monitor) friendly name
            DISPLAYCONFIG_TARGET_DEVICE_NAME targetName = {};
            targetName.header.adapterId = path.targetInfo.adapterId;
            targetName.header.id = path.targetInfo.id;
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
                auto vendorId =
                    monitor::decodeEdidManufactureId(targetName.edidManufactureId);
                out = "#" + std::to_string(monitorIndex + 1) + ": ";
                out += getOutputTechnology(targetName.outputTechnology);
                out += " ";
                out += getManufacturerName(vendorId.c_str());
                out += " ";
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
