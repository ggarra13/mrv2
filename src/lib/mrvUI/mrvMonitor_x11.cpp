// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.


#include <cstring>

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include <FL/platform.H>

#include <X11/Xlib.h>
#include <X11/extensions/Xrandr.h>

#include "mrvUI/mrvDesktop.h"

namespace
{

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
        auto vendorId = mrv::monitor::decodeEdidManufacturerId(edid + 8);
        const std::string& vendorName =
            mrv::monitor::getManufacturerName(vendorId.c_str());
        return vendorName + " " + modelName;
    }

    std::string X11MonitorName(Display* display, RROutput output)
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
            if (mrv::desktop::XWayland())
            {
                return "";
            }
            throw std::runtime_error("Invalid EDID data");
        }

        std::string monitor_name = parse_edid(prop);

        XFree(prop);
        return monitor_name;
    }
    
} // namespace

namespace mrv
{
    namespace monitor
    {
        // Get the monitor name given its FLTK screen index
        std::string getX11Name(int monitorIndex)
        {
            std::string out;
            
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

                if (output_info->connection == RR_Connected &&
                    output_info->crtc != 0)
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
                            X11MonitorName(display, res->outputs[j]);
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
            return out;
        }

    } // namespace monitor
} // namespace mrv
