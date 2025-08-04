// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.


#ifdef _WIN32
#    include <winsock2.h>
#    include <windows.h>
#    include <dxgi1_6.h>
#endif

#include <cstdint>
#include <fstream>
#include <vector>
#include <iostream>
#include <iomanip>
#include <iostream>

namespace mrv
{

#ifdef _WIN32
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
    
    bool is_hdr_display_active(int screen_index, const bool silent)
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

    bool is_hdr_display_active()
    {
        return is_hdr_display_active(-1, false);
    }
    
#endif

#ifdef __linux__
    bool has_hdr_support(const std::string& edid_path)
    {
        std::ifstream file(edid_path, std::ios::binary);
        if (!file) return false;

        std::vector<unsigned char> edid((std::istreambuf_iterator<char>(file)), {});
        if (edid.size() < 128) return false;

        size_t extensions = edid[126];
        for (size_t i = 0; i < extensions; ++i) {
            size_t offset = 128 * (i + 1);
            if (offset + 128 > edid.size()) break;

            if (edid[offset] == 0x02) {  // CTA extension block
                uint8_t dtd_start = edid[offset + 2];
                size_t idx = offset + 4; // start of Data Block Collection

                while (idx < offset + dtd_start) {
                    uint8_t tag = edid[idx] >> 5;
                    uint8_t len = edid[idx] & 0x1F;

                    if (tag == 0x07) { // Use Extended tag
                        uint8_t ext_tag = edid[idx + 1];
                        if (ext_tag == 0x06) {
                            // std::cout << "HDR Static Metadata block found!\n";
                            return true;
                        }
                    }
                    idx += len + 1;
                }
            }
        }
        return false;
    }

    bool is_hdr_display_active(int screen)
    {
        char buf[256];
        snprintf(buf, 256, "/sys/class/drm/card0-HDMI-A-%d/edid", screen + 1);
        if (has_hdr_support(buf))
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    
    bool is_hdr_display_active()
    {
        return is_hdr_display_active(0);
    }
#endif

} // namespace mrv
