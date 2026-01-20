// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.


#ifdef _WIN32
#    include <winsock2.h>
#    include <windows.h>
#    include <dxgi1_6.h>
#endif

#include <iostream>
#include <sstream>
#include <string>

#include <vector>
#include <codecvt>
#include <windows.h>

#include "mrvUI/mrvDesktop.h"

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

        // On Windows, we don't need to parse EDID as it provides a good API for it.
        HDRCapabilities get_hdr_capabilities(int screen)
        {
            HDRCapabilities out;  // SDR
            
            IDXGIFactory6* factory = nullptr;
            HRESULT hr = CreateDXGIFactory1(IID_PPV_ARGS(&factory));
            if (FAILED(hr))
            {
                if (!silent)
                    std::cerr << "Error: Could not create DXGI Factory: "
                              << hr << std::endl;
                return 1000.F;
            }

            bool hdr_found = false;
            int current_monitor_index = 0;
            IDXGIAdapter1* adapter = nullptr;
            for (UINT i = 0;
                 factory->EnumAdapters1(i, &adapter) != DXGI_ERROR_NOT_FOUND;
                 ++i)
            {
                // Enumerate outputs for the current adapter
                IDXGIOutput* output = nullptr;
                for (UINT j = 0; adapter->EnumOutputs(j, &output) !=
                              DXGI_ERROR_NOT_FOUND; ++j)
                {
                    // If we are looking for a specific index and this isn't it, skip logic but increment counter
                    if (screen_index != -1 && current_monitor_index != screen_index)
                    {
                        current_monitor_index++;
                        SafeRelease(output);
                        continue;
                    }
                    
                    // We are at the target monitor, or scanning all (-1)
                    IDXGIOutput6* output6 = nullptr;
                    HDRCapabilities local_hdr;
                    
                    if (SUCCEEDED(output->QueryInterface(IID_PPV_ARGS(&output6))))
                    {
                        DXGI_OUTPUT_DESC1 desc;
                        if (SUCCEEDED(output6->GetDesc1(&desc)))
                        {
                            // Check for HDR support
                            switch(desc.ColorSpace)
                            {
                            case DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020:
                            case DXGI_COLOR_SPACE_YCBCR_STUDIO_G2084_LEFT_P2020:
                            case DXGI_COLOR_SPACE_YCBCR_STUDIO_GHLG_TOPLEFT_P2020:
                            case DXGI_COLOR_SPACE_YCBCR_FULL_GHLG_TOPLEFT_P2020:
                                local_hdr.supported = true;
                                local.hdr.min_nits = desc.MinLuminance;
                                local.hdr.max_nits = desc.MaxLuminance;
                                break;
                            default:
                                break;
                            }
                        }
                        SafeRelease(output6); // Release output6 after use
                    }
                    SafeRelease(output); // Release output after use

                    // Logic handling based on search mode
                    if (screen_index != -1)
                    {
                        // TARGET MODE: We found the specific monitor index.
                        // Return exactly what this monitor's state is.
                        out = local_hdr;
                        SafeRelease(adapter); // Clean up before jumping
                        goto cleanup;
                    }
                    else
                    {
                        // ANY MODE: If this monitor is HDR, we are done (result is true).
                        // If not, we continue searching the next monitor.
                        if (local_hdr.supported)
                        {
                            out = local_hdr;
                            SafeRelease(adapter);
                            goto cleanup;
                        }
                    }

                    current_monitor_index++;
                }
                SafeRelease(adapter); // Release adapter after its outputs are processed
            }
            
        cleanup:
            SafeRelease(factory); // Release factory at the very end

            return out;
        }
        
    } // namespace monitor
} // namespace mrv
