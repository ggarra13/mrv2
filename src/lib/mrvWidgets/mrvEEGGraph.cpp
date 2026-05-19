
#include "mrvWidgets/mrvEEGGraph.h"

#include <FL/Fl.H>
#include <FL/Fl_Widget.H>
#include <FL/fl_draw.H>

#include <cstdint>
#include <vector>
#include <algorithm>
#include <cmath>
#include <string>
#include <limits>
#include <mutex>

namespace mrv
{

    EEGGraph::EEGGraph(int X, int Y, int W, int H, const char *L) :
        Fl_Widget(X, Y, W, H, L)
    {
        bg = fl_rgb_color( 8, 12, 8);
        box(FL_NO_BOX);

    }

    void EEGGraph::push_sample(const std::string& name, int64_t v) {
        std::lock_guard<std::mutex> lk(mtx_);
        auto i = stats.find(name);
        if (i == stats.end())
        {
            stats[name] = Stats();
            stats[name].fg = colors[stats.size()-1];
        }
        Stats& s = stats[name];
        s.buffer[s.write_pos] = v;
        s.write_pos = (s.write_pos + 1) % s.capacity;
        if (s.sample_count < s.capacity) ++s.sample_count;
        redraw();
    }
    
    void EEGGraph::clear() {
        std::lock_guard<std::mutex> lk(mtx_);
        stats.clear();
        redraw();
    }

    void EEGGraph::draw()
    {
        const int X = x(), Y = y(), W = w(), H = h();
        if (W <= 0 || H <= 0) return;

        // ── background ───────────────────────────────────────────────────
        fl_color(bg);

        int CH = H - 20;
        fl_rectf(X, Y, W, CH);

        int CX = X; 
        for (auto& [name, s] : stats)
        {
            // ── snapshot the ring-buffer ─────────────────────────────────────
            std::vector<int64_t> snap;
            std::size_t cap, wpos, count;
            {
                std::lock_guard<std::mutex> lk(mtx_);
                snap   = s.buffer;
                cap    = s.capacity;
                wpos   = s.write_pos;
                count  = s.sample_count;
            }

            if (count == 0)
                continue;

            // ── visible sample window: one sample per pixel column ───────────
            const int cols = W;

            // Build a display vector (oldest → newest) mapping to pixel cols
            // We only need `cols` samples at most.
            std::size_t use = std::min((std::size_t)cols, count);
            std::vector<int64_t> vis(cols, 0);
            // oldest visible sample index in ring
            std::size_t start = (wpos + cap - use) % cap;
            for (std::size_t i = 0; i < use; ++i) {
                std::size_t ri = (start + i) % cap;

                // right-align: put newest sample at rightmost pixel
                vis[cols - (int)use + i] = snap[ri];
            }

            // ── Y autoscale ──────────────────────────────────────────────────
            int64_t ylo = s.y_min, yhi = s.y_max;
            if (s.auto_scale) {
                ylo = std::numeric_limits<int64_t>::max();
                yhi = std::numeric_limits<int64_t>::min();
                for (auto v : vis) { ylo = std::min(ylo, v); yhi = std::max(yhi, v); }
                if (ylo == yhi) { ylo -= 1; yhi += 1; }
                // 5 % padding
                int64_t pad = (yhi - ylo) / 20 + 1;
                ylo -= pad; yhi += pad;
            }
    
            const double yrange = (double)(yhi - ylo);

            // helper: sample value → pixel Y (flipped: large value → top)
            auto sampleY = [&](int64_t v) -> int {
                double t = (double)(v - ylo) / yrange;
                return Y + CH - 1 - (int)(t * (CH - 1));
            };

            // ── write-head pixel column ──────────────────────────────────────
            // The newest sample sits at pixel col = cols-1.
            // We model the "write head" as scrolling left; the gap trails it.
            // Since we right-align, we draw the gap at the right end.
            const int head_col = cols - 1;

            // ── main trace ───────────────────────────────────────────────────
            fl_color(s.fg);
            fl_line_style(FL_SOLID, (int)(s.line_width + 0.5f));
            _draw_trace(vis, cols, head_col, X, Y, CH, sampleY);

            fl_font(FL_HELVETICA, 10);
            int NW, NH;
            fl_measure(name.c_str(), NW, NH); 
            fl_draw(name.c_str(), CX, Y + H);

            CX += NW;
        }

        fl_line_style(FL_SOLID, 0); // reset

    }

}
