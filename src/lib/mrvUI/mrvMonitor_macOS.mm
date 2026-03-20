
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
#import <ApplicationServices/ApplicationServices.h> // For ColorSync
#import <IOKit/IOKitLib.h>

#pragma clang diagnostic pop

#import <iostream>
#import <string>
#import <sstream>
#import <cstring>
#import <vector>

// Helper to decode ICC s15Fixed16Number to standard float
static float decode_s15Fixed16(const uint8_t* bytes) {
    // ICC profiles are Big-Endian
    int32_t val = (bytes[0] << 24) | (bytes[1] << 16) | (bytes[2] << 8) | bytes[3];
    return val / 65536.0f;
}

using tl::monitor::Capabilities;


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
        Capabilities getCapabilitiesForDisplay(CGDirectDisplayID displayID) {
            Capabilities caps;
    
            // 2. Supplement/Override with NSScreen headroom if available
            // macOS EDR (Extended Dynamic Range) is a multiplier of SDR (usually 100 nits)
            for (NSScreen* screen in [NSScreen screens]) {
                NSDictionary* deviceDescription = [screen deviceDescription];
                NSNumber* screenNumber = [deviceDescription objectForKey:@"NSScreenNumber"];
        
                if (screenNumber && [screenNumber unsignedIntValue] == displayID) {
                    CGFloat headroom = screen.maximumPotentialExtendedDynamicRangeColorComponentValue;
            
                    // If the multiplier is > 2.0, it's an HDR-capable display
                    if (headroom > 2.0) {
                        caps.hdr_supported = true;
                        caps.hdr_enabled = true;
                        caps.max_nits = headroom * 100.0f; 
                    }
                    break;
                }
            }

            
            // 3. Extract Primaries and White Point via ColorSync
            for (NSScreen* screen in [NSScreen screens]) {
                NSNumber* screenNumber = [[screen deviceDescription] objectForKey:@"NSScreenNumber"];
                if (screenNumber && [screenNumber unsignedIntValue] == displayID) {
            
                    CGColorSpaceRef colorSpace = screen.colorSpace.CGColorSpace;
                    if (!colorSpace) break;
            
                    // Extract the raw ICC profile data
                    CFDataRef iccData = CGColorSpaceCopyICCData(colorSpace);
                    if (iccData) {
                        CFErrorRef err = NULL;
                        ColorSyncProfileRef profile = ColorSyncProfileCreate(iccData, &err);
                
                        if (profile) {
                            // Lambda to read an XYZ tag and convert it to x,y chromaticity
                            auto extractXY = ^(CFStringRef tag, float *outX, float *outY) {
                                CFDataRef tagData = ColorSyncProfileCopyTag(profile, tag);
                                // XYZType tags are exactly 20 bytes long
                                if (tagData && CFDataGetLength(tagData) >= 20) {
                                    const uint8_t* b = CFDataGetBytePtr(tagData);
                            
                                    // Bytes 8-19 contain X, Y, Z
                                    float X = decode_s15Fixed16(b + 8);
                                    float Y = decode_s15Fixed16(b + 12);
                                    float Z = decode_s15Fixed16(b + 16);
                            
                                    float sum = X + Y + Z;
                                    if (sum > 0.0f) {
                                        *outX = X / sum;
                                        *outY = Y / sum;
                                    }
                                    CFRelease(tagData);
                                }
                            };

                            // Extract using standard ICC tag signatures
                            extractXY(CFSTR("rXYZ"), &caps.red.x, &caps.red.y);
                            extractXY(CFSTR("gXYZ"), &caps.green.x, &caps.green.y);
                            extractXY(CFSTR("bXYZ"), &caps.blue.x, &caps.blue.y);
                            extractXY(CFSTR("wtpt"), &caps.white.x, &caps.white.y);

                            CFRelease(profile);
                        }
                        CFRelease(iccData);
                    }
                    break; // Found the screen, stop looping
                }
            }
    
            return caps;
        }

        // Optimized version to return the full struct instead of just a bool
        Capabilities get_hdr_capabilities(int screen_index) {
            uint32_t displayCount;
            CGGetActiveDisplayList(0, NULL, &displayCount);
    
            std::vector<CGDirectDisplayID> displays(displayCount);
            CGGetActiveDisplayList(displayCount, displays.data(), &displayCount);

            // If index is valid, return that specific monitor's caps
            if (screen_index >= 0 && screen_index < (int)displayCount) {
                return getCapabilitiesForDisplay(displays[screen_index]);
            }

            // Default return (not found or invalid index)
            Capabilities empty;
            return empty;
        }

    } // namespace monitor
} // namespace mrv


