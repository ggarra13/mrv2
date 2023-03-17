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

#include "mrViewer.h"

namespace tl
{
    namespace imaging
    {
        inline std::ostream& operator<<(std::ostream& o, const Mirror& a)
        {
            o << "<mrv2.imaging.Mirror x=" << (a.x ? "True" : "False")
              << " y=" << (a.y ? "True" : "False") << ">";
            return o;
        }
    } // namespace imaging

    namespace timeline
    {
        inline std::ostream& operator<<(std::ostream& o, const ImageFilters& a)
        {
            o << "<mrv2.timeline.ImageFilters minify=" << a.minify
              << " magnify=" << a.magnify << ">";
            return o;
        }
        inline std::ostream& operator<<(std::ostream& o, const Color& a)
        {
            o << "<mrv2.timeline.Color add=" << a.add
              << " brightness=" << a.brightness << " contrast=" << a.contrast
              << " saturation=" << a.saturation << " tint=" << a.tint
              << " invert=" << (a.invert ? "True" : "False") << ">";
            return o;
        }

        inline std::ostream& operator<<(std::ostream& o, const Levels& a)
        {
            o << "<mrv2.timeline.Levels inLow=" << a.inLow
              << " inHigh=" << a.inHigh << " gamma=" << a.gamma
              << " outLow=" << a.outLow << " outHigh=" << a.outHigh << ">";
            return o;
        }

        inline std::ostream&
        operator<<(std::ostream& o, const DisplayOptions& a)
        {
            o << "<mrv2.timeline.DisplayOptions channels" << a.channels
              << " mirror=" << a.mirror << " colorEnabled=" << a.colorEnabled
              << " color=" << a.color << " levelsEnabled=" << a.levelsEnabled
              << " levels=" << a.levels
              << " softClipEnabled=" << a.softClipEnabled
              << " softClip=" << a.softClip
              << " imageFilters=" << a.imageFilters
              << " videoLevels=" << a.videoLevels << ">";
            return o;
        }
    } // namespace timeline
} // namespace tl

namespace mrv
{
    namespace timeline
    {

        void stop()
        {
            Preferences::ui->uiView->stop();
        }

        void seek(const otime::RationalTime& t)
        {
            auto player = Preferences::ui->uiView->getTimelinePlayer();
            if (!player)
                return;
            player->seek(t);
        }

        void seek(const int64_t& frame)
        {
            auto player = Preferences::ui->uiView->getTimelinePlayer();
            if (!player)
                return;
            otime::RationalTime t(frame, player->defaultSpeed());
            player->seek(t);
        }

        void seek(const double seconds)
        {
            auto player = Preferences::ui->uiView->getTimelinePlayer();
            if (!player)
                return;
            otime::RationalTime t(seconds, 1.0);
            player->seek(t);
        }

        void playForwards()
        {
            Preferences::ui->uiView->playForwards();
        }

        void playBackwards()
        {
            Preferences::ui->uiView->playBackwards();
        }

        void frameNext()
        {
            Preferences::ui->uiView->frameNext();
        }

        void framePrev()
        {
            Preferences::ui->uiView->framePrev();
        }

        otime::RationalTime time()
        {
            auto player = Preferences::ui->uiView->getTimelinePlayer();
            if (!player)
                return tl::time::invalidTime;

            return player->currentTime();
        }

        int64_t frame()
        {
            auto player = Preferences::ui->uiView->getTimelinePlayer();
            if (!player)
                return 0;

            otime::RationalTime t = player->currentTime();
            return t.to_frames();
        }

        double seconds()
        {
            auto player = Preferences::ui->uiView->getTimelinePlayer();
            if (!player)
                return 0.0;

            otime::RationalTime t = player->currentTime();
            return t.to_seconds();
        }

        tl::timeline::Loop loop()
        {
            auto player = Preferences::ui->uiView->getTimelinePlayer();
            if (!player)
                return tl::timeline::Loop::Loop;

            return player->loop();
        }

        void setLoop(const tl::timeline::Loop& value)
        {
            TimelineClass* c = Preferences::ui->uiTimeWindow;
            c->uiLoopMode->value(static_cast<int>(value));
            c->uiLoopMode->do_callback();
        }

        otio::TimeRange timeRange()
        {
            auto player = Preferences::ui->uiView->getTimelinePlayer();
            if (!player) return otio::TimeRange();

            return player->timeRange();
        }
        
        otio::TimeRange inOutRange()
        {
            auto player = Preferences::ui->uiView->getTimelinePlayer();
            if (!player) return otio::TimeRange();

            return player->inOutRange();
        }

        void setInOutRange( const otio::TimeRange& value )
        {
            auto player = Preferences::ui->uiView->getTimelinePlayer();
            if (!player) return;

            player->setInOutRange(value);

            TimelineClass* c = Preferences::ui->uiTimeWindow;
            c->uiTimeline->redraw();
        }
        
        void setIn( const otio::RationalTime& value )
        {
            auto player = Preferences::ui->uiView->getTimelinePlayer();
            if (!player) return;

            otio::TimeRange range = player->inOutRange();
            const auto& endTime = range.end_time_exclusive();
            
            auto new_range = otime::TimeRange::range_from_start_end_time( value,
                                                                          endTime );
            setInOutRange(new_range);
        }
        
        void setIn( const int64_t& value )
        {
            auto player = Preferences::ui->uiView->getTimelinePlayer();
            if (!player) return;

            otio::TimeRange range = player->inOutRange();
            const auto& endTime = range.end_time_exclusive();
            otime::RationalTime time = otime::RationalTime( value, endTime.rate() );
            setIn(time);
        }
        
        void setIn( const double& value )
        {
            auto player = Preferences::ui->uiView->getTimelinePlayer();
            if (!player) return;

            otio::TimeRange range = player->inOutRange();
            const auto& endTime = range.end_time_exclusive();
            otime::RationalTime time = otime::RationalTime( value, 1.0 );
            setIn(time);
        }
        
        void setOut( const otio::RationalTime& value )
        {
            auto player = Preferences::ui->uiView->getTimelinePlayer();
            if (!player) return;

            otio::TimeRange range = player->inOutRange();
            const auto& startTime = range.start_time();
            
            auto new_range = otime::TimeRange::range_from_start_end_time( startTime,
                                                                          value );
            setInOutRange(new_range);
        }
        
        void setOut( const int64_t& value )
        {
            auto player = Preferences::ui->uiView->getTimelinePlayer();
            if (!player) return;

            otio::TimeRange range = player->inOutRange();
            const auto& startTime = range.start_time();
            otime::RationalTime time = otime::RationalTime( value, startTime.rate() );
            setOut(time);
        }
        
        void setOut( const double& value )
        {
            auto player = Preferences::ui->uiView->getTimelinePlayer();
            if (!player) return;

            otio::TimeRange range = player->inOutRange();
            const auto& startTime = range.start_time();
            otime::RationalTime time = otime::RationalTime( value, 1.0 );
            setOut(time);
        }
        
    } // namespace timeline
} // namespace mrv

void mrv2_timeline(pybind11::module& m)
{
    using namespace tl;

    py::module imaging = m.def_submodule("imaging");

    py::class_<imaging::Mirror>(imaging, "Mirror")
        .def(py::init<>())
        .def_readwrite("x", &imaging::Mirror::x, _("Flip image on X."))
        .def_readwrite("y", &imaging::Mirror::y, _("Flip image on Y."))
        .def(
            "__repr__",
            [](const imaging::Mirror& o)
            {
                std::ostringstream s;
                s << o;
                return s.str();
            })
        .doc() = _("Image mirroring.");

    py::module timeline = m.def_submodule("timeline");

    // Cannot be timeline as it clashes with timeline::Color class
    py::class_<timeline::Color>(m, "Color")
        .def(py::init<>())
        .def_readwrite("add", &timeline::Color::add,
                       _("Add a mrv2.math.Vector3f to image."))
        .def_readwrite("brightness", &timeline::Color::brightness,
                       _("Change a mrv2.math.Vector3f of brightness"
                         " to image."))
        .def_readwrite("contrast", &timeline::Color::contrast,
                       _("Change a mrv2.math.Vector3f of contrast"
                         " to image."))
        .def_readwrite("saturation", &timeline::Color::saturation,
                       _("Change a mrv2.math.Vector3f of saturation"
                         " to image."))
        .def_readwrite("tint", &timeline::Color::tint,
                       _("Change tint of image to image between 0 and 1."))
        .def_readwrite("invert", &timeline::Color::invert,
                       _("Invert the color values."))
        .def(
            "__repr__",
            [](const timeline::Color& o)
            {
                std::ostringstream s;
                s << o;
                return s.str();
            })
        .doc() = _("Color values.");

    py::class_<timeline::Levels>(timeline, "Levels")
        .def(py::init<>())
        .def_readwrite("inLow", &timeline::Levels::inLow,
                       _("In Low Level value."))
        .def_readwrite("inHigh", &timeline::Levels::inHigh,
                       _("In High Level value."))
    .def_readwrite("gamma", &timeline::Levels::gamma,
                       _("Gamma Level value."))
        .def_readwrite("outLow", &timeline::Levels::outLow,
                       _("Out Low Level value."))
        .def_readwrite("outHigh", &timeline::Levels::outHigh,
                       _("Out High Level value."))
        .def(
            "__repr__",
            [](const timeline::Levels& o)
            {
                std::ostringstream s;
                s << o;
                return s.str();
            })
        .doc() = _("Levels values.");

    py::class_<timeline::ImageFilters>(timeline, "ImageFilters")
        .def(py::init<>())
        .def_readwrite("minify", &timeline::ImageFilters::minify,
                       _(R"PYTHON(Minify filter. One of:
* timeline.ImageFilter.Nearest
* timeline.ImageFilter.Linear)PYTHON"))
        .def_readwrite("magnify", &timeline::ImageFilters::magnify,
                       _(R"PYTHON(Magnify filter. One of:
* timeline.ImageFilter.Nearest
* timeline.ImageFilter.Linear)PYTHON"))
        .def(
            "__repr__",
            [](const timeline::Levels& o)
            {
                std::ostringstream s;
                s << o;
                return s.str();
            })
        .doc() = _("Image filters.");

    py::class_<timeline::DisplayOptions>(timeline, "DisplayOptions")
        .def(py::init<>())
        .def_readwrite("channels", &timeline::DisplayOptions::channels,
                       _("Color channels."))
        .def_readwrite("mirror", &timeline::DisplayOptions::mirror,
                       _("Mirror on X, Y or both."))
        .def_readwrite("colorEnabled", &timeline::DisplayOptions::colorEnabled,
                       _("Enable color transforms."))
        .def_readwrite("color", &timeline::DisplayOptions::color,
                       _("Color options"))
        .def_readwrite(
            "levelsEnabled", &timeline::DisplayOptions::levelsEnabled,
            _("Enable levels transforms."))
        .def_readwrite("levels", &timeline::DisplayOptions::levels,
                       _("Levels options."))
        .def_readwrite(
            "softClipEnabled", &timeline::DisplayOptions::softClipEnabled,
            _("Enable soft clip."))
        .def_readwrite("softClip", &timeline::DisplayOptions::softClip,
                       _("Soft clip value."))
        .def(
            "__repr__",
            [](const timeline::DisplayOptions& o)
                {
                    std::ostringstream s;
                    s << o;
                    return s.str();
                })
        .doc() = _("Display options.");

    py::class_<timeline::LUTOptions>(timeline, "LUTOptions")
        .def(py::init<>())
        .def_readwrite("fileName", &timeline::LUTOptions::fileName,
                       _("LUT filename."))
        .def_readwrite("order", &timeline::LUTOptions::order,
                       _("LUT transformation order."))
        .doc() = _("LUT options.");

    py::class_<timeline::ImageOptions>(timeline, "ImageOptions")
        .def(py::init<>())
        .def_readwrite("videoLevels", &timeline::ImageOptions::videoLevels,
                       _("Video Levels."))
        .def_readwrite("alphaBlend", &timeline::ImageOptions::alphaBlend,
                       _("Alpha blending algorithm"))
        .def_readwrite("imageFilters", &timeline::ImageOptions::imageFilters,
                       _("Image Filters"))
        .doc() = _("Image options.");
    
    py::class_<timeline::CompareOptions>(timeline, "CompareOptions")
        .def(py::init<>())
        .def_readwrite("mode", &timeline::CompareOptions::mode,
                       _(R"PYTHON(Compare mode.  One of:
           * mrv2.CompareMode.A 
           * mrv2.CompareMode.B
           * mrv2.CompareMode.Wipe
           * mrv2.CompareMode.Overlay
           * mrv2.CompareMode.Horizontal
           * mrv2.CompareMode.Vertical
           * mrv2.CompareMode.Tile)PYTHON"))
        .def_readwrite("wipeCenter", &timeline::CompareOptions::wipeCenter,
                       _("Wipe center in X and Y") )
        .def_readwrite("wipeRotation", &timeline::CompareOptions::wipeRotation,
                       _("Wipe Rotation") )
        .def_readwrite("overlay", &timeline::CompareOptions::overlay,
                       _("Overlay ( A over B )") )
        .def(
            "__repr__",
            [](const timeline::CompareOptions& o)
            {
                std::stringstream s;
                s << "<mrv2.timeline.CompareOptions mode=" << o.mode
                  << " wipeCenter=" << o.wipeCenter
                  << " wipeRotation=" << o.wipeRotation
                  << " overlay=" << o.overlay << ">";
                return s.str();
            })
        .doc() = "Comparison options.";

    timeline.def(
        "playForwards", &mrv::timeline::playForwards, _("Play forwards."));
    timeline.def(
        "playBackwards", &mrv::timeline::playBackwards, _("Play backwards."));
    timeline.def("stop", &mrv::timeline::stop, _("Play forwards."));
    timeline.def(
        "seek",
        py::overload_cast<const otime::RationalTime&>(&mrv::timeline::seek),
        _("Seek to a time in timeline."), py::arg("time"));
    timeline.def(
        "seek", py::overload_cast<const int64_t&>(&mrv::timeline::seek),
        _("Seek to a frame in timeline."), py::arg("frame"));
    timeline.def(
        "seek", py::overload_cast<const double>(&mrv::timeline::seek),
        _("Seek to a second in timeline."), py::arg("frame"));

    timeline.def("timeRange", &mrv::timeline::timeRange, _("Time range of the timeline."));
    
    timeline.def("inOutRange", &mrv::timeline::inOutRange, _("Selected time range of the timeline."));
    
    timeline.def("setInOutRange", &mrv::timeline::setInOutRange, _("Set the selected time range of the timeline."));
    
    timeline.def("setIn",
                 py::overload_cast<const otime::RationalTime&>(&mrv::timeline::setIn), _("Set the in time of the selected time range of the timeline."));
    timeline.def("setIn",
                 py::overload_cast<const int64_t&>(&mrv::timeline::setIn), _("Set the in frame of the selected time range of the timeline."));
    timeline.def("setIn",
                 py::overload_cast<const double&>(&mrv::timeline::setIn), _("Set the in seconds of the selected time range of the timeline."));

    
    timeline.def("setOut",
                 py::overload_cast<const otime::RationalTime&>(&mrv::timeline::setOut), _("Set the out time of the selected time range of the timeline."));
    timeline.def("setOut",
                 py::overload_cast<const int64_t&>(&mrv::timeline::setOut), _("Set the out frame of the selected time range of the timeline."));
    timeline.def("setOut",
                 py::overload_cast<const double&>(&mrv::timeline::setOut), _("Set the out seconds of the selected time range of the timeline."));
    
    
    timeline.def("time", &mrv::timeline::time, _("Current time in timeline."));
    timeline.def(
        "frame", &mrv::timeline::frame, _("Current frame in timeline."));
    timeline.def(
        "seconds", &mrv::timeline::seconds, _("Current seconds in timeline."));


    timeline.def(
        "loop", &mrv::timeline::loop,
        _("Return current loop mode of timeline."));
    timeline.def(
        "setLoop", &mrv::timeline::setLoop,
        _("Set current loop mode of timeline."));
}
