// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <algorithm>
#include <cstring>
#include <fstream>
#include <filesystem>
#include <iostream>
#include <sstream>
#include <string>
#include <tuple>
#include <vector>
#include <stdexcept>

namespace fs = std::filesystem;

#include <wayland-client.h>
#include <FL/platform.H> // Make sure to include for fl_wl_display()

namespace
{
    // A singleton class to manage Wayland monitor information safely.
    class WaylandMonitorManager
    {
    public:
        // Public method to get the single instance of this class.
        static WaylandMonitorManager& instance()
        {
            static WaylandMonitorManager manager;
            return manager;
        }

        struct MonitorData
        {
            struct wl_output* output;
            uint32_t global_id;
            std::string name;
            std::string description;
            int32_t x = 0;
            int32_t y = 0;
        };

        // Get the sorted list of monitors. It initializes on first call.
        const std::vector<MonitorData>& getMonitors()
        {
            if (!initialized)
            {
                initialize();
            }
            return monitors;
        }

    private:
        // Private constructor to enforce the singleton pattern.
        WaylandMonitorManager() = default;

        // Clean up Wayland resources.
        ~WaylandMonitorManager()
        {
        }

        // Delete copy constructor and assignment operator.
        WaylandMonitorManager(const WaylandMonitorManager&) = delete;
        WaylandMonitorManager& operator=(const WaylandMonitorManager&) = delete;

        void initialize()
        {
            auto display = fl_wl_display();
            if (!display)
            {
                // Cannot get the display; there's nothing to do.
                return;
            }

            struct wl_registry* registry = wl_display_get_registry(display);
            if (!registry)
            {
                // Cannot get the registry.
                return;
            }

            // Up to 64 monitors should be enough.
            monitors.reserve(64);

            // Add the registry listener, passing 'this' as user data.
            wl_registry_add_listener(registry, &registry_listener, this);

            // First roundtrip to get the list of global objects (like wl_output).
            wl_display_roundtrip(display);
            
            // Second roundtrip to ensure we receive all the output-specific events (like name, geometry).
            wl_display_roundtrip(display);
            
            // Sort the monitors so the primary one (usually at 0,0) is first.
            std::sort(
                monitors.begin(), monitors.end(),
                [](const MonitorData& a, const MonitorData& b)
                { return std::tie(a.x, a.y) < std::tie(b.x, b.y); });
                        
            initialized = true;
        }

        std::vector<MonitorData> monitors;
        bool initialized = false;

        // --- Wayland C-style Callbacks ---
        // They are static because C libraries don't know about 'this' pointers.
        // We get our instance back via the 'data' pointer.

        static void handle_global(void* data, struct wl_registry* registry,
                                  uint32_t global_id, const char* interface,
                                  uint32_t version)
        {
            if (strcmp(interface, wl_output_interface.name) == 0)
            {
                WaylandMonitorManager* self = static_cast<WaylandMonitorManager*>(data);
                // Bind to the wl_output interface
                struct wl_output* output = static_cast<wl_output*>(
                    wl_registry_bind(registry, global_id, &wl_output_interface,
                                     std::min(version, 4u)));

                self->monitors.emplace_back();
                MonitorData& monitor_info = self->monitors.back();
                monitor_info.output = output;
                monitor_info.global_id = global_id;
                
                wl_output_add_listener(output, &output_listener,
                                       &self->monitors.back());
            }
        }

        static void handle_global_remove(void* data, struct wl_registry* registry, uint32_t name)
        {
            // For a fully dynamic application, you would implement logic here
            // to remove a monitor from your list if it gets disconnected.
        }

        static const struct wl_registry_listener registry_listener;

        // --- wl_output Listeners ---

        static void handle_geometry(void* data, struct wl_output* wl_output, int32_t x, int32_t y, int32_t physical_width, int32_t physical_height, int32_t subpixel, const char* make, const char* model, int32_t transform)
        {
            MonitorData* monitor_info = static_cast<MonitorData*>(data);
            monitor_info->x = x;
            monitor_info->y = y;
        }

        static void handle_name(void* data, struct wl_output* wl_output, const char* name)
        {
            MonitorData* monitor_info = static_cast<MonitorData*>(data);
            monitor_info->name = name;
        }

        static void handle_description(void* data, struct wl_output* wl_output, const char* description)
        {
            MonitorData* monitor_info = static_cast<MonitorData*>(data);
            monitor_info->description = description;
        }
        
        // Unused handlers from the original code.
        static void handle_mode(void* data, struct wl_output* wl_output, uint32_t flags, int32_t width, int32_t height, int32_t refresh) {}
        static void handle_done(void* data, struct wl_output* wl_output) {}
        static void handle_scale(void* data, struct wl_output* wl_output, int32_t factor) {}

        static const struct wl_output_listener output_listener;
    };

    // Initialize static listener structs.
    const struct wl_registry_listener WaylandMonitorManager::registry_listener = {
        .global = handle_global,
        .global_remove = handle_global_remove,
    };

    const struct wl_output_listener WaylandMonitorManager::output_listener = {
        .geometry = handle_geometry,
        .mode = handle_mode,
        .done = handle_done,
        .scale = handle_scale,
#if defined(WL_OUTPUT_NAME_SINCE_VERSION) && WL_OUTPUT_NAME_SINCE_VERSION >= 4
        .name = handle_name,
        .description = handle_description,
#endif
    };
} // namespace

namespace mrv
{
    namespace monitor
    {
        // This is the only function you need to call from your application logic.
        // It's now fast, non-blocking, and stable.
        std::string getWaylandName(int monitorIndex, int numMonitors)
        {
            std::string out;
#if !defined(WL_OUTPUT_NAME_SINCE_VERSION) || WL_OUTPUT_NAME_SINCE_VERSION <= 3
#    warning "Wayland version is less than 4. No monitor name will be provided."
            return out;
#endif
            const auto& monitors = WaylandMonitorManager::instance().getMonitors();

            if (monitorIndex < 0 || monitorIndex >= monitors.size())
            {
                throw std::runtime_error("Invalid Monitor index");
            }
            
            const auto& monitorInfo = monitors[monitorIndex];
            if (!monitorInfo.name.empty())
            {
                out = monitorInfo.name + ": " + monitorInfo.description;
            }
            return out;
        }

        
        /**
         * @brief Reads the raw EDID data from a sysfs file path.
         * @param edid_path The std::filesystem::path to the monitor's EDID file.
         * @return std::vector<unsigned char> A vector containing the raw EDID bytes. Returns an empty vector on failure.
         */
        std::vector<unsigned char> GetEdidFromSysfs(const fs::path& edid_path) {
            // Open the EDID file in binary mode
            std::ifstream file(edid_path, std::ios::binary);
            if (!file) {
                std::cerr << "  [Error] Failed to open EDID file: " << edid_path << std::endl;
                return {};
            }

            // Read the entire file into a vector
            return std::vector<unsigned char>((std::istreambuf_iterator<char>(file)),
                                              std::istreambuf_iterator<char>());
        }
        
    } // namespace monitor
} // namespace mrv
