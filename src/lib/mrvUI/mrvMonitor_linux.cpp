// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <FL/platform.H>

#ifdef FLTK_USE_X11
#    include "mrvUI/mrvMonitor_x11.cpp"
#endif

#ifdef FLTK_USE_WAYLAND
#    include "mrvUI/mrvMonitor_wayland.cpp"
#endif

#include <filesystem>
#include <fstream>
#include <string>
#include <vector>
#include <map>

namespace fs = std::filesystem;

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
            
#ifdef FLTK_USE_X11
            if (fl_x11_display())
                out = getX11Name(monitorIndex);
#endif
#ifdef FLTK_USE_WAYLAND
            if (fl_wl_display())
                out = getWaylandName(monitorIndex, numMonitors);
#endif

            names[monitorIndex] = out;
            
            return out;
        }
        
        HDRCapabilities get_hdr_capabilities(int screen_index)
        {
            HDRCapabilities out;

#ifdef FLTK_USE_X11
            if (fl_x11_display())
                return out;
#endif
            
            int current_monitor_index = 0;
            const std::string drm_path = "/sys/class/drm/";

            // 1. Iterate through cards (card0, card1, etc.)
            for (const auto& card_entry : fs::directory_iterator(drm_path)) {
                std::string card_name = card_entry.path().filename().string();
        
                // Skip render nodes (renderD128) and only look at cards
                if (card_name.find("card") == std::string::npos ||
                    card_name.find("-") != std::string::npos)
                    continue;

                // 2. Iterate through connectors belonging to this card
                // These look like card1-DP-1, card1-HDMI-A-1
                for (const auto& conn_entry : fs::directory_iterator(drm_path)) {
                    std::string conn_name = conn_entry.path().filename().string();
            
                    // Match connectors to the current card (e.g., card1 matches card1-DP-1)
                    if (conn_name.find(card_name + "-") == 0) {
                
                        // Check if a monitor is actually plugged in
                        std::ifstream status_file(conn_entry.path() / "status");
                        std::string status;
                        status_file >> status;
                        if (status != "connected") continue;

                        // If we are looking for a specific index and this isn't it, skip
                        if (screen_index != -1 && current_monitor_index != screen_index) {
                            current_monitor_index++;
                            continue;
                        }

                        // 3. Read EDID and Parse
                        std::ifstream edid_file(conn_entry.path() / "edid", std::ios::binary);
                        std::vector<uint8_t> edid_data((std::istreambuf_iterator<char>(edid_file)),
                                                       std::istreambuf_iterator<char>());

                        if (!edid_data.empty()) {
                            out = monitor::parseEDIDLuminance(edid_data.data(), edid_data.size());
                        }

                        if (screen_index != -1) {
                            // Target Mode: Return this specific monitor's status
                            return out;
                        } else if (out.supported) {
                            // Any Mode: Found one HDR monitor, we are done
                            return out;
                        }

                        current_monitor_index++;
                    }
                }
            }

            return out;
        }
        
    } // namespace monitor
} // namespace mrv
