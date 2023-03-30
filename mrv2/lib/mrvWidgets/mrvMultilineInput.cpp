// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <math.h>
#include <iostream>

#include <FL/Fl.H>
#include <FL/fl_draw.H>
#include <FL/names.h>

#include "mrvMultilineInput.h"
#include "mrViewer.h"

namespace mrv
{

    const int kCrossSize = 10;

    /* If called with maxbuf==0, use an internally allocated buffer and enlarge
     it as needed. Otherwise, use buf as buffer but don't go beyond its length
     of maxbuf.
    */
    static const char* expand_text_(
        const char* from, char*& buf, int maxbuf, double maxw, int& n,
        double& width, int wrap, int draw_symbols)
    {
        char* e = buf + (maxbuf - 4);
        double w = 0;
        static int l_local_buff = 500;
        static char* local_buf =
            (char*)malloc(l_local_buff); // initial buffer allocation
        if (maxbuf == 0)
        {
            buf = local_buf;
            e = buf + l_local_buff - 4;
        }
        char* o = buf;
        char* word_end = o;
        const char* word_start = from;

        const char* p = from;
        for (;; p++)
        {

            int c = *p & 255;

            if (!c || c == ' ' || c == '\n')
            {
                if (!c)
                    break;
                else if (c == '\n')
                {
                    p++;
                    break;
                }
                word_start = p + 1;
            }

            if (o > e)
            {
                if (maxbuf)
                    break;                        // don't overflow buffer
                l_local_buff += int(o - e) + 200; // enlarge buffer
                buf = (char*)realloc(local_buf, l_local_buff);
                e = buf + l_local_buff - 4; // update pointers to buffer content
                o = buf + (o - local_buf);
                word_end = buf + (word_end - local_buf);
                local_buf = buf;
            }

            if (c == '\t')
            {
                for (c = fl_utf_nb_char((uchar*)buf, (int)(o - buf)) % 8;
                     c < 8 && o < e; c++)
                    *o++ = ' ';
            }
            else if (c < ' ' || c == 127)
            { // ^X
                *o++ = '^';
                *o++ = c ^ 0x40;
            }
            else
            {
                *o++ = c;
            }
        }

        width = w + fl_width(word_end, (int)(o - word_end));
        *o = 0;
        n = (int)(o - buf);
        return p;
    }

    void measure(const char* str, int& w, int& h, int draw_symbols = 0)
    {
        h = fl_height();
        if (!str || !*str)
        {
            w = 0;
            return;
        }
        const char* p;
        const char* e;
        char* linebuf = NULL;
        int buflen;
        int lines;
        double width = 0;
        int W = 0;

        for (p = str, lines = 1; p;)
        {
            e = mrv::expand_text_(p, linebuf, 0, w, buflen, width, 0, 0);
            if ((int)ceil(width) > W)
                W = (int)ceil(width);
            if (*(e - 1) == '\n')
                lines++;
            if (!*e)
                break;
            p = e;
        }

        w = W;
        h = lines * h;
    }

    MultilineInput::MultilineInput(int X, int Y, int W, int H, char* L) :
        Fl_Multiline_Input(X, Y, W, H, L)
    {
        box(FL_ROUNDED_BOX);
        color(FL_FREE_COLOR);
        wrap(false);
        tab_nav(false);
        textfont(FL_HELVETICA);
        textsize(30);
        fl_font(textfont(), textsize());
        pos.x = X;
        pos.y = Y;
    };

    void MultilineInput::draw()
    {
        const Fl_Boxtype b = box();
        draw_box(b, color());
        const int X = x() + Fl::box_dx(b) + kCrossSize;
        const int Y = y() + Fl::box_dy(b) + kCrossSize;
        const int W = w() - Fl::box_dw(b);
        const int H = h() - Fl::box_dh(b);
        Fl_Input_::drawtext(X, Y, W, H);

        // if widget has focus, draw an X on corner
        if (Fl::focus() == this)
        {
            const char* text = value();
            if (text && strlen(text) > 0)
                fl_color(0, 255, 0);
            else
                fl_color(255, 0, 0);
            fl_line_style(FL_SOLID, 3);
            fl_line(x(), y(), x() + kCrossSize, y() + kCrossSize);
            fl_line(x() + kCrossSize, y(), x(), y() + kCrossSize);
            fl_line_style(0);
        }
    }

    void MultilineInput::recalc()
    {
        int W = 0, H = 0;
        fl_font(textfont(), textsize());
        mrv::measure(value(), W, H);
        W += kCrossSize * 2 + 10; // use 10 for padding and cursor.
        H += kCrossSize * 2;
        size(W, H);
        redraw();
    }

    int MultilineInput::textsize() const
    {
        return Fl_Multiline_Input::textsize();
    }

    void MultilineInput::textsize(int x)
    {
        Fl_Multiline_Input::textsize(x);
        recalc();
    }

    int MultilineInput::accept()
    {
        Viewport* view = static_cast< Viewport* >(window());
        return view->acceptMultilineInput();
    }

    MultilineInput::~MultilineInput() {}

    int MultilineInput::handle(int e)
    {
        switch (e)
        {
        case FL_PUSH:
        {
            if (Fl::event_button1())
            {
                if (Fl::event_inside(x(), y(), kCrossSize, kCrossSize))
                {
                    Fl_Widget_Tracker wp(this); // watch myself
                    return accept();
                }
                // Adjust Fl::event_x() to compensate for cross.
                // This is needed so cursor is placed properly in text window.
                if (Fl::event_inside(this))
                {
                    Fl::e_x -= kCrossSize;
                    Fl::e_y -= kCrossSize;
                }
            }
            break;
        }
        case FL_ENTER:
        case FL_MOVE:
        {
            if (Fl::event_inside(x(), y(), kCrossSize, kCrossSize))
            {
                window()->cursor(FL_CURSOR_ARROW);
                return 1;
            }
            break;
        }
        case FL_LEAVE:
            window()->cursor(FL_CURSOR_CROSS);
            return 1;
        }

        const int ret = Fl_Multiline_Input::handle(e);
        if (e == FL_KEYBOARD)
        {
            int rawkey = Fl::event_key();
            // If user pressed ESC, cancel the text input.
            if (rawkey == FL_Escape)
            {
                value("");
                Fl_Widget_Tracker wp(this); // watch myself
                accept();                   // With no text accept will return 0
                return 1;
            }
            recalc();
            return 1;
        }
        return ret;
    }

} // namespace mrv
