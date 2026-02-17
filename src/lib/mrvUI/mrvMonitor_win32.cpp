// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.


#ifdef _WIN32
#    include <winsock2.h>
#    include <windows.h>
#    include <dxgi1_6.h>
#endif

#include <iostream>
#include <unordered_map>
#include <sstream>
#include <string>
#include <vector>
#include <codecvt>
#include <windows.h>


#include "mrvUI/mrvDesktop.h"

#include "mrvFl/mrvIO.h"

#include "mrvCore/mrvI8N.h"

#include <tlCore/StringFormat.h>

namespace
{
    
    // Helper to release COM objects
    template <typename T>
    void SafeRelease(T*& p)
    {
        if (p)
        {
            p->Release();
            p = nullptr;
        }
    }
}

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
        
        const char* kModule = "mntr";
        
        // Get the monitor name given its FLTK screen index
        std::string getName(int monitorIndex, int numMonitors)
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

        
        // Callback struct for EnumDisplayMonitors
        struct MonitorInfo {
            std::wstring deviceName;
            HMONITOR hMonitor;
        };

        // Callback for EnumDisplayMonitors
        BOOL CALLBACK MonitorEnumProc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData) {
            auto* monitors = reinterpret_cast<std::vector<MonitorInfo>*>(dwData);
            MONITORINFOEXW info;
            info.cbSize = sizeof(MONITORINFOEXW);
            if (GetMonitorInfoW(hMonitor, &info)) {
                monitors->push_back({info.szDevice, hMonitor});
            }
            return TRUE;
        }

        HDRCapabilities get_hdr_capabilities(int screen_index) {
            HDRCapabilities out;  // Default to SDR

            // Step 1: Enumerate monitors in logical (FLTK-matching) order using EnumDisplayMonitors
            std::vector<MonitorInfo> logicalMonitors;
            EnumDisplayMonitors(NULL, NULL, MonitorEnumProc, reinterpret_cast<LPARAM>(&logicalMonitors));

            if (logicalMonitors.empty()) {
                return out;
            }

            // If screen_index == -1, we'll scan all; else, target specific index
            int targetIndex = (screen_index == -1) ? -1 : screen_index;
            if (targetIndex >= static_cast<int>(logicalMonitors.size()) || targetIndex < -1) {
                return out;  // Invalid index
            }

            // Step 2: Create DXGI factory
            IDXGIFactory6* factory = nullptr;
            HRESULT hr = CreateDXGIFactory1(IID_PPV_ARGS(&factory));
            if (FAILED(hr)) {
                return out;
            }

            // Step 3: Enumerate all DXGI adapters and outputs to build a map by device name
            std::unordered_map<std::wstring, IDXGIOutput*> outputMap;
            IDXGIAdapter1* adapter = nullptr;
            for (UINT i = 0; factory->EnumAdapters1(i, &adapter) != DXGI_ERROR_NOT_FOUND; ++i) {
                IDXGIOutput* output = nullptr;
                for (UINT j = 0; adapter->EnumOutputs(j, &output) != DXGI_ERROR_NOT_FOUND; ++j) {
                    DXGI_OUTPUT_DESC desc;
                    if (SUCCEEDED(output->GetDesc(&desc))) {
                        outputMap[desc.DeviceName] = output;
                        output = nullptr;  // Don't release yet; we'll release from map later
                    } else {
                        SafeRelease(output);
                    }
                }
                SafeRelease(adapter);
            }

            // Step 4: Process in logical order
            bool foundAnyHDR = false;
            bool matchedMonitor = false;
            for (size_t idx = 0; idx < logicalMonitors.size(); ++idx) {
                if (targetIndex != -1 && static_cast<int>(idx) != targetIndex) {
                    continue;  // Skip if not targeting this index
                }

                auto it = outputMap.find(logicalMonitors[idx].deviceName);
                if (it != outputMap.end()) {
                    IDXGIOutput* output = it->second;
                    IDXGIOutput6* output6 = nullptr;
                    if (SUCCEEDED(output->QueryInterface(IID_PPV_ARGS(&output6)))) {
                        DXGI_OUTPUT_DESC1 desc1;
                        if (SUCCEEDED(output6->GetDesc1(&desc1))) {
                            HDRCapabilities local_cap;
                            // Check if HDR is active based on current color space
                            switch (desc1.ColorSpace) {
                            case DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020:
                            case DXGI_COLOR_SPACE_YCBCR_STUDIO_G2084_LEFT_P2020:
                            case DXGI_COLOR_SPACE_YCBCR_STUDIO_GHLG_TOPLEFT_P2020:
                            case DXGI_COLOR_SPACE_YCBCR_FULL_GHLG_TOPLEFT_P2020:
                                local_cap.supported = true;
                                local_cap.min_nits = desc1.MinLuminance;
                                local_cap.max_nits = desc1.MaxLuminance;
                                break;
                            default:
                                break;
                            }

                            if (targetIndex != -1) {
                                // Specific index: Return this monitor's caps
                                out = local_cap;
                                SafeRelease(output6);
                                matchedMonitor = true;
                                goto cleanup;
                            } else if (local_cap.supported) {
                                // Any mode: Found an HDR monitor, return its caps
                                out = local_cap;
                                foundAnyHDR = true;
                                matchedMonitor = true;
                                SafeRelease(output6);
                                goto cleanup;
                            }

                            SafeRelease(output6);
                        }
                    }
                }
            }

        cleanup:
            // Release all outputs from map
            for (auto& pair : outputMap) {
                SafeRelease(pair.second);
            }
            SafeRelease(factory);

            if (!matchedMonitor)
            {
                std::string msg = tl::string::Format(_("Did not match any monitor at {0}.")).arg(screen_index);
                LOG_ERROR(msg);
            }
            return out;
        }

        
    } // namespace monitor
} // namespace mrv
