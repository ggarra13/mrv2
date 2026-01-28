// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <cstdint>
#include <string>

namespace mrv
{
    namespace monitor
    {
        //! Function to decode edidManufactureId to a three-letter char
        inline std::string decodeEdidManufacturerId(const unsigned char* edid)
        {
            char vendorId[4];

            snprintf(
                vendorId, 4, "%c%c%c", (edid[0] >> 2 & 0x1f) + 'A' - 1,
                (((edid[0] & 0x3) << 3) | ((edid[1] & 0xe0) >> 5)) + 'A' - 1,
                (edid[1] & 0x1f) + 'A' - 1);

            return std::string(vendorId);
        }

        //! Function to decode edidManufactureId to a three-letter code
        inline std::string decodeEdidManufactureId(uint16_t edidUint)
        {
            const unsigned char* edid =
                reinterpret_cast<unsigned char*>(&edidUint);
            return decodeEdidManufacturerId(edid);
        }

        std::string getManufacturerName(const char* vendorId);
        
        struct HDRCapabilities {
            bool supported = false;
            float max_nits = 0.0f;
            float min_nits = 0.0f;
        };

        HDRCapabilities parseEDIDLuminance(const uint8_t* edid, size_t length);
        
        std::string getName(int monitorIndex, int numMonitors);

        HDRCapabilities get_hdr_capabilities(int screen = -1);

#ifdef FLTK_USE_WAYLAND
        HDRCapabilities
        get_hdr_capabilities_by_name(const std::string& target_connector);
#endif
        
        
    } // namespace monitor
} // namespace mrv
