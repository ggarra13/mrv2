// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <iostream>
#include <sstream>

#include <pybind11/pybind11.h>

namespace py = pybind11;

#include <tlCore/Image.h>
#include <tlTimeline/ImageOptions.h>
#include <tlTimeline/LUTOptions.h>
#include <tlTimeline/DisplayOptions.h>

#include "mrvCore/mrvI8N.h"

#include "mrvApp/App.h"

#include "mrViewer.h"

namespace mrv2
{
    using namespace mrv;

    namespace timeline
    {
        void setInOutRange(const otio::TimeRange& value);

        /**
         * @brief Return the current timeline position in frames.
         *
         *
         * @return in64_t frame
         */
        int64_t frame()
        {
            auto player = App::ui->uiView->getTimelinePlayer();
            if (!player)
                return 0;

            otime::RationalTime t = player->currentTime();
            return t.to_frames();
        }

        /**
         * @brief Go to next frame.
         *
         */
        void frameNext()
        {
            App::ui->uiView->frameNext();
        }

        /**
         * @brief Go to previous frame.
         *
         */
        void framePrev()
        {
            App::ui->uiView->framePrev();
        }

        /**
         * @brief Return the in/out range of the timeline.
         *
         * This is the start and end of the timeline taking into account its
         * in and out points.
         *
         * @return TimeRange
         */
        otio::TimeRange inOutRange()
        {
            auto player = App::ui->uiView->getTimelinePlayer();
            if (!player)
                return otio::TimeRange();

            return player->inOutRange();
        }

        /**
         * @brief Return the current loop setting in timeline.
         *
         *
         * @return timeline.Loop enum.
         */
        tl::timeline::Loop loop()
        {
            auto player = App::ui->uiView->getTimelinePlayer();
            if (!player)
                return tl::timeline::Loop::Loop;

            return player->loop();
        }

        /**
         * @brief Play current timeline backwards.
         *
         */
        void playBackwards()
        {
            App::ui->uiView->playBackwards();
        }

        /**
         * @brief Play current timeline forward.
         *
         */
        void playForwards()
        {
            App::ui->uiView->playForwards();
        }

        /**
         * @brief Return the current timeline position in seconds.
         *
         *
         * @return double seconds.
         */
        double seconds()
        {
            auto player = App::ui->uiView->getTimelinePlayer();
            if (!player)
                return 0.0;

            otime::RationalTime t = player->currentTime();
            return t.to_seconds();
        }

        /**
         * @brief Seek to a RationalTime.
         *
         * @param t RationalTime
         */
        void seek(const otime::RationalTime& t)
        {
            auto player = App::ui->uiView->getTimelinePlayer();
            if (!player)
                return;
            player->seek(t);
        }

        /**
         * @brief Seek to a frame
         *
         * @param frame int64_t
         */
        void seek(const int64_t& frame)
        {
            auto player = App::ui->uiView->getTimelinePlayer();
            if (!player)
                return;
            otime::RationalTime t(frame, player->defaultSpeed());
            player->seek(t);
        }

        /**
         * @brief Seek to a time in seconds.
         *
         * @param seconds float
         */
        void seek(const double seconds)
        {
            auto player = App::ui->uiView->getTimelinePlayer();
            if (!player)
                return;
            otime::RationalTime t(seconds, 1.0);
            player->seek(t);
        }

        /**
         * @brief Stop playback.
         *
         */
        void stop()
        {
            App::ui->uiView->stop();
        }

        /**
         * @brief Set the In point as a RationalTime.
         *
         * @param value RationalTime.
         */
        void setIn(const otio::RationalTime& value)
        {
            auto player = App::ui->uiView->getTimelinePlayer();
            if (!player)
                return;

            otio::TimeRange range = player->inOutRange();
            const auto& endTime = range.end_time_exclusive();

            auto new_range =
                otime::TimeRange::range_from_start_end_time(value, endTime);
            setInOutRange(new_range);
        }

        /**
         * @brief Set the In point as a frame
         *
         * @param value frame int64_t
         */
        void setIn(const int64_t& value)
        {
            auto player = App::ui->uiView->getTimelinePlayer();
            if (!player)
                return;

            otio::TimeRange range = player->inOutRange();
            const auto& endTime = range.end_time_exclusive();
            otime::RationalTime time =
                otime::RationalTime(value, endTime.rate());
            setIn(time);
        }

        /**
         * @brief Set the In point as seconds.
         *
         * @param value double.
         */
        void setIn(const double& value)
        {
            auto player = App::ui->uiView->getTimelinePlayer();
            if (!player)
                return;

            otio::TimeRange range = player->inOutRange();
            const auto& endTime = range.end_time_exclusive();
            otime::RationalTime time = otime::RationalTime(value, 1.0);
            setIn(time);
        }

        /**
         * @brief Set the In/Out range of the timeline.
         *
         * @param value TimeRange.
         */
        void setInOutRange(const otio::TimeRange& value)
        {
            auto player = App::ui->uiView->getTimelinePlayer();
            if (!player)
                return;

            player->setInOutRange(value);

            App::ui->uiTimeline->redraw();
        };

        /**
         * @brief Set the Out point as a RationalTime.
         *
         * @param value RationalTime.
         */
        void setOut(const otio::RationalTime& value)
        {
            auto player = App::ui->uiView->getTimelinePlayer();
            if (!player)
                return;

            otio::TimeRange range = player->inOutRange();
            const auto& startTime = range.start_time();

            auto new_range =
                otime::TimeRange::range_from_start_end_time(startTime, value);
            setInOutRange(new_range);
        }

        /**
         * @brief Set the Out point as a frame
         *
         * @param value frame int64_t
         */
        void setOut(const int64_t& value)
        {
            auto player = App::ui->uiView->getTimelinePlayer();
            if (!player)
                return;

            otio::TimeRange range = player->inOutRange();
            const auto& startTime = range.start_time();
            otime::RationalTime time =
                otime::RationalTime(value, startTime.rate());
            setOut(time);
        }

        /**
         * @brief Set the Out point as seconds.
         *
         * @param value double.
         */
        void setOut(const double& value)
        {
            auto player = App::ui->uiView->getTimelinePlayer();
            if (!player)
                return;

            otio::TimeRange range = player->inOutRange();
            const auto& startTime = range.start_time();
            otime::RationalTime time = otime::RationalTime(value, 1.0);
            setOut(time);
        }

        void setLoop(const tl::timeline::Loop value)
        {
            TimelineClass* c = App::ui->uiTimeWindow;
            c->uiLoopMode->value(static_cast<int>(value));
            c->uiLoopMode->do_callback();
        }

        /**
         * @brief Return current timeline position in RationTime.
         *
         *
         * @return RationalTime
         */
        otime::RationalTime time()
        {
            auto player = App::ui->uiView->getTimelinePlayer();
            if (!player)
                return tl::time::invalidTime;

            return player->currentTime();
        }

        /**
         * @brief Return the current time range of the timeline.
         *
         * This is the start and ending of the timeline with no in/out points.
         *
         * @return TimeRange
         */
        otio::TimeRange timeRange()
        {
            auto player = App::ui->uiView->getTimelinePlayer();
            if (!player)
                return otio::TimeRange();

            return player->timeRange();
        }

        /**
         * @brief Sets the FPS.
         *
         * @return TimeRange
         */
        double defaultSpeed(double fps)
        {
            auto player = App::ui->uiView->getTimelinePlayer();
            if (!player)
                return 24.0;

            return player->defaultSpeed();
        }

        /**
         * @brief Sets the FPS.
         *
         * @return TimeRange
         */
        double speed(double fps)
        {
            auto player = App::ui->uiView->getTimelinePlayer();
            if (!player)
                return 24.0;

            return player->speed();
        }

        /**
         * @brief Sets the FPS.
         *
         * @return TimeRange
         */
        void setSpeed(double value)
        {
            TimelineClass* c = App::ui->uiTimeWindow;
            c->uiFPS->value(value);
            c->uiFPS->do_callback();
        }

    } // namespace timeline
} // namespace mrv2

void mrv2_timeline(pybind11::module& m)
{
    using namespace tl;

    py::module timeline = m.def_submodule("timeline");
    timeline.doc() = _(R"PYTHON(
Timeline module.

Contains all functions related to the timeline control.
)PYTHON");

    timeline.def(
        "playForwards", &mrv2::timeline::playForwards, _("Play forwards."));
    timeline.def(
        "playBackwards", &mrv2::timeline::playBackwards, _("Play backwards."));
    timeline.def("stop", &mrv2::timeline::stop, _("Play forwards."));
    timeline.def(
        "seek",
        py::overload_cast<const otime::RationalTime&>(&mrv2::timeline::seek),
        _("Seek to a time in timeline."), py::arg("time"));
    timeline.def(
        "seek", py::overload_cast<const int64_t&>(&mrv2::timeline::seek),
        _("Seek to a frame in timeline."), py::arg("frame"));
    timeline.def(
        "seek", py::overload_cast<const double>(&mrv2::timeline::seek),
        _("Seek to a second in timeline."), py::arg("frame"));

    timeline.def(
        "timeRange", &mrv2::timeline::timeRange,
        _("Time range of the timeline."));

    timeline.def(
        "inOutRange", &mrv2::timeline::inOutRange,
        _("Selected time range of the timeline."));

    timeline.def(
        "setInOutRange", &mrv2::timeline::setInOutRange,
        _("Set the selected time range of the timeline."));

    timeline.def(
        "setIn",
        py::overload_cast<const otime::RationalTime&>(&mrv2::timeline::setIn),
        _("Set the in time of the selected time range of the timeline."));
    timeline.def(
        "setIn", py::overload_cast<const int64_t&>(&mrv2::timeline::setIn),
        _("Set the in frame of the selected time range of the timeline."));
    timeline.def(
        "setIn", py::overload_cast<const double&>(&mrv2::timeline::setIn),
        _("Set the in seconds of the selected time range of the timeline."));

    timeline.def(
        "setOut",
        py::overload_cast<const otime::RationalTime&>(&mrv2::timeline::setOut),
        _("Set the out time of the selected time range of the timeline."));
    timeline.def(
        "setOut", py::overload_cast<const int64_t&>(&mrv2::timeline::setOut),
        _("Set the out frame of the selected time range of the timeline."));
    timeline.def(
        "setOut", py::overload_cast<const double&>(&mrv2::timeline::setOut),
        _("Set the out seconds of the selected time range of the timeline."));

    timeline.def("time", &mrv2::timeline::time, _("Current time in timeline."));
    timeline.def(
        "frame", &mrv2::timeline::frame, _("Current frame in timeline."));
    timeline.def(
        "seconds", &mrv2::timeline::seconds, _("Current seconds in timeline."));

    timeline.def(
        "loop", &mrv2::timeline::loop,
        _("Return current loop mode of timeline."));
    timeline.def(
        "setLoop", &mrv2::timeline::setLoop,
        _("Set current loop mode of timeline."));

    timeline.def(
        "speed", &mrv2::timeline::speed,
        _("Gets the current FPS of timeline."));

    timeline.def(
        "defaultSpeed", &mrv2::timeline::defaultSpeed,
        _("Gets the default speed of the timeline."));

    timeline.def(
        "setSpeed", &mrv2::timeline::setSpeed,
        _("Set current FPS of timeline."), py::arg("fps"));
}
