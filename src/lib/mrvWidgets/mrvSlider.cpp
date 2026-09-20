// BSD 3-Clause License
//
// Copyright (c) 2024, mrv2 project contributors.
// All rights reserved.
//
// ---------------------------------------------------------------------
// See mrvSlider.h for a description of what this widget does. The value
// <-> pixel mapping supports three modes, chosen from the range
// [minimum(), maximum()]:
//
//   * linear             -- used whenever slider_type() == kNORMAL.
//   * logarithmic        -- kLOG with a strictly positive minimum().
//   * squared / signed    -- kLOG with minimum() <= 0 (gives extra
//                            resolution near zero on both sides).
//
// This file is an original implementation and does not derive from
// FLTK2's fltk::Slider sources.
// ---------------------------------------------------------------------

#include <cmath>
#include <cstdio>
#include <cstring>

#include <FL/Fl.H>
#include <FL/Enumerations.H>
#include <FL/fl_draw.H>

#include "mrvFl/mrvPreferences.h"

#include "mrvSlider.h"

namespace
{
    using tl::math::Box2i;

    // Describes the drawable track as a start point, an end point, and a
    // unit step (dx, dy) between them, so that the tick-drawing code can
    // stay orientation-agnostic.
    struct TrackGeometry
    {
        int originX, originY; // near edge of the tick line
        int labelX, labelY;   // near edge of the (shorter) label tick line
        int farX, farY;       // far edge of the tick line
        int stepX, stepY;     // unit vector along the track, in pixels
        int length;           // track length in pixels
    };

    TrackGeometry trackGeometry(const Box2i& r, int knobSize, bool horiz)
    {
        TrackGeometry g{};
        if (horiz)
        {
            g.originX = g.labelX = g.farX = r.x() + (knobSize - 1) / 2;
            g.stepX = 1;
            g.stepY = 0;
            g.originY = r.y();
            g.farY = r.max.y;
            g.labelY = g.originY + 1 + r.h() / 4;
            g.length = r.w();
        }
        else
        {
            g.originX = r.x();
            g.farX = r.max.x;
            g.labelX = g.originX + 1 + r.w() / 4;
            g.stepX = 0;
            g.stepY = 1;
            g.originY = g.labelY = g.farY = r.y() + (knobSize - 1) / 2;
            g.length = r.h();
        }
        return g;
    }

    // Renders a tick value compactly ("1000", "0.005", "-3.2" ...),
    // trimming redundant leading zeroes the way a ruler label would.
    const char* formatTickLabel(char* buffer, size_t bufSize, double v)
    {
        if (std::fabs(v) >= 1.0)
        {
            std::snprintf(buffer, bufSize, "%g", v);
            return buffer;
        }
        std::snprintf(buffer, bufSize, "%.3g", v);
        char* p = buffer;
        bool negative = v < 0;
        if (negative)
            ++p;
        while (p[0] == '0' && p[1])
            ++p;
        if (negative)
            *--p = '-';
        return p;
    }

    // Parameters that control how far apart tick marks/labels are placed.
    struct TickSpacing
    {
        double mul = 1.0, div = 1.0;
        int minorEvery = 5;   // every Nth tick is drawn "major" (longer)
        int labelEvery = 10;  // every Nth tick gets a printed number
        int growEvery = 10000; // ticks get 10x further apart every N steps
    };

    // Works out a "nice" tick increment (a multiple of 1/2/5 * 10^k) for
    // the given range and mapping so ticks don't end up cluttered or too
    // sparse for the available pixel width.
    TickSpacing computeTickSpacing(
        double lo, double hi, int pixelWidth, int minSpacingPx, double step,
        bool logMapped)
    {
        TickSpacing s;
        if (!logMapped)
        {
            double perPixel = (hi - lo) * minSpacingPx / pixelWidth;
            if (perPixel < step)
                perPixel = step;
            while (s.mul * 5 <= perPixel)
                s.mul *= 10;
            while (s.mul > perPixel * 2 * s.div)
                s.div *= 10;
            if (perPixel * s.div > s.mul * 2)
            {
                s.mul *= 5;
                s.minorEvery = 2;
            }
            else if (perPixel * s.div > s.mul)
            {
                s.mul *= 2;
                s.labelEvery = 5;
            }
        }
        else if (lo > 0)
        {
            while (s.mul * 5 <= lo)
                s.mul *= 10;
            while (s.mul > lo * 2 * s.div)
                s.div *= 10;
            s.growEvery = 10;
            double ratioPerTick =
                std::exp(minSpacingPx * std::log(hi / lo) / pixelWidth * 3);
            if (ratioPerTick >= 5)
            {
                s.mul *= 10;
                s.minorEvery = s.labelEvery = 1;
                s.growEvery = 1;
            }
            else if (ratioPerTick >= 2)
            {
                s.mul *= 5;
                s.minorEvery = s.growEvery = s.labelEvery = 2;
            }
        }
        else
        {
            // squared mapping: slope is zero at the edge, so estimate it
            // from the value one pixel in instead.
            double perPixel =
                hi * minSpacingPx * minSpacingPx / (double(pixelWidth) * pixelWidth);
            if (lo < 0)
                perPixel *= 4;
            if (perPixel < step)
                perPixel = step;
            while (s.mul < perPixel)
                s.mul *= 10;
            while (s.mul >= 10 * perPixel * s.div)
                s.div *= 10;
            s.growEvery = 10;
        }
        return s;
    }

    // Value <-> unit-fraction mapping shared by slider_position() and
    // position_value(). "unit fraction" is 0 at minimum()/maximum()
    // (whichever is nearer the track origin) and 1 at the far end.
    double valueToUnitFraction(double value, double lo, double hi, bool logMapped)
    {
        if (!logMapped)
            return (value - lo) / (hi - lo);
        if (lo > 0)
            return value <= lo ? 0.0
                                : (std::log(value) - std::log(lo)) /
                                      (std::log(hi) - std::log(lo));
        if (lo == 0)
            return value <= 0.0 ? 0.0 : std::sqrt(value / hi);
        // signed squared mapping, centered at 0
        return value < 0 ? (1.0 - std::sqrt(value / lo)) * 0.5
                          : (1.0 + std::sqrt(value / hi)) * 0.5;
    }

} // namespace

namespace mrv
{

    void Slider::draw_ticks(const Box2i& r, int minSpacingPx)
    {
        const bool horiz = horizontal();
        const TrackGeometry g = trackGeometry(r, slider_size(), horiz);
        if (g.length <= 0)
            return;

        double lo = minimum();
        double hi = maximum();
        if (lo > hi)
            std::swap(lo, hi);

        if (minSpacingPx < 1)
            minSpacingPx = 10; // fallback for zero-thickness "fill" sliders

        const TickSpacing sp = computeTickSpacing(
            lo, hi, g.length, minSpacingPx, step(), log());

        fl_push_clip(r.x(), r.y(), r.w(), r.h());

        if (Preferences::schemes.name == "Black")
            m_tickColor = fl_rgb_color(70, 70, 70);
        const Fl_Color lineColor = m_tickColor;
        const Fl_Color textColor = labelcolor();

        fl_color(lineColor);
        fl_font(fl_font(), labelsize());
        const float labelBaselineOffset =
            horiz ? g.originY + fl_size() - fl_descent() : g.originY - 1;

        char buf[20];

        auto drawOneTick = [&](double v, bool major)
        {
            if (!(v > lo && v < hi))
                return;
            const int t = slider_position(v, g.length);
            if (major)
            {
                fl_line(
                    g.originX + g.stepX * t, g.originY + g.stepY * t,
                    g.farX + g.stepX * t, g.farY + g.stepY * t);
            }
            else
            {
                fl_line(
                    g.labelX + g.stepX * t, g.labelY + g.stepY * t,
                    g.farX + g.stepX * t, g.farY + g.stepY * t);
            }
        };

        auto drawOneLabel = [&](double v)
        {
            const int t = slider_position(v, g.length);
            const char* text = formatTickLabel(buf, sizeof(buf), v);
            const float lx = g.originX + g.stepX * t + 1;
            const float ly = labelBaselineOffset + g.stepY * t + fl_descent();
            const bool tooCloseX =
                g.stepX && (lx < r.x() + 3 * minSpacingPx ||
                            lx >= r.max.x - 5 * minSpacingPx);
            const bool tooCloseY =
                g.stepY && (ly < r.y() + 5 * minSpacingPx ||
                            ly >= r.max.y - 3 * minSpacingPx);
            if (tooCloseX || tooCloseY)
                return;
            fl_color(textColor);
            fl_draw(text, lx, ly);
            fl_color(lineColor);
        };

        double mul = sp.mul;
        for (int n = 0;; ++n)
        {
            // Ticks get progressively further apart on log sliders.
            if (n > sp.growEvery)
            {
                mul *= 10;
                n = (n - 1) / 10 + 1;
            }
            const double v = mul * n / sp.div;
            if (v >= std::fabs(lo) && v >= std::fabs(hi))
                break;

            const bool major = (n % sp.minorEvery) == 0;
            drawOneTick(v, major);
            if (v && (-v > lo) && (-v < hi))
                drawOneTick(-v, major);

            if (major && (n % sp.labelEvery) == 0)
            {
                if (v > lo && v < hi)
                    drawOneLabel(v);
                if (v && -v > lo && -v < hi)
                    drawOneLabel(-v);
            }
        }

        // Always label the two endpoints explicitly, even if the loop
        // above landed a regular tick very close to them.
        for (double endpoint : {minimum(), maximum()})
        {
            const int t = slider_position(endpoint, g.length);
            fl_line(
                g.originX + g.stepX * t, g.originY + g.stepY * t,
                g.farX + g.stepX * t, g.farY + g.stepY * t);
            const char* text = formatTickLabel(buf, sizeof(buf), endpoint);
            fl_color(textColor);
            fl_draw(
                text, g.originX + g.stepX * t + 1,
                labelBaselineOffset + g.stepY * t + fl_descent());
            fl_color(lineColor);
        }

        fl_pop_clip();
    }

    int Slider::slider_position(double value, int trackWidth)
    {
        double lo = minimum();
        double hi = maximum();
        bool flip = hi < lo;
        if (flip)
            std::swap(lo, hi);
        if (!horizontal())
            flip = !flip;

        // If the whole range sits at or below zero, mirror it to positive
        // so the log/squared math (which assumes hi > 0) still applies.
        if (hi <= 0)
        {
            flip = !flip;
            std::swap(lo, hi);
            lo = -lo;
            hi = -hi;
            value = -value;
        }

        double fraction = valueToUnitFraction(value, lo, hi, log());
        if (flip)
            fraction = 1.0 - fraction;

        trackWidth -= slider_size();
        if (trackWidth <= 0)
            return 0;
        if (fraction >= 1.0)
            return trackWidth;
        if (fraction <= 0.0)
            return 0;
        return int(fraction * trackWidth + 0.5);
    }

    double Slider::position_value(int pixelOffset, int trackWidth)
    {
        trackWidth -= slider_size();
        if (trackWidth <= 0)
            return minimum();

        double lo = minimum();
        double hi = maximum();
        bool flip = hi < lo;
        if (flip)
            std::swap(lo, hi);
        if (!horizontal())
            flip = !flip;
        if (flip)
            pixelOffset = trackWidth - pixelOffset;

        double fraction = double(pixelOffset) / trackWidth;
        if (fraction <= 0.0)
            return lo;
        if (fraction >= 1.0)
            return hi;

        const bool mirrored = (hi <= 0);
        if (mirrored)
        {
            std::swap(lo, hi);
            lo = -lo;
            hi = -hi;
            fraction = 1.0 - fraction;
        }

        double value, slope;
        if (!log())
        {
            value = fraction * (hi - lo) + lo;
            slope = (hi - lo) / trackWidth;
        }
        else if (lo > 0)
        {
            const double logSpan = std::log(hi) - std::log(lo);
            value = std::exp(fraction * logSpan + std::log(lo));
            slope = value * logSpan / trackWidth;
        }
        else if (lo == 0)
        {
            value = fraction * fraction * hi;
            slope = 2 * fraction * hi / trackWidth;
        }
        else
        {
            double signedFraction = 2 * fraction - 1;
            double edge = signedFraction < 0 ? lo : hi;
            value = signedFraction * signedFraction * edge;
            slope = 4 * signedFraction * edge / trackWidth;
        }

        // Snap to the nicest multiple of 10/5/2 * step() that is close to
        // one pixel of movement, so dragging lands on tidy values.
        if (step() && slope > step())
        {
            const double logSlope = std::log10(slope);
            const double ceilLogSlope = std::ceil(logSlope);
            double numerator = 1.0;
            for (int i = 0; i < ceilLogSlope; ++i)
                numerator *= 10;
            double denominator = 1.0;
            for (int i = -1; i >= ceilLogSlope; --i)
                denominator *= 10;
            const double frac = ceilLogSlope - logSlope;
            if (frac > 0.69897)
                denominator *= 5;
            else if (frac > 0.30103)
                denominator *= 2;
            value = std::floor(value * denominator / numerator + 0.5) *
                    numerator / denominator;
        }

        return mirrored ? -value : value;
    }

    int Slider::handleDrag(int event, const Box2i& r)
    {
        const bool horiz = horizontal();
        const int trackLen = horiz ? r.w() : r.h();
        const int mousePos =
            horiz ? Fl::event_x() - r.x() : Fl::event_y() - r.y();

        if (trackLen <= slider_size())
            return 1;

        static int grabOffset;
        int knobPos = slider_position(value(), trackLen);

        if (event == FL_PUSH)
        {
            grabOffset = mousePos - knobPos;
            // Clicking directly on the knob just starts a drag from here.
            if (grabOffset >= (slider_size() ? 0 : -8) &&
                grabOffset <= slider_size())
                return 1;
            if (Fl::event_button() > FL_LEFT_MOUSE)
                // Non-primary click: snap the near edge of the knob to the
                // cursor (useful for scrollbar-style paging).
                grabOffset = (grabOffset < 0) ? 0 : slider_size();
            else
                // Primary click: center the knob under the cursor.
                grabOffset = slider_size() / 2;
        }

        for (;;)
        {
            int X = mousePos - grabOffset;
            if (X < 0)
            {
                X = 0;
                grabOffset = mousePos < 0 ? 0 : mousePos;
            }
            else if (X > trackLen - slider_size())
            {
                X = trackLen - slider_size();
                grabOffset = mousePos - X;
                if (grabOffset > slider_size())
                    grabOffset = slider_size();
            }

            handle_drag(position_value(X, trackLen));

            // A push that lands outside the knob and doesn't move the
            // value yet should still relocate the knob under the cursor.
            if (event == FL_PUSH && value() == previous_value())
            {
                grabOffset = slider_size() / 2;
                event = FL_DRAG;
                continue;
            }
            return 1;
        }
    }

    int Slider::handle(int event)
    {
        if (slider_type() != kLOG)
            return Fl_Slider::handle(event);

        const Box2i r(x(), y(), w(), h());

        switch (event)
        {
        case FL_FOCUS:
        case FL_UNFOCUS:
            damage(FL_DAMAGE_ALL);
            redraw();
            return 1;

        case FL_PUSH:
            damage(FL_DAMAGE_EXPOSE);
            redraw();
            handle_push();
            return handleDrag(event, r);

        case FL_DRAG:
            return handleDrag(event, r);

        case FL_RELEASE:
            handle_release();
            redraw();
            return 1;

        case FL_KEYBOARD:
            // Only the arrow keys aligned with this slider's axis are
            // consumed, leaving the perpendicular ones free to move focus
            // between a row/column of sliders.
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
                break;
            }
            return Fl_Slider::handle(event);

        default:
            return Fl_Slider::handle(event);
        }
    }

    void Slider::draw()
    {
        draw_box();

        const Box2i r(
            x() + Fl::box_dx(box()), y() + Fl::box_dy(box()),
            w() - Fl::box_dw(box()), h() - Fl::box_dh(box()));
        draw_ticks(r, 10);

        const int knobWidth = 10;
        const int X = r.x() + slider_position(value(), r.w() - knobWidth);
        const Fl_Color knobColor = fl_lighter(color());
        draw_box(FL_EMBOSSED_BOX, X, r.y(), knobWidth, r.h(), knobColor);

        clear_damage();
    }

} // namespace mrv
