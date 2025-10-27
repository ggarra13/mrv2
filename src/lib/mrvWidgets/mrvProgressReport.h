// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <chrono>

class Fl_Window;
class Fl_Progress;
class Fl_Output;

namespace mrv
{

    class ProgressReport
    {
    public:
        ProgressReport(
            Fl_Window* main, int64_t start, int64_t end, const char* title);
        ~ProgressReport();

        Fl_Window* window() const { return w; }

        bool tick();

        void show(); // hides automatically when it goes out of scope.

    protected:
        //! Convert a double in seconds to hour, minutes, seconds and
        //! milliseconds
        void
        to_hour_min_sec(const double t, int& hour, int& min, int& sec, int& ms);

    protected:
        Fl_Window* w;
        Fl_Progress* progress;
        Fl_Output* elapsed;
        Fl_Output* remain;
        Fl_Output* fps;

        int64_t _frame;
        int64_t _end;
        int64_t _start;

        double _frameDuration = 0;
        double _actualFrameRate = 0.F;
        int _framesSinceLastFpsFrame = 0;

        std::chrono::steady_clock::time_point _startTime;
        std::chrono::steady_clock::time_point _lastFpsFrameTime;
    };

} // namespace mrv
