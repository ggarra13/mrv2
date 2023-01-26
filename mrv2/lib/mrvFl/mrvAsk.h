// SPDX-License-Identifier: BSD-3-Clause
// mrv2 (mrViewer2)
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include "FL/Enumerations.H"


class Fl_Widget;


namespace mrv {



#  ifdef __GNUC__
/* the GNUC-specific attribute appearing below in prototypes with a variable list of arguments
   helps detection of mismatches between format string and argument list at compilation time */
#    define __fl_attr(x) __attribute__ (x)
#  else
#    define __fl_attr(x)
#  endif // __GNUC__

// fl_ask() is deprecated since it uses "Yes" and "No" for the buttons,
// which does not conform to the current FLTK Human Interface Guidelines.
// Use fl_choice() instead with the appropriate verbs instead.
    FL_EXPORT int fl_choice(const char *q,const char *b0,const char *b1,const char *b2,...) __fl_attr((__format__ (__printf__, 1, 5)));
    FL_EXPORT const char *fl_input(const char *label, const char *deflt = 0, ...) __fl_attr((__format__ (__printf__, 1, 3)));
    FL_EXPORT void fl_alert(const char *,...) __fl_attr((__format__ (__printf__, 1, 2)));

    FL_EXPORT Fl_Widget *fl_message_icon();
    extern FL_EXPORT Fl_Font fl_message_font_;
    extern FL_EXPORT Fl_Fontsize fl_message_size_;
    inline void fl_message_font(Fl_Font f, Fl_Fontsize s) {
        fl_message_font_ = f; fl_message_size_ = s;}

    FL_EXPORT void fl_message_hotspot(int enable);
    FL_EXPORT int fl_message_hotspot(void);

    FL_EXPORT void fl_message_title(const char *title);
    FL_EXPORT void fl_message_title_default(const char *title);

// pointers you can use to change FLTK to another language:
    extern FL_EXPORT const char* fl_no;
    extern FL_EXPORT const char* fl_yes;
    extern FL_EXPORT const char* fl_ok;
    extern FL_EXPORT const char* fl_cancel;
    extern FL_EXPORT const char* fl_close;

}  // namespace mrv

