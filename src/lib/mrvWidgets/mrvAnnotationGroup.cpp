// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.



#include "mrvWidgets/mrvAnnotationGroup.h"
#include "mrvWidgets/mrvAnnotationWidget.h"
#include "mrvWidgets/mrvPack.h"

#include "mrvOS/mrvI8N.h"

#include <FL/Fl_Button.H>
#include <FL/Fl_Window.H>
#include <FL/fl_draw.H>

#include <cstring>
#include <cstdlib>
#include <cstdio>

#define BUTTON_H 20
#define GROUP_MARGIN 8 // compensates for FL_ROUND_BUTTON (?)
#define TOOLS_MARGIN 30

static const int HEADER_H = 28;
static const int DEFAULT_ANNOTATION_H = 130;

namespace mrv
{

    // Change button label based on open/closed state of pack
    void AnnotationGroup::relabel_button()
    {
        int open =contents_->visible() ? 1 : 0;
        char buf[256];
        // Draw arrow in label
        //    Text to the right of arrow based on parent's label()
        //
        if (open)
            snprintf(buf, 256, "@2>  %s", label() ? label() : "(no label)");
        else
            snprintf(buf, 256, "@>  %s", label() ? label() : "(no label)");
        button_->copy_label(buf);
    }

    // Enforce layout
    void AnnotationGroup::layout()
    {

        // Size self based on if open() or close()ed
        int gh = button_->h() + (GROUP_MARGIN * 2);
        if (is_collapsed())
            gh +=contents_->h(); // include content's height if we're 'open'

        // Note: resizable() set to zero, so this just resizes us, not children.
        Fl_Group::resize(x(), y(), w(), gh);

        // Manage size/position of children
        //    No need to call init_sizes(): resizable(0) is set in ctor,
        //    and we adjust children ourself.
        //
        button_->resize(
            x() + GROUP_MARGIN,       // x always inset (leaves room for ROUND
                                      // box)
            y() + GROUP_MARGIN,       // y always inset ("")
            w() - (GROUP_MARGIN * 2) - TOOLS_MARGIN, // width tracks group's w()
            button_->h());            // height fixed



        // Leavecontents_->h() alone, we shouldn't change it; Fl_Pack
        // calculates its own height, we don't want to mess that up by changing
        // it. visible() will control whether it's drawn or not. it's seen or
        // not.
        //
       contents_->resize(
            x() + GROUP_MARGIN,                // x always same as button
            y() + button_->h() + GROUP_MARGIN * 2, // y always "below button"
            w() - (GROUP_MARGIN * 2),          // width tracks group's w()
           contents_->h()); // leave height ofcontents_ alone

        // DEBUG

        // printf("--- LAYOUT CHANGED (%s) ------\n", label() ? label() : "(no
        // label)"); printf(" grp: %d,%d,%d,%d\n", x(),y(),w(),h()); printf("
        // but: %d,%d,%d,%d\n", button_->x(), button_->y(), button_->w(),
        // button_->h()); printf("pack: %d,%d,%d,%d\n",contents_->x(),
        //contents_->y(),contents_->w(),contents_->h()); printf("\n");
    }

    void AnnotationGroup::toggle_tab_cb(Fl_Button* w, void* data)
    {
        mrv::AnnotationGroup* g = (mrv::AnnotationGroup*)data;
        if (g->is_collapsed())
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
        // box(FL_BORDER_BOX);

        // Disable label from being shown; we show the label only in the button.
        labeltype(FL_NO_LABEL);

        // NOTNEEDED Fl_Group::begin();
        //  Button
        button_ = new Fl_Button(
            x,       // margin leaves room for FL_ROUND_BOX
            y + GROUP_MARGIN,       // margin leaves room for FL_ROUND_BOX
            w - TOOLS_MARGIN * 2, // width same as group within margin
            BUTTON_H);              // button height fixed size
        button_->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
        button_->labelsize(16);
        button_->box(FL_FLAT_BOX);
        button_->labelcolor(Fl_Color(254));
        button_->color(Fl_Color(255)); //
        button_->callback((Fl_Callback*)toggle_tab_cb, this);

        add_btn_ = new Fl_Button(x + w - (GROUP_MARGIN + TOOLS_MARGIN * 2),
                             y + GROUP_MARGIN,
                             TOOLS_MARGIN, BUTTON_H, "+");
        add_btn_->copy_tooltip(_("Add a new note at the current time."));
        add_btn_->callback(add_button_cb, this);

        remove_btn_ = new Fl_Button(x + w - (GROUP_MARGIN + TOOLS_MARGIN),
                                y + GROUP_MARGIN,
                                TOOLS_MARGIN, BUTTON_H, "-");
        remove_btn_->copy_tooltip(_("Remove the current note."));
        remove_btn_->callback(remove_button_cb, this);

       contents_ = new Pack(
            button_->x(),                    // lines up with button on x
            y + button_->y() + button_->h() + (GROUP_MARGIN * 2), // just below button
            w - (GROUP_MARGIN * 2), // width same as group within margin
            10);                    // changes when child add()ed

        // end()contents_; we don't want it to begin() sucking up child widgets
        // on return
       contents_->end();
        Fl_Group::end();

        relabel_button(); // relabel button once pack created
        resizable(0); // prevent FLTK auto-sizing -- we handle children ourself
    }

    AnnotationGroup::~AnnotationGroup()
    {
       contents_->clear();
       Fl_Group::clear(); // delete button_ andcontents_
    }

    void AnnotationGroup::spacing(int x)
    {
        contents_->spacing(x);
        redraw();
    }

    void AnnotationGroup::clear()
    {
        contents_->clear();
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
    //           contents_->y(),
    //           contents_->w(),
    //           contents_->h());
    // }

    void AnnotationGroup::add(Fl_Widget* w)
    {
       contents_->add(w);
       contents_->redraw();
    }

    void AnnotationGroup::resize(int X, int Y, int W, int H)
    {
        Fl_Group::resize(X, Y, W, H); // let group resize
        layout();                     // let layout() handle child pos/sizes
    }

    // Open the widget
    void AnnotationGroup::open()
    {
        if (is_collapsed())
            return; // already open? do nothing
       contents_->show();
        relabel_button();
        layout();           // layout changed
        window()->redraw(); // force redraw (_contents->hide()/show() doesn't)
    }

    // Close the widget
    void AnnotationGroup::close()
    {
        if (!is_collapsed())
            return; // already closed? do nothing
        contents_->hide();
        relabel_button();
        layout();           // layout changed
        window()->redraw(); // force redraw (_contents->hide()/show() doesn't)c
    }

    void AnnotationGroup::draw()
    {
        Fl_Group::draw();
    }

    AnnotationWidget*
    AnnotationGroup::add_annotation(
        const OTIO_NS::RationalTime& time,
        tl::draw::NoteShape* note_shape)
    {
        contents_->begin();
        int note_w = contents_->w();
        auto *note = new AnnotationWidget(0, 0, note_w, DEFAULT_ANNOTATION_H,
                                          time, note_shape);
        contents_->end();

        note->expand_callback([this](AnnotationWidget *w) { enforce_single_active(w); });
        annotations_.push_back(note);

        // The new note starts fully expanded; make sure it's the only one.
        enforce_single_active(note);

        contents_->redraw();
        if (window()) window()->redraw();
        return note;
    }

    void AnnotationGroup::remove_annotation(AnnotationWidget *note)
    {
        if (!note) return;
        auto it = std::find(annotations_.begin(), annotations_.end(), note);
        if (it == annotations_.end()) return;

        contents_->remove(note);
        delete note;
        annotations_.erase(it);

        contents_->redraw();
        if (window()) window()->redraw();
    }

    void AnnotationGroup::set_collapsed(bool collapse)
    {
        if (collapse == collapsed_) return;
        collapsed_ = collapse;

        if (collapsed_) {
            expanded_h_ = h();
            Fl_Group::resize(x(), y(), w(), HEADER_H);
        } else {
            Fl_Group::resize(x(), y(), w(), expanded_h_);
        }

        // Keep the add/remove buttons pinned to the header's right edge
        // even if our width changed while collapsed.
        const int btn_w = 24;
        const int btn_h = HEADER_H - 6;
        const int btn_y = y() + (HEADER_H - btn_h) / 2;
        add_btn_->resize(x() + w() - pad_ - btn_w, btn_y, btn_w, btn_h);
        remove_btn_->resize(x() + w() - pad_ - 2 * btn_w - pad_, btn_y, btn_w, btn_h);

        if (window()) window()->redraw();
        redraw();
    }

    void AnnotationGroup::enforce_single_active(AnnotationWidget *keep_active)
    {
        for (AnnotationWidget *w : annotations_) {
            if (w != keep_active && !w->is_collapsed()) {
                w->set_collapsed(true);
            }
        }
    }

    AnnotationWidget *AnnotationGroup::add_annotation()
    {
        ++next_id_;
        char tc[32];
        OTIO_NS::RationalTime time(0, 24);
        // @todo:
        return nullptr; //add_annotation(time, note);
    }

    void AnnotationGroup::remove_annotation()
    {
        if (annotations_.empty()) return;

        AnnotationWidget *target = nullptr;
        for (AnnotationWidget *w : annotations_) {
            if (!w->is_collapsed()) { target = w; break; }  // the active one, if any
        }
        if (!target) target = annotations_.back();

        remove_annotation(target);
    }

    void AnnotationGroup::add_button_cb(Fl_Widget *, void *data)
    {
        static_cast<AnnotationGroup *>(data)->add_annotation();
    }

    void AnnotationGroup::remove_button_cb(Fl_Widget *, void *data)
    {
        static_cast<AnnotationGroup *>(data)->remove_annotation();
    }
} // namespace mrv
