// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.


#include <iostream>
#include <sstream>
#include <string>

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

#include "mrvUI/mrvDesktop.h"

namespace
{
    const char* kModule = "monitor";
}

namespace mrv
{
    namespace desktop
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
                return "California Insitute of Technology";
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
        

        std::string monitorName(int monitorIndex)
        {
            std::string out;
            try
            {
                out = getMonitorName(monitorIndex);
            }
            catch (const std::exception& e)
            {
                LOG_INFO(e.what());
            }

            if (out.empty())
            {
                // Unknown OS, or could not retrieve monitor name.
                // Just return Monitor #
                out = _("Monitor ") + std::to_string(monitorIndex +1) + ":";
            }

            return out;
        }
    } // namespace desktop
} // namespace mrv
