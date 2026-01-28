// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.


#include <iostream>
#include <sstream>
#include <string>

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

#ifdef __limux__
extern "C" {
   #include <libdisplay-info/info.h>
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
#endif

#ifdef __linux__
        
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

#ifdef __APPLE__
        HDRCapabilities parseEDIDLuminance(const uint8_t* edid, size_t length) {
            HDRCapabilities out;
            
            if (length < 128) return out;

            uint8_t numExtensions = edid[126];
            const uint8_t* ext = edid + 128;

            for (int i = 0; i < numExtensions && (ext + 128 <= edid + length); ++i) {
                if (ext[0] == 0x02 && ext[1] == 0x03) { // CTA-861 Extension Block
                    std::cerr << "Got CTA-861" << std::endl;
                    uint8_t dtdStart = ext[2];

                    // dtdStart should not exceed the block size (usually 128)
                    // or point before the data block collection starts (byte 4)
                    if (dtdStart > 127 || dtdStart < 4) dtdStart = 127;
            
                    for (int j = 4; j < dtdStart;) {
                        if (j + 1 > dtdStart) break; // Safety check
                        
                        uint8_t tag = (ext[j] & 0xE0) >> 5;
                        uint8_t len = ext[j] & 0x1F;

                        // Ensure we don't read past the data block collection
                        if (j + len + 1 > dtdStart) break;
                
                        // Tag 7 + Extended Tag 6 = HDR Static Metadata Block
                        if (tag == 0x07 && len >= 3 && ext[j + 1] == 0x06) {

                            std::cerr << "Got HDR" << std::endl;
                    
                            uint8_t eotf = ext[j + 2];
                            if ((eotf & 0x0E) == 0) { // No HDR bits set
                                return out; // Keep supported=false
                            }
                            
                            // Byte j+3: Static Metadata Descriptor Type
                            // We only know how to parse Type 1 (0x01).

                            // ChatGPT tells me to compare type agains 0
                            // Gemini tells me to compare against byte 0x01
                            uint8_t type = ext[j + 3];
                            if (type & 0x01 || (eotf & 0x0E))
                            {
                                out.supported = true;
                            }

                            // Now, try to get the real min/max nits.
                            if (out.supported) {
                                
                                // Byte j+4: Desired Content Max Luminance
                                if (len >= 4) {
                                    uint8_t max_cv = ext[j + 4];
                                    if (max_cv > 0) {
                                        out.max_nits = 50.0f * powf(2.0f, (float)max_cv / 32.0f);
                                    }
                                }

                                // Byte j+6: Desired Content Min Luminance
                                if (len >= 6) {
                                    uint8_t min_cv = ext[j + 6];
                                    if (min_cv > 0 && out.max_nits > 0) {
                                        // Must divide by 100 contrary to
                                        // what Gemini an ChatGPT say.
                                        out.min_nits = (out.max_nits * powf((float)min_cv / 255.0f, 2.0f)) / 100.0F;
                                    }
                                }
                                
                                return out;
                            }
                        }
                        j += len + 1;
                    }
                }
                ext += 128;
            }
            return out;
        }
#endif

#ifndef  __linux__
        HDRCapabilities get_hdr_capabilities_by_name(
            const std::string& target_connector)
        {
            HDRCapabilities out;
            return out;
        }
#endif
        
    } // namespace monitor
} // namespace mrv
