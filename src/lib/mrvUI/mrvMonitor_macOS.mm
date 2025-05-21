#include <iostream>
#include <sstream>
#include <string>

#import <Foundation/Foundation.h>
#import <IOKit/graphics/IOGraphicsLib.h>
#import <IOKit/IOKitLib.h>
#import <Cocoa/Cocoa.h>
#import <CoreGraphics/CoreGraphics.h>
#import <iostream>
#import <vector>
#import <string>
#import <cstring>
#import <iostream>


bool getDisplayNameForDispID(CGDirectDisplayID dispID,
                             std::string& out)
{
    bool bRes = false;
    
    out.clear();
    
    NSArray *screens = [NSScreen screens];

    for (NSScreen *screen in screens)
    {
        NSRect screenFrame = [screen frame];
        CGRect cgDisplayBounds = CGDisplayBounds(dispID);

        // We match by bound rectangle as matching by display id did not work.
        if (CGRectEqualToRect(NSRectToCGRect(screenFrame), cgDisplayBounds))
        {
            //Got it
            NSString* pName = [screen localizedName];
                    
            out.assign([pName UTF8String], [pName lengthOfBytesUsingEncoding:NSUTF8StringEncoding]);
                    
            bRes = true;
                    
            break;
        }
    }    

    return bRes;
}





// Parse EDID to detect HDR static metadata support
bool parseEDIDForHDR(const uint8_t* edid, size_t length) {
    if (length < 128) return false;

    uint8_t numExtensions = edid[126];
    const uint8_t* ext = edid + 128;

    for (int i = 0; i < numExtensions && (ext + 128 <= edid + length); ++i) {
        if (ext[0] == 0x02 && ext[1] == 0x03) {  // CTA-861 Extension Block
            uint8_t dtdStart = ext[2];
            if (dtdStart == 0 || dtdStart > 127) dtdStart = 127;

            for (int j = 4; j < dtdStart - 4;) {
                uint8_t tag = (ext[j] & 0xE0) >> 5;
                uint8_t len = ext[j] & 0x1F;
                if (tag == 0x07 && ext[j + 1] == 0x06) {
                    std::cout << "→ HDR static metadata block found in EDID\n";
                    return true;
                }
                j += len + 1;
            }
        }
        ext += 128;
    }

    return false;
}

bool displaySupportsHDR(CGDirectDisplayID cgDisplayID) {
    io_iterator_t iter;
    io_service_t service = 0;

    CFMutableDictionaryRef match = IOServiceMatching("IODisplayConnect");
    if (IOServiceGetMatchingServices(kIOMasterPortDefault, match, &iter) != KERN_SUCCESS)
        return false;

    while ((service = IOIteratorNext(iter))) {
        CFDictionaryRef info = IODisplayCreateInfoDictionary(service, kIODisplayOnlyPreferredName);
        if (!info) {
            IOObjectRelease(service);
            continue;
        }

        CFNumberRef vendorRef = (CFNumberRef)CFDictionaryGetValue(info, CFSTR(kDisplayVendorID));
        CFNumberRef productRef = (CFNumberRef)CFDictionaryGetValue(info, CFSTR(kDisplayProductID));

        uint32_t vendor = 0, product = 0;
        if (vendorRef) CFNumberGetValue(vendorRef, kCFNumberIntType, &vendor);
        if (productRef) CFNumberGetValue(productRef, kCFNumberIntType, &product);

        CFRelease(info);

        // Match against CoreGraphics display info
        uint32_t cgVendor = CGDisplayVendorNumber(cgDisplayID);
        uint32_t cgProduct = CGDisplayModelNumber(cgDisplayID);

        if (vendor == cgVendor && product == cgProduct) {
            CFDataRef edid = (CFDataRef)IORegistryEntryCreateCFProperty(service, CFSTR("IODisplayEDID"), kCFAllocatorDefault, 0);
            bool result = false;

            if (edid) {
                const UInt8* bytes = CFDataGetBytePtr(edid);
                CFIndex length = CFDataGetLength(edid);
                result = parseEDIDForHDR(bytes, length);
                CFRelease(edid);
            }

            IOObjectRelease(service);
            IOObjectRelease(iter);
            return result;
        }

        IOObjectRelease(service);
    }

    IOObjectRelease(iter);
    return false;
}








namespace mrv
{
    namespace monitor
    {
        //! \@todo: handle multiple screens on macOS
        bool is_hdr_active(int screen, const bool silent)
        {
            CGDirectDisplayID displayID = CGMainDisplayID();
            if (displaySupportsHDR(displayID)) {
                if (!silent)
                    std::cout << "This display claims to support HDR (via EDID).\n";
                return true;
            } else {
                if (!silent)
                    std::cout << "This display does not advertise HDR support in EDID.\n";
                return false;
            }
        }
    }
}
