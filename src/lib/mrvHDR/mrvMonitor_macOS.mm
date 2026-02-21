
#include "mrvUI/mrvMonitor.h"
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"

#import <Foundation/Foundation.h>
#import <Cocoa/Cocoa.h>
#import <CoreGraphics/CoreGraphics.h>
#import <IOKit/IOKitLib.h>

#pragma clang diagnostic pop

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



        // Helper to get capabilities for a specific ID
        HDRCapabilities getCapabilitiesForDisplay(CGDirectDisplayID displayID) {
            HDRCapabilities caps;

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


