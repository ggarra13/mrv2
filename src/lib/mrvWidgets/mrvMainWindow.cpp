// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

// #define DEBUG_CLICK_THROUGH 1

#include <cstring>

#include <tlCore/StringFormat.h>

#include "mrvCore/mrvHotkey.h"
#include "mrvCore/mrvString.h"

#include "mrvWidgets/mrvPanelWindow.h"

#include "mrvFl/mrvSession.h"

#include "mrvUI/mrvAsk.h"
#include "mrvUI/mrvDesktop.h"

#include "mrvMainWindow.h"
#include "mrvPreferencesUI.h"

#ifdef __linux__
#  ifdef FLTK_USE_WAYLAND
#    include <regex>
#    include <cairo/cairo.h>
#    include <wayland-client.h>
#  endif
#endif

#include "mrvFl/mrvIO.h"

#include "mrViewer.h"
#include "mrvCore/mrvOS.h"

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
#endif

namespace
{
    const char* kModule = "main";
}

namespace
{
#ifdef FLTK_USE_X11
    Window last_window = None; // Tracks the last window under the cursor
    bool is_dragging = false;

    unsigned long get_current_time_ms()
    {
        struct timeval tv;
        gettimeofday(&tv, NULL);
        return tv.tv_sec * 1000UL + tv.tv_usec / 1000;
        // return CurrentTime;
    }

    // Function to get a window's title
    std::string get_window_name(Window window)
    {
        Display* display = fl_x11_display();
        std::string out;
        char* window_name = NULL;
        if (XFetchName(display, window, &window_name) && window_name)
        {
            out = window_name;
            XFree(window_name);
        }
        return out + " (" + std::to_string(window) + ")";
    }

    // Function to get window geometry (position and size)
    void get_window_geometry(
        Display* display, Window window, int* x, int* y, unsigned int* width,
        unsigned int* height)
    {
        Window root, child;
        int wx, wy;
        unsigned int ww, wh, border_width, depth;

        // Get the geometry of the window
        if (XGetGeometry(
                display, window, &root, &wx, &wy, &ww, &wh, &border_width,
                &depth))
        {
            // Translate coordinates to root window (absolute screen position)
            if (XTranslateCoordinates(
                    display, window, root, 0, 0, &wx, &wy, &child))
            {
                *x = wx;
                *y = wy;
                *width = ww;
                *height = wh;
            }
        }
    }

    // Function to check whether a window is minimized
    static int is_window_minimized(Display* display, Window window)
    {
        Atom actual_type;
        int actual_format;
        unsigned long nitems, bytes_after;
        unsigned char* prop_value = NULL;
        Atom* atoms;
        int minimized = 0;

        Atom net_wm_state = XInternAtom(display, "_NET_WM_STATE", False);
        Atom net_wm_state_iconified =
            XInternAtom(display, "_NET_WM_STATE_ICONIFIED", False);

        Atom net_wm_state_hidden =
            XInternAtom(display, "_NET_WM_STATE_HIDDEN", False);

        if (net_wm_state != None)
        {
            int result = XGetWindowProperty(
                display, window, net_wm_state, 0, (~0L), False, XA_ATOM,
                &actual_type, &actual_format, &nitems, &bytes_after,
                &prop_value);
            if (result == Success && prop_value != NULL)
            {
                atoms = (Atom*)prop_value;
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
    static std::vector<Window> fetch_window_list(Display* display)
    {
        std::vector<Window> window_list;
        Atom atom = XInternAtom(display, "_NET_CLIENT_LIST_STACKING", False);
        Atom actual_type;
        int actual_format;
        unsigned long nitems, bytes_after;
        unsigned char* data = nullptr;

        if (XGetWindowProperty(
                display, DefaultRootWindow(display), atom, 0, 1024, False,
                XA_WINDOW, &actual_type, &actual_format, &nitems, &bytes_after,
                &data) == Success &&
            data)
        {
            Window* windows = (Window*)data;
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
    std::vector<Window>
    get_windows_below(Display* display, Window target_window)
    {
        std::vector<Window> below_windows;
        int max_retries = 3; // Maximum number of retries
        int retry_count = 0;

        while (retry_count < max_retries)
        {
            // Install error handler to suppress X11 errors which
            // can happen due to fetch_window_list returning outdated
            // Window descriptors.
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
                if (XGetWindowAttributes(display, window, &attr))
                {
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

        return below_windows;
    }

    // Function to find the  window below a mouse coordinate and given vector of
    // windows in stacked order.
    static Window find_window_below(
        Display* display, const std::vector<Window>& windows, int x_root,
        int y_root)
    {
        Window closest_window = None;

        for (Window window : windows)
        {
            int wx, wy;
            unsigned int ww, wh;
            get_window_geometry(display, window, &wx, &wy, &ww, &wh);

            if (x_root >= wx && y_root >= wy && x_root <= wx + ww &&
                y_root <= wy + wh)
            {
                closest_window = window;
                break;
            }
        }

        return closest_window;
    }

#    define CHECK_EVENT_TYPE(x)                                                \
    case x:                                                                    \
    {                                                                          \
        s << #x;                                                               \
        return s.str();                                                        \
        break;                                                                 \
    }

    // Debugging Function to return the event name.
    std::string get_event_name(XEvent* event)
    {
        std::stringstream s;
        switch (event->type)
        {
            CHECK_EVENT_TYPE(ButtonPress);
            CHECK_EVENT_TYPE(ButtonRelease);
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

    void mrv2_XSendEvent(
        Display* display, Window target_window, Bool value, int mask,
        XEvent* event)
    {
        XSendEvent(display, target_window, value, mask, event);
        XFlush(display);
    }

    // Function to send an FLTK event as an XEvent to the specified X11 window
    void send_fltk_event_to_x11(
        Display* display, Window target_window, int fltk_event, int root_x,
        int root_y, int button = 0, unsigned int X11keycode = 0,
        unsigned int modifiers = 0, const char* text = "")
    {
        // Convert root coordinates to local window coordinates
        int local_x = root_x, local_y = root_y;
        Window child_return;
        XTranslateCoordinates(
            display, DefaultRootWindow(display), target_window, root_x, root_y,
            &local_x, &local_y, &child_return);

        // Create an empty event
        XEvent event;
        memset(&event, 0, sizeof(event));
        int mask = NoEventMask;
        KeySym keysym;
        unsigned long evTime = get_current_time_ms();
        event.xany.type = 0;
        event.xany.display = display;
        event.xany.window = target_window;

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
            mask = ButtonPressMask | ButtonReleaseMask |
                   PointerMotionMask; // Use correct mask
            event.type = ButtonPress;
            event.xbutton.button = button;
            event.xbutton.x = local_x;
            event.xbutton.y = local_y;
            event.xbutton.x_root = root_x;
            event.xbutton.y_root = root_y;
            event.xbutton.time = evTime;
            mrv2_XSendEvent(display, target_window, True, mask, &event);
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
            event.xkey.x = local_x;
            event.xkey.y = local_y;
            event.xkey.x_root = root_x;
            event.xkey.y_root = root_y;
            event.xkey.keycode = X11keycode;
            event.xkey.state = modifiers;
            event.xkey.time = evTime;
            mrv2_XSendEvent(display, target_window, True, mask, &event);
            break;
        case FL_KEYUP:
            event.type = KeyRelease;
            event.xkey.x = local_x;
            event.xkey.y = local_y;
            event.xkey.x_root = root_x;
            event.xkey.y_root = root_y;
            event.xkey.keycode = X11keycode;
            event.xkey.state = modifiers;
            mrv2_XSendEvent(display, target_window, True, mask, &event);
            break;
        case FL_MOUSEWHEEL:
            mask = ButtonPressMask | ButtonReleaseMask |
                   PointerMotionMask; // Use correct mask
            event.type = ButtonPress;
            if (Fl::e_dy == -1 && !Fl::event_shift())
                button = Button4;
            else if (Fl::e_dy == +1 && !Fl::event_shift())
                button = Button5;
            if (Fl::e_dy == -1 && Fl::event_shift())
                button = 6;
            else if (Fl::e_dy == +1 && Fl::event_shift())
                button = 7;
            event.xbutton.button = button;
            event.xbutton.x = local_x;
            event.xbutton.y = local_y;
            event.xbutton.x_root = root_x;
            event.xbutton.y_root = root_y;
            event.xbutton.time = evTime;
            break;
        default:
            // Ignore any other FLTK messages
            return;
        }
    }

    static int x11_handler(void* event, void* data)
    {

        XEvent* e = static_cast<XEvent*>(event);
        Fl_Window* w = static_cast<Fl_Window*>(data);

        Display* display = fl_x11_display();
        Window window = fl_x11_xid(w);

        int x_root = 0, y_root = 0;
        int fltk_event = FL_NO_EVENT; // Default
        int button = 0;
        unsigned int key = 0;
        unsigned int modifiers = 0;

        switch (e->type)
        {
        case ButtonPress:
            x_root = e->xbutton.x_root;
            y_root = e->xbutton.y_root;
            fltk_event = FL_PUSH;
            button = e->xbutton.button;
            break;
        case ButtonRelease:
            x_root = e->xbutton.x_root;
            y_root = e->xbutton.y_root;
            fltk_event = FL_RELEASE;
            button = e->xbutton.button;
            break;
        case EnterNotify:
            x_root = e->xcrossing.x_root;
            y_root = e->xcrossing.y_root;
            fltk_event = FL_ENTER;
            break;
        case LeaveNotify:
            x_root = e->xcrossing.x_root;
            y_root = e->xcrossing.y_root;
            fltk_event = FL_LEAVE; // No FLTK equivalent, but we'll use it.
            break;
        case MotionNotify:
            x_root = e->xmotion.x_root;
            y_root = e->xmotion.y_root;
            fltk_event = FL_MOVE; // Or FL_DRAG, depending on button state
            if (e->xmotion.state & (Button1Mask | Button2Mask | Button3Mask))
            {
                fltk_event = FL_DRAG;
                modifiers = e->xmotion.state;
            }
            break;
        case KeyPress:
            x_root = e->xkey.x_root;
            y_root = e->xkey.y_root;
            fltk_event = FL_KEYBOARD;
            modifiers = e->xkey.state;
            break;
        case KeyRelease:
            x_root = e->xkey.x_root;
            y_root = e->xkey.y_root;
            fltk_event = FL_KEYUP;
            modifiers = e->xkey.state;
            break;
        default:
            return 0;
            break;
        }

        const std::vector<Window>& below_windows =
            get_windows_below(display, window);
        Window new_window =
            find_window_below(display, below_windows, x_root, y_root);

        if (new_window == None)
        {
            return 1;
        }

        if (fltk_event != FL_NO_EVENT)
        {
            send_fltk_event_to_x11(
                display, new_window, fltk_event, x_root, y_root, button,
                e->xkey.keycode, modifiers);
            return 1;
        }

        return 0;
    }

#endif

} // namespace

namespace mrv
{
    struct MainWindow::Private
    {
        Fl_Offscreen offscreen;
        bool hidden = false;
        int mute = 0;
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
        allow_screen_saver(true);
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

        if (desktop::Windows() || desktop::X11())
        {
            Fl_PNG_Image* rgb = load_png("mrv2.png");
            icon(rgb);
            delete rgb;
        }
    }
    
    //! Turn off screen saver.
    void MainWindow::allow_screen_saver(bool value)
    {
        if (value)
        {
#ifdef __linux__
#  ifdef FLTK_USE_WAYLAND
            // Restore screensaver/black screen
#  elif defined(FLTK_USE_X11)
            if (fl_x11_display())
            {
                XScreenSaverSuspend(fl_x11_display(), False);
            }
#  endif
#elif defined(_WIN32)
            SetThreadExecutionState(ES_CONTINUOUS);
#elif defined(__APPLE__)
            success = IOPMAssertionRelease(assertionID);
#endif
        }
        else
        {
            //! Turn off screensaver and black screen
#ifdef __linux__
#  if defined(FLTK_USE_X11)
            if (fl_x11_display())
            {
                int event_base, error_base;
                Bool ok = XScreenSaverQueryExtension(
                    fl_x11_display(), &event_base, &error_base);
                if (ok == True)
                    XScreenSaverSuspend(fl_x11_display(), True);
            }
#endif
#elif defined(_WIN32)
            SetThreadExecutionState(
                ES_CONTINUOUS | ES_DISPLAY_REQUIRED);
#elif defined(__APPLE__)
            CFStringRef reason = CFSTR("mrv2 playback");
            success = IOPMAssertionCreateWithName(
                kIOPMAssertionTypeNoDisplaySleep, kIOPMAssertionLevelOn, reason,
                &assertionID);
#endif
        }
    }

#if !defined(__APPLE__)

    void MainWindow::always_on_top(int value, bool synthetic)
    {
        if (on_top == static_cast<bool>(value))
            return;

        on_top = value;

#    if defined(_WIN32)
        HWND action;
        if (value)
            action = HWND_TOPMOST;
        else
            action = HWND_NOTOPMOST;
        // Microsoft (R) Windows(TM)
        SetWindowPos(
            fl_win32_xid(this), action, NULL, NULL, NULL, NULL,
            SWP_NOMOVE | SWP_NOSIZE);
#    elif defined(__linux__)
        if (desktop::X11())
        {
            // XOrg / XWindows(TM)
            Display* display = fl_x11_display();
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
            mrv2_XSendEvent(
                display, DefaultRootWindow(display), False,
                SubstructureNotifyMask | SubstructureRedirectMask, &ev);
        }
#    endif
#    ifdef FLTK_USE_WAYLAND
        if (desktop::Wayland() && !synthetic)
        {
            const std::string& compositor_name = desktop::WaylandCompositor();
            if (compositor_name == "gnome-shell")
            {
                const std::string msg =
                    string::Format(_("Wayland supports Float on Top only "
                                     "through hotkeys.  Press {0}."))
                        .arg(kToggleFloatOnTop.to_s());
                LOG_WARNING(msg);
            }
            else
            {
                LOG_WARNING(_("Wayland does not support Float on Top.  Use "
                              "your Window Manager to set a hotkey for it."));
            }
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
        TLRENDER_P();
#ifdef __linux__
#  ifdef FLTK_USE_WAYLAND
        if (click_through && ignoreFocus && desktop::Wayland() &&
            event == FL_UNFOCUS)
        {
            return 0;
        }
#  endif
#endif
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
                return 1;
            }
        }
        else if (event == FL_HIDE)
        {
            TimelineClass* t = App::ui->uiTimeWindow;
            p.hidden = true;
            p.mute = t->uiAudioTracks->value();
            t->uiAudioTracks->value(1);
            t->uiAudioTracks->do_callback();
        }
        else if (event == FL_SHOW)
        {
            if (p.hidden)
            {
                TimelineClass* t = App::ui->uiTimeWindow;
                t->uiAudioTracks->value(p.mute);
                t->uiAudioTracks->do_callback();
                p.hidden = false;
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
            std::string unsaved;
            if (App::unsaved_annotations)
                unsaved = "(A) ";
            if (App::unsaved_edits)
                unsaved += "(E) ";
            snprintf(
                buf, 256, "mrv2 %s v%s %s%s %s %s",
                mrv::backend(),
                mrv::version(),
                unsaved.c_str(),
                fileName.c_str(),
                ss.str().c_str(),
                session.c_str());
        }
        else
        {
            App::unsaved_annotations = false;
            App::unsaved_edits = false;
            snprintf(
                buf, 256, "mrv2 %s v%s %s%s",
                mrv::backend(),
                mrv::version(),
                mrv::build_date().c_str(), session.c_str());
        }
        copy_label(buf);
    }

    void MainWindow::show()
    {
        DropWindow::show();

#ifdef __linux__
#  ifdef FLTK_USE_WAYLAND
        if (desktop::Wayland())
        {
            // This call is needed for proper transparency of the main window.
            wl_surface_set_opaque_region(fl_wl_surface(fl_wl_xid(this)),
                                         NULL);

            //
            // \@todo: Implement proper Always on Top on all compositors on
            //         a per application, instead of a global key for just
            //         GNOME Shell.
            //
            const std::string& compositor_name = desktop::WaylandCompositor();
            if (compositor_name == "gnome-shell")
            {
                int ret;
                std::string out, err;
                std::string cmd =
                    "gsettings get org.gnome.shell.extensions.tiling-assistant "
                    "toggle-always-on-top";
                ret = os::exec_command(cmd, out, err);
                std::string hotkey = string::stripWhitespace(out);
                if (ret == 0 &&
                    (hotkey.empty() ||
                     (hotkey.substr(hotkey.size() - 2, 2)) == "[]"))
                {
                    // If an FLTK hotkey is assigned, turn it into a GNOME
                    // Hotkey.
                    std::string hotkey = kToggleFloatOnTop.to_s();
                    if (!hotkey.empty())
                    {
                        size_t startPos = 0;
                        if (startPos = hotkey.find("Meta") != std::string::npos)
                        {
                            hotkey.replace(startPos, 4, "Super");
                        }
                        // Use a GNOME shell extension and set always on top
                        // to Meta + w
                        cmd = "gsettings set "
                              "org.gnome.shell.extensions.tiling-assistant "
                              "toggle-always-on-top \"['" +
                              hotkey + "']\"";
                        ret = os::exec_command(cmd, out, err);
                        if (ret == 0)
                        {
                            const std::string msg =
                                string::Format(
                                    _("No GNOME Shell Hotkey for Float On Top.  "
                                      "Setting it globally to '{0}'"))
                                .arg(hotkey);
                            LOG_STATUS(msg);
                        }
                    }
                }
                else
                {
                    if (hotkey[0] != '[' || hotkey[hotkey.size() - 1] != ']')
                        return;

                    // Turn a GNOME-Shell hotkey into an mrv2's hotkey
                    const std::regex kSuper("<Super>");
                    const std::regex kShift("<Shift>");
                    const std::regex kControl("<Ctrl>");
                    const std::regex kAlt("<Alt>");
                    kToggleFloatOnTop.meta = false;
                    if (std::regex_search(hotkey, kSuper))
                        kToggleFloatOnTop.meta = true;
                    kToggleFloatOnTop.shift = false;
                    if (std::regex_search(hotkey, kShift))
                        kToggleFloatOnTop.shift = true;
                    kToggleFloatOnTop.ctrl = false;
                    if (std::regex_search(hotkey, kControl))
                        kToggleFloatOnTop.ctrl = true;
                    kToggleFloatOnTop.alt = false;
                    if (std::regex_search(hotkey, kAlt))
                        kToggleFloatOnTop.alt = true;
                    std::string key = hotkey.substr(hotkey.size() - 3, 1);
                    char ch = key[0]; // Access the character
                    int asciiValue = static_cast<int>(ch); // Convert to ASCII
                    kToggleFloatOnTop.key = asciiValue;
                    App::ui->uiMain->fill_menu(App::ui->uiMenuBar);
                }
            }
        }
#  endif
#endif
    }

    void MainWindow::draw()
    {
        TLRENDER_P();

#ifdef __linux__
#  ifdef FLTK_USE_WAYLAND
        if (fl_wl_display() && win_alpha < 255)
        {
            cairo_t* cr = fl_wl_gc();

            float scale = 1.F;
            int screen = 0;
            int valid_scaling = Fl::screen_scaling_supported();
            if (valid_scaling == 2)
                screen = screen_num();  // scale different on each window.
            else if (valid_scaling == 1)
                scale = Fl::screen_scale(0);  // scale same for all windows.
            else if (valid_scaling == 0)
                scale = 1.F;  // scale not supported
            
            // 1. Make an offscreen surface bigger for antialiasing.
            p.offscreen = fl_create_offscreen(w()*4*scale, h()*4*scale);
            
            // 2. Draw child widgets to an offscreen buffer
            fl_begin_offscreen(p.offscreen);
            Fl_Double_Window::draw_children(); // Draw all the window's children
                                               // (widgets)
            fl_end_offscreen();

            // 3. Get the offscreen Cairo context
            cairo_t* offscreen_cr = (cairo_t*)p.offscreen;

            // 4. Create a surface from the offscreen
            cairo_surface_t* offscreen_surface = cairo_get_target(offscreen_cr);


            // 5. Clear the canvas (needed as FLTK seems to accumulate
            //    transparency)
            cairo_set_source_rgba(cr, 0, 0, 0, 0);
            cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
            cairo_paint(cr);

            // 6. Paint with alpha channel the offscreen widgets
            cairo_set_source_surface(cr, offscreen_surface, 0, 0);
            cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
            
            const double alpha = (double)win_alpha / 255.0;
            cairo_paint_with_alpha(cr, alpha);
            
            fl_delete_offscreen(p.offscreen);

            clear_damage();
        }
        else
        {
            DropWindow::draw();
        }
#  else  // !FLTK_USE_WAYLAND
        DropWindow::draw();
#  endif  // FLTK_USE_WAYLAND
#else
        DropWindow::draw();
#endif
    }

    void MainWindow::set_alpha(int new_alpha)
    {
        int minAlpha = 95;
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

        // \@bug: We must redraw all panels under Wayland/XWayland
        App::ui->uiView->redraw();
        App::ui->uiMenuBar->redraw();
        App::ui->uiTopBar->redraw();
        App::ui->uiBottomBar->redraw();
        App::ui->uiStatusBar->redraw();
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

    void
    setClickThroughWayland(struct wl_surface* surface, int width, int height)
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
#ifdef _WIN32
        HWND win = fl_win32_xid(this);
        setClickThroughWin32(win, enable);
#elif defined(__linux__)

#  ifdef FLTK_USE_X11
        auto dpy = fl_x11_display();
        if (dpy)
        {
            if (enable)
            {
                Fl::add_system_handler((Fl_System_Handler)x11_handler, this);
            }
            else
            {
                Fl::remove_system_handler((Fl_System_Handler)x11_handler);
            }
        }
#  endif
#  ifdef FLTK_USE_WAYLAND
        auto wldpy = fl_wl_display();
        if (wldpy)
        {
            Fl_Window* window = Fl::first_window();
            for (; window; window = Fl::next_window(window))
                setClickThroughWayland(window, enable);
        }
#  endif // FLTK_USE_WAYLAND

#endif // __linux__
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
                      "to pass through to windows below it.\n\n"
                      "Give window focus to disable."),
                    _("Stop"), _("Continue"), NULL, NULL);
            }
            else
            {
                ok = mrv::fl_choice(
                    _("This will disable events for mrv2, allowing them "
                      "to pass through to windows below it.\n\n"
                      "Give window focus (click on the taskbar icon)\n"
                      "to disable."),
                    _("Stop"), _("Continue"), NULL, NULL);
            }
            if (!ok)
                return;
        }

        click_through = value;

        if (value)
        {
            LOG_STATUS(_("Window click through ON"));

            if (win_alpha >= 250)
            {
                set_alpha(0);
            }
            ignoreFocus = true;
        }
        else
        {
            LOG_STATUS(_("Window click through OFF"));
            set_alpha(255);
        }

        always_on_top(click_through, true);

        setClickThrough(value);
    }

} // namespace mrv
