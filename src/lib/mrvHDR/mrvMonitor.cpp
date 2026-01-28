// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.



#include "mrvMonitor.h"

#ifdef _WIN32
#    include "mrvMonitor_win32.cpp"
#endif

#ifdef __linux__
#    include "mrvMonitor_linux.cpp"
#endif

#ifdef __APPLE__
#    include "mrvMonitor_macOS.cpp"
#endif

#ifndef _WIN32
extern "C" {
#   include <libdisplay-info/info.h>
}
#endif

#include <iostream>
#include <sstream>
#include <string>
#include <cstdint>
#include <cmath>

namespace
{
    const char* kModule = "monitor";
}

namespace mrv
{
    namespace monitor
    {

#ifdef _WIN32
        // \@note: on Windows we don't parse EDID, we use Windows API
        HDRCapabilities parseEDIDLuminance(const uint8_t* edid, size_t length) {
            HDRCapabilities out;
            return out;
        }
#else
        
        HDRCapabilities parseEDIDLuminance(const uint8_t* raw, size_t size)
        {
            HDRCapabilities out;
            struct di_info *info;
            
            info = di_info_parse_edid(raw, size);
            if (!info) {
                perror("di_edid_parse failed");
                return out;
            }
            
            // Get HDR static metadata
            const struct di_hdr_static_metadata *hdr = di_info_get_hdr_static_metadata(info);

            // Check for HDR presence (supported if any HDR-related EOTF is true)
            bool has_hdr = hdr->traditional_hdr || hdr->pq || hdr->hlg || hdr->type1;

            out.supported = has_hdr;
            
            // Retrieve min and max nits (0.0 if unset)
            out.min_nits = hdr->desired_content_min_luminance;
            out.max_nits = hdr->desired_content_max_luminance;
            //out.max_frame_avg_nits = hdr->desired_content_max_frame_avg_luminance;  // Optional: frame-average max

            di_info_destroy(info);
            return out;
        }
#endif

#ifndef __linux__
        HDRCapabilities get_hdr_capabilities_by_name(const std::string& target_connector)
        {
            HDRCapabilities out;
            return out;
        }
#endif
        
    } // namespace monitor
} // namespace mrv
