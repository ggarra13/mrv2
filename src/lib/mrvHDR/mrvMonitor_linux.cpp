// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <FL/platform.H>

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
        /** 
         * \@bug: This function is wrong.  While it can match an HDR monitor
         * if passed a screen_index of -1, there's no guarantee that an index
         * for 0 or more will match the connector name correctly, as both
         * GNOME and KWin number monitors in different order than Wayland.
         * 
         * @param screen_index screen index (0 or more). -1 to match any. 
         * 
         * @return HDR capabilities struct
         */
        HDRCapabilities get_hdr_capabilities(int screen_index)
        {
            HDRCapabilities out;
            
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

        HDRCapabilities get_hdr_capabilities_by_name(const std::string& target)
        {
            HDRCapabilities out;
            const std::string drm_path = "/sys/class/drm/";

            for (const auto& card_entry : fs::directory_iterator(drm_path)) {
                std::string card_name = card_entry.path().filename().string();
                if (card_name.find("card") == std::string::npos || card_name.find("-") != std::string::npos)
                    continue;

                for (const auto& conn_entry : fs::directory_iterator(drm_path)) {
                    std::string conn_full_name = conn_entry.path().filename().string();
            
                    // Extract the connector part: "card1-DP-1" -> "DP-1"
                    std::string conn_name = conn_full_name.substr(card_name.length() + 1);

                    if (conn_full_name.find(card_name + "-") == 0) {
                        // Match the FLTK label to the DRM connector name
                        if (conn_name != target) continue;

                        std::ifstream status_file(conn_entry.path() / "status");
                        std::string status;
                        status_file >> status;
                        if (status != "connected") continue;

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
            return out;
        }
        
    } // namespace monitor
} // namespace mrv
