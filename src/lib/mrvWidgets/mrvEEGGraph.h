// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the feather-tk project.

#pragma once

#include <FL/Fl_Widget.H>

#include <cstdint>
#include <map>
#include <string>
#include <vector>

namespace mrv
{
    //! Single-group EEG-style graph widget.
    //!
    //! Each EEGGraph instance represents one StatsSystem *group*.
    //! Every StatsSystem *name* within that group is drawn as a separate
    //! colour-coded trace on the shared canvas.
    //!
    //! The full StatsSystem key format is "Group/Name", e.g.
    //!   "Image Memory/Images: {0}MB"
    //!    ^^^^^^^^^^^^  ^^^^^^^^^^^^^^^^
    //!    → group_       → one trace
    //!
    //! Typical wiring (one observer per EEGGraph):
    //! @code
    //!   auto graph = new mrv::EEGGraph(x, y, w, h, "Image Memory");
    //!   graph->setGroup("Image Memory");
    //!
    //!   _samplesObserver = observer::MapObserver<std::string,
    //!       std::vector<int64_t>>::create(
    //!           statsSystem->observeSamples(),
    //!           [graph](const std::map<std::string,
    //!                                  std::vector<int64_t>>& v)
    //!           {
    //!               graph->setSamples(v);   // filters by group internally
    //!               graph->redraw();
    //!           });
    //! @endcode
    class EEGGraph : public Fl_Widget
    {
    public:
        //! Per-trace visual configuration.
        //! Keyed by the *name* part of the StatsSystem id (after the '/').
        //! Auto-populated with defaults on first appearance.
        struct TraceStyle
        {
            Fl_Color fg         = 0x32FF5000; // green
            float    line_width = 2.0f;
            int64_t  y_min      = 0;
            int64_t  y_max      = 100;
        };

        //! @param group  Must match the StatsSystem group name exactly.
        EEGGraph(int X, int Y, int W, int H, const char* L = nullptr);

        //! Set or change the StatsSystem group this widget represents.
        void setGroup(const std::string& group);
        const std::string& getGroup() const;

        //! Receive the full StatsSystem sample map.
        //! The widget extracts only the entries belonging to its group.
        void setSamples(const std::map<std::string,
                                       std::vector<int64_t>>& allSamples);

        //! Override the visual style for one trace (identified by *name*,
        //! i.e. the part after the '/' in the StatsSystem id).
        //! May be called before the trace appears in any sample data.
        void setTraceStyle(const std::string& name, const TraceStyle& style);
        const TraceStyle& getTraceStyle(const std::string& name) const;

        //! Clear displayed data (styles are preserved).
        void clear();

        void draw() override;

    private:
        template<typename SampleYFn>
        void _draw_trace(
            const std::vector<int64_t>& vis,
            int cols, int X, int Y, int CH,
            SampleYFn sampleY) const;

        Fl_Color _next_color() const;

        //! The StatsSystem group this widget belongs to.
        std::string group_;

        //! Filtered snapshot: key = *name* (after '/'), value = sample vector.
        std::map<std::string, std::vector<int64_t>> traces_;

        //! Per-trace styles, keyed by *name*. Persists across setSamples().
        mutable std::map<std::string, TraceStyle> styles_;

        Fl_Color bg_;

        static constexpr Fl_Color kPalette[] = {
            0x32FF5000,  // green
            0x50A0FF00,  // blue
            0xFFC83200,  // amber
            0xFF505000,  // red
            0xC850FF00,  // violet
            0x00E0E000,  // cyan
        };
        static constexpr int kPaletteSize = 6;
    };

} // namespace mrv
