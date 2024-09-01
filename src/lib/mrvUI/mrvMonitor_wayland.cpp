// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.


#include <cstring>

#include <iostream>
#include <sstream>
#include <string>
#include <tuple>
#include <vector>
#include <algorithm>

#include <wayland-client.h>

namespace
{

    // Structure to hold information about the outputs
    struct WaylandMonitorData
    {
        struct wl_output* output;
        uint32_t global_id;
        uint32_t version;
        std::string name;
        std::string description;
        int32_t x = 0; // X position of the monitor
        int32_t y = 0; // Y position of the monitor
    };

    std::vector<WaylandMonitorData> outputs;

    void handle_description(
        void* data, struct wl_output* output, const char* description)
    {
        WaylandMonitorData* info = static_cast<WaylandMonitorData*>(data);
        info->description = description;
    }

    void handle_name(void* data, struct wl_output* output, const char* name)
    {
        WaylandMonitorData* info = static_cast<WaylandMonitorData*>(data);
        info->name = name;
    }

    void handle_geometry(
        void* data, struct wl_output* wl_output, int32_t x, int32_t y,
        int32_t physical_width, int32_t physical_height, int32_t subpixel,
        const char* make, const char* model, int32_t transform)
    {
        WaylandMonitorData* info = static_cast<WaylandMonitorData*>(data);
        info->x = x;
        info->y = y;
    }

    void handle_mode(
        void* data, struct wl_output* wl_output, uint32_t flags, int32_t width,
        int32_t height, int32_t refresh)
    {
        // Handle mode event
    }

    void handle_done(void* data, struct wl_output* wl_output)
    {
        // Handle done event
    }

    void handle_scale(void* data, struct wl_output* wl_output, int32_t factor)
    {
        // Handle scale event
    }

    const struct wl_output_listener output_listener = {
        .geometry = handle_geometry,
        .mode = handle_mode,
        .done = handle_done,
        .scale = handle_scale,
#if defined(WL_OUTPUT_NAME_SINCE_VERSION) && WL_OUTPUT_NAME_SINCE_VERSION >= 4
        .name = handle_name,
        .description = handle_description,
#endif
    };

    // Callback function to handle the global objects announced by the registry
    void handle_global(
        void* data, struct wl_registry* registry, uint32_t name,
        const char* interface, uint32_t version)
    {
        if (strcmp(interface, wl_output_interface.name) == 0)
        {
            // Bind to the wl_output interface
            struct wl_output* output = static_cast<wl_output*>(wl_registry_bind(
                registry, name, &wl_output_interface, std::min(version, 4u)));

            WaylandMonitorData outputInfo;
            outputInfo.output = output;
            outputInfo.global_id = name;
            outputInfo.version = version;

            outputs.push_back(outputInfo);

            wl_output_add_listener(output, &output_listener, &outputs.back());
        }
    }

    void handle_global_remove(
        void* data, struct wl_registry* registry, uint32_t name)
    {
    }

    // Registry listener
    struct wl_registry_listener registry_listener = {
        .global = handle_global,
        .global_remove = handle_global_remove,
    };

} // namespace

namespace mrv
{
    namespace desktop
    {
        // Get the monitor name given its FLTK screen index
        std::string getWaylandMonitorName(int monitorIndex)
        {
            std::string out;
            auto display = fl_wl_display();

            outputs.clear();

            struct wl_registry* registry = wl_display_get_registry(display);

            // Add the registry listener
            wl_registry_add_listener(registry, &registry_listener, nullptr);

            // Process the registry events to trigger the listener
            wl_display_roundtrip(display);

            if (monitorIndex < 0 || monitorIndex >= outputs.size())
                throw std::runtime_error("Invalid Monitor index");

            // Process the registry events to trigger the listener
            wl_display_roundtrip(display);

            // Sort the outputs so that the primary monitor (at (0, 0))
            // comes first
            std::sort(
                outputs.begin(), outputs.end(),
                [](const WaylandMonitorData& a, const WaylandMonitorData& b)
                { return std::tie(a.x, a.y) < std::tie(b.x, b.y); });

            const WaylandMonitorData& monitorInfo = outputs[monitorIndex];

#if defined(WL_OUTPUT_NAME_SINCE_VERSION) && WL_OUTPUT_NAME_SINCE_VERSION >= 4
            if (!monitorInfo.name.empty())
                out = monitorInfo.name + ": " + monitorInfo.description;
#else
#    warning "Wayland version is less than 4.  No monitor will be provided."
#endif
            return out;
        }

    } // namespace desktop
} // namespace mrv
