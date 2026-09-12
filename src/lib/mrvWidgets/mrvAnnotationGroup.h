// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include "mrvWidgets/mrvPack.h"
#include "mrvWidgets/mrvAnnotationWidget.h"

#include <tlCore/ValueObserver.h>

#include <FL/Fl_Group.H>
#include <FL/Fl_Button.H>

#include <memory>

class Fl_Box;

namespace mrv
{

    class TimelinePlayer;

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
            contents_->begin();
        }
        void end()
        {
            contents_->end();
            Fl_Group::end();
            layout(); // recalc our own layout
        }
        void add(Fl_Widget* w);
        void add(Fl_Widget& w) { add(&w); }
        void clear();
        void spacing(int x);
        void resize(int X, int Y, int W, int H);
        Pack* contents() { return contents_; }
        Fl_Button* button() { return button_; }
        void layout();


        // Whole-group collapse state (hides/shows the entire note list).
        bool is_collapsed() const { return collapsed_; }
        void set_collapsed(bool collapse);
        void toggle_collapsed() { set_collapsed(!collapsed_); }

        // Set timeline player
        void setPlayer(TimelinePlayer* player);

        // Adds a fully-specified annotation. It becomes the active
        // (expanded) note; every other note in the group is collapsed.
        AnnotationWidget *add_annotation(const OTIO_NS::RationalTime& time,
                                         std::shared_ptr<tl::draw::NoteShape>& note);

        // Adds a blank annotation with an auto-generated timecode/date.
        // This is what the header's [+] button calls.
        void add_annotation();

        // Removes a specific annotation belonging to this group.
        void remove_annotation(AnnotationWidget *note);
    protected:
        std::string title_;
        bool collapsed_;
        int  expanded_h_;
        int  pad_;
        int  next_id_;

        TimelinePlayer* player_ = nullptr;

        void currentTimeChanged(const OTIO_NS::RationalTime& value);
        std::shared_ptr<tl::observer::ValueObserver<OTIO_NS::RationalTime> >
            currentTimeObserver;

        Fl_Button* add_btn_;
        Fl_Button* remove_btn_;
        Fl_Button* button_;
        Pack* contents_;
        Fl_Box*  empty_box_ = nullptr;   // "No annotations added yet." placeholder

        std::vector<AnnotationWidget *> annotations_;

        // Shows empty_box_ inside contents_ when annotations_ is empty,
        // removes it otherwise. Safe to call any time annotations_ changes.
        void update_empty_state();

        // Collapses every annotation in the group except keep_active.
        void enforce_single_active(AnnotationWidget *keep_active);

        // Removes active annotation.
        void remove_annotation();

        static void toggle_tab_cb(Fl_Button* w, void* data);
        static void add_button_cb(Fl_Widget *, void *data);
        static void remove_button_cb(Fl_Widget *, void *data);

        void relabel_button();
        void toggle_tab(Fl_Button* w);
    };

} // namespace mrv
