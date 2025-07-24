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
    bool is_hdr_display_active()
    {
        IDXGIFactory6* factory = nullptr;
        CreateDXGIFactory1(IID_PPV_ARGS(&factory));

        IDXGIAdapter1* adapter = nullptr;
        for (UINT i = 0;
             factory->EnumAdapters1(i, &adapter) != DXGI_ERROR_NOT_FOUND; ++i)
        {
            IDXGIOutput* output = nullptr;
            if (SUCCEEDED(adapter->EnumOutputs(0, &output)))
            {
                IDXGIOutput6* output6 = nullptr;
                if (SUCCEEDED(output->QueryInterface(IID_PPV_ARGS(&output6))))
                {
                    DXGI_OUTPUT_DESC1 desc;
                    output6->GetDesc1(&desc);

                    // Check for HDR support
                    if (desc.ColorSpace ==
                            DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020 ||
                        desc.ColorSpace ==
                            DXGI_COLOR_SPACE_RGB_FULL_G10_NONE_P709)
                    {
                        std::cout << "HDR is active on display.\n";
                        output6->Release();
                        output->Release();
                        adapter->Release();
                        factory->Release();
                        return true;
                    }

                    output6->Release();
                }
                output->Release();
            }
            adapter->Release();
        }
        factory->Release();
        return false;
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
