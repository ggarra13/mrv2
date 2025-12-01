
#include <FL/platform.H>
#include <mrvMainWindow.h>

// X11 has conflicting macros, so we include them last
#if defined(FLTK_USE_X11)
#    include <unistd.h>
#    include <sys/time.h>
#    include <X11/Xlib.h>

namespace
{
    void mrv2_XSendEvent(
        Display* display, Window target_window, Bool value, int mask,
        XEvent* event)
    {
        XSendEvent(display, target_window, value, mask, event);
        XFlush(display);
    }
}

#endif

#if !defined(__APPLE__)
    void MainWindow::always_on_top()
    {
#    if defined(_WIN32)
            HWND action;
            action = HWND_TOPMOST;
            // Microsoft (R) Windows(TM)
            SetWindowPos(
                fl_win32_xid(this), action, NULL, NULL, NULL, NULL,
                SWP_NOMOVE | SWP_NOSIZE);
#    elif defined(__linux__)
            Display* display = fl_x11_display();
            if (display)
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
            ev.xclient.data.l[0] = 1;
            ev.xclient.data.l[1] = net_wm_state_above;
            ev.xclient.data.l[2] = 0;
            mrv2_XSendEvent(
                display, DefaultRootWindow(display), False,
                SubstructureNotifyMask | SubstructureRedirectMask, &ev);
        }
            // Wayland does not allow always on top
#    endif // FLTK_USE_X11
    }
#endif
