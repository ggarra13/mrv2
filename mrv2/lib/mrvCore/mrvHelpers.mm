
#include <cstdlib>
#include <iostream>

// Objective-C header for NSWorkspace
#import <Cocoa/Cocoa.h>


#include "mrvCore/mrvHelpers.h"
#include "mrvCore/mrvI8N.h"

#include "mrvFl/mrvIO.h"

namespace
{
    const char* kModule = "helpers";
}

namespace mrv
{
    namespace file_manager
    {

        int show_uri(const std::string& file)
        {
            int ret = -1;
        
            const char* path = file.c_str();

            @autoreleasepool {
                // Convert the C-string path to NSString
                NSString *filePath = [NSString stringWithCString:path encoding:NSUTF8StringEncoding];

                // Prepare the AppleScript command to reveal the file or directory in Finder
                NSString *scriptCommand = [NSString stringWithFormat:@"tell application \"Finder\" to reveal POSIX file \"%@\"", filePath];

                // Create an NSTask to run the AppleScript
                NSTask *task = [[NSTask alloc] init];
                [task setLaunchPath:@"/usr/bin/osascript"];
                [task setArguments:@[@"-e", scriptCommand]];
                [task launch];
                [task waitUntilExit];

                int status = [task terminationStatus];
                if (status != 0) {
                    std::cerr << "Error: Failed to open file or directory." << std::endl;
                    return 1;
                }
            }

            return 0;
        }
	
    }  // namespace file_manage

} // namespace mrv
