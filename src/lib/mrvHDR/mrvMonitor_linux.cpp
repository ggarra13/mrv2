// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <FL/platform.H>

#ifdef FLTK_USE_WAYLAND
#    include "mrvHDR/mrvMonitor_wayland.cpp"
#endif

#include "mrvCore/mrvI8N.h"

#include <filesystem>
#include <fstream>
#include <string>
#include <regex>
#include <vector>
#include <map>

namespace fs = std::filesystem;

namespace
{
    std::string normalize_connector(std::string name) {
        // 1. Remove the "cardX-" prefix
        // Matches "card" followed by digits and a dash at the start of the string
        name = std::regex_replace(name, std::regex(R"(^card\d+-)"), "");

        // 2. Remove technical sub-types (HDMI-A-1 -> HDMI-1, DVI-I-1 -> DVI-1)
        // Matches (HDMI or DVI) followed by a dash, a single letter, and another dash
        // We use a capture group ($1) to keep the "HDMI" or "DVI" part
        name = std::regex_replace(name, std::regex(R"((HDMI|DVI)-[A-Z]-)"), "$1-");

        return name;
    }
}

namespace mrv
{
    namespace monitor
    {
        // We cache the names as getting them from X11 can be slow
        std::map<int, std::string> names;
        
        // Get the monitor name given its FLTK screen index
        std::string getName(int monitorIndex, int numMonitors)
        {
            std::string out;

            if (names.find(monitorIndex) != names.end() &&
                !names[monitorIndex].empty())
            {
                return names[monitorIndex];
            }
            
#ifdef FLTK_USE_WAYLAND
            if (fl_wl_display())
                out = getWaylandName(monitorIndex, numMonitors);
#endif

            names[monitorIndex] = out;
            
            return out;
        }

        /** 
         * Don't use this function as it is unreliable on linux.  Use
         *
         * 
         * @param screen_index 
         * 
         * @return 
         */
        HDRCapabilities get_hdr_capabilities(int screen_index)
        {
            HDRCapabilities out;

#ifdef FLTK_USE_X11
            if (fl_x11_display())
                return out;
#endif
            out.supported = true;
            out.min_nits = 0;
            out.max_nits = 1000.F;
            return out;
        }
        

        HDRCapabilities get_hdr_capabilities_by_name(
            const std::string& target_connector)
        {
            HDRCapabilities out;
            const std::string drm_path = "/sys/class/drm/";

            bool activeMonitorFound = false;
            std::vector<std::string> connections;
            
            for (const auto& card_entry : fs::directory_iterator(drm_path)) {
                std::string card_name = card_entry.path().filename().string();
                if (card_name.find("card") == std::string::npos || card_name.find("-") != std::string::npos)
                    continue;

                for (const auto& conn_entry : fs::directory_iterator(drm_path + card_name)) {
                    std::string conn_full_name = conn_entry.path().filename().string();
            
                    if (conn_full_name.find("DP-") == std::string::npos &&
                        conn_full_name.find("HDMI-") == std::string::npos &&
                        conn_full_name.find("eDP-") == std::string::npos &&
                        conn_full_name.find("DVI-") == std::string::npos)
                        continue;
                
                    // Extract the connector part: "card1-DP-1" -> "DP-1"
                    std::string conn_name = conn_full_name.substr(card_name.length() + 1);
                    connections.push_back(conn_name);
                    
                    if (conn_full_name.find(card_name + "-") == 0) {
                        // Match the FLTK label to the DRM connector name
                        conn_name = normalize_connector(conn_name);
                        if (conn_name != target_connector) continue;
                        
                        std::ifstream status_file(conn_entry.path() / "status");
                        std::string status;
                        status_file >> status;
                        if (status != "connected") continue;

                        activeMonitorFound = true;
                        
                        // 3. Read EDID and Parse
                        std::ifstream edid_file(conn_entry.path() / "edid", std::ios::binary);
                        std::vector<uint8_t> edid_data((std::istreambuf_iterator<char>(edid_file)),
                                                       std::istreambuf_iterator<char>());

                        if (!edid_data.empty()) {
                            out = monitor::parseEDIDLuminance(edid_data.data(), edid_data.size());
                        }
                        
                        return out;
                    }
                }
            }

            if (!activeMonitorFound)
            {
                std::cerr << _("Failed to find a monitor with connection ")
                          << target_connector << std::endl;
                std::cerr << "Valid connections:" << std::endl;
                for (auto& connection : connections)
                {
                    std::cerr << connection << std::endl;
                }
            }
            return out;
        }
        
    } // namespace monitor
} // namespace mrv
