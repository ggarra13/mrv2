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
#include <windows.h>

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

namespace mrv
{
    namespace monitor
    {

        // On Windows, we don't need to parse EDID as it provides a good API for it.
        HDRCapabilities get_hdr_capabilities(int screen_index)
        {
            HDRCapabilities out;  // SDR
            
            IDXGIFactory6* factory = nullptr;
            HRESULT hr = CreateDXGIFactory1(IID_PPV_ARGS(&factory));
            if (FAILED(hr))
            {
                return out;
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
                    HDRCapabilities local_cap;
                    
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
                                local_cap.supported = true;
                                local_cap.min_nits = desc.MinLuminance;
                                local_cap.max_nits = desc.MaxLuminance;
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
                        out = local_cap;
                        SafeRelease(adapter); // Clean up before jumping
                        goto cleanup;
                    }
                    else
                    {
                        // ANY MODE: If this monitor is HDR, we are done (result is true).
                        // If not, we continue searching the next monitor.
                        if (local_cap.supported)
                        {
                            out = local_cap;
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
