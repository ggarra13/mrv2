// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.
//
// NOTE: not compiled here (no macOS toolchain available when this was
// written); please build and test on a Mac, with an internal XDR/HDR display,
// an external SDR display and (ideally) an external HDR display.
//
// All AppKit calls must run on the main thread (FLTK's UI thread is).

#include "mrvUI/mrvMonitor.h"
#include "mrvMonitor_macOS.h"

#include <string>
#include <vector>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#import <Cocoa/Cocoa.h>
#import <CoreGraphics/CoreGraphics.h>
#import <ApplicationServices/ApplicationServices.h> // ColorSync
#pragma clang diagnostic pop

using tl::monitor::Capabilities;

namespace
{
    // ICC s15Fixed16Number, big-endian.
    double decode_s15Fixed16(const uint8_t* b)
    {
        const int32_t v = (int32_t)((uint32_t(b[0]) << 24) | (uint32_t(b[1]) << 16) |
                                    (uint32_t(b[2]) << 8) | uint32_t(b[3]));
        return v / 65536.0;
    }

    NSScreen* screenForDisplayID(CGDirectDisplayID id)
    {
        for (NSScreen* s in [NSScreen screens])
        {
            NSNumber* n = [[s deviceDescription] objectForKey:@"NSScreenNumber"];
            if (n && [n unsignedIntValue] == id)
                return s;
        }
        return nil;
    }

    // Reads an 'XYZ ' tag (20 bytes: sig, reserved, X, Y, Z).
    bool readXYZ(ColorSyncProfileRef profile, CFStringRef tag, double xyz[3])
    {
        CFDataRef d = ColorSyncProfileCopyTag(profile, tag);
        if (!d)
            return false;
        bool ok = false;
        if (CFDataGetLength(d) >= 20)
        {
            const uint8_t* b = CFDataGetBytePtr(d);
            xyz[0] = decode_s15Fixed16(b + 8);
            xyz[1] = decode_s15Fixed16(b + 12);
            xyz[2] = decode_s15Fixed16(b + 16);
            ok = true;
        }
        CFRelease(d);        // released on every path (the old code leaked here)
        return ok;
    }

    // Reads the 'chad' tag: 3x3 chromatic adaptation matrix (sf32, row-major).
    bool readChad(ColorSyncProfileRef profile, double m[9])
    {
        CFDataRef d = ColorSyncProfileCopyTag(profile, CFSTR("chad"));
        if (!d)
            return false;
        bool ok = false;
        if (CFDataGetLength(d) >= 8 + 9 * 4)
        {
            const uint8_t* b = CFDataGetBytePtr(d) + 8;
            for (int i = 0; i < 9; ++i)
                m[i] = decode_s15Fixed16(b + 4 * i);
            ok = true;
        }
        CFRelease(d);
        return ok;
    }

    bool invert3x3(const double m[9], double r[9])
    {
        const double a = m[0], b = m[1], c = m[2];
        const double d = m[3], e = m[4], f = m[5];
        const double g = m[6], h = m[7], i = m[8];
        const double A = e * i - f * h, B = -(d * i - f * g), C = d * h - e * g;
        const double det = a * A + b * B + c * C;
        if (det > -1e-12 && det < 1e-12)
            return false;
        const double s = 1.0 / det;
        r[0] = A * s;                  r[1] = -(b * i - c * h) * s; r[2] = (b * f - c * e) * s;
        r[3] = B * s;                  r[4] = (a * i - c * g) * s;  r[5] = -(a * f - c * d) * s;
        r[6] = C * s;                  r[7] = -(a * h - b * g) * s; r[8] = (a * e - b * d) * s;
        return true;
    }

    // ICC stores primaries and white point adapted to the D50 PCS, so
    // x,y computed straight from rXYZ/gXYZ/bXYZ/wtpt report a D50 white for
    // *every* display.  Undo the adaptation with inverse('chad') to get the
    // display's native chromaticities (sRGB -> red ~(0.64, 0.33), white D65).
    void fillChromaticity(ColorSyncProfileRef profile, Capabilities& caps)
    {
        double chad[9], inv[9];
        const bool undo = readChad(profile, chad) && invert3x3(chad, inv);

        auto xy = [&](CFStringRef tag, float* ox, float* oy)
        {
            double v[3];
            if (!readXYZ(profile, tag, v))
                return;                       // e.g. LUT-based profile: keep defaults
            if (undo)
            {
                const double X = inv[0] * v[0] + inv[1] * v[1] + inv[2] * v[2];
                const double Y = inv[3] * v[0] + inv[4] * v[1] + inv[5] * v[2];
                const double Z = inv[6] * v[0] + inv[7] * v[1] + inv[8] * v[2];
                v[0] = X; v[1] = Y; v[2] = Z;
            }
            const double sum = v[0] + v[1] + v[2];
            if (sum > 0.0)
            {
                *ox = float(v[0] / sum);
                *oy = float(v[1] / sum);
            }
        };

        xy(CFSTR("rXYZ"), &caps.red.x,   &caps.red.y);
        xy(CFSTR("gXYZ"), &caps.green.x, &caps.green.y);
        xy(CFSTR("bXYZ"), &caps.blue.x,  &caps.blue.y);
        xy(CFSTR("wtpt"), &caps.white.x, &caps.white.y);
    }

    Capabilities capsForScreen(NSScreen* screen)
    {
        Capabilities caps;
        if (!screen)
            return caps;

        // "Potential" EDR headroom is a multiple of SDR white.  Plain SDR
        // displays report 1.0, but some non-HDR Mac displays (e.g. Intel
        // MacBook Pros) report a small value above 1, so "> 1.0" wrongly
        // marks them as HDR.  Require clearly more than 2x; real HDR/XDR
        // displays report well above that.  Tune kMinEDRHeadroom if needed.
        // The headroom is relative to SDR white, so max_nits is approximate.
        // (maximumExtendedDynamicRangeColorComponentValue is the *current*
        // headroom and stays 1.0 until something requests EDR, so it is not
        // usable for selection.)
        static const CGFloat kMinEDRHeadroom = 2.0;
        const CGFloat headroom =
            screen.maximumPotentialExtendedDynamicRangeColorComponentValue;
        if (getenv("MRV2_DEBUG_EDR"))
            NSLog(@"EDR headroom: potential=%f current=%f",
                  (double)headroom,
                  (double)screen.maximumExtendedDynamicRangeColorComponentValue);
        if (headroom > kMinEDRHeadroom)
        {
            caps.hdr_supported = true;
            caps.hdr_enabled   = true;
            caps.max_nits      = float(headroom * 100.0);
        }

        NSColorSpace* ns = screen.colorSpace;
        CGColorSpaceRef cs = ns ? ns.CGColorSpace : nullptr;
        if (cs)
        {
            CFDataRef icc = CGColorSpaceCopyICCData(cs);
            if (icc)
            {
                CFErrorRef err = nullptr;
                ColorSyncProfileRef profile = ColorSyncProfileCreate(icc, &err);
                if (profile)
                {
                    fillChromaticity(profile, caps);
                    CFRelease(profile);
                }
                if (err)
                    CFRelease(err);
                CFRelease(icc);
            }
        }
        return caps;
    }
} // anonymous namespace


// Name of the monitor, by CGDirectDisplayID.
bool getDisplayNameForDispID(CGDirectDisplayID dispID, std::string& out)
{
    out.clear();
    // Match on NSScreenNumber.  Comparing NSScreen.frame with CGDisplayBounds
    // fails for any display not at the origin: AppKit's y axis points up from
    // the main display's bottom-left, CoreGraphics' points down from its
    // top-left.
    NSScreen* screen = screenForDisplayID(dispID);
    if (!screen)
        return false;
    NSString* name = [screen localizedName];
    out = [name UTF8String];
    return true;
}

namespace mrv
{
    namespace monitor
    {
        // By index in [NSScreen screens] (FLTK's Cocoa screen list is built
        // from that array; CGGetActiveDisplayList does not guarantee the same
        // order).  Prefer the window-based call below.
        Capabilities get_hdr_capabilities(int screen_index)
        {
            NSArray<NSScreen*>* screens = [NSScreen screens];
            if (screen_index >= 0 && screen_index < (int)[screens count])
                return capsForScreen([screens objectAtIndex:screen_index]);
            return Capabilities();
        }

        // Most robust: the screen the window is actually on (the one holding
        // most of it).  Pass fl_xid(window) / glfwGetCocoaWindow().
        // Declare it in mrvMonitor.h next to get_hdr_capabilities().
        Capabilities get_hdr_capabilities_for_window(void* nsWindow)
        {
            NSWindow* w = (__bridge NSWindow*)nsWindow;
            NSScreen* s = w ? w.screen : nil;
            return capsForScreen(s ? s : [NSScreen mainScreen]);
        }
    } // namespace monitor
} // namespace mrv
