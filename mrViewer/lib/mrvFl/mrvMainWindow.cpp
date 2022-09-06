/*
    mrViewer - the professional movie and flipbook playback
    Copyright (C) 2007-2022  Gonzalo Garramuño

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
/**
 * @file   mrvMainWindow.cpp
 * @author gga
 * @date   Wed Jul  4 23:17:46 2007
 *
 * @brief  main window for mrViewer
 *
 *
 */

#ifdef USE_R3DSDK
#  include "R3DSDK.h"


using namespace R3DSDK;
#endif

#include "mrvCore/mrvI8N.h"
#undef snprintf
#include <iostream>


// #include "mrViewer.h"
#include "mrvFlGL/mrvGLViewport.h"
#include "mrvFl/mrvMainWindow.h"

#include <FL/Enumerations.H>
#include <FL/Fl.H>
#include <FL/Fl_XPM_Image.H>

// Must come before FL/Fl_x11.H
#include <FL/platform.H>
#include <FL/fl_utf8.h>

#if defined(FLTK_USE_X11)
#include <X11/extensions/scrnsaver.h>
#endif

#include "icons/viewer16.xpm"
#include "resource.h"

#include "mrvPreferencesUI.h"

using namespace std;

namespace {
    const char* kModule = "main";
}

namespace mrv {


MainWindow::MainWindow( int W, int H, const char* title ) :
Fl_Double_Window( W, H, title )
{
    xclass("mrViewer");
    set_icon();

    const char* r = fl_getenv( "MRV_ROOT" );
    if ( r )
    {
        Preferences::root = r;
    }

    // if ( Preferences::root.empty() )
    // {
    //     throw std::runtime_error("Environment variable MRV_ROOT not set.  Aborting");
    // }

    Fl::use_high_res_GL(true);
}

MainWindow::~MainWindow()
{
    // Restore screensaver/black screen
#if defined(FLTK_USE_X11)
    XScreenSaverSuspend( fl_display, False );
#elif defined(_WIN32)
    SetThreadExecutionState(ES_CONTINUOUS);
#elif defined(__APPLE__)
    if ( success )
    {
        success = IOPMAssertionRelease( assertionID );
    }
#endif

    // Close the R3D SDK
#ifdef USE_R3DSDK
    R3DSDK::FinalizeSdk();
#endif
}


void MainWindow::set_icon()
{
    fl_open_display();  // Needed for icons

    // Turn off screensaver and black screen
#if defined(FLTK_USE_X11)
    int event_base, error_base;
    Bool ok = XScreenSaverQueryExtension(fl_display, &event_base, &error_base );
    if ( ok == True )
        XScreenSaverSuspend( fl_display, True );
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
     HICON data = LoadIcon(fl_display, MAKEINTRESOURCE(IDI_ICON1));
     this->icon(data);
#else
    Fl_Pixmap* pic = new Fl_Pixmap( viewer16_xpm );
    Fl_RGB_Image* rgb = new Fl_RGB_Image( pic );
    delete pic;
    icon( rgb );
    delete rgb;
#endif

}


#if !defined(__APPLE__)

void MainWindow::always_on_top( int t )
{
#if defined(_WIN32)
    HWND action;
    if ( t ) action = HWND_TOPMOST;
    else     action = HWND_NOTOPMOST;
    // Microsoft (R) Windows(TM)
    SetWindowPos(fl_xid(this), action,
                 NULL, NULL, NULL, NULL, SWP_NOMOVE | SWP_NOSIZE );
#elif defined(FLTK_USE_X11)
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
#endif
} // above_all function


#endif


/**
 * Iconize all windows
 *
 */
void MainWindow::iconize_all()
{
    // @todo:
    // Fl_Window* uiReelWindow = uiMain->uiReelWindow->uiMain;
    // if (uiReelWindow) uiReelWindow->iconize();
    return Fl_Double_Window::iconize();
}

/**
 * Handle MainWindow's Fl::events
 *
 * @param event Fl::event enumeration
 *
 * @return 1 if handled, 0 if not.
 */
int MainWindow::handle( int event )
{
    return Fl_Double_Window::handle( event );
}


void MainWindow::fullscreen()
{
    Fl_Double_Window::fullscreen();
}

void MainWindow::resize( int X, int Y, int W, int H )
{
    Fl_Double_Window::resize( X, Y, W, H );
}



} // namespace mrv
