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


// --- Set this to 0 if M1 compiles don't work on Apple Siilicon ---
#define USE_MACOS_PRIVATE_API 1


#if MAC_OS_X_VERSION_MAX_ALLOWED < 120000
#  define kIOMainPortDefault kIOMasterPortDefault
#endif

#import <CoreFoundation/CoreFoundation.h>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#import <Cocoa/Cocoa.h>
#import <IOKit/IOKitLib.h>
#import <CoreGraphics/CoreGraphics.h>
#import <ApplicationServices/ApplicationServices.h> // ColorSync
#pragma clang diagnostic pop

// --- Apple Silicon (ARM64) IOAVService Private API declarations ---
#if defined(__aarch64__)
typedef CFTypeRef IOAVServiceRef;
extern "C" {
    IOAVServiceRef IOAVServiceCreateWithService(CFAllocatorRef allocator, io_service_t service);
    IOReturn IOAVServiceCopyEDID(IOAVServiceRef service, CFDataRef *edidData);
}
#endif

#include <string>
#include <vector>
#include <cmath>
#include <cstdint>
#include <cstdlib>

using tl::monitor::Capabilities;

namespace
{
        // ---- EDID -> declared HDR peak luminance ---------------------------

    // Parses the CTA-861 "HDR Static Metadata" data block (extended tag 6).
    // Desired Content Max Luminance: nits = 50 * 2^(code/32); 0 = undefined.
    bool parseEDIDMaxNits(const uint8_t* e, size_t len, float& maxNits)
    {
        if (!e || len < 128)
            return false;
        const size_t numExt = e[126];
        for (size_t i = 1; i <= numExt; ++i)
        {
            if (len < (i + 1) * 128)
                break;
            const uint8_t* blk = e + 128 * i;
            if (blk[0] != 0x02)               // CTA-861 extension
                continue;
            const size_t dtd = blk[2];         // offset of first DTD
            if (dtd < 4)
                continue;
            const size_t end = dtd < 128 ? dtd : 128;
            size_t pos = 4;
            while (pos < end)
            {
                const uint8_t hdr = blk[pos];
                const int tag = hdr >> 5;
                const size_t n = hdr & 0x1F;
                if (pos + 1 + n > end)
                    break;
                // pos+1 ext tag, +2 EOTFs, +3 descriptors, +4 max luminance
                if (tag == 7 && n >= 4 && blk[pos + 1] == 6)
                {
                    const uint8_t code = blk[pos + 4];
                    if (code == 0)
                        return false;
                    maxNits = float(50.0 * std::pow(2.0, code / 32.0));
                    return true;
                }
                pos += 1 + n;
            }
        }
        return false;
    }

    struct EDIDMatch
    {
        uint32_t vendor;
        uint32_t model;
        uint32_t serial;
    };

    // Walks the IORegistry looking for an EDID blob that matches the display.
    // A recursive scan is used on purpose: CGDisplayIOServicePort() does not
    // exist on Apple Silicon, and the node that carries the EDID differs
    // between Intel (IODisplayConnect / AppleDisplay) and Apple Silicon
    // (AppleCLCD2 / DCP proxies).
    bool edidMaxNitsForDisplay(CGDirectDisplayID id, float& maxNits)
    {
        if (CGDisplayIsBuiltin(id))
            return false;                       // internal panels: no useful EDID

        const EDIDMatch want = {CGDisplayVendorNumber(id),
                                CGDisplayModelNumber(id),
                                CGDisplaySerialNumber(id)};
#if defined(__aarch64__)

#    if USE_MACOS_PRIVATE_API
        // -----------------------------------------------------------------
        // APPLE SILICON PATH: Use IOAVService iterator to locate EDID blobs
        // -----------------------------------------------------------------
        io_iterator_t iterator = 0;
        CFMutableDictionaryRef matching = IOServiceMatching("IOAVService");
        if (IOServiceGetMatchingServices(kIOMainPortDefault, matching, &iterator) != KERN_SUCCESS)
            return false;

        bool found = false;
        float best = 0.f;
        io_service_t service;

        while ((service = IOIteratorNext(iterator)) != IO_OBJECT_NULL)
        {
            IOAVServiceRef avService = IOAVServiceCreateWithService(kCFAllocatorDefault, service);
            IOObjectRelease(service);
            if (!avService)
                continue;

            CFDataRef edidData = nullptr;
            if (IOAVServiceCopyEDID(avService, &edidData) == kIOReturnSuccess && edidData)
            {
                const uint8_t* e = CFDataGetBytePtr(edidData);
                const size_t len = (size_t)CFDataGetLength(edidData);
                if (len >= 128)
                {
                    const uint32_t vendor = (uint32_t(e[8]) << 8) | e[9];
                    const uint32_t model  = e[10] | (uint32_t(e[11]) << 8);
                    const uint32_t serial = e[12] | (uint32_t(e[13]) << 8) |
                                            (uint32_t(e[14]) << 16) |
                                            (uint32_t(e[15]) << 24);
                    float nits;
                    if (vendor == want.vendor && model == want.model &&
                        parseEDIDMaxNits(e, len, nits))
                    {
                        best = nits;
                        found = true;
                        CFRelease(edidData);
                        CFRelease(avService);
                        break; // Stop at the first matching external display
                    }
                }
                CFRelease(edidData);
            }
            CFRelease(avService);
        }
        IOObjectRelease(iterator);

        if (found)
            maxNits = best;
        return found;
#else
        return false;
#endif

#else
        // -----------------------------------------------------------------
        // INTEL PATH: Recursive IORegistry traversal for IODisplayEDID
        // -----------------------------------------------------------------
        io_iterator_t it = 0;
        if (IORegistryCreateIterator(kIOMainPortDefault, kIOServicePlane,
                                     kIORegistryIterateRecursively,
                                     &it) != KERN_SUCCESS)
            return false;

        static CFStringRef const keys[] = {CFSTR("IODisplayEDID"),
                                           CFSTR("IODisplayEDIDOriginal"),
                                           CFSTR("EDID")};
        bool found = false, haveSerialMatch = false;
        float best = 0.f;

        while (io_registry_entry_t entry = IOIteratorNext(it))
        {
            for (CFStringRef key : keys)
            {
                CFTypeRef p = IORegistryEntryCreateCFProperty(
                    entry, key, kCFAllocatorDefault, 0);
                if (!p)
                    continue;
                if (CFGetTypeID(p) == CFDataGetTypeID())
                {
                    CFDataRef d = (CFDataRef)p;
                    const uint8_t* e = CFDataGetBytePtr(d);
                    const size_t len = (size_t)CFDataGetLength(d);
                    if (len >= 128)
                    {
                        const uint32_t vendor = (uint32_t(e[8]) << 8) | e[9];
                        const uint32_t model  = e[10] | (uint32_t(e[11]) << 8);
                        const uint32_t serial = e[12] | (uint32_t(e[13]) << 8) |
                                                (uint32_t(e[14]) << 16) |
                                                (uint32_t(e[15]) << 24);
                        float nits;
                        if (vendor == want.vendor && model == want.model &&
                            parseEDIDMaxNits(e, len, nits))
                        {
                            const bool serialOk =
                                want.serial && serial == want.serial;
                            if (!found || (serialOk && !haveSerialMatch))
                            {
                                best = nits;
                                found = true;
                                haveSerialMatch = serialOk;
                            }
                        }
                    }
                }
                CFRelease(p);
            }
            IOObjectRelease(entry);
            if (haveSerialMatch)
                break;
        }
        IOObjectRelease(it);

        if (found)
            maxNits = best;
        return found;
#endif
    }
    
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

        // macOS only exposes EDR headroom (peak / current SDR white), which
        // moves with the brightness setting.  The headroom decides whether
        // macOS will actually deliver EDR; the *declared* peak comes from the
        // monitor's EDID, as on Windows/Linux.  The headroom estimate below
        // is only a fallback.
        static const CGFloat kMinEDRHeadroom = 2.0;
        static const double  kAssumedSDRWhiteNits = 100.0; // fallback only
        const CGFloat headroom =
            screen.maximumPotentialExtendedDynamicRangeColorComponentValue;

        NSNumber* dn = [[screen deviceDescription] objectForKey:@"NSScreenNumber"];
        const CGDirectDisplayID dispID = dn ? (CGDirectDisplayID)[dn unsignedIntValue] : 0;

        float edidNits = 0.f;
        const bool haveEDID = dispID && edidMaxNitsForDisplay(dispID, edidNits);

        if (getenv("MRV2_DEBUG_EDR"))
            NSLog(@"EDR headroom: potential=%f current=%f edid_max_nits=%@",
                  (double)headroom,
                  (double)screen.maximumExtendedDynamicRangeColorComponentValue,
                  haveEDID ? [NSString stringWithFormat:@"%.0f", edidNits]
                           : @"n/a");

        if (headroom > kMinEDRHeadroom)
        {
            caps.hdr_supported = true;
            caps.hdr_enabled   = true;
            caps.max_nits      = haveEDID
                                     ? edidNits
                                     : float(headroom * kAssumedSDRWhiteNits);
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
