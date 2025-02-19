// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#define DEBUG_CLICK_THROUGH 1

#include <cstring>

#include "mrvCore/mrvHotkey.h"

#include "mrvWidgets/mrvPanelWindow.h"

#include "mrvFl/mrvSession.h"

#include "mrvUI/mrvAsk.h"
#include "mrvUI/mrvDesktop.h"

#include "mrvMainWindow.h"
#include "mrvPreferencesUI.h"

#include <FL/names.h>
#include <FL/platform.H>
#include <FL/fl_utf8.h>
#include <FL/Enumerations.H>
#include <FL/Fl.H>

#if defined(FLTK_USE_X11)
#    include <unistd.h>
#    include <sys/time.h>
#    include <X11/Xlib.h>
#    include <X11/extensions/scrnsaver.h>
#    include <X11/extensions/shape.h>
#    include <X11/extensions/Xfixes.h>
#    include <X11/extensions/XTest.h>
#endif

#ifdef FLTK_USE_WAYLAND
#    include <cairo/cairo.h>
#    include <wayland-client.h>
#endif

#include "mrViewer.h"

#include "mrvFl/mrvIO.h"

namespace
{
    const char* kModule = "main";
}


namespace
{
#ifdef FLTK_USE_X11
    Window last_window = None; // Tracks the last window under the cursor
    bool   is_dragging = false;

    unsigned long get_current_time_ms() {
        struct timeval tv;
        gettimeofday(&tv, NULL);
        return tv.tv_sec * 1000UL + tv.tv_usec / 1000;
    }

    // Convert FLTK keycode to X11 KeySym
    KeySym fltkToX11KeySym(int fltkKey)
    {
        switch (fltkKey) {
        case FL_BackSpace: return XK_BackSpace;
        case FL_Tab:       return XK_Tab;
        case FL_Enter:     return XK_Return;
        case FL_Escape:    return XK_Escape;
        case FL_Delete:    return XK_Delete;
        case FL_Left:      return XK_Left;
        case FL_Right:     return XK_Right;
        case FL_Up:        return XK_Up;
        case FL_Down:      return XK_Down;
        default:           return fltkKey; // FLTK keycodes are mostly compatible with X KeySyms
        }
}
    
    // Function to get a window's title
    std::string get_window_name(Display *display, Window window)
    {
        std::string out;
        char *window_name = NULL;
        if (XFetchName(display, window, &window_name) && window_name)
        {
            out = window_name;
            XFree(window_name);
        }
        return out;
    }
    
    // Function to get window geometry (position and size)
    void get_window_geometry(Display *display, Window window, int *x, int *y,
                             unsigned int *width, unsigned int *height)
    {
        Window root, child;
        int wx, wy;
        unsigned int ww, wh, border_width, depth;

        // Get the geometry of the window
        if (XGetGeometry(display, window, &root, &wx, &wy, &ww, &wh,
                         &border_width, &depth))
        {
            // Translate coordinates to root window (absolute screen position)
            if (XTranslateCoordinates(display, window, root,
                                      0, 0, &wx, &wy, &child))
            {
                *x = wx;
                *y = wy;
                *width = ww;
                *height = wh;
            }
        }
    }

    // Function to check whether a window is minimized
    static int is_window_minimized(Display *display, Window window)
    {
        Atom actual_type;
        int actual_format;
        unsigned long nitems, bytes_after;
        unsigned char* prop_value = NULL;
        Atom* atoms;
        int minimized = 0;

        Atom net_wm_state = XInternAtom(display, "_NET_WM_STATE", False);
        Atom net_wm_state_iconified = XInternAtom(display,
                                                  "_NET_WM_STATE_ICONIFIED",
                                                  False);

        Atom net_wm_state_hidden = XInternAtom(display, "_NET_WM_STATE_HIDDEN",
                                               False);

        if (net_wm_state != None)
        {
            int result = XGetWindowProperty(display, window, net_wm_state,
                                            0, (~0L), False, XA_ATOM, &actual_type,
                                            &actual_format, &nitems, &bytes_after,
                                            &prop_value);
            if (result == Success && prop_value != NULL)
            {
                atoms = (Atom *)prop_value;
                for (unsigned long i = 0; i < nitems; i++)
                {
                    if (atoms[i] == net_wm_state_iconified ||
                        atoms[i] == net_wm_state_hidden)
                    {
                        minimized = 1;
                        break;
                    }
                }
                XFree(prop_value);
            }
        }
        return minimized;
    }


    // Helper function to fetch the window list
    static std::vector<Window> fetch_window_list(Display *display)
    {
        std::vector<Window> window_list;
        Atom atom = XInternAtom(display, "_NET_CLIENT_LIST_STACKING", False);
        Atom actual_type;
        int actual_format;
        unsigned long nitems, bytes_after;
        unsigned char *data = nullptr;

        if (XGetWindowProperty(display, DefaultRootWindow(display), atom,
                               0, 1024, False, XA_WINDOW,
                               &actual_type, &actual_format, &nitems,
                               &bytes_after, &data) == Success && data)
        {
            Window* windows = (Window *)data;
            window_list.assign(windows, windows + nitems);
            XFree(data);
        }
        return window_list;
    }

    // Structure to hold the old error handler
    static int (*old_error_handler)(Display*, XErrorEvent*);

    // Custom error handler to suppress errors
    static int suppress_x_errors(Display* display, XErrorEvent* event)
    {
        // Suppress all errors
        return 0;
    }

    // Function to get all windows below my_window in the stacking order
    std::vector<Window> get_windows_below(Display *display, Window target_window)
    {
        std::vector<Window> below_windows;
        int max_retries = 3; // Maximum number of retries
        int retry_count = 0;

        while (retry_count < max_retries) {
            // Install error handler to suppress X11 errors
            old_error_handler = XSetErrorHandler(suppress_x_errors);

            std::vector<Window> windows = fetch_window_list(display);
            bool found_target_window = false;

            int i;
            for (i = windows.size() - 1; i >= 0; --i)
            {
                const Window& window = windows[i];
                XWindowAttributes attr;
                bool viewable = false;
                bool minimized = false;

                // Don't include unmapped windows on the list
                if (XGetWindowAttributes(display, window, &attr)) {
                    if (attr.map_state == IsViewable)
                    {
                        viewable = true;
                    }
                    else
                    {
                        viewable = false;
                    }
                }
                else
                {
                    // Invalid window...
                    break; // Refetch window list
                }

                // Don't include minimized windows on the list
                if (is_window_minimized(display, window))
                {
                    minimized = true;
                }

                if (found_target_window && viewable && !minimized)
                {
                    // Add all windows below target_window
                    below_windows.push_back(window);
                }

                if (window == target_window)
                {
                    found_target_window = true;
                }
            }

            // Restore old error handler
            XSetErrorHandler(old_error_handler);
            XSync(display, False); // Flush pending errors

            // If the loop completed without errors, break the retry loop
            if ((i < 0 && found_target_window) || windows.empty())
            {
                break;
            }

            retry_count++;
            usleep(5000); // Small delay before retrying
        }

        if (retry_count == max_retries)
        {
            std::cerr << "Max retries reached, get_windows_below may be incomplete." << std::endl;
        }

        return below_windows;
    }
    
    // Function to find the  window below a mouse coordinate and given vector of
    // windows in stacked order.
    Window find_window_below(Display *display,
                             const std::vector<Window> &windows, int mx, int my)
    {
        Window closest_window = None;

        for (Window win : windows)
        {
            int wx, wy;
            unsigned int ww, wh;
            get_window_geometry(display, win, &wx, &wy, &ww, &wh);

            if (mx >= wx && my >= wy && mx <= wx + ww && my <= wy + wh)
            {
                closest_window = win;
                break;
            }
        }

        return closest_window;
    }
    
#define CHECK_EVENT_TYPE(x)                     \
    case x:                                     \
    {                                           \
        s << #x;                                \
        return s.str();                         \
        break;                                  \
    }
    
    // Debugging Function to return the event name.
    std::string get_event_name(XEvent* event)
    {
        std::stringstream s;
        switch(event->type)
        {
            CHECK_EVENT_TYPE(ButtonPress);
            CHECK_EVENT_TYPE(ButtonRelease);;
            CHECK_EVENT_TYPE(CirculateNotify);
            CHECK_EVENT_TYPE(CirculateRequest);
            CHECK_EVENT_TYPE(ClientMessage);
            CHECK_EVENT_TYPE(ColormapNotify);
            CHECK_EVENT_TYPE(ConfigureNotify);
            CHECK_EVENT_TYPE(ConfigureRequest);
            CHECK_EVENT_TYPE(CreateNotify);
            CHECK_EVENT_TYPE(DestroyNotify);
            CHECK_EVENT_TYPE(EnterNotify);
            CHECK_EVENT_TYPE(Expose);
            CHECK_EVENT_TYPE(FocusIn);
            CHECK_EVENT_TYPE(FocusOut);
            CHECK_EVENT_TYPE(GraphicsExpose);
            CHECK_EVENT_TYPE(KeymapNotify);
            CHECK_EVENT_TYPE(KeyPress);
            CHECK_EVENT_TYPE(KeyRelease);
            CHECK_EVENT_TYPE(LeaveNotify);
            CHECK_EVENT_TYPE(MapNotify);
            CHECK_EVENT_TYPE(MappingNotify);
            CHECK_EVENT_TYPE(MapRequest);
            CHECK_EVENT_TYPE(MotionNotify);
            CHECK_EVENT_TYPE(NoExpose);
            CHECK_EVENT_TYPE(PropertyNotify);
            CHECK_EVENT_TYPE(ReparentNotify);
            CHECK_EVENT_TYPE(ResizeRequest);
            CHECK_EVENT_TYPE(SelectionClear);
            CHECK_EVENT_TYPE(SelectionRequest);
            CHECK_EVENT_TYPE(SelectionNotify);
            CHECK_EVENT_TYPE(UnmapNotify);
            CHECK_EVENT_TYPE(VisibilityNotify);
        default:
            s << "UNKNOWN (" << event->type << ")";
            return s.str();
            break;
        }
    }

    void mrv2_XSendEvent(Display* display, Window target_window, Bool value,
                         int mask, XEvent* event)
    {
        XSendEvent(display, target_window, value, mask, event);
        XFlush(display);
    }

    // Function to send an FLTK event as an XEvent to the specified X11 window
    void send_fltk_event_to_x11(Display *display, Window target_window,
                                int fltk_event, int root_x, int root_y,
                                int button = 0, unsigned int key = 0,
                                unsigned int modifiers = 0,
                                const char* text = "")
    {
        if (target_window == None)
            return;
        
        // Convert root coordinates to local window coordinates
        int local_x = root_x, local_y = root_y;
        Window child_return;
        XTranslateCoordinates(display, DefaultRootWindow(display),
                              target_window, root_x, root_y,
                              &local_x, &local_y, &child_return);

        // Create an empty event
        XEvent event;
        memset(&event, 0, sizeof(event));
        int mask = NoEventMask;
        unsigned long evTime = get_current_time_ms();        
        event.xany.type = 0;
        event.xany.display = display;
        event.xany.window = target_window;

        // We need to send an EnterNotify so that the X11 window behind will
        // receive input as we are inside the non-transparent front FLTK window
        // if (fltk_event != FL_LEAVE && fltk_event != FL_ENTER &&
        //     fltk_event != FL_NO_EVENT)
        if (fltk_event == FL_PUSH || fltk_event == FL_MOVE ||
            fltk_event == FL_RELEASE)
        {
            event.type = EnterNotify;
            event.xcrossing.x = local_x;
            event.xcrossing.y = local_y;
            event.xcrossing.x_root = root_x;
            event.xcrossing.y_root = root_y;
            mrv2_XSendEvent(display, target_window, True, mask, &event);
            
            KeySym keysym = fltkToX11KeySym(key);
            event.xkey.x = local_x;
            event.xkey.y = local_y;
            event.xkey.x_root = root_x;
            event.xkey.y_root = root_y;
            event.xkey.keycode = XKeysymToKeycode(display, keysym);
            event.xkey.state = modifiers;
            event.xkey.time = evTime;

        }

        switch (fltk_event)
        {
        case FL_ENTER:
            event.type = EnterNotify;
            event.xcrossing.x = local_x;
            event.xcrossing.y = local_y;
            event.xcrossing.x_root = root_x;
            event.xcrossing.y_root = root_y;
            mrv2_XSendEvent(display, target_window, True, mask, &event);
            
            break;
        case FL_PUSH:
            mask = ButtonPressMask | ButtonReleaseMask | PointerMotionMask; // Use correct mask
            event.type = ButtonPress;
            event.xbutton.button = button;
            event.xbutton.x = local_x;
            event.xbutton.y = local_y;
            event.xbutton.x_root = root_x;
            event.xbutton.y_root = root_y;
            event.xbutton.time = evTime;
            mrv2_XSendEvent(display, target_window, True, mask, &event);


            XGrabPointer(display, target_window, True,
                         PointerMotionMask | ButtonReleaseMask,
                         GrabModeAsync, GrabModeAsync, None, None, CurrentTime);

            break;
            
        case FL_DRAG:
            mask = PointerMotionMask | SubstructureRedirectMask;
            event.type = MotionNotify;
            event.xmotion.x = local_x;
            event.xmotion.y = local_y;
            event.xmotion.x_root = root_x;
            event.xmotion.y_root = root_y;
            event.xmotion.state = modifiers;
            event.xmotion.time = evTime;
            mrv2_XSendEvent(display, target_window, True, mask, &event);
            break;
        case FL_RELEASE:
            mask = ButtonReleaseMask; // Use correct mask
            event.type = ButtonRelease;
            event.xbutton.button = button;
            event.xbutton.x = local_x;
            event.xbutton.y = local_y;
            event.xbutton.x_root = root_x;
            event.xbutton.y_root = root_y;
            event.xbutton.time = evTime;
            mrv2_XSendEvent(display, target_window, True, mask, &event);

            XUngrabPointer(display, CurrentTime);

            break;
        case FL_MOVE:
            mask = PointerMotionMask;
            event.type = MotionNotify;
            event.xmotion.x = local_x;
            event.xmotion.y = local_y;
            event.xmotion.x_root = root_x;
            event.xmotion.y_root = root_y;
            event.xmotion.time = evTime;
            mrv2_XSendEvent(display, target_window, True, mask, &event);
            break;
        case FL_FOCUS:
            event.type = FocusIn;
            mrv2_XSendEvent(display, target_window, True, mask, &event);
            break;
        case FL_UNFOCUS:
            event.type = FocusOut;
            mrv2_XSendEvent(display, target_window, True, mask, &event);
            break;
        case FL_KEYBOARD:
        case FL_SHORTCUT:
            event.type = KeyPress;
            mrv2_XSendEvent(display, target_window, True, mask, &event);
            break;
        case FL_KEYUP:
            event.type = KeyRelease;
            mrv2_XSendEvent(display, target_window, True, mask, &event);
            break;
        default:
            return; // Ignore other events
        }

    }
    
#endif
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
            icon(rgb);
            delete rgb;
        }
    }

#if !defined(__APPLE__)

    void MainWindow::always_on_top(int value)
    {
        on_top = value;

#    if defined(_WIN32)
        HWND action;
        if (value)
            action = HWND_TOPMOST;
        else
            action = HWND_NOTOPMOST;
        // Microsoft (R) Windows(TM)
        SetWindowPos(fl_win32_xid(this), action, NULL, NULL, NULL, NULL,
                     SWP_NOMOVE | SWP_NOSIZE);
#    elif defined(FLTK_USE_X11)
        Display* display = fl_x11_display();
        if (display)
        {
            // XOrg / XWindows(TM)
            XEvent ev;
            static const char* const names[2] = {
                "_NET_WM_STATE", "_NET_WM_STATE_ABOVE"};
            Atom atoms[2];
            fl_open_display();
            XInternAtoms(display, (char**)names, 2, False, atoms);
            Atom net_wm_state = atoms[0];
            Atom net_wm_state_above = atoms[1];
            ev.type = ClientMessage;
            ev.xclient.window = fl_x11_xid(this);
            ev.xclient.message_type = net_wm_state;
            ev.xclient.format = 32;
            ev.xclient.data.l[0] = value;
            ev.xclient.data.l[1] = net_wm_state_above;
            ev.xclient.data.l[2] = 0;
            XSendEvent(display, DefaultRootWindow(display), False,
                       SubstructureNotifyMask | SubstructureRedirectMask, &ev);
            XFlush(display);
        }
#    endif
    } // above_all function

#endif

    void MainWindow::iconize_all()
    {
        return Fl_Double_Window::iconize();
    }

    int MainWindow::handle(int event)
    {
        if (event == FL_FOCUS && click_through)
        {
#ifdef __linux__
            if (ignoreFocus)
            {
                ignoreFocus = false;
                return 0;
            }
#endif
            set_click_through(false);
            return 1;
        }
        
#ifdef FLTK_USE_X11
        if (click_through && desktop::X11() )
        {
            if (event == FL_ENTER || event == FL_LEAVE)
                return 0;
            
            if (event == FL_FOCUS)
            {
                set_click_through(false);
                return 1;
            }
            
            int x = Fl::event_x();
            int y = Fl::event_y();
            if (x < 0 || x > w() || y < 0 || y > h())
                return 0;
            
            int root_x = Fl::event_x_root();
            int root_y = Fl::event_y_root();
            int button = Fl::event_button();
            unsigned int key = Fl::event_key();  // \@bug: international characters not handled
            const char* text = Fl::event_text();
            
            unsigned int modifiers = Fl::e_state >> 16 | Fl::event_buttons();

            Display* display = fl_x11_display();
            Window this_window = fl_x11_xid(this);
            
            const std::vector<Window>& below_windows =
                get_windows_below(display, this_window);

            Window new_window = find_window_below(display, below_windows,
                                                  root_x, root_y);

            if (new_window != last_window)
            {
                if (last_window != None)
                {
                    send_fltk_event_to_x11(display, last_window, FL_LEAVE, root_x, root_y);
                }
                if (new_window != None)
                {
                    send_fltk_event_to_x11(display, new_window, FL_ENTER, root_x, root_y);
                }
            }
            
            send_fltk_event_to_x11(display, new_window, event, root_x, root_y,
                                   button, key, modifiers, text);
            last_window = new_window;
            return 1;
        }
#endif
        if (event == FL_FULLSCREEN)
        {
            App::ui->uiTimeline->requestThumbnail();
        }
        else if (event == FL_KEYBOARD)
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
                ignoreFocus = true;
                Fl::focus(nullptr);
                return 1;
            }
        }

        int ret = DropWindow::handle(event);
        return ret;
    }

    //! Resize override to handle tile
    void MainWindow::resize(int X, int Y, int W, int H)
    {
        int oldW = w();
        int oldH = h();
        
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
                buf, 256, "mrv2 v%s %s%s", mrv::version(),
                mrv::build_date().c_str(),
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
#ifdef DEBUG_CLICK_THROUGH
        set_alpha(0);
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
        int minAlpha = 96;
        if (desktop::Wayland())
            minAlpha = 64;
        // Don't allow fully transparent window
        if (new_alpha < minAlpha)
        {
            win_alpha = minAlpha;
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
        HWND hwnd = fl_win32_xid(this);
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
                fl_display, fl_x11_xid(this), atom, XA_CARDINAL, 32,
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

#    ifdef FLTK_USE_WAYLAND
    
    void setClickThroughWayland(struct wl_surface* surface,
                                int width, int height)
    {
        auto compositor = fl_wl_compositor();
        if (!compositor)
        {
            LOG_ERROR("No compositor");
        }
        struct wl_region* region = wl_compositor_create_region(compositor);
        if (width > 0 && height > 0)
        {
            // Define the *non*-transparent area
            wl_region_add(region, 0, 0, width, height);
        }
        wl_surface_set_input_region(surface, region);
        wl_surface_commit(surface);
        wl_region_destroy(region);
    }
    
    void setClickThroughWayland(const Fl_Window* window, const bool enable)
    {
        auto win = fl_wl_xid(window);
        if (!win)
        {
            LOG_ERROR("No window");
        }
        auto surface = fl_wl_surface(win);
        if (!surface)
        {
            LOG_ERROR("No surface");
        }

        int W = window->w();
        int H = window->h();
        if (enable)
            W = H = 0;
        setClickThroughWayland(surface, W, H);
    }

#    endif // FLTK_USE_WAYLAND

#endif // __linux__

#if !defined(__APPLE__)

    void MainWindow::setClickThrough(bool enable)
    {
#    ifdef _WIN32
        HWND win = fl_win32_xid(this);
        setClickThroughWin32(win, enable);
#    elif defined(__linux__)

#        ifdef FLTK_USE_WAYLAND
        auto wldpy = fl_wl_display();
        if (wldpy)
        {
            Fl_Window* window = Fl::first_window();
            for (; window; window = Fl::next_window(window))
                setClickThroughWayland(window, enable);
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

        if (value)
        {
            int ok;

            if (desktop::Wayland())
            {
                ok = mrv::fl_choice(
                    _("This will disable events for mrv2, allowing them "
                      "to pass through.\n\n"
                      "Give window focus to disable."),
                    _("Stop"), _("Continue"), NULL, NULL);
            }
            else
            {
                ok = mrv::fl_choice(
                    _("This will disable events for mrv2, allowing them "
                      "to pass through.\n\n"
                      "Give window focus (click on the taskbar icon)\n"
                      "to disable."),
                    _("Stop"), _("Continue"), NULL, NULL);
            }
            if (!ok)
                return;
        }

        click_through = value;

        setClickThrough(value);

        always_on_top(click_through);
    }

} // namespace mrv
