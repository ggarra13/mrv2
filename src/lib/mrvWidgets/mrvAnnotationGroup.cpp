// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include "mrvFlmm/Flmm_ColorA_Chooser.h"

#include "mrvWidgets/mrvAnnotationGroup.h"

#include "mrvOS/mrvI8N.h"

#include <FL/Fl_Button.H>
#include <FL/Fl_Window.H>
#include <FL/fl_draw.H>

#include <cstring>
#include <cstdlib>
#include <cstdio>

#define CIRCLE_D 10
#define BUTTON_H 12
#define TITLE_H 16
#define GROUP_MARGIN 4 // compensates for FL_ROUND_BUTTON (?)

namespace mrv
{
    // Enforce layout
    void AnnotationGroup::layout()
    {

        // Size self based on if open() or close()ed
        int gh = _button->h() + (GROUP_MARGIN * 2);
        if (is_open())
            gh += _contents->h(); // include content's height if we're 'open'

        // Note: resizable() set to zero, so this just resizes us, not children.
        Fl_Group::resize(x(), y(), w(), gh);

        // Manage size/position of children
        //    No need to call init_sizes(): resizable(0) is set in ctor,
        //    and we adjust children ourself.
        //
        _button->resize(
            x() + GROUP_MARGIN,       // x always inset (leaves room for ROUND
                                      // box)
            y() + GROUP_MARGIN,       // y always inset ("")
            w() - (GROUP_MARGIN * 2), // width tracks group's w()
            _button->h());            // height fixed

        // Leave _contents->h() alone, we shouldn't change it; Fl_Pack
        // calculates its own height, we don't want to mess that up by changing
        // it. visible() will control whether it's drawn or not. it's seen or
        // not.
        //
        _contents->resize(
            x() + GROUP_MARGIN,                // x always same as button
            y() + _button->h() + GROUP_MARGIN * 2, // y always "below button"
            w() - (GROUP_MARGIN * 2),          // width tracks group's w()
            _contents->h()); // leave height of _contents alone

        // DEBUG

        // printf("--- LAYOUT CHANGED (%s) ------\n", label() ? label() : "(no
        // label)"); printf(" grp: %d,%d,%d,%d\n", x(),y(),w(),h()); printf("
        // but: %d,%d,%d,%d\n", _button->x(), _button->y(), _button->w(),
        // _button->h()); printf("pack: %d,%d,%d,%d\n", _contents->x(),
        // _contents->y(), _contents->w(), _contents->h()); printf("\n");
    }

    void AnnotationGroup::toggle_tab_cb(Fl_Button* w, void* data)
    {
        mrv::AnnotationGroup* g = (mrv::AnnotationGroup*)data;
        if (g->is_open())
            g->close();
        else
            g->open(); // toggle open/close state
    }

    // CTOR
    AnnotationGroup::AnnotationGroup(
        const int x, const int y, const int w, const int h, const char* l) :
        Fl_Group(x, y, w, h, l)
    {

        // Use a border box for now, so we can see our bounds in parent
        // mrv::Pack.
        box(FL_BORDER_BOX);

        // Disable label from being shown; we show the label only in the button.
        labeltype(FL_NO_LABEL);

        // NOTNEEDED Fl_Group::begin();
        //  Button
        _button = new Fl_Button(
            x,       // margin leaves room for FL_ROUND_BOX
            y + GROUP_MARGIN,       // margin leaves room for FL_ROUND_BOX
            w, // width same as group within margin
            BUTTON_H);              // button height fixed size
        _button->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
        _button->labelsize(16);
        _button->box(FL_FLAT_BOX);
        _button->labelcolor(Fl_Color(254));
        _button->color(Fl_Color(255)); //
        _button->callback((Fl_Callback*)toggle_tab_cb, this);

        _contents = new Pack(
            _button->x(),                    // lines up with button on x
            y + _button->y() + _button->h() + (GROUP_MARGIN * 2), // just below button
            w - (GROUP_MARGIN * 2), // width same as group within margin
            10);                    // changes when child add()ed

        input_ = new Fl_Multiline_Input(x, 0, w - (GROUP_MARGIN * 2), 100);
        input_->cursor_color(FL_RED);
        input_->textcolor(FL_BLACK);
        input_->textsize(12);
        input_->wrap(1);

        // end() _contents; we don't want it to begin() sucking up child widgets
        // on return
        _contents->end();
        Fl_Group::end();

        // Defaults
        circle_color(FL_RED);
        date_string("Today");
        timecode("00:00:00:00");

        resizable(0); // prevent FLTK auto-sizing -- we handle children ourself
    }

    int AnnotationGroup::handle(int event)
    {
        switch (event) {
        case FL_PUSH: {
            int ex = Fl::event_x();
            int ey = Fl::event_y();

            if (ex >= x() + 10  && ex <= x() + 10 + CIRCLE_D &&
                ey >= y() && ey <= y() + CIRCLE_D)
            {
                uchar r, g, b;
                uchar a = 255;

                Fl::get_color(circle_color_, r, g, b, a);

                int ret = flmm_color_a_chooser(_("Circle Color"),
                                               r, g, b, a);
                if (ret)
                {
                    circle_color_ = fl_rgb_color(r, g, b);
                    redraw();
                }
                return 1;
            }

            if (ex >= x() && ex <= x() + w() &&
                ey >= y() && ey <= y() + TITLE_H) {
                if (is_open())
                    close();
                else
                    open();
                return 1;
            }
            break;
        }
        default:
            break;
        }

        if (!is_open()) {
            // Body is hidden; nothing below the title row should get events.
            return 0;
        }
        return Fl_Group::handle(event);
    }

    AnnotationGroup::~AnnotationGroup()
    {
        _contents->clear();
        Fl_Group::clear(); // delete _button and _contents
    }

    void AnnotationGroup::spacing(int x)
    {
        _contents->spacing(x);
        redraw();
    }

    void AnnotationGroup::clear()
    {
        _contents->clear();
        redraw();
    }

    // // DEBUG
    // void AnnotationGroup::draw() {
    //    fl_push_clip(x(), y(), w(), h());  // enforce clipping
    //    Fl_Group::draw();                  // let group draw itself and children
    //    fl_pop_clip();                     // enforce clipping

    //    fl_color(FL_RED);
    //    fl_rect(x(), y(), w(), h());   // red line around group's xywh
    //    fl_color(FL_GREEN);
    //    fl_rect(_contents->x(),        // grn line around pack's xywh
    //            _contents->y(),
    //            _contents->w(),
    //            _contents->h());
    // }

    void AnnotationGroup::resize(int X, int Y, int W, int H)
    {
        Fl_Group::resize(X, Y, W, H); // let group resize
        layout();                     // let layout() handle child pos/sizes
    }

    // Open the widget
    void AnnotationGroup::open()
    {
        if (is_open())
            return; // already open? do nothing
        _contents->show();
        layout();           // layout changed
        window()->redraw(); // force redraw (_contents->hide()/show() doesn't)
    }

    // Close the widget
    void AnnotationGroup::close()
    {
        if (!is_open())
            return; // already closed? do nothing
        _contents->hide();
        layout();           // layout changed
        window()->redraw(); // force redraw (_contents->hide()/show() doesn't)
    }

    void AnnotationGroup::draw()
    {
        Fl_Group::draw();

        // --- Title row -----------------------------------------------------
        fl_push_clip(x(), y(), w(), TITLE_H);

        const int circle_r = CIRCLE_D / 2;
        int cx = x() + 10 + circle_r;
        int cy = y() + TITLE_H / 2;

        // Filled marker circle with a thin dark outline
        fl_color(circle_color_);
        fl_pie(cx - circle_r, cy - circle_r, CIRCLE_D, CIRCLE_D, 0.0, 360.0);
        fl_color(fl_darker(circle_color_));
        fl_arc(cx - circle_r, cy - circle_r, CIRCLE_D, CIRCLE_D, 0.0, 360.0);

        // Timecode (bold, left of center, after the circle)
        fl_font(FL_HELVETICA_BOLD, 13);
        fl_color(FL_FOREGROUND_COLOR);
        int tc_x = cx + circle_r + 8;
        int tc_w = w() / 2;
        fl_draw(timecode_.c_str(), tc_x, y(), tc_w, TITLE_H,
                (Fl_Align)(FL_ALIGN_LEFT | FL_ALIGN_INSIDE));

        // Creation date (regular weight, right-aligned)
        fl_font(FL_HELVETICA, 12);
        fl_color(fl_gray_ramp(10));
        fl_draw(date_str_.c_str(), x(), y(), w() - 10, TITLE_H,
                (Fl_Align)(FL_ALIGN_RIGHT | FL_ALIGN_INSIDE));

        fl_pop_clip();

        // Divider line under the title row (only meaningful when expanded,
        // but harmless to draw regardless)
        fl_color(fl_darker(FL_BACKGROUND_COLOR));
        fl_line(x() + 1, y() + TITLE_H, x() + w() - 2, y() + TITLE_H);
    }

} // namespace mrv
