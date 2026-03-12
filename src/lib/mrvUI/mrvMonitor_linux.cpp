// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <FL/platform.H>

#include "mrvCore/mrvI8N.h"

#ifdef FLTK_USE_X11
#    include "mrvUI/mrvMonitor_x11.cpp"
#endif

#ifdef FLTK_USE_WAYLAND
#    include "mrvUI/mrvMonitor_wayland.cpp"
#endif

#include <filesystem>
#include <fstream>
#include <regex>
#include <string>
#include <vector>
#include <map>

#include <xf86drm.h>
#include <xf86drmMode.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

namespace fs = std::filesystem;

namespace
{
    
    std::string remove_card_prefix(std::string name) {
        // 1. Remove the "cardX-" prefix
        // Matches "card" followed by digits and a dash at the start of the string
        name = std::regex_replace(name, std::regex(R"(^card\d+-)"), "");
        return name;
    }

    std::string normalize_connector(std::string name) {
        // This matches the Prefix, the middle letter + dash (if it exists), 
        // and captures the prefix and the trailing number.
        // Example: HDMI-A-1 -> HDMI-1
        // Example: DVI-I-3  -> DVI-3
        // Example: eDP-1    -> eDP-1 (No change)
        return std::regex_replace(name, std::regex(R"((HDMI|DVI|DP|eDP|VGA)-[A-Z]-(\d+))"), "$1-$2");
    }
}

namespace mrv
{
    namespace monitor
    {
        using tl::monitor::Capabilities;
        
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

        /** 
         * This function is unreliable but it is kept to handle a single
         * monitor or old Linux distros that do not return any names.
         * 
         * @param screen_index -1 for any, 0+ for corresponding monitor
         * 
         * @return Capabilities struct.
         */
        Capabilities get_hdr_capabilities(int screen_index)
        {
            Capabilities out;
            
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
                        
                        // \@todo: How to check HDR is enabled?
                        if (out.hdr_supported)
                        {
                            out.hdr_enabled = out.hdr_supported;
                        }

                        if (screen_index != -1) {
                            // Target Mode: Return this specific monitor's status
                            return out;
                        } else if (out.hdr_supported) {
                            // Any Mode: Found one HDR monitor, we are done
                            return out;
                        }

                        current_monitor_index++;
                    }
                }
            }

            return out;
        }            
        
        Capabilities get_hdr_capabilities_by_name(
            const std::string& target_connector)
        {
            Capabilities out;
            const std::string drm_path = "/sys/class/drm/";


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
                        conn_name = remove_card_prefix(conn_name);
                        std::string normalized = normalize_connector(conn_name);

                        connections.push_back(normalized);
                    
                        if (conn_name != target_connector &&
                            normalized != target_connector) continue;
                        
                        std::ifstream status_file(conn_entry.path() / "status");
                        if (!status_file.is_open())
                        {
                            LOG_ERROR("Failed to open "
                                      << (conn_entry.path() / "status"));
                            continue;
                        }
                        std::string status;
                        status_file >> status;
                        if (status != "connected")
                        {
                            LOG_ERROR("Target connector " << target_connector << " is not connected");
                            continue;
                        }

                        // 3. Read EDID and Parse
                        std::ifstream edid_file(conn_entry.path() / "edid", std::ios::binary);
                        if (!edid_file.is_open())
                        {
                            std::cerr << "Failed to open "
                                      << (conn_entry.path() / "edid")
                                      << std::endl;
                            continue;
                        }

                        std::vector<uint8_t> edid_data((std::istreambuf_iterator<char>(edid_file)),
                                                       std::istreambuf_iterator<char>());

                        if (!edid_data.empty())
                        {
                            out = monitor::parseEDIDLuminance(edid_data.data(),  edid_data.size());
                            if (out.hdr_supported)
                            {
                                out.hdr_enabled = false; // Default to false
                        
                                // Open the DRM device
                                std::string drm_device =
                                    "/dev/dri/" + card_name;
                                int fd = open(drm_device.c_str(), O_RDWR);
                                if (fd >= 0)
                                {
                                    drmModeRes *resources = drmModeGetResources(fd);
                                    if (resources)
                                    {
                                        // Iterate through all connectors to find the matching one
                                        for (int i = 0; i < resources->count_connectors; i++)
                                        {
                                            drmModeConnector *connector = drmModeGetConnector(fd, 
                                                                                              resources->connectors[i]);
                                            if (!connector) continue;
                                    
                                            // Build connector name to match against our target
                                            const char* conn_type_name = drmModeGetConnectorTypeName(
                                                connector->connector_type);
                                            if (conn_type_name)
                                            {
                                                std::string drm_conn_name = std::string(conn_type_name) + 
                                                                            "-" + std::to_string(connector->connector_type_id);
                                        
                                                // Check if this matches our connector
                                                if (drm_conn_name == conn_name || 
                                                    normalize_connector(drm_conn_name) == normalized)
                                                {
                                                    // Get connector properties
                                                    drmModeObjectProperties *props = drmModeObjectGetProperties(fd,
                                                                                                                connector->connector_id,
                                                                                                                DRM_MODE_OBJECT_CONNECTOR);
                                                    if (props)
                                                    {
                                                        // Look for HDR_OUTPUT_METADATA property
                                                        for (uint32_t j = 0; j < props->count_props; j++)
                                                        {
                                                            drmModePropertyRes *prop = drmModeGetProperty(fd, 
                                                                                                          props->props[j]);
                                                            if (prop)
                                                            {
                                                                if (strcmp(prop->name, "HDR_OUTPUT_METADATA") == 0)
                                                                {
                                                                    uint64_t value = props->prop_values[j];
                                                                    // Non-zero blob ID means HDR is active
                                                                    out.hdr_enabled = (value != 0);
                                                                }
                                                                drmModeFreeProperty(prop);
                                                            }
                                                        }
                                                        drmModeFreeObjectProperties(props);
                                                    }
                                            
                                                    drmModeFreeConnector(connector);
                                                    break; // Found our connector, stop searching
                                                }
                                            }
                                    
                                            drmModeFreeConnector(connector);
                                        }
                                        drmModeFreeResources(resources);
                                    }
                                    close(fd);
                                }
                                else
                                {
                                    LOG_ERROR("Failed to open DRM device " << drm_device 
                                              << ": " << strerror(errno));
                                }
                            }

                            std::cerr << "HDR supported=" << out.hdr_supported
                                      << std::endl;
                            std::cerr << "HDR enabled=" << out.hdr_enabled
                                      << std::endl;
                        
                            return out;
                        }
                        
                        
                    }
                }
            }

            std::cerr << _("Failed to find a monitor with connector ")
                      << target_connector << std::endl;
            std::cerr << "Valid connections:" << std::endl;
            for (auto& connection : connections)
            {
                std::cerr << connection << std::endl;
            }
            return out;
        }
        
    } // namespace monitor
} // namespace mrv
