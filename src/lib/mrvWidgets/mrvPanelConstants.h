// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

// On macOS, the buttons go to the left of the window.
#ifdef __APPLE__
#    define LEFT_BUTTONS 1
#endif


// Constants used in the Window/Panels
namespace
{
    // Constant used to avoid Wayland bug.
    const int kMaxMove = 640;

    // Margins of Panel Windows
    const int kMargin = 3;

    // Width of Panel Buttons
    const int kButtonW = 20;

    // Window's title bar size
    const int kTitleBar = 20;

    // Windows' minimum sizes
    const int kMinWidth = 200;
    const int kMinHeight = 121;
}
