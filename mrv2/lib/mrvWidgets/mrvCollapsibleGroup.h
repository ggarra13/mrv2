// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <FL/Fl_Group.H>
#include <FL/Fl_Button.H>

#include "mrvPack.h"

namespace mrv
{

    class CollapsibleGroup : public Fl_Group
    {
    public:
        CollapsibleGroup(
            const int x, const int y, const int w, const int h,
            const char* l = 0);
        ~CollapsibleGroup();
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
        void add(Fl_Widget* w);
        void add(Fl_Widget& w) { add(&w); }
        void clear();
        void spacing(int x);
        void resize(int X, int Y, int W, int H);
        Pack* contents() { return _contents; }
        Fl_Button* button() { return _button; }
        void layout();

    protected:
        Fl_Button* _button;
        Pack* _contents;

        static void toggle_tab_cb(Fl_Button* w, void* data);
        void relabel_button();
        void toggle_tab(Fl_Button* w);
        // virtual void draw();  // DEBUG

    public: // added these -erco
        // Open/close the widget
        void open();
        void close();
        // Is widget open?
        bool is_open() const { return _contents->visible() ? true : false; }
    };

} // namespace mrv
