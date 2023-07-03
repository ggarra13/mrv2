// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <math.h>
#include <FL/fl_draw.H>
#include <FL/Enumerations.H>
#include <FL/Fl.H>

#include "mrvFl/mrvPreferences.h"

#include "mrvSlider.h"

namespace mrv
{

    static char* printtick(char* buffer, double v)
    {
        if (fabs(v) >= 1)
        {
            snprintf(buffer, 20, "%g", v);
            return buffer;
        }
        else
        {
            snprintf(buffer, 20, "%.3g", v);
            char* p = buffer;
            if (v < 0)
                p++;
            while (p[0] == '0' && p[1])
                p++;
            if (v < 0)
                *--p = '-';
            return p;
        }
    }

    void Slider::draw_ticks(const tl::math::BBox2i& r, int min_spacing)
    {
        int x1, sx1, y1, sy1, x2, y2, dx, dy, w;
        if (horizontal())
        {
            sx1 = x1 = x2 = r.x() + (slider_size() - 1) / 2;
            dx = 1;
            y1 = r.y();
            y2 = r.max.y;
            dy = 0;
            sy1 = y1 + 1 + r.h() / 4;
            w = r.w();
        }
        else
        {
            x1 = r.x();
            x2 = r.max.x;
            dx = 0;
            sx1 = x1 + 1 + r.w() / 4;
            sy1 = y1 = y2 = r.y() + (slider_size() - 1) / 2;
            dy = 1;
            w = r.h();
        }
        if (w <= 0)
            return;
        double A = minimum();
        double B = maximum();
        if (A > B)
        {
            A = B;
            B = minimum();
        }

        if (min_spacing < 1)
            min_spacing = 10; // fix for fill sliders

        double mul = 1; // how far apart tick marks are
        double div = 1;
        int smallmod = 5; // how many tick marks apart "larger" ones are
        int nummod = 10;  // how many tick marks apart numbers are
        int powincr = 10000;

        if (!log())
        {
            double derivative = (B - A) * min_spacing / w;
            if (derivative < step())
                derivative = step();
            while (mul * 5 <= derivative)
                mul *= 10;
            while (mul > derivative * 2 * div)
                div *= 10;
            if (derivative * div > mul * 2)
            {
                mul *= 5;
                smallmod = 2;
            }
            else if (derivative * div > mul)
            {
                mul *= 2;
                nummod = 5;
            }
        }
        else if (A > 0)
        {
            // log slider
            while (mul * 5 <= A)
                mul *= 10;
            while (mul > A * 2 * div)
                div *= 10;
            powincr = 10;
            double d = exp(min_spacing * ::log(B / A) / w * 3);
            if (d >= 5)
            {
                mul *= 10;
                smallmod = nummod = 1;
                powincr = 1;
            }
            else if (d >= 2)
            {
                mul *= 5;
                smallmod = powincr = nummod = 2;
            }
        }
        else
        {
            // squared slider, derivative at edge is zero, use value at 1 pixel
            double derivative = B * min_spacing * min_spacing / (w * w);
            if (A < 0)
                derivative *= 4;
            if (derivative < step())
                derivative = step();
            while (mul < derivative)
                mul *= 10;
            while (mul >= 10 * derivative * div)
                div *= 10;
            powincr = 10;
            // if (derivative > num) {num *= 5; smallmod = powincr = nummod =
            // 2;}
        }

        fl_push_clip(r.x(), r.y(), r.w(), r.h());

        Fl_Color textcolor = this->labelcolor();
        if (Preferences::schemes.name == "Black")
        {
            _tick_color = fl_rgb_color(70, 70, 70);
        }
        Fl_Color linecolor = _tick_color;

        fl_color(linecolor);
        fl_font(fl_font(), labelsize());

        float yt = horizontal() ? y1 + fl_size() - fl_descent() : y1 - 1;
        double v;
        char buffer[20];
        char* p;
        int t;
        float x, y;
        for (int n = 0;; n++)
        {
            // every ten they get further apart for log slider:
            if (n > powincr)
            {
                mul *= 10;
                n = (n - 1) / 10 + 1;
            }
            v = mul * n / div;
            if (v >= fabs(A) && v >= fabs(B))
                break;
            if (n % smallmod)
            {
                if (v > A && v < B)
                {
                    t = slider_position(v, w);
                    fl_line(
                        sx1 + dx * t, sy1 + dy * t, x2 + dx * t, y2 + dy * t);
                }
                if (v && -v > A && -v < B)
                {
                    t = slider_position(-v, w);
                    fl_line(
                        sx1 + dx * t, sy1 + dy * t, x2 + dx * t, y2 + dy * t);
                }
            }
            else
            {
                if (v > A && v < B)
                {
                    t = slider_position(v, w);
                    fl_line(x1 + dx * t, y1 + dy * t, x2 + dx * t, y2 + dy * t);
                    if (n % nummod == 0)
                    {
                        p = printtick(buffer, v);
                        x = x1 + dx * t + 1;
                        y = yt + dy * t;
                        if (dx && (x < r.x() + 3 * min_spacing ||
                                   x >= r.max.x - 5 * min_spacing))
                            ;
                        else if (
                            dy && (y < r.y() + 5 * min_spacing ||
                                   y >= r.max.y - 3 * min_spacing))
                            ;
                        else
                        {
                            fl_color(textcolor);
                            fl_draw(p, x, y);
                            fl_color(linecolor);
                        }
                    }
                }
                if (v && -v > A && -v < B)
                {
                    t = slider_position(-v, w);
                    fl_line(x1 + dx * t, y1 + dy * t, x2 + dx * t, y2 + dy * t);
                    if (n % nummod == 0)
                    {
                        p = printtick(buffer, v);
                        x = x1 + dx * t + 1;
                        y = yt + dy * t;
                        if (dx && (x < r.x() + 3 * min_spacing ||
                                   x >= r.max.x - 5 * min_spacing))
                            ;
                        else if (
                            dy && (y < r.y() + 5 * min_spacing ||
                                   y >= r.max.y - 3 * min_spacing))
                            ;
                        else
                        {
                            fl_color(textcolor);
                            fl_draw(p, x, y);
                            fl_color(linecolor);
                        }
                    }
                }
            }
        }

        // draw the end ticks with numbers:

        v = minimum();
        t = slider_position(v, w);
        fl_line(x1 + dx * t, y1 + dy * t, x2 + dx * t, y2 + dy * t);
        p = printtick(buffer, v);
        x = x1 + dx * t + 1;
        y = yt + dy * t;
        fl_color(textcolor);
        fl_draw(p, x, y);
        fl_color(linecolor);

        v = maximum();
        t = slider_position(v, w);
        fl_line(x1 + dx * t, y1 + dy * t, x2 + dx * t, y2 + dy * t);
        p = printtick(buffer, v);
        x = x1 + dx * t + 1;
        if (dx)
        {
            float w = fl_width(p);
            if (x + w > r.max.x)
                x -= 2 + w;
        }
        y = yt + dy * t;
        if (dy)
            y += fl_size();
        fl_color(textcolor);
        fl_draw(p, x, y);

        fl_pop_clip();
    }

    int Slider::slider_position(double value, int w)
    {
        double A = minimum();
        double B = maximum();
        if (B == A)
            return 0;
        bool flip = B < A;
        if (flip)
        {
            A = B;
            B = minimum();
        }
        if (!horizontal())
            flip = !flip;
        // if both are negative, make the range positive:
        if (B <= 0)
        {
            flip = !flip;
            double t = A;
            A = -B;
            B = -t;
            value = -value;
        }
        double fraction;
        if (!log())
        {
            // linear slider
            fraction = (value - A) / (B - A);
        }
        else if (A > 0)
        {
            // logatithmic slider
            if (value <= A)
                fraction = 0;
            else
                fraction = (::log(value) - ::log(A)) / (::log(B) - ::log(A));
        }
        else if (A == 0)
        {
            // squared slider
            if (value <= 0)
                fraction = 0;
            else
                fraction = sqrt(value / B);
        }
        else
        {
            // squared signed slider
            if (value < 0)
                fraction = (1 - sqrt(value / A)) * .5;
            else
                fraction = (1 + sqrt(value / B)) * .5;
        }
        if (flip)
            fraction = 1 - fraction;
        w -= slider_size();
        if (w <= 0)
            return 0;
        if (fraction >= 1)
            return w;
        else if (fraction <= 0)
            return 0;
        else
            return int(fraction * w + .5);
    }

    double Slider::position_value(int X, int w)
    {
        w -= slider_size();
        if (w <= 0)
            return minimum();
        double A = minimum();
        double B = maximum();
        bool flip = B < A;
        if (flip)
        {
            A = B;
            B = minimum();
        }
        if (!horizontal())
            flip = !flip;
        if (flip)
            X = w - X;
        double fraction = double(X) / w;
        if (fraction <= 0)
            return A;
        if (fraction >= 1)
            return B;
        // if both are negative, make the range positive:
        flip = (B <= 0);
        if (flip)
        {
            double t = A;
            A = -B;
            B = -t;
            fraction = 1 - fraction;
        }
        double value;
        double derivative;
        if (!log())
        {
            // linear slider
            value = fraction * (B - A) + A;
            derivative = (B - A) / w;
        }
        else if (A > 0)
        {
            // log slider
            double d = (::log(B) - ::log(A));
            value = exp(fraction * d + ::log(A));
            derivative = value * d / w;
        }
        else if (A == 0)
        {
            // squared slider
            value = fraction * fraction * B;
            derivative = 2 * fraction * B / w;
        }
        else
        {
            // squared signed slider
            fraction = 2 * fraction - 1;
            if (fraction < 0)
                B = A;
            value = fraction * fraction * B;
            derivative = 4 * fraction * B / w;
        }
        // find nicest multiple of 10,5, or 2 of step() that is close to 1
        // pixel:
        if (step() && derivative > step())
        {
            double w = log10(derivative);
            double l = ceil(w);
            int num = 1;
            int i;
            for (i = 0; i < l; i++)
                num *= 10;
            int denom = 1;
            for (i = -1; i >= l; i--)
                denom *= 10;
            if (l - w > 0.69897)
                denom *= 5;
            else if (l - w > 0.30103)
                denom *= 2;
            value = floor(value * denom / num + .5) * num / denom;
        }
        if (flip)
            return -value;
        return value;
    }

    int Slider::handle(int event)
    {
        if (slider_type() != kLOG)
            return Fl_Slider::handle(event);

        tl::math::BBox2i r(x(), y(), w(), h());

        switch (event)
        {
        case FL_FOCUS:
        case FL_UNFOCUS:
            damage(FL_DAMAGE_ALL);
            redraw();
            return 1;
        case FL_PUSH:
            damage(FL_DAMAGE_EXPOSE); // DAMAGE_HIGHLIGHT
            redraw();
            handle_push();
        case FL_DRAG:
        {
            // figure out the space the slider moves in and where the event is:
            int w, mx;
            if (horizontal())
            {
                w = r.w();
                mx = Fl::event_x() - r.x();
            }
            else
            {
                w = r.h();
                mx = Fl::event_y() - r.y();
            }
            if (w <= slider_size())
                return 1;
            static int offcenter;
            int X = slider_position(value(), w);
            if (event == FL_PUSH)
            {
                offcenter = mx - X;
                // we are done if they clicked on the slider:
                if (offcenter >= (slider_size() ? 0 : -8) &&
                    offcenter <= slider_size())
                    return 1;
                if (Fl::event_button() > FL_LEFT_MOUSE)
                {
                    // Move the near end of the slider to the cursor.
                    // This is good for scrollbars.
                    offcenter = (offcenter < 0) ? 0 : slider_size();
                }
                else
                {
                    // Center the slider under the cursor, what most toolkits do
                    offcenter = slider_size() / 2;
                }
            }
            double v;
        RETRY:
            X = mx - offcenter;
            if (X < 0)
            {
                X = 0;
                offcenter = mx;
                if (offcenter < 0)
                    offcenter = 0;
            }
            else if (X > (w - slider_size()))
            {
                X = w - slider_size();
                offcenter = mx - X;
                if (offcenter > slider_size())
                    offcenter = slider_size();
            }
            v = position_value(X, w);
            handle_drag(v);
            // make sure a click outside the sliderbar moves it:
            if (event == FL_PUSH && value() == previous_value())
            {
                offcenter = slider_size() / 2;
                event = FL_DRAG;
                goto RETRY;
            }
            return 1;
        }
        case FL_RELEASE:
            handle_release();
            redraw(); // DAMAGE_HIGHLIGHT);
            return 1;
        case FL_KEYBOARD:
            // Only arrows in the correct direction are used.  This allows the
            // opposite arrows to be used to navigate between a set of parellel
            // sliders.
            switch (Fl::event_key())
            {
            case FL_Up:
            case FL_Down:
                if (horizontal())
                    return 0;
                break;
            case FL_Left:
            case FL_Right:
                if (!horizontal())
                    return 0;
            }
        default:
            return Fl_Valuator::handle(event);
        }
        return 1;
    }

    void Slider::draw()
    {
        draw_box();

        tl::math::BBox2i r(
            x() + Fl::box_dx(box()), y() + Fl::box_dy(box()),
            w() - Fl::box_dw(box()), h() - Fl::box_dh(box()));
        draw_ticks(r, 10);

        int X = r.x() + slider_position(value(), r.w() - 10);
        int Y = r.y();
        int W = 10;
        int H = r.h();
        // Fl_Color c = fl_rgb_color(255, 255, 255); // fl_lighter(color());
        Fl_Color c = fl_lighter(color());
        draw_box(FL_EMBOSSED_BOX, X, Y, W, H, c);
        clear_damage();
    }

} // namespace mrv
