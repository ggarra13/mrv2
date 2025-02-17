// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#define DEBUG_CLICK_THROUGH 1

#include <cstring>

#include "mrvCore/mrvHotkey.h"

#include "mrvWidgets/mrvPanelWindow.h"

#include "mrvFl/mrvSession.h"

#include "mrvUI/mrvDesktop.h"

#include "mrvMainWindow.h"
#include "mrvPreferencesUI.h"

#include <FL/names.h>
#include <FL/platform.H>
#include <FL/fl_utf8.h>
#include <FL/Enumerations.H>
#include <FL/Fl.H>

#if defined(FLTK_USE_X11)
#    include <X11/extensions/scrnsaver.h>
#    include <X11/Xlib.h>
#    include <X11/extensions/Xfixes.h>
#    include <X11/extensions/shape.h>
#endif

#ifdef FLTK_USE_WAYLAND
#    include <cairo/cairo.h>
#    include <wayland-client.h>
// #    include <wayland/protocols/xdg-shell-client-protocol.h>
#endif

#include "mrViewer.h"

#include "mrvFl/mrvIO.h"

namespace
{
    const char* kModule = "main";
}

namespace mrv
{
    struct MainWindow::Private
    {
        Fl_Offscreen offscreen;
    };

    MainWindow::MainWindow(int X, int Y, int W, int H, const char* title) :
        DropWindow(X, Y, W, H, title),
        _p(new Private)
    {
        box(FL_FLAT_BOX);
        init();
    }

    MainWindow::MainWindow(int W, int H, const char* title) :
        DropWindow(W, H, title),
        _p(new Private)
    {
        box(FL_FLAT_BOX);
        init();
    }

    MainWindow::~MainWindow()
    {
#ifdef FLTK_USE_WAYLAND
        // Restore screensaver/black screen
#elif defined(FLTK_USE_X11)
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
        if (desktop::Windows() || desktop::X11())
        {
            Fl_PNG_Image* rgb = load_png("mrv2.png");
            ;
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
#ifdef DEBUG_CLICK_THROUGH
        if (click_through)
            std::cerr << fl_eventnames[e] << std::endl;
#endif
        if (e == FL_FULLSCREEN)
        {
            App::ui->uiTimeline->requestThumbnail();
        }
        else if (e == FL_KEYBOARD)
        {
            // If we have a text widget, don't swallow key presses
            unsigned rawkey = Fl::event_key();
#if defined(FLTK_USE_WAYLAND)
            if (rawkey >= 'A' && rawkey <= 'Z')
            {
                rawkey = tolower(rawkey);
            }
#endif

            if (kUITransparencyMore.match(rawkey))
            {
                set_alpha(get_alpha() - 5);
                return 1;
            }
            else if (kUITransparencyLess.match(rawkey))
            {
                set_alpha(get_alpha() + 5);
                return 1;
            }
            else if (kToggleClickThrough.match(rawkey))
            {
                set_click_through(!click_through);
                return 1;
            }
        }

#if defined(_WIN32)
        // THis does not work on macOS
        if ((e == FL_UNFOCUS) && click_through)
        {
            set_click_through(false);
            return 1;
        }
#endif

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
                ss << "A: " << static_cast<size_t>(ioInfo.audio.channelCount)
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

    void MainWindow::show()
    {
        DropWindow::show();

#ifdef FLTK_USE_WAYLAND
        if (fl_wl_display())
        {
            wl_surface_set_opaque_region(fl_wl_surface(fl_wl_xid(this)), NULL);
        }
#endif
    }

    void MainWindow::draw()
    {
        TLRENDER_P();

#ifdef FLTK_USE_WAYLAND
        if (fl_wl_display())
        {
            p.offscreen = fl_create_offscreen(w(), h());

            // 1. Draw child widgets to an offscreen buffer
            fl_begin_offscreen(p.offscreen);
            Fl_Double_Window::draw_children(); // Draw all the window's children
                                               // (widgets)
            fl_end_offscreen();

            // 2. Get the offscreen Cairo context
            cairo_t* offscreen_cr = (cairo_t*)p.offscreen;

            // 3. Create a surface from the offscreen
            cairo_surface_t* offscreen_surface = cairo_get_target(offscreen_cr);

            cairo_t* cr = fl_wl_gc();

            // 4. Clear the canvas (needed as FLTK seems to accumulate
            //    transparency)
            cairo_set_source_rgba(cr, 0, 0, 0, 0);
            cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
            cairo_paint(cr);

            // 5. Paint with alpha channel the offscreen widgets
            cairo_set_source_surface(cr, offscreen_surface, 0, 0);
            cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);

            const double alpha = (double)win_alpha / 255.0;
            cairo_paint_with_alpha(cr, alpha);

            fl_delete_offscreen(p.offscreen);
        }
        else
        {
            DropWindow::draw();
        }
#else
        DropWindow::draw();
#endif
    }

    void MainWindow::set_alpha(int new_alpha)
    {
        // Don't allow fully transparent window
        if (new_alpha < 96)
        {
            win_alpha = 96;
        }
        else if (new_alpha > 255)
        {
            win_alpha = 255;
        }
        else
        {
            win_alpha = new_alpha;
        }

        const double alpha = (double)win_alpha / 255.0;

#if defined(_WIN32)
        HWND hwnd = fl_xid(this);
        LONG_PTR exstyle = GetWindowLongPtr(hwnd, GWL_EXSTYLE);
        if (!(exstyle & WS_EX_LAYERED))
        {
            SetWindowLongPtr(hwnd, GWL_EXSTYLE, exstyle | WS_EX_LAYERED);
        }
        SetLayeredWindowAttributes(hwnd, 0, BYTE(win_alpha), LWA_ALPHA);
//        SetLayeredWindowAttributes(hwnd, RGB(0, 0, 0), 0, LWA_COLORKEY);
#elif defined(__APPLE__)
        set_window_transparency(alpha); // defined in transp_cocoa.mm
#elif defined(FLTK_USE_X11)
        if (fl_x11_display())
        {
            uint32_t cardinal_alpha = (uint32_t)(UINT32_MAX * alpha);
            Atom atom =
                XInternAtom(fl_display, "_NET_WM_WINDOW_OPACITY", False);
            XChangeProperty(
                fl_display, fl_xid(this), atom, XA_CARDINAL, 32,
                PropModeReplace, (unsigned char*)&cardinal_alpha, 1);
        }
#endif
        redraw();

        App::ui->uiView->redraw();
        App::ui->uiTimeline->redraw();
    }

#ifdef _WIN32

    void setClickThroughWin32(HWND hwnd, bool enable)
    {
        LONG style = GetWindowLong(hwnd, GWL_EXSTYLE);
        if (enable)
        {
            SetWindowLong(
                hwnd, GWL_EXSTYLE, style | WS_EX_LAYERED | WS_EX_TRANSPARENT);
        }
        else
        {
            SetWindowLong(
                hwnd, GWL_EXSTYLE,
                style & ~(WS_EX_LAYERED | WS_EX_TRANSPARENT));
        }
    }

#endif

#if defined(__linux__)

#    ifdef FLTK_USE_X11

    void setClickThroughX11(
        Display* display, Window window, bool enable, short unsigned int W,
        short unsigned int H)
    {
        int event_base, error_base;
        if (!XShapeQueryExtension(display, &event_base, &error_base))
        {
            LOG_ERROR("X Shape extension not available");
            return;
        }

        if (enable)
        {
            Region region = XCreateRegion();
            XShapeCombineRegion(
                display, window, ShapeInput, 0, 0, region, ShapeSet);
            XDestroyRegion(region);
        }
        else
        {
            XShapeCombineMask(
                display, window, ShapeInput, 0, 0, None, ShapeSet);
        }
        XFlush(display);
    }
#    endif // FLTK_USE_X11

#    ifdef FLTK_USE_WAYLAND

    void setClickThroughWayland(
        struct wl_surface* surface, struct wl_compositor* compositor,
        bool enable, int W, int H)
    {
        struct wl_region* region = wl_compositor_create_region(compositor);
        if (!enable)
        {
            // Restore normal behavior
            wl_region_add(region, 0, 0, W, H);
        }
        wl_surface_set_input_region(surface, enable ? NULL : region);
        wl_region_destroy(region);
    }

#    endif // FLTK_USE_WAYLAND

#endif // __linux__

#if !defined(__APPLE__)

    void MainWindow::setClickThrough(bool enable)
    {
        int W = w();
        int H = h();
#    ifdef _WIN32
        HWND win = fl_win32_xid(this);
        setClickThroughWin32(win, enable);
#    elif defined(__linux__)

#        ifdef FLTK_USE_X11
        auto display = fl_x11_display();
        if (display)
        {
            Window win = fl_x11_xid(this);
            if (!win)
            {
                LOG_ERROR("No window");
            }
            setClickThroughX11(display, win, enable, W, H);
        }
#        endif // FLTK_USE_X11

#        ifdef FLTK_USE_WAYLAND
        auto wldpy = fl_wl_display();
        if (wldpy)
        {
            auto win = fl_wl_xid(this);
            if (!win)
            {
                LOG_ERROR("No window");
            }
            auto compositor = fl_wl_compositor();
            if (!compositor)
            {
                LOG_ERROR("No compositor");
            }
            auto surface = fl_wl_surface(win);
            if (!surface)
            {
                LOG_ERROR("No surface");
            }

            setClickThroughWayland(surface, compositor, enable, W, H);
        }
#        endif // FLTK_USE_WAYLAND

#    endif // __linux__

        set_alpha(win_alpha);
    }

#endif // !__APPLE__

    void MainWindow::set_click_through(bool value)
    {
        if (click_through == value)
            return;

        click_through = value;

        always_on_top(click_through);

        setClickThrough(value);
    }

} // namespace mrv
