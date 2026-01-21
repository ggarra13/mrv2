
#include "mrvUI/mrvMonitor.h"
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#import <Foundation/Foundation.h>
#import <Cocoa/Cocoa.h>
#import <CoreGraphics/CoreGraphics.h>
#import <IOKit/graphics/IOGraphicsLib.h>
#import <IOKit/IOKitLib.h>

#import <iostream>
#import <string>
#import <sstream>
#import <cstring>
#import <vector>

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
        if (CGRectEqualToRect(NSRectToCGRect(screenFrame),
                              cgDisplayBounds))
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

namespace mrv {
    namespace monitor {

        
        HDRCapabilities displaySupportsHDR(CGDirectDisplayID cgDisplayID) {
            HDRCapabilities out;
            io_iterator_t iter;
            io_service_t service = 0;

            CFMutableDictionaryRef match = IOServiceMatching("IODisplayConnect");
            if (IOServiceGetMatchingServices(kIOMasterPortDefault, match, &iter) != KERN_SUCCESS)
                return out;

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

                    if (edid) {
                        const UInt8* bytes = CFDataGetBytePtr(edid);
                        CFIndex length = CFDataGetLength(edid);
                        out = parseEDIDLuminance(bytes, length);
                        CFRelease(edid);
                    }

                    IOObjectRelease(service);
                    IOObjectRelease(iter);
                    return out;
                }

                IOObjectRelease(service);
            }

            IOObjectRelease(iter);
            return out;
        }


        // Helper to get capabilities for a specific ID
        HDRCapabilities getCapabilitiesForDisplay(CGDirectDisplayID displayID) {
            HDRCapabilities caps;
    
            // 1. Try to get hardware info via EDID (via IOKit)
            caps = displaySupportsHDR(displayID);

            // 2. Supplement/Override with NSScreen headroom if available
            // macOS EDR (Extended Dynamic Range) is a multiplier of SDR (usually 100 nits)
            for (NSScreen* screen in [NSScreen screens]) {
                NSDictionary* deviceDescription = [screen deviceDescription];
                NSNumber* screenNumber = [deviceDescription objectForKey:@"NSScreenNumber"];
        
                if (screenNumber && [screenNumber unsignedIntValue] == displayID) {
                    CGFloat headroom = screen.maximumPotentialExtendedDynamicRangeColorComponentValue;
            
                    // If the multiplier is > 1.0, it's an HDR-capable display
                    if (headroom > 1.0) {
                        caps.supported = true;
                        // Only override EDID if EDID failed to provide a max_nits value
                        if (caps.max_nits <= 0.0f) {
                            caps.max_nits = headroom * 100.0f; 
                        }
                    }
                    break;
                }
            }
    
            return caps;
        }

        // Optimized version to return the full struct instead of just a bool
        HDRCapabilities get_hdr_capabilities(int screen_index) {
            uint32_t displayCount;
            CGGetActiveDisplayList(0, NULL, &displayCount);
    
            std::vector<CGDirectDisplayID> displays(displayCount);
            CGGetActiveDisplayList(displayCount, displays.data(), &displayCount);

            // If index is valid, return that specific monitor's caps
            if (screen_index >= 0 && screen_index < (int)displayCount) {
                return getCapabilitiesForDisplay(displays[screen_index]);
            }

            // Default return (not found or invalid index)
            HDRCapabilities empty;
            return empty;
        }

    } // namespace monitor
} // namespace mrv



// bool builtInDisplaySupportsHDR(CGDirectDisplayID cgDisplayID) {
//     NSArray<NSScreen*>* screens = [NSScreen screens];
//     for (NSScreen* screen in screens) {
//         NSDictionary* deviceDescription = [screen deviceDescription];
//         NSNumber* screenNumber = [deviceDescription objectForKey:@"NSScreenNumber"];
//         if (screenNumber && [screenNumber unsignedIntValue] == cgDisplayID) {
//             CGFloat maxEDR = screen.maximumPotentialExtendedDynamicRangeColorComponentValue;
//             if (maxEDR > 1.0) {
//                 return true;
//             }
//         }
//     }
//     return false;
// }








// namespace mrv
// {
//     namespace monitor
//     {
        
//         HDRCapabilities displaySupportsHDR(CGDirectDisplayID cgDisplayID) {
//             HDRCapabilities out;
//             io_iterator_t iter;
//             io_service_t service = 0;

//             CFMutableDictionaryRef match = IOServiceMatching("IODisplayConnect");
//             if (IOServiceGetMatchingServices(kIOMasterPortDefault, match, &iter) != KERN_SUCCESS)
//                 return out;

//             while ((service = IOIteratorNext(iter))) {
//                 CFDictionaryRef info = IODisplayCreateInfoDictionary(service, kIODisplayOnlyPreferredName);
//                 if (!info) {
//                     IOObjectRelease(service);
//                     continue;
//                 }

//                 CFNumberRef vendorRef = (CFNumberRef)CFDictionaryGetValue(info, CFSTR(kDisplayVendorID));
//                 CFNumberRef productRef = (CFNumberRef)CFDictionaryGetValue(info, CFSTR(kDisplayProductID));

//                 uint32_t vendor = 0, product = 0;
//                 if (vendorRef) CFNumberGetValue(vendorRef, kCFNumberIntType, &vendor);
//                 if (productRef) CFNumberGetValue(productRef, kCFNumberIntType, &product);

//                 CFRelease(info);

//                 // Match against CoreGraphics display info
//                 uint32_t cgVendor = CGDisplayVendorNumber(cgDisplayID);
//                 uint32_t cgProduct = CGDisplayModelNumber(cgDisplayID);

//                 if (vendor == cgVendor && product == cgProduct) {
//                     CFDataRef edid = (CFDataRef)IORegistryEntryCreateCFProperty(service, CFSTR("IODisplayEDID"), kCFAllocatorDefault, 0);

//                     if (edid) {
//                         const UInt8* bytes = CFDataGetBytePtr(edid);
//                         CFIndex length = CFDataGetLength(edid);
//                         out = monitor::parseEDIDLuminance(bytes, length);
//                         CFRelease(edid);
//                     }

//                     IOObjectRelease(service);
//                     IOObjectRelease(iter);
//                     return out;
//                 }

//                 IOObjectRelease(service);
//             }

//             IOObjectRelease(iter);
//             return out;
//         }

//         //! \@todo: handle multiple screens on macOS
//         bool is_hdr_active(int screen, const bool silent)
//         {
//             CGDirectDisplayID displayID = CGMainDisplayID();
            
//             if (builtInDisplaySupportsHDR(displayID) ||
//                 displaySupportsHDR(displayID)) {
//                 if (!silent)
//                     std::cout << "This display claims to support HDR (via EDID).\n";
//                 return true;
//             } else {
//                 if (!silent)
//                     std::cout << "This display does not advertise HDR support in EDID.\n";
//                 return false;
//             }
//         }
//     }
// }
