// SPDX-License-Identifier: BSD-3-Clause
// mrv2 
// Copyright Contributors to the mrv2 Project. All rights reserved.



#include "mrvCore/mrvHotkey.h"

#include "mrvMainWindow.h"

#include "mrvPreferencesUI.h"

#include <FL/platform.H>
#include <FL/fl_utf8.h>
#include <FL/Enumerations.H>
#include <FL/Fl.H>
#include <FL/Fl_XPM_Image.H>


#if defined(FLTK_USE_X11)
#include <X11/extensions/scrnsaver.h>
#endif

#include "icons/viewer16.xpm"

#include "mrvFl/mrvIO.h"


namespace {
    const char* kModule = "main";
}

namespace mrv
{

    MainWindow::MainWindow( int X, int Y,int W, int H, const char* title ) :
    DropWindow( X, Y, W, H, title )
    {
        init();
    }

    MainWindow::MainWindow( int W, int H, const char* title ) :
        DropWindow( W, H, title )
    {
        init();
    }

    MainWindow::~MainWindow()
    {
        // Restore screensaver/black screen
#if defined(FLTK_USE_X11)
        if ( fl_x11_display() )
            XScreenSaverSuspend( fl_display, False );
#elif defined(_WIN32)
        SetThreadExecutionState(ES_CONTINUOUS);
#elif defined(__APPLE__)
        if ( success )
        {
            success = IOPMAssertionRelease( assertionID );
        }
#endif
    }



    void MainWindow::init()
    {
        xclass("mrViewer");
        set_icon();

        const char* r = fl_getenv( "MRV_ROOT" );
        if ( r )
        {
            Preferences::root = r;
        }

        if ( Preferences::root.empty() )
        {
            throw std::runtime_error("Environment variable MRV_ROOT not set.  "
                                     "Aborting");
        }

    }


    void MainWindow::set_icon()
    {
        fl_open_display();  // Needed for icons

        // Turn off screensaver and black screen
#if defined(FLTK_USE_X11)
        if ( fl_x11_display() )
        {
            int event_base, error_base;
            Bool ok = XScreenSaverQueryExtension(fl_display, &event_base, &error_base );
            if ( ok == True )
                XScreenSaverSuspend( fl_display, True );
        }
#elif defined(_WIN32)
        SetThreadExecutionState( ES_CONTINUOUS | ES_SYSTEM_REQUIRED |
                                 ES_DISPLAY_REQUIRED );
#elif defined(__APPLE__)
        CFStringRef reason = CFSTR( "mrViewer playback" );
        success = IOPMAssertionCreateWithName( kIOPMAssertionTypeNoDisplaySleep,
                                               kIOPMAssertionLevelOn,
                                               reason, &assertionID );
#endif

#if defined(_WIN32)
        if ( fl_win32_display() )
        {
            Fl_Pixmap* pic = new Fl_Pixmap( viewer16_xpm );
            Fl_RGB_Image* rgb = new Fl_RGB_Image( pic );
            delete pic;
            icon( rgb );
            delete rgb;
        }
#elif defined(FLTK_USE_X11)
        if ( fl_x11_display() )
        {
            Fl_Pixmap* pic = new Fl_Pixmap( viewer16_xpm );
            Fl_RGB_Image* rgb = new Fl_RGB_Image( pic );
            delete pic;
            icon( rgb );
            delete rgb;
        }
#endif

    }


#if !defined(__APPLE__)

    void MainWindow::always_on_top( int t )
    {
        on_top = t;
        
#if defined(_WIN32)
        HWND action;
        if ( t ) action = HWND_TOPMOST;
        else     action = HWND_NOTOPMOST;
        // Microsoft (R) Windows(TM)
        SetWindowPos(fl_xid(this), action,
                     NULL, NULL, NULL, NULL, SWP_NOMOVE | SWP_NOSIZE );
#elif defined(FLTK_USE_X11)
        if ( fl_x11_display() )
        {
            // XOrg / XWindows(TM)
            XEvent ev;
            static const char* const names[2] = { "_NET_WM_STATE",
                "_NET_WM_STATE_ABOVE"
            };
            Atom atoms[ 2 ];
            fl_open_display();
            XInternAtoms(fl_display, (char**)names, 2, False, atoms );
            Atom net_wm_state = atoms[ 0 ];
            Atom net_wm_state_above = atoms[ 1 ];
            ev.type = ClientMessage;
            ev.xclient.window = fl_xid(this);
            ev.xclient.message_type = net_wm_state;
            ev.xclient.format = 32;
            ev.xclient.data.l[ 0 ] = t;
            ev.xclient.data.l[ 1 ] = net_wm_state_above;
            ev.xclient.data.l[ 2 ] = 0;
            XSendEvent(fl_display,
                       DefaultRootWindow(fl_display),  False,
                       SubstructureNotifyMask|SubstructureRedirectMask, &ev);
        }
#endif
    } // above_all function


#endif


    void MainWindow::iconize_all()
    {
        return Fl_Double_Window::iconize();
    }


} // namespace mrv
