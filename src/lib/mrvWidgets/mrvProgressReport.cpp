// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <cinttypes>
#include <cmath>

#include <FL/Fl_Output.H>
#include <FL/Fl_Progress.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Box.H>
#include <FL/Fl.H>
#include <FL/fl_ask.H>

#include "mrvCore/mrvI8N.h"

#include "mrvProgressReport.h"

namespace mrv
{

    ProgressReport::ProgressReport(
        Fl_Window* main, int64_t start, int64_t end, const char* title) :
        _frame(start),
        _start(start),
        _end(end),
        _time(0)
    {
        Fl_Group::current(0);
        w = new Fl_Window(
            main->x() + main->w() / 2 - 320,
            main->y() + main->h() / 2 - 120 / 2, 640, 120);
        w->size_range(640, 120);
        w->begin();
        Fl_Group* g = new Fl_Group(0, 0, w->w(), 120);
        g->begin();
        g->box(FL_UP_BOX);
        progress = new Fl_Progress(20, 20, g->w() - 40, 40);
        progress->minimum(0);
        progress->maximum(float(end - start + 1));
        progress->align(FL_ALIGN_TOP);
        progress->copy_label(title);
        elapsed = new Fl_Output(120, 80, 120, 20, _("Elapsed"));
        elapsed->labelsize(16);
        elapsed->box(FL_FLAT_BOX);
        elapsed->textcolor(FL_BLACK);
        elapsed->set_output(); // needed so no selection appears
        remain = new Fl_Output(350, 80, 120, 20, _("Remaining"));
        remain->labelsize(16);
        remain->box(FL_FLAT_BOX);
        remain->textcolor(FL_BLACK);
        remain->set_output(); // needed so no selection appears
        fps = new Fl_Output(550, 80, 60, 20, _("FPS"));
        fps->labelsize(16);
        fps->box(FL_FLAT_BOX);
        fps->textcolor(FL_BLACK);
        fps->set_output(); // needed so no selection appears
        g->end();
        w->resizable(w);
        w->set_modal();
        w->end();

        _startTime = std::chrono::steady_clock::now();
        _frameDuration = 0;
    }

    ProgressReport::~ProgressReport()
    {
        delete w;
        w = NULL;
    }

    void ProgressReport::show()
    {
        w->show();
    }

    void ProgressReport::to_hour_min_sec(
        const double t, int& hour, int& min, int& sec, int& ms)
    {
        long long milliseconds = t;
        long long total_seconds = milliseconds / 1000;
        hour = total_seconds / 3600;
        min = (total_seconds % 3600) / 60;
        sec = total_seconds % 60;
        ms = milliseconds % 1000;
    }

    bool ProgressReport::tick()
    {
        progress->value(progress->value() + 1);

        const auto now = std::chrono::steady_clock::now();
        std::chrono::duration<float> diff = now - _startTime;
        _time += diff.count();

        int hour, min, sec, ms;
        to_hour_min_sec(_time, hour, min, sec, ms);

        char buf[120];
        snprintf(buf, 120, " %02d:%02d:%02d.%d", hour, min, sec, ms);
        elapsed->value(buf);

        double r = 0;
        int64_t frame_diff = _end - _frame;
        if (frame_diff > 0 && _actualFrameRate > 0)
            r = frame_diff / _actualFrameRate * 1000.0;

        to_hour_min_sec(r, hour, min, sec, ms);

        snprintf(buf, 120, " %02d:%02d:%02d.%d", hour, min, sec, ms);
        remain->value(buf);

        //
        // Calculate our actual frame rate, averaged over several frames.
        //

        if (_framesSinceLastFpsFrame >= 24)
        {
            std::chrono::duration<float> diff = now - _lastFpsFrameTime;

            float t = diff.count();
            if (t > 0)
                _actualFrameRate = _framesSinceLastFpsFrame / t;

            _framesSinceLastFpsFrame = 0;

            snprintf(buf, 120, " %3.2f", _actualFrameRate);
            fps->value(buf);
        }

        if (_framesSinceLastFpsFrame == 0)
            _lastFpsFrameTime = now;

        ++_framesSinceLastFpsFrame;

        Fl::check();
        ++_frame;

        if (!w->visible())
        {
            delete w;
            w = nullptr;
            return false;
        }
        return true;
    }

} // namespace mrv
