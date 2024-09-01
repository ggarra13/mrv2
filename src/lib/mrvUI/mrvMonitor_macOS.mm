#include <iostream>
#include <sstream>
#include <string>

#import <Foundation/Foundation.h>
#import <CoreGraphics/CoreGraphics.h>
#import <IOKit/graphics/IOGraphicsLib.h>
#import <Cocoa/Cocoa.h>

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
