#pragma once
// ============================================================
//  EcgGraph.h  —  FLTK 1.5 electrocardiogram-style graph widget
//  Usage:
//    EcgGraph *g = new EcgGraph(x, y, w, h, "label");
//    g->push_sample(value);   // call from any thread via Fl::lock()
//    g->set_capacity(1024);   // ring-buffer size (default 2048)
//    g->set_grid(10, 5);      // horizontal / vertical grid lines
// ============================================================

#include <FL/Fl_Widget.H>
#include <FL/fl_draw.H>

#include <cstdint>
#include <cmath>
#include <algorithm>
#include <array>
#include <map>
#include <mutex>
#include <vector>

namespace mrv
{
    class EEGGraph : public Fl_Widget
    {
    public:
        constexpr static std::array<Fl_Color, 6> colors =
            {
                FL_CYAN,
                FL_MAGENTA,
                FL_YELLOW,
                FL_RED,
                FL_GREEN,
                FL_BLUE
            };
        
        EEGGraph(int X, int Y, int W, int H, const char *L = nullptr);

        /** Append one sample.  Safe to call from a worker thread when
         *  the caller holds Fl::lock(). */
        void push_sample(const std::string& name, int64_t v);

        /** Clear all samples. */
        void clear();
        
        void draw() override;

    private:
        
        struct Stats
        {
            std::vector<int64_t>  buffer;
            std::size_t           capacity;
            std::size_t           write_pos;
            std::size_t           sample_count;
            int64_t               y_min, y_max;
            bool                  auto_scale;
            float                 line_width;
            Fl_Color              fg;

            Stats() :
                capacity(100),
                write_pos(0),
                sample_count(0),
                y_min(0),
                y_max(0),
                auto_scale(true),
                line_width(1.5f),
                fg(FL_GREEN)
                {
                    buffer.resize(capacity, 0);
                }
        };
        std::map<std::string, Stats> stats;
        
        Fl_Color              bg;
        std::mutex            mtx_;

        template<typename SampleYFn>
        void _draw_trace(const std::vector<int64_t> &vis, int cols,
                         int /*head_col*/,
                         int X, int Y, int H,
                         SampleYFn sampleY) {
            bool in_gap = false;
            bool started = false;
            int prev_px = 0, prev_py = 0;
            for (int c = 0; c < cols; ++c) {
                int px = X + c;
                int py = sampleY(vis[c]);
                if (!started) { prev_px = px; prev_py = py; started = true; continue; }
                if (!in_gap)
                    fl_line(prev_px, prev_py, px, py);
                prev_px = px; prev_py = py;
            }
        }
    };
    
}
