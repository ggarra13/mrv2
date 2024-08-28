
#include <cstring>

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include <FL/platform.H>

#ifdef __linux__
#    ifdef FLTK_USE_X11
#        include <X11/Xlib.h>
#        include <X11/extensions/Xrandr.h>
#    endif
#    ifdef FLTK_USE_WAYLAND
#        include <wayland-client.h>
#    endif
#endif


namespace
{
#ifdef FLTK_USE_WAYLAND

    
    // Structure to hold information about the outputs
    struct WaylandMonitorData {
        struct wl_output* output;
        uint32_t global_id;
        uint32_t version;
        std::string name;
        std::string description;
    };

    std::vector<WaylandMonitorData> outputs;

    
    void handle_description(void* data, struct wl_output* output,
                            const char* description)
    {
        WaylandMonitorData* info = static_cast<WaylandMonitorData*>(data);
        info->description = description;
    }
    
    void handle_name(void* data, struct wl_output* output,
                     const char* name)
    {
        WaylandMonitorData* info = static_cast<WaylandMonitorData*>(data);
        info->name = name;
    }
    
    void handle_geometry(void *data, struct wl_output *wl_output,
                         int32_t x, int32_t y, int32_t physical_width,
                         int32_t physical_height, int32_t subpixel,
                         const char *make, const char *model, int32_t transform) {
        // Handle geometry event
    }

    void handle_mode(void *data, struct wl_output *wl_output,
                     uint32_t flags, int32_t width, int32_t height,
                     int32_t refresh) {
        // Handle mode event
    }

    void handle_done(void *data, struct wl_output *wl_output) {
        // Handle done event
    }

    void handle_scale(void *data, struct wl_output *wl_output,
                      int32_t factor) {
        // Handle scale event
    }
    
    const struct wl_output_listener output_listener = {
        .geometry = handle_geometry,
        .mode = handle_mode,
        .done = handle_done,
        .scale = handle_scale,
        .name = handle_name,
        .description = handle_description,
    };

    // Callback function to handle the global objects announced by the registry
    void handle_global(void* data, struct wl_registry* registry, uint32_t name, const char* interface, uint32_t version)
    {
        if (strcmp(interface, wl_output_interface.name) == 0)
        {
            // Bind to the wl_output interface
            struct wl_output* output = static_cast<wl_output*>(wl_registry_bind(registry, name, &wl_output_interface, std::min(version, 4u)));

            WaylandMonitorData outputInfo;
            outputInfo.output = output;
            outputInfo.global_id = name;
            outputInfo.version = version;

            outputs.push_back(outputInfo);
            
            wl_output_add_listener(output, &output_listener, &outputs.back());
        }
    }
    
    void handle_global_remove(void* data, struct wl_registry* registry,
                              uint32_t name)
    {
        // Handle removal of global objects if necessary
        for (auto it = outputs.begin(); it != outputs.end(); ++it)
        {
            if (it->global_id == name) {
                outputs.erase(it);
                break;
            }
        }
    }


    // Registry listener
    struct wl_registry_listener registry_listener = {
        .global = handle_global,
        .global_remove = handle_global_remove,
    };


#endif

#ifdef FLTK_USE_X11    
    std::string getManufacturerName(const char* edidCode)
    {
        if (strcmp(edidCode, "AOC") == 0)
            return "AOC International";
        if (strcmp(edidCode, "ACR") == 0)
            return "Acer Inc.";
        if (strcmp(edidCode, "APP") == 0)
            return "Apple Inc.";
        if (strcmp(edidCode, "ASU") == 0)
            return "ASUSTeK Computer Inc.";
        if (strcmp(edidCode, "BNQ") == 0)
            return "BenQ Corporation";
        if (strcmp(edidCode, "CMN") == 0)
            return "Chimei Innolux Corporation";
        if (strcmp(edidCode, "DEL") == 0)
            return "Dell Inc.";
        if (strcmp(edidCode, "ENC") == 0)
            return "EIZO Nanao Corporation";
        if (strcmp(edidCode, "FUJ") == 0)
            return "Fujitsu Limited";
        if (strcmp(edidCode, "GSM") == 0)
            return "LG Electronics";
        if (strcmp(edidCode, "HWP") == 0)
            return "Hewlett-Packard (HP)";
        if (strcmp(edidCode, "LEN") == 0)
            return "Lenovo Group Ltd.";
        if (strcmp(edidCode, "LPL") == 0)
            return "LG Philips LCD";
        if (strcmp(edidCode, "MSI") == 0)
            return "Micro-Star International";
        if (strcmp(edidCode, "NEC") == 0)
            return "NEC Corporation";
        if (strcmp(edidCode, "PHL") == 0)
            return "Philips";
        if (strcmp(edidCode, "SAM") == 0)
            return "Samsung Electronics";
        if (strcmp(edidCode, "SEC") == 0)
            return "Seiko Epson Corporation";
        if (strcmp(edidCode, "SGI") == 0)
            return "Silicon Graphics, Inc.";
        if (strcmp(edidCode, "SON") == 0)
            return "Sony Corporation";
        if (strcmp(edidCode, "TOS") == 0)
            return "Toshiba Corporation";
        if (strcmp(edidCode, "VIZ") == 0)
            return "Vizio, Inc.";
        if (strcmp(edidCode, "VSC") == 0)
            return "ViewSonic Corporation";
        if (strcmp(edidCode, "YMH") == 0)
            return "Yamaha Corporation";

        // Not a name we know, just return its edidCode
        return std::string(edidCode);
    }

    //! This code taken almost verbatim from parse-edid.
    std::string parse_edid(const unsigned char* const edid)
    {
        int i;
        int j;
        char modelname[13];

        // check header
        for (i = 0; i < 8; i++)
        {
            if (!(((i == 0 || i == 7) && edid[i] == 0x00) ||
                  (edid[i] == 0xff))) // 0x00 0xff 0xff 0xff 0xff 0xff 0x00
                throw std::runtime_error("Invalid EDID header");
        }

        // Product Identification
        /* Model Name: Only thing I do out of order of edid, to comply with X
         * standards... */
        for (i = 0x36; i < 0x7E; i += 0x12)
        { // read through descriptor blocks...
            if (edid[i] == 0x00)
            { // not a timing descriptor
                if (edid[i + 3] == 0xfc)
                { // Model Name tag
                    for (j = 0; j < 13; j++)
                    {
                        if (edid[i + 5 + j] == 0x0a)
                            modelname[j] = 0x00;
                        else
                            modelname[j] = edid[i + 5 + j];
                    }
                }
            }
        }
        std::string modelName = modelname;

        /* Vendor Name: 3 characters, standardized by microsoft, somewhere.
         * bytes 8 and 9: f e d c b a 9 8  7 6 5 4 3 2 1 0
         * Character 1 is e d c b a
         * Character 2 is 9 8 7 6 5
         * Character 3 is 4 3 2 1 0
         * Those values start at 0 (0x00 is 'A', 0x01 is 'B', 0x19 is 'Z', etc.)
         */
        char vendorId[4];
        snprintf(
            vendorId, 4, "%c%c%c", (edid[8] >> 2 & 0x1f) + 'A' - 1,
            (((edid[8] & 0x3) << 3) | ((edid[9] & 0xe0) >> 5)) + 'A' - 1,
            (edid[9] & 0x1f) + 'A' - 1);

        std::string vendorName = getManufacturerName(vendorId);
        return vendorName + " " + modelName;
    }

    std::string getX11MonitorName(Display* display, RROutput output)
    {
        Atom edid_atom = XInternAtom(display, "EDID", True);
        if (edid_atom == None)
        {
            throw std::runtime_error("EDID atom not found");
        }

        Atom actual_type;
        int actual_format;
        unsigned long nitems, bytes_after;
        unsigned char* prop;

        if (XRRGetOutputProperty(
                display, output, edid_atom, 0, 100, False, False,
                AnyPropertyType, &actual_type, &actual_format, &nitems,
                &bytes_after, &prop) != Success)
        {
            throw std::runtime_error("Failed to get EDID property");
        }

        if (nitems < 128 || actual_format != 8)
        {
            XFree(prop);
            throw std::runtime_error("Invalid EDID data");
        }

        std::string monitor_name = parse_edid(prop);

        XFree(prop);
        return monitor_name;
    }
#endif
    
} // namespace

namespace mrv
{
    namespace desktop
    {
        // Get the monitor name given its FLTK screen index
        std::string getMonitorName(int monitorIndex)
        {
            std::string out;
#ifdef FLTK_USE_X11
            if (fl_x11_display())
            {
                Display* display = fl_x11_display();

                Window root = DefaultRootWindow(display);

                // Retrieve monitors in correct order
                int monitor_count = 0;
                XRRMonitorInfo* info =
                    XRRGetMonitors(display, root, 0, &monitor_count);

                // Retrieve the output information
                XRRScreenResources* res = XRRGetScreenResources(display, root);

                std::string connectionType =
                    XGetAtomName(display, info[monitorIndex].name);

                // Get the name of the monitor from XRRGetMonitors
                // Match the monitor from XRRGetMonitors with the
                // corresponding output from XRRGetScreenResources
                for (int j = 0; j < res->noutput; j++)
                {
                    XRROutputInfo* output_info =
                        XRRGetOutputInfo(display, res, res->outputs[j]);

                    if (output_info->connection == RR_Connected)
                    {

                        // Retrieve the CRTC information for this output
                        XRRCrtcInfo* crtc_info =
                            XRRGetCrtcInfo(display, res, output_info->crtc);

                        // Compare the position and size to match the monitor
                        if (info[monitorIndex].x == crtc_info->x &&
                            info[monitorIndex].y == crtc_info->y &&
                            info[monitorIndex].width == crtc_info->width &&
                            info[monitorIndex].height == crtc_info->height)
                        {
                            const std::string& monitorName =
                                getX11MonitorName(display, res->outputs[j]);
                            out = connectionType + ": ";
                            out += monitorName;
                            XRRFreeCrtcInfo(crtc_info);
                            XRRFreeOutputInfo(output_info);
                            break;
                        }

                        XRRFreeCrtcInfo(crtc_info);
                    }

                    XRRFreeOutputInfo(output_info);
                }
            }
#endif
#ifdef FLTK_USE_WAYLAND
            auto display = fl_wl_display();
            if (display)
            {
                outputs.clear();
                
                struct wl_registry* registry = wl_display_get_registry(display);
                
                // Add the registry listener
                wl_registry_add_listener(registry, &registry_listener,
                                         nullptr);

                // Process the registry events to trigger the listener
                wl_display_roundtrip(display);

                if (monitorIndex < 0 || monitorIndex >= outputs.size())
                    throw std::runtime_error("Invalid Monitor index");
                
                // Process the registry events to trigger the listener
                wl_display_roundtrip(display);
                
                const WaylandMonitorData& monitorInfo = outputs[monitorIndex];

                return std::to_string(monitorIndex) + " " +
                    monitorInfo.name + ": " + monitorInfo.description;
            }
#endif
            return out;
        }

    } // namespace desktop
} // namespace mrv
