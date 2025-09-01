// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include "mrvTimecode.h"

#include <FL/fl_string_functions.h>
#include <FL/fl_utf8.h>
#include <FL/Fl.H>

#include <stdlib.h>
#include <ctype.h>
#include <string.h>

namespace mrv
{
    struct Timecode::Private
    {
        otime::RationalTime value = tl::time::invalidTime;
        TimeUnits units = TimeUnits::Timecode;
    };

    inline void Timecode::_textUpdate() noexcept
    {
        TLRENDER_P();
        char buf[24];
        timeToText(buf, p.value, p.units);
        value(buf);
    }

    Timecode::Timecode(int X, int Y, int W, int H, const char* L) :
        Fl_Input(X, Y, W, H, L),
        _p(new Private)
    {
        color((Fl_Color)0xf98a8a800);
        cursor_color(FL_RED);
        textcolor(FL_BLACK);
        _textUpdate();
    }

    Timecode::~Timecode() {}

    const otime::RationalTime& Timecode::time() const
    {
        return _p->value;
    }

    TimeUnits Timecode::units() const
    {
        return _p->units;
    }

    void Timecode::setTime(const otime::RationalTime& value) noexcept
    {
        TLRENDER_P();
        if (value == p.value)
            return;
        p.value = value;
        _textUpdate();
    }

    void Timecode::setUnits(TimeUnits units)
    {
        TLRENDER_P();
        if (units == p.units)
            return;
        p.units = units;
        _textUpdate();
    }

    int Timecode::handle(int e)
    {
        TLRENDER_P();
        
        switch(e)
        {
        case FL_KEYBOARD:
        {
            unsigned rawkey = Fl::event_key();
            if (rawkey == FL_Enter ||
                rawkey == FL_KP_Enter ||
                rawkey == FL_Tab)
            {
                double v;
                switch(p.units)
                {
                case TimeUnits::Frames:
                    v = eval(value());
                    p.value = otime::RationalTime(v, p.value.rate());
                    break;
                case TimeUnits::Seconds:
                    v = eval(value());
                    p.value = otime::RationalTime(v, 1.0);
                    break;
                default:
                    break;
                }
                _textUpdate();
                do_callback();
                return 1;
            }
            break;
        }
        default:
            break;
        }
        
        return Fl_Input::handle(e);
    }
    
    /**
       Evaluate a formula into a double, recursive part.
       \param s remaining text in this formula, must return a pointer to the next
       character that will be interpreted.
       \param prio priority of current operation
       \return the value so far
    */
    double Timecode::eval(uchar *&s, int prio) const {
        double v = 0;
        int sgn = 1, dec = 0; 
        uchar c = *s++;

        // check for end of text
        if (c==0) { 
            s--; return sgn*v;
        }

        // check for unary operator
        if (c=='-') { sgn = -1; c = *s++; }
        else if (c=='+') { sgn = 1; c = *s++; }

        if (_p->units == TimeUnits::Seconds && (c == '.' || c == ','))
        {
            c = *s++;
            dec = 1;
        }
        
        // read value or bracketed term
        if (c==0) {
            s--; return sgn*v;
        } else if (c>='0' && c<='9') {
            if (dec) {
                int d = 10;
                while (c>='0' && c<='9') {
                    v = v + double(c-'0') / d;
                    c = *s++;
                    d *= 10;
                }
            }
            else {
                // numeric value 
                while (c>='0' && c<='9') {
                    v = v*10 + (c-'0');
                    c = *s++;
                }
            }
            if (c == '.' || c == ',') {
                --s;
                v += eval(s, 5);
            }
        } else if (c=='(') {
            // opening bracket
            v = eval(s, 5);
        } else {
            return sgn*v; // syntax error
        }
        if (sgn==-1) v = -v;

        // Now evaluate all following binary operators
        for (;;) {
            if (c==0) {
                s--;
                return v;
            } else if (c=='+' || c=='-') {
                if (prio<=4) { s--; return v; }
                if (c=='+') { v += eval(s, 4); }
                else if (c=='-') { v -= eval(s, 4); }
            } else if (c=='*' || c=='/') {
                if (prio<=3) { s--; return v; }
                if (c=='*') { v *= eval(s, 3); }
                else if (c=='/') {
                    int x = eval(s, 3);
                    if (x!=0) // if x is zero, don't divide
                        v /= x;
                }
            } else if (c==')') {
                return v;
            } else {
                return v; // syntax error or final parsing
            }
            c = *s++;
        }
        return v;
    }

/**
   Evaluate a formula into an integer.

   The Formula_Input widget includes a formula interpreter that allows you
   to evaluate a string containing a mathematical formula and obtain the result
   as an integer. The interpreter supports unary plus and minus, basic integer
   math operations (such as addition, subtraction, multiplication, and division),
   and brackets. It also allows you to define a list of variables by name and use
   them in the formula. The interpreter does not perform error checking, so it is
   assumed that the formula is entered correctly.

   \param s formula as a C string
   \return the calculated value
*/
    double Timecode::eval(const char *s) const
    {
        // duplicate the text, so we can modify it
        uchar *buf = (uchar*)fl_strdup(s);
        uchar *src = buf, *dst = buf;
        // remove all whitespace to make the parser easier
        for (;;) {
            uchar c = *src++;
            if (c==' ' || c=='\t') continue;
            *dst++ = c;
            if (c==0) break;
        }
        src = buf;
        // now jump into the recursion
        double ret = eval(src, 5);
        ::free(buf);
        return ret;
    }
} // namespace mrv
