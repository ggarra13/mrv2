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

        HDRCapabilities parseEDIDLuminance(const uint8_t* edid, size_t length) {
            HDRCapabilities caps;
            if (length < 128) return caps;

            uint8_t numExtensions = edid[126];
            const uint8_t* ext = edid + 128;

            for (int i = 0; i < numExtensions && (ext + 128 <= edid + length); ++i) {
                if (ext[0] == 0x02 && ext[1] == 0x03) { // CTA-861 Extension Block
                    uint8_t dtdStart = ext[2];
            
                    for (int j = 4; j < dtdStart - 4;) {
                        uint8_t tag = (ext[j] & 0xE0) >> 5;
                        uint8_t len = ext[j] & 0x1F;

                        // Tag 7 + Extended Tag 6 = HDR Static Metadata Block
                        if (tag == 0x07 && len >= 3 && ext[j + 1] == 0x06) {
                            caps.supported = true;

                            // Byte j+4: Desired Content Max Luminance
                            if (len >= 4) {
                                uint8_t max_cv = ext[j + 4];
                                if (max_cv > 0) {
                                    caps.max_nits = 50.0f * powf(2.0f, (float)max_cv / 32.0f);
                                }
                            }

                            // Byte j+6: Desired Content Min Luminance
                            if (len >= 6) {
                                uint8_t min_cv = ext[j + 6];
                                if (min_cv > 0) {
                                    // Formula for min luminance is slightly different in CTA-861
                                    caps.min_nits = (caps.max_nits * powf((float)min_cv / 255.0f, 2.0f)) / 100.0f;
                                }
                            }
                    
                            return caps;
                        }
                        j += len + 1;
                    }
                }
                ext += 128;
            }
            return caps;
        }
        
    } // namespace monitor
} // namespace mrv
