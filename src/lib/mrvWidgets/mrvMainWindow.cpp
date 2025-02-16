// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include "mrvCore/mrvHotkey.h"

#include "mrvWidgets/mrvPanelWindow.h"

#include "mrvFl/mrvSession.h"

#include "mrvUI/mrvDesktop.h"

#include "mrvMainWindow.h"
#include "mrvPreferencesUI.h"

#include <FL/platform.H>
#include <FL/fl_utf8.h>
#include <FL/Enumerations.H>
#include <FL/Fl.H>

#if defined(FLTK_USE_X11)
#    include <X11/extensions/scrnsaver.h>
#endif

#include "mrViewer.h"

#include "mrvFl/mrvIO.h"

namespace
{
    const char* kModule = "main";
}

namespace mrv
{

    MainWindow::MainWindow(int X, int Y, int W, int H, const char* title) :
        DropWindow(X, Y, W, H, title)
    {
        box(FL_FLAT_BOX);
        init();
    }

    MainWindow::MainWindow(int W, int H, const char* title) :
        DropWindow(W, H, title)
    {
        box(FL_FLAT_BOX);
        init();
    }

    MainWindow::~MainWindow()
    {
        // Restore screensaver/black screen
#if defined(FLTK_USE_X11)
        if (fl_x11_display())
            XScreenSaverSuspend(fl_x11_display(), False);
#elif defined(_WIN32)
        SetThreadExecutionState(ES_CONTINUOUS);
#elif defined(__APPLE__)
        if (success)
        {
            success = IOPMAssertionRelease(assertionID);
        }
#endif
    }

    void MainWindow::init()
    {
        xclass("mrv2");
        set_icon();

        const char* r = fl_getenv("MRV2_ROOT");
        if (r)
        {
            Preferences::root = r;
        }

        if (Preferences::root.empty())
        {
            throw std::runtime_error("Environment variable MRV2_ROOT not set.  "
                                     "Aborting");
        }
    }

    void MainWindow::set_icon()
    {
        fl_open_display(); // Needed for icons

        // Turn off screensaver and black screen
#if defined(FLTK_USE_X11)
        if (fl_x11_display())
        {
            int event_base, error_base;
            Bool ok = XScreenSaverQueryExtension(
                fl_x11_display(), &event_base, &error_base);
            if (ok == True)
                XScreenSaverSuspend(fl_x11_display(), True);
        }
#elif defined(_WIN32)
        SetThreadExecutionState(
            ES_CONTINUOUS | ES_SYSTEM_REQUIRED | ES_AWAYMODE_REQUIRED);
#elif defined(__APPLE__)
        CFStringRef reason = CFSTR("mrv2 playback");
        success = IOPMAssertionCreateWithName(
            kIOPMAssertionTypeNoDisplaySleep, kIOPMAssertionLevelOn, reason,
            &assertionID);
#endif
        if (desktop::Windows() ||
            desktop::X11())
        {
            Fl_PNG_Image* rgb = load_png("mrv2.png");;
            icon(rgb);
            delete rgb;
        }
    }

#if !defined(__APPLE__)

    void MainWindow::always_on_top(int t)
    {
        on_top = t;

#    if defined(_WIN32)
        HWND action;
        if (t)
            action = HWND_TOPMOST;
        else
            action = HWND_NOTOPMOST;
        // Microsoft (R) Windows(TM)
        SetWindowPos(
            fl_xid(this), action, NULL, NULL, NULL, NULL,
            SWP_NOMOVE | SWP_NOSIZE);
#    elif defined(FLTK_USE_X11)
        if (fl_x11_display())
        {
            // XOrg / XWindows(TM)
            XEvent ev;
            static const char* const names[2] = {
                "_NET_WM_STATE", "_NET_WM_STATE_ABOVE"};
            Atom atoms[2];
            fl_open_display();
            XInternAtoms(fl_x11_display(), (char**)names, 2, False, atoms);
            Atom net_wm_state = atoms[0];
            Atom net_wm_state_above = atoms[1];
            ev.type = ClientMessage;
            ev.xclient.window = fl_xid(this);
            ev.xclient.message_type = net_wm_state;
            ev.xclient.format = 32;
            ev.xclient.data.l[0] = t;
            ev.xclient.data.l[1] = net_wm_state_above;
            ev.xclient.data.l[2] = 0;
            XSendEvent(
                fl_x11_display(), DefaultRootWindow(fl_display), False,
                SubstructureNotifyMask | SubstructureRedirectMask, &ev);
        }
#    endif
    } // above_all function

#endif

    void MainWindow::iconize_all()
    {
        return Fl_Double_Window::iconize();
    }

    int MainWindow::handle(int e)
    {
        if (e == FL_FULLSCREEN)
        {
            App::ui->uiTimeline->requestThumbnail();
        }

        return DropWindow::handle(e);
    }

    //! Resize override to handle tile
    void MainWindow::resize(int X, int Y, int W, int H)
    {
        wayland_resize = true;
        DropWindow::resize(X, Y, W, H);
        wayland_resize = false;

        // Redraw viewport windows in case we go from one monitor to another
        // with different OCIO display/view.
        App::ui->uiView->redrawWindows();
    }

    void MainWindow::update_title_bar()
    {
        App* app = App::app;
        auto model = app->filesModel();

        size_t numFiles = model->observeFiles()->getSize();

        char buf[256];
        std::string session = session::current();
        if (!session.empty())
        {
            file::Path path(session);
            session = " | ";
            session +=
                _("Session: ") + path.get(-1, file::PathType::FileName) + " ";
        }
        const int aIndex = model->observeAIndex()->get();
        std::string fileName;
        if (numFiles > 0 && aIndex >= 0 && aIndex < numFiles)
        {
            const auto& files = model->observeFiles()->get();
            fileName = files[aIndex]->path.get(-1, file::PathType::FileName);

            const auto& ioInfo = files[aIndex]->ioInfo;
            std::stringstream ss;
            ss.precision(2);
            if (!ioInfo.video.empty())
            {
                {
                    ss << "V:" << ioInfo.video[0].size.w << "x"
                       << ioInfo.video[0].size.h << ":" << std::fixed
                       << ioInfo.video[0].size.getAspect() << " "
                       << ioInfo.video[0].pixelType;
                }
            }
            if (ioInfo.audio.isValid())
            {
                if (!ss.str().empty())
                    ss << ", ";
                ss << "A: "
                   << static_cast<size_t>(ioInfo.audio.channelCount)
                   << " " << ioInfo.audio.dataType << " "
                   << ioInfo.audio.sampleRate;
            }
            snprintf(
                buf, 256, "%s %s%s", fileName.c_str(), ss.str().c_str(),
                session.c_str());
        }
        else
        {
            snprintf(
                buf, 256, "mrv2 v%s %s%s", mrv::version(), mrv::build_date(),
                session.c_str());
        }
        copy_label(buf);
    }

    void MainWindow::draw()
    {
        
#ifdef FLTK_USE_WAYLAND
        if (fl_wl_display())
        {
            // double alpha = (double)win_alpha / 255.0;
            // cairo_set_source_rgba(fl_wl_cairo(), 0, 0, 0, alpha); 
            // fl_rectf(0, 0, w(), h());
        }
#endif
        Fl_Double_Window::draw();
    }

    void MainWindow::set_alpha(int new_alpha)
    {
        if (new_alpha < 0)
        {
            win_alpha = 0;
        }
        else if (new_alpha > 255)
        {
            win_alpha = 255;
        }
        else
        {
            win_alpha = new_alpha;
        }
        
#if defined (_WIN32)
        HWND hwnd = fl_xid(this);
        LONG_PTR exstyle = GetWindowLongPtr(hwnd, GWL_EXSTYLE);
        if (!(exstyle & WS_EX_LAYERED))
        {
            SetWindowLongPtr(hwnd, GWL_EXSTYLE, exstyle | WS_EX_LAYERED);
        }
        SetLayeredWindowAttributes(hwnd, 0, BYTE(win_alpha), LWA_ALPHA);
//        SetLayeredWindowAttributes(hwnd, RGB(0, 0, 0), 0, LWA_COLORKEY);
#elif defined(__APPLE__)
        double alpha = (double)win_alpha / 255.0;
        set_window_transparency(this, alpha); // defined in transp_cocoa.mm
#elif defined(FLTK_USE_X11)
        if (fl_x11_display())
        {
            double alpha = (double)win_alpha / 255.0;
            uint32_t cardinal_alpha = (uint32_t)(UINT32_MAX * alpha);
            Atom atom = XInternAtom(fl_display, "_NET_WM_WINDOW_OPACITY",
                                    False);
            XChangeProperty(fl_display, fl_xid(this),
                            atom, XA_CARDINAL, 32,
                            PropModeReplace,
                            (unsigned char *)&cardinal_alpha, 1);
        }
#endif
    }
} // namespace mrv
