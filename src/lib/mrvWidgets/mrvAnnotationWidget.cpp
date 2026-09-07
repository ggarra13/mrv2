// mrvAnnotationWidget.cpp
#include "mrvWidgets/mrvAnnotationWidget.h"

#include "mrvOS/mrvI8N.h"

#include <FL/Fl.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Window.H>
#include <cctype>

namespace mrv
{
    AnnotationWidget::AnnotationWidget(
        int X, int Y, int W, int H,
        const OTIO_NS::RationalTime& time,
        tl::draw::NoteShape* note)
        : Fl_Group(X, Y, W, H),
          time_(time),
          note_(note),
          input_(nullptr),
          collapsed_(false),
          shrunk_(false),
          expanded_h_(H),
          pad_(4)
    {
        box(FL_UP_BOX);

        input_ = new Fl_Multiline_Input(X + pad_, Y + TITLE_H + pad_,
                                        W - 2 * pad_, H - TITLE_H - 2 * pad_);
        input_->wrap(1);
        input_->cursor_color(FL_RED);
        input_->textcolor(FL_BLACK);
        input_->box(FL_FLAT_BOX);
        input_->value(note_->text.c_str());

        end();
        resizable(0);
    }

    void AnnotationWidget::resize(int X, int Y, int W, int H)
    {
        Fl_Group::resize(X, Y, W, H);
        if (!collapsed_ && !shrunk_) expanded_h_ = H;
    }

    bool AnnotationWidget::has_note_text() const
    {
        const char *v = input_->value();
        if (!v) return false;
        for (const char *p = v; *p; ++p) {
            if (!std::isspace(static_cast<unsigned char>(*p))) return true;
        }
        return false;
    }

    void AnnotationWidget::measure_note_size(int max_w, int &out_w, int &out_h) const
    {
        // Save/restore the current drawing font, since fl_measure() and
        // fl_height() depend on it.
        Fl_Font saved_font = fl_font();
        Fl_Fontsize saved_size = fl_size();

        fl_font(input_->textfont(), input_->textsize());

        std::string text = input_->value() ? input_->value() : "";

        // fl_measure() honors embedded '\n's. Given a non-zero starting
        // width it also word-wraps to that width -- but only as much as
        // needed, so a short line is measured at its own (smaller) width
        // rather than the full max_w. That gives us exactly the "minimum
        // size needed" behavior, bounded by max_w so we never overflow
        // the card.
        int mw = max_w > 0 ? max_w : 0;
        int mh = 0;
        fl_measure(text.c_str(), mw, mh, 0);
        if (mh <= 0) mh = fl_height();

        out_w = max_w;   // always reach to the margin
        out_h = mh + 6;  // small vertical padding

        fl_font(saved_font, saved_size);
    }

    void AnnotationWidget::set_collapsed(bool collapse)
    {
        if (collapse == is_collapsed()) return;

        if (collapse) {
            // Only capture expanded_h_ here: at this point we know we were
            // fully expanded (guarded by the check above), so h() is the
            // real expanded height to restore later.
            expanded_h_ = h();

            if (!has_note_text()) {
                // Nothing to preview: collapse down to just the title row.
                input_->hide();
                Fl_Group::resize(x(), y(), w(), TITLE_H);
                collapsed_ = true;
                shrunk_ = false;
            } else {
                // Shrink the input down to just fit its text, and lock it
                // so it reads as a compact, read-only preview.
                int avail_w = w() - 2 * pad_;
                int mw = 0, mh = 0;
                measure_note_size(avail_w, mw, mh);
                if (mw > avail_w) mw = avail_w;  // never exceed the card width

                input_->insert(0);
                input_->resize(x() + pad_, y() + TITLE_H + pad_, mw, mh);
                input_->deactivate();
                note_->text = input_->value();

                Fl_Group::resize(x(), y(), w(), TITLE_H + mh + 2 * pad_);
                collapsed_ = false;
                shrunk_ = true;
            }
        } else {
            // Restore full editable size.
            input_->activate();
            input_->resize(x() + pad_, y() + TITLE_H + pad_,
                           w() - 2 * pad_, expanded_h_ - TITLE_H - 2 * pad_);
            input_->show();
            Fl_Group::resize(x(), y(), w(), expanded_h_);
            collapsed_ = false;
            shrunk_ = false;
        }

        if (window()) window()->redraw();  // let an Fl_Pack re-flow siblings
        redraw();
    }

    int AnnotationWidget::handle(int event)
    {
        switch (event) {
        case FL_PUSH: {
            int ex = Fl::event_x();
            int ey = Fl::event_y();
            if (ex >= x() && ex <= x() + w() &&
                ey >= y() && ey <= y() + TITLE_H) {
                toggle_collapsed();
                return 1;
            }
            if (ex >= x() && ex <= x() + w() &&
                ey >= y() && ey <= y() + h()) {
                set_collapsed(false);
                return Fl_Group::handle(event);
            }
            break;
        }
        case FL_ENTER:
        case FL_MOVE: {
            int ey = Fl::event_y();
            bool over_title = (ey >= y() && ey <= y() + TITLE_H);
            fl_cursor(over_title ? FL_CURSOR_HAND : FL_CURSOR_DEFAULT);
            if (over_title) return 1;
            break;
        }
        default:
            break;
        }

        if (collapsed_) {
            // Body is hidden; nothing below the title row should get events.
            return 0;
        }
        return Fl_Group::handle(event);
    }

    void AnnotationWidget::draw()
    {
        // Draw group
        Fl_Group::draw();

        // --- Title row -----------------------------------------------------
        fl_push_clip(x(), y(), w(), TITLE_H);

        const int circle_d = 14;
        const int circle_r = circle_d / 2;
        int cx = x() + 10 + circle_r;
        int cy = y() + TITLE_H / 2;

        // Filled marker circle with a thin dark outline
        fl_color(note_->color);
        fl_pie(cx - circle_r, cy - circle_r, circle_d, circle_d, 0.0, 360.0);
        fl_color(fl_darker(note_->color));
        fl_arc(cx - circle_r, cy - circle_r, circle_d, circle_d, 0.0, 360.0);

        // Timecode (bold, left of center, after the circle)
        fl_font(FL_HELVETICA_BOLD, 13);
        fl_color(FL_FOREGROUND_COLOR);
        int tc_x = cx + circle_r + 8;
        int tc_w = w() / 2;

        const std::string& timecode = time_.to_timecode();
        fl_draw(timecode.c_str(), tc_x, y(), tc_w, TITLE_H,
                (Fl_Align)(FL_ALIGN_LEFT | FL_ALIGN_INSIDE));

        // Creation date (regular weight, right-aligned)
        fl_font(FL_HELVETICA, 12);
        fl_color(fl_gray_ramp(10));
        fl_draw(note_->date.c_str(), x(), y(), w() - 10, TITLE_H,
                (Fl_Align)(FL_ALIGN_RIGHT | FL_ALIGN_INSIDE));

        fl_pop_clip();

        // Divider line under the title row (only meaningful when expanded,
        // but harmless to draw regardless)
        fl_color(fl_darker(FL_BACKGROUND_COLOR));
        fl_line(x() + 1, y() + TITLE_H, x() + w() - 2, y() + TITLE_H);
    }

}
