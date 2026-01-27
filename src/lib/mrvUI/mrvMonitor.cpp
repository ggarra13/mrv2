// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.


#include <iostream>
#include <sstream>
#include <string>

#include "mrvUI/mrvMonitor.h"

#ifdef _WIN32
#    include "mrvUI/mrvMonitor_win32.cpp"
#endif

#ifdef __linux__
#    include "mrvUI/mrvMonitor_linux.cpp"
#endif

#ifdef __APPLE__
#    include "mrvUI/mrvMonitor_macOS.cpp"
#endif

#include "mrvCore/mrvI8N.h"

#undef Status
#undef None
#include "mrvFl/mrvIO.h"

namespace
{
    const char* kModule = "monitor";
}

namespace mrv
{
    namespace monitor
    {
        std::string getManufacturerName(const char* edidCode)
        {
            if (strcmp(edidCode, "AAA") == 0)
                return "Avolites Ltd.";
            else if (strcmp(edidCode, "ABC") == 0)
                return "AboComo System Inc.";
            else if (strcmp(edidCode, "ACB") == 0)
                return "Aculab Ltd.";
            else if (strcmp(edidCode, "ACI") == 0)
                return "Ancor Communications Inc.";
            else if (strcmp(edidCode, "ACL") == 0)
                return "Apricot Computers";
            else if (strcmp(edidCode, "ACR") == 0)
                return "Acer Inc.";
            else if (strcmp(edidCode, "ADA") == 0)
                return "Addi-Data GmbH";
            else if (strcmp(edidCode, "ADI") == 0)
                return "ADI Systems Inc.";
            else if (strcmp(edidCode, "AGO") == 0)
                return "AlgolTek, Inc.";
            else if (strcmp(edidCode, "AIC") == 0)
                return "Arnos Instruments & Computer Systems";
            else if (strcmp(edidCode, "AJA") == 0)
                return "AJA Video Systems, Inc.";
            else if (strcmp(edidCode, "AMI") == 0)
                return "American Megatrends Inc.";
            else if (strcmp(edidCode, "AML") == 0)
                return "Anderson Multimedia Communications (HK) Limited";
            else if (strcmp(edidCode, "AMT") == 0)
                return "AMT International Industry";
            else if (strcmp(edidCode, "ANW") == 0)
                return "Analog Way SAS";
            else if (strcmp(edidCode, "ANX") == 0)
                return "Acer Netxus Inc.";
            else if (strcmp(edidCode, "API") == 0)
                return "A Plus Info Corporation";
            else if (strcmp(edidCode, "APP") == 0)
                return "Apple Inc.";
            else if (strcmp(edidCode, "ARD") == 0)
                return "AREC Inc.";
            else if (strcmp(edidCode, "ASU") == 0)
                return "ASUSTeK Computer Inc.";
            else if (strcmp(edidCode, "ATT") == 0)
                return "AT&T";
            else if (strcmp(edidCode, "BDS") == 0)
                return "Barco Display Systems";
            else if (strcmp(edidCode, "BMD") == 0)
                return "Blackmagic Design";
            else if (strcmp(edidCode, "BNQ") == 0)
                return "BenQ Corporation";
            else if ((strcmp(edidCode, "BPS") == 0) ||
                     (strcmp(edidCode, "DDS") == 0))
                return "Barco, N.V.";
            else if (strcmp(edidCode, "CAS") == 0)
                return "CASIO COMPUTER CO., LTD.";
            else if (strcmp(edidCode, "CMN") == 0)
                return "Chimei Innolux Corporation";
            else if (strcmp(edidCode, "CSO") == 0)
                return "California Institute of Technology";
            else if (strcmp(edidCode, "CUK") == 0)
                return "Calibre UK Ltd";
            else if (strcmp(edidCode, "DEL") == 0)
                return "Dell Inc.";
            else if (strcmp(edidCode, "DWE") == 0)
                return "Daewoo Electronics Company Ltd.";
            else if (strcmp(edidCode, "ENC") == 0)
                return "EIZO Nanao Corporation";
            else if (strcmp(edidCode, "FUS") == 0)
                return "Fujitsu Siemens Computers GmbH";
            else if (strcmp(edidCode, "FUJ") == 0)
                return "Fujitsu Limited";
            else if (strcmp(edidCode, "GBT") == 0)
                return "GIGA-BYTE TECHNOLOGY CO., LTD.";
            else if (strcmp(edidCode, "GGL") == 0)
                return "Google Inc.";
            else if (strcmp(edidCode, "GSM") == 0)
                return "LG Electronics";
            else if (strcmp(edidCode, "HWP") == 0)
                return "Hewlett-Packard (HP)";
            else if (strcmp(edidCode, "LEN") == 0)
                return "Lenovo Group Ltd.";
            else if (strcmp(edidCode, "LPL") == 0)
                return "LG Philips LCD";
            else if (strcmp(edidCode, "MSI") == 0)
                return "Micro-Star International";
            else if (strcmp(edidCode, "NEC") == 0)
                return "NEC Corporation";
            else if (strcmp(edidCode, "PHL") == 0)
                return "Philips";
            else if (strcmp(edidCode, "SAM") == 0)
                return "Samsung Electronics";
            else if (strcmp(edidCode, "SEC") == 0)
                return "Seiko Epson Corporation";
            else if (strcmp(edidCode, "SGI") == 0)
                return "Silicon Graphics, Inc.";
            else if (strcmp(edidCode, "SON") == 0)
                return "Sony Corporation";
            else if (strcmp(edidCode, "TOS") == 0)
                return "Toshiba Corporation";
            else if (strcmp(edidCode, "VIZ") == 0)
                return "Vizio, Inc.";
            else if (strcmp(edidCode, "VSC") == 0)
                return "ViewSonic Corporation";
            else if (strcmp(edidCode, "YMH") == 0)
                return "Yamaha Corporation";

            // Not a name we know, just return its edidCode
            return std::string(edidCode);
        }

        // Parse EDID to detect HDR static metadata support
        bool parseEDIDForHDR(const uint8_t* edid, size_t length) {
            if (length < 128) return false;

            uint8_t numExtensions = edid[126];
            const uint8_t* ext = edid + 128;

            for (int i = 0; i < numExtensions && (ext + 128 <= edid + length); ++i) {
                if (ext[0] == 0x02 && ext[1] == 0x03) {  // CTA-861 Extension Block
                    uint8_t dtdStart = ext[2];
                    if (dtdStart == 0 || dtdStart > 127) dtdStart = 127;

                    for (int j = 4; j < dtdStart - 4;) {
                        uint8_t tag = (ext[j] & 0xE0) >> 5;
                        uint8_t len = ext[j] & 0x1F;
                        if (tag == 0x07 && ext[j + 1] == 0x06) {
                            std::cout << "→ HDR static metadata block found in EDID\n";
                            return true;
                        }
                        j += len + 1;
                    }
                }
                ext += 128;
            }

            return false;
        }

        HDRCapabilities parseEDIDLuminance(const uint8_t* edid, size_t length) {
            HDRCapabilities caps;
            caps.supported = false; // Initialize to false
            caps.max_nits = 0.0f;
            caps.min_nits = 0.0f;
            
            if (length < 128) return caps;

            uint8_t numExtensions = edid[126];
            const uint8_t* ext = edid + 128;

            for (int i = 0; i < numExtensions && (ext + 127 <= edid + length); ++i) {
                if (ext[0] == 0x02 && ext[1] == 0x03) { // CTA-861 Extension Block
                    uint8_t dtdStart = ext[2];

                    // dtdStart should not exceed the block size (usually 128)
                    // or point before the data block collection starts (byte 4)
                    if (dtdStart == 0) dtdStart = 127;
                    if (dtdStart < 4 || dtdStart > 127) continue;
            
                    for (int j = 4; j < dtdStart;) {
                        if (j + 1 > dtdStart) break; // Safety check
                        
                        uint8_t tag = (ext[j] & 0xE0) >> 5;
                        uint8_t len = ext[j] & 0x1F;

                        // Ensure we don't read past the data block collection
                        if (j + len + 1 > dtdStart) break;
                
                        // Tag 7 + Extended Tag 6 = HDR Static Metadata Block
                        if (tag == 0x07 && len >= 3 && ext[j + 1] == 0x06) {

                           // Byte j+3: Static Metadata Descriptor Type
                            // We only know how to parse Type 1 (0x00).
                            uint8_t type = ext[j + 3];
                            if (type & 0x01) {
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
                                    if (min_cv > 0 && caps.max_nits > 0) {
                                        // Formula for min luminance is slightly different in CTA-861
                                        caps.min_nits = (caps.max_nits * powf((float)min_cv / 255.0f, 2.0f));
                                    }
                                }
                                
                                return caps;
                            }
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
