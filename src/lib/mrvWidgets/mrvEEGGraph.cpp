// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the feather-tk project.

#include "mrvWidgets/mrvEEGGraph.h"

#include "mrvOS/mrvI8N.h"

#include <tlCore/Memory.h>
#include <tlCore/StringFormat.h>

#include <FL/Fl.H>
#include <FL/fl_draw.H>

#include <algorithm>
#include <cmath>
#include <limits>

namespace mrv
{
    // ── Construction ──────────────────────────────────────────────────────────

    EEGGraph::EEGGraph(int X, int Y, int W, int H, const char* L) :
        Fl_Widget(X, Y, W, H, L),
        bg_(fl_rgb_color(8, 12, 8))
    {
        box(FL_NO_BOX);

        // If the FLTK label was supplied use it as the initial group name so
        // that callers who do:
        //   new EEGGraph(x, y, w, h, "Image Memory");
        // don't also have to call setGroup().
        if (L)
            group_ = L;
    }

    // ── Group ─────────────────────────────────────────────────────────────────

    void EEGGraph::setGroup(const std::string& group)
    {
        group_ = group;
        traces_.clear();   // old data belongs to the previous group
    }

    const std::string& EEGGraph::getGroup() const
    {
        return group_;
    }

    // ── Data input ───────────────────────────────────────────────────────────

    void EEGGraph::setSamples(
        const std::map<std::string, std::vector<int64_t>>& allSamples)
    {
        traces_.clear();

        // StatsSystem keys are "Group/Name".  Extract only entries whose
        // group prefix matches this widget's group.
        const std::string prefix = group_ + "/";

        for (const auto& [id, buf] : allSamples)
        {
            if (id.compare(0, prefix.size(), prefix) != 0)
                continue;

            const std::string name = id.substr(prefix.size());
            traces_[name] = buf;

            // Assign a palette colour the first time this name is seen.
            if (styles_.find(name) == styles_.end())
            {
                TraceStyle ts;
                ts.fg = _next_color();
                styles_[name] = ts;
            }
        }
    }

    // ── Style ─────────────────────────────────────────────────────────────────

    void EEGGraph::setTraceStyle(const std::string& name,
                                  const TraceStyle& style)
    {
        styles_[name] = style;
    }

    const EEGGraph::TraceStyle& EEGGraph::getTraceStyle(
        const std::string& name) const
    {
        auto it = styles_.find(name);
        if (it == styles_.end())
        {
            TraceStyle ts;
            ts.fg = _next_color();
            styles_[name] = ts;
            return styles_[name];
        }
        return it->second;
    }

    // ── Clear ─────────────────────────────────────────────────────────────────

    void EEGGraph::clear()
    {
        traces_.clear();
        // Styles are intentionally kept so the caller doesn't have to
        // re-configure them after a data reset.
        redraw();
    }

    // ── Drawing ───────────────────────────────────────────────────────────────

    void EEGGraph::draw()
    {
        using namespace tl;
        
        const int X = x(), Y = y(), W = w(), H = h();
        if (W <= 0 || H <= 0) return;

        // Bottom 20 px are reserved for per-trace labels.
        const int CH = H - 20;

        fl_antialias(0);
        fl_color(bg_);
        fl_rectf(X, Y, W, CH);
        
        fl_color(FL_BACKGROUND_COLOR);
        fl_rectf(X, Y + CH, W, H - CH);

        int labelX = X;

        for (const auto& [name, buf] : traces_)
        {
            if (buf.empty())
                continue;

            const TraceStyle& ts = getTraceStyle(name);

            // ── Visible window ───────────────────────────────────────────────
            // StatsSystem stores samples oldest-first; just take the tail.
            const int         cols = W;
            const std::size_t use  =
                std::min(static_cast<std::size_t>(cols), buf.size());

            std::vector<int64_t> vis(cols, 0);
            const std::size_t src_start = buf.size() - use;
            for (std::size_t i = 0; i < use; ++i)
                vis[cols - static_cast<int>(use) + i] = buf[src_start + i];

            // ── Y scale ──────────────────────────────────────────────────────
            int64_t ylo = ts.y_min, yhi = ts.y_max;
            if (ts.auto_scale)
            {
                ylo = std::numeric_limits<int64_t>::max();
                yhi = std::numeric_limits<int64_t>::min();
                for (auto v : vis)
                {
                    ylo = std::min(ylo, v);
                    yhi = std::max(yhi, v);
                }
                if (ylo == yhi) { ylo -= 1; yhi += 1; }
                const int64_t pad = (yhi - ylo) / 20 + 1;
                ylo -= pad;
                yhi += pad;
            }

            const double yrange = static_cast<double>(yhi - ylo);
            auto sampleY = [&](int64_t v) -> int
            {
                const double t = static_cast<double>(v - ylo) / yrange;
                return Y + CH - 1 - static_cast<int>(t * (CH - 1));
            };

            // ── Trace ────────────────────────────────────────────────────────
            fl_color(ts.fg);
            fl_line_style(FL_SOLID, static_cast<int>(ts.line_width + 0.5f));
            _draw_trace(vis, cols, X, Y, CH, sampleY);
            fl_line_style(FL_SOLID, 0);

            // ── Label in the bottom strip ────────────────────────────────────
            // `name` is the StatsSystem name template, e.g. "Images: {0}MB".
            // Format in the latest (rightmost) value so the label is live.
            fl_color(ts.fg);
            fl_font(FL_HELVETICA, 10);

            char label[256];
            double value = buf.back();
            auto j = group_.find(_("Memory"));
            if (j != std::string::npos)
            {
                std::string memorySuffix = "B";
                if (value >= memory::gigabyte)
                {
                    memorySuffix = "GB";
                    value /= memory::gigabyte;
                }
                else if (value >= memory::megabyte)
                {
                    memorySuffix = "MB";
                    value /= memory::megabyte;
                }
                double rounded = std::round(value * 100.0) / 100.0;
                snprintf(label, 256, "%s%.0f%s", name.c_str(), value,
                         memorySuffix.c_str());
            }
            else
            {
                snprintf(label, 256, "%s%.0f", name.c_str(), value);
            }
            int lw = 0, lh = 0;
            fl_measure(label, lw, lh);
            fl_draw(label, labelX, Y + CH + 14);
            labelX += lw + 8;
        }
        
        fl_antialias(1);
    }

    // ── Private ───────────────────────────────────────────────────────────────

    template<typename SampleYFn>
    void EEGGraph::_draw_trace(
        const std::vector<int64_t>& vis,
        int cols, int X, int Y, int /*CH*/,
        SampleYFn sampleY) const
    {
        int prev_px = X, prev_py = sampleY(vis[0]);
        for (int c = 1; c < cols; ++c)
        {
            const int px = X + c;
            const int py = sampleY(vis[c]);
            fl_line(prev_px, prev_py, px, py);
            prev_px = px;
            prev_py = py;
        }
    }

    Fl_Color EEGGraph::_next_color() const
    {
        const int idx = static_cast<int>(styles_.size()) % kPaletteSize;
        return kPalette[idx];
    }

} // namespace mrv
