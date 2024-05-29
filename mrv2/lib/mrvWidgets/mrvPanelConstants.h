#pragma once

// Defines

//! Set this to under X11 or XWayland create the Panel Windows as children
//! of the Drop Window (top_window()).
//! Under Wayland, they are always created as parent of the Drop Window.
//
//#define PARENT_TO_TOP_WINDOW 1

// On macOS, the buttons go to the left of the window.
#ifdef __APPLE__
#    define LEFT_BUTTONS 1
#endif


// Constants used in the Window/Panels
namespace
{
    const int kMargin = 3;
    const int kButtonW = 20;
    const int kTitleBar = 20;
}
