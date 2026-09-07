// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include "mrvWidgets/mrvPack.h"

#include <FL/Fl_Button.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Multiline_Input.H>

namespace mrv
{

    class AnnotationGroup : public Fl_Group
    {
    public:
        AnnotationGroup(
            const int x, const int y, const int w, const int h,
            const char* l = 0);
        ~AnnotationGroup();
        void begin()
        {
            Fl_Group::begin();
            _contents->begin();
        }
        void end()
        {
            _contents->end();
            Fl_Group::end();
            layout(); // recalc our own layout
        }
        void clear();
        void spacing(int x);
        void resize(int X, int Y, int W, int H);
        Pack* contents() { return _contents; }
        Fl_Button* button() { return _button; }
        void layout();

        void draw() FL_OVERRIDE;
        int handle(int e) FL_OVERRIDE;

        // Open/close the widget
        void open();
        void close();
        // Is widget open?
        bool is_open() const { return _contents->visible() ? true : false; }

        // Accessors
        const std::string &timecode() const { return timecode_; }
        void timecode(const std::string &tc) { timecode_ = tc; redraw(); }

        const std::string &date_string() const { return date_str_; }
        void date_string(const std::string &d) { date_str_ = d; redraw(); }

        Fl_Color circle_color() const { return circle_color_; }
        void circle_color(Fl_Color c) { circle_color_ = c; redraw(); }

        std::string note_text() const { return input_->value() ? input_->value() : ""; }
        void note_text(const std::string &t) { input_->value(t.c_str()); }

        Fl_Multiline_Input *input() const { return input_; }

    protected:
        Fl_Button* _button;
        Pack* _contents;
        Fl_Multiline_Input* input_;
        Fl_Color circle_color_;
        std::string timecode_;
        std::string date_str_;

        static void toggle_tab_cb(Fl_Button* w, void* data);
        void relabel_button();
        void toggle_tab(Fl_Button* w);
    };

} // namespace mrv
