// AnnotationWidget.H
//
// A small FLTK widget representing a single "annotation" / note card,
// as used for e.g. OpenTimelineIO (OTIO) style timeline markers/comments.
//
// Layout:
//   +--------------------------------------------------------------+
//   |  (o)  01:00:12:05                          2024-06-01 14:32  |  <- title row (always visible)
//   +--------------------------------------------------------------+
//   |  Fl_Multiline_Input with the note text                       |  <- body (collapsible)
//   |                                                                |
//   +--------------------------------------------------------------+
//
// Clicking anywhere on the title row toggles the widget out of its
// full ("expanded") height. What "collapsed" means then depends on
// whether the note has any text:
//
//   - Empty note   : the Fl_Multiline_Input is hidden entirely and the
//                    widget shrinks to just the title row.
//   - Non-empty note: the Fl_Multiline_Input is resized down to the
//                    minimum width/height needed to display its text
//                    (no wrapping beyond what's needed, no wasted
//                    space) and deactivated (read-only, grayed out),
//                    so the card becomes a compact read-only preview
//                    instead of disappearing.
//
// Clicking the title row again restores the full editable size.
//
// The widget is a plain Fl_Group subclass, so it can be dropped into
// an Fl_Pack (see demo.cxx) and will report its own height via h();
// Fl_Pack will re-flow siblings automatically whenever a child resizes
// and the pack is redrawn.

#pragma once

#include <tlDraw/Shape.h>

#include <FL/Fl_Group.H>
#include <FL/Fl_Multiline_Input.H>
#include <string>

namespace mrv
{

    class AnnotationWidget : public Fl_Group {
    public:
        // X,Y,W,H       : initial geometry. H is the *expanded* height; the
        //                 widget will remember this as the height to restore
        //                 to when the user re-expands it.
    AnnotationWidget(int X, int Y, int W, int H,
                     const std::string& timecode,
                     tl::draw::NoteShape* note);

    // Fl_Group overrides
    void draw() override;
    int  handle(int event) override;
    void resize(int X, int Y, int W, int H) override;

    // Collapse state.
    // is_collapsed() is true in EITHER compact state (fully collapsed
    // with the input hidden, or shrunk-to-fit with the input visible
    // but deactivated) -- i.e. "not currently fully expanded".
    bool is_collapsed() const { return collapsed_ || shrunk_; }
    void set_collapsed(bool collapse);
    void toggle_collapsed() { set_collapsed(!is_collapsed()); }

    // True only when the note has text and is in the shrunk-to-fit,
    // deactivated preview state (as opposed to fully collapsed/hidden).
    bool is_shrunk() const { return shrunk_; }

    // True if the note field has any non-whitespace text.
    bool has_note_text() const;

    // Accessors
    const std::string &timecode() const { return timecode_; }
    void timecode(const std::string &tc) { timecode_ = tc; redraw(); }

    const std::string &date_string() const { return note_->date; }
    void date_string(const std::string &d) { note_->date = d; redraw(); }

    Fl_Color circle_color() const { return note_->color; }
    void circle_color(Fl_Color c) { note_->color = c; redraw(); }

    std::string note_text() const { return input_->value() ? input_->value() : ""; }
    void note_text(const std::string &t) { input_->value(t.c_str()); }

    Fl_Multiline_Input *input() const { return input_; }

    // Height reserved for the title row, public so callers can lay
    // things out around the widget if they need to.
    static const int TITLE_H = 28;

private:
    tl::draw::NoteShape* note_;
    std::string timecode_;

    Fl_Multiline_Input *input_;

    bool collapsed_;    // true: input hidden, title-row-only (empty note)
    bool shrunk_;       // true: input shown, shrunk-to-fit, deactivated (non-empty note)
    int  expanded_h_;   // height to restore to on expand
    int  pad_;          // inner padding used around the input field

    // Computes the smallest (w,h) needed to display the note's current
    // text without wrapping more than necessary, given at most max_w
    // pixels of width to work with (wrapping only kicks in if the text
    // wouldn't otherwise fit).
    void measure_note_size(int max_w, int &out_w, int &out_h) const;
};

}
