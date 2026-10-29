// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include "mrvFl/mrvInit.h"

#include <FL/platform.H>
#import <Cocoa/Cocoa.h>

namespace mrv
{
    void osx_open_cb(const char*);

    void init_macos(const std::shared_ptr<tl::system::Context>& context)
    {
        // FLTK must create NSApp and process launch events with its document
        // callback installed. Creating NSApp or calling finishLaunching here
        // skips FLTK's launch loop and lets GLFW consume the initial document
        // event with a delegate that cannot open files.
        fl_open_callback(osx_open_cb);
        id fltkDelegate = [[NSApp delegate] retain];
        mrv::init(context);
        // GLFW creates helper windows; FLTK handles application documents.
        [NSApp setDelegate:fltkDelegate];
        [fltkDelegate release];
    }
}
