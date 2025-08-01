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

        bool is_hdr_active(int screen_index, const bool silent)
        {
            IDXGIFactory6* factory = nullptr;
            HRESULT hr = CreateDXGIFactory1(IID_PPV_ARGS(&factory));
            if (FAILED(hr))
            {
                if (!silent)
                    std::cerr << "Error: Could not create DXGI Factory: "
                              << hr << std::endl;
                return false;
            }

            bool hdr_found = false;
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
                    // If you need to target a specific screen by index, you'd add logic here.
                    // For example, if 'screen_index' means the j-th output of the i-th adapter:
                    // if (i == target_adapter_index && j == screen_index) { ... }
                    // Or if screen_index is a global index:
                    // if (global_output_counter == screen_index) { ... }
                    // For now, let's assume 'screen_index' is not strictly tied to the j-th output and
                    // we're checking if *any* display has HDR. If 'screen_index' is meant to be a direct
                    // target, more complex matching via GetDesc().Monitor or similar is needed.

                    IDXGIOutput6* output6 = nullptr;
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
                                hdr_found = true;
                                break;
                            default:
                                break;
                            }
                        }
                        SafeRelease(output6); // Release output6 after use
                    }
                    SafeRelease(output); // Release output after use
                    if (hdr_found && screen_index <= 0)
                    {
                        SafeRelease(output);
                        SafeRelease(adapter);
                        goto cleanup;
                    }
                }
                SafeRelease(adapter); // Release adapter after its outputs are processed
            }

        cleanup:
            SafeRelease(factory); // Release factory at the very end

            return hdr_found;
        }
        
    } // namespace monitor
} // namespace mrv
