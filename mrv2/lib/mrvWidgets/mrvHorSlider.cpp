// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <iostream>
#include <cmath>

#include <FL/fl_draw.H>
#include <FL/Fl_Input_.H>

#include "mrvCore/mrvI8N.h"

#include "mrvHorSlider.h"

#include "mrvFunctional.h"

namespace mrv
{

    HorSlider::HorSlider(int X, int Y, int W, int H, const char* L) :
        Fl_Group(X, Y, W, H),
        default_value_(0.0)
    {
        begin();

        int Xoffset = 0, Yoffset = 0;

        if (L)
        {
            fl_font(FL_HELVETICA, 12);
            fl_measure(L, Xoffset, Yoffset);

            if (Xoffset < 70)
                Xoffset = 70;
            else if (Xoffset < 90)
                Xoffset = 90;
        }

        auto uiValueW = new Widget<Fl_Float_Input>(X + Xoffset, Y, 50, H, L);
        uiValue = uiValueW;
        uiValue->color((Fl_Color)0xf98a8a800);
        uiValue->textcolor(FL_BLACK);
        uiValue->labelsize(12);
        auto uiSliderW = new Widget<Fl_Hor_Slider>(
            X + Xoffset + 50, Y, W - Xoffset - 50 - 13, H);
        uiSlider = uiSliderW;
        uiSlider->when(FL_WHEN_CHANGED);
        auto uiResetW = new Widget<Fl_Button>(X + W - 13, Y, 10, H, "@-31+");
        uiReset = uiResetW;
        uiReset->box(FL_NO_BOX);
        uiReset->tooltip(_("Reset to default value"));
        end();
        resizable(uiSlider);

        uiSliderW->callback(
            [=](auto s)
            {
                double v = s->value();
                format_value(v);
                do_callback();
            });

        uiValueW->callback(
            [=](auto o)
            {
                double v = atof(o->value());
                uiSlider->value(v);
                do_callback();
            });

        uiResetW->callback(
            [=](auto o)
            {
                uiSlider->value(default_value_);
                uiSlider->do_callback();
            });

        range(0.F, 10.F);
        step(0.1F);
    }

    void HorSlider::format_value( double v ) noexcept
    {
        char buf[32];
        if ( uiValue->input_type() & FL_INT_INPUT )
        {
            snprintf(buf, 32, "% 6d", static_cast<int>(v));
        }
        else
        {
            snprintf(buf, 32, "%6.2f", v);
        }
        uiValue->value(buf);
    }
    
    void HorSlider::default_value(double d) noexcept
    {
        default_value_ = d;
        uiSlider->value(d);
        uiSlider->do_callback();
    }

    void HorSlider::minimum(double mn) noexcept
    {
        uiSlider->minimum(mn);
        check_size();
    }

    void HorSlider::maximum(double mx) noexcept
    {
        uiSlider->maximum(mx);
        check_size();
    }

    void HorSlider::check_size() noexcept
    {
        double mn = uiSlider->minimum();
        double mx = uiSlider->maximum();
        if (mn < -1000 || mx > 1000)
        {
            uiValue->size(70, uiValue->h());
            uiSlider->resize(
                x() + uiValue->x() + 70, uiSlider->y(),
                w() - 70 - 13 - uiValue->x(), uiSlider->h());
        }
    }

    void HorSlider::range(double mn, double mx) noexcept
    {
        uiSlider->bounds(mn, mx);
        check_size();
    }

    void HorSlider::step(double s) noexcept
    {
        if (s == 1.0F)
        {
            uiValue->input_type(FL_INT_INPUT);
            uiValue->redraw();
        }
        uiSlider->step(s);
    }

    void HorSlider::value(double x) noexcept
    {
        if (x > uiSlider->maximum())
            x = uiSlider->maximum();
        if (x < uiSlider->minimum())
            x = uiSlider->minimum();
        uiSlider->value(x);
        format_value(x);
    }

    double HorSlider::value() const noexcept
    {
        return uiSlider->value();
    }

    void HorSlider::tooltip(const char* t)
    {
        uiSlider->tooltip(t);
        uiValue->tooltip(t);
    }
} // namespace mrv
