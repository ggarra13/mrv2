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
    } // namespace timeline
} // namespace mrv

void mrv2_timeline(pybind11::module& m)
{
    using namespace tl;

    py::module imaging = m.def_submodule("imaging");

    py::class_<imaging::Mirror>(imaging, "Mirror")
        .def(py::init<>())
        .def_readwrite("x", &imaging::Mirror::x)
        .def_readwrite("y", &imaging::Mirror::y)
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
        .def_readwrite("add", &timeline::Color::add)
        .def_readwrite("brightness", &timeline::Color::brightness)
        .def_readwrite("contrast", &timeline::Color::contrast)
        .def_readwrite("saturation", &timeline::Color::saturation)
        .def_readwrite("tint", &timeline::Color::tint)
        .def_readwrite("tinver", &timeline::Color::invert)
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
        .def_readwrite("inLow", &timeline::Levels::inLow)
        .def_readwrite("inHigh", &timeline::Levels::inHigh)
        .def_readwrite("gamma", &timeline::Levels::gamma)
        .def_readwrite("outLow", &timeline::Levels::outLow)
        .def_readwrite("outHigh", &timeline::Levels::outHigh)
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
        .def_readwrite("minify", &timeline::ImageFilters::minify)
        .def_readwrite("magnify", &timeline::ImageFilters::magnify)
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
        .def_readwrite("channels", &timeline::DisplayOptions::channels)
        .def_readwrite("mirror", &timeline::DisplayOptions::mirror)
        .def_readwrite("colorEnabled", &timeline::DisplayOptions::colorEnabled)
        .def_readwrite("color", &timeline::DisplayOptions::color)
        .def_readwrite(
            "levelsEnabled", &timeline::DisplayOptions::levelsEnabled)
        .def_readwrite("levels", &timeline::DisplayOptions::levels)
        .def_readwrite(
            "softClipEnabled", &timeline::DisplayOptions::softClipEnabled)
        .def_readwrite("softClip", &timeline::DisplayOptions::softClip)
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
        .def_readwrite("fileName", &timeline::LUTOptions::fileName)
        .def_readwrite("order", &timeline::LUTOptions::order)
        .doc() = _("LUT options.");

    py::class_<timeline::ImageOptions>(timeline, "ImageOptions")
        .def(py::init<>())
        .def_readwrite("videoLevels", &timeline::ImageOptions::videoLevels)
        .def_readwrite("alphaBlend", &timeline::ImageOptions::alphaBlend)
        .def_readwrite("imageFilters", &timeline::ImageOptions::imageFilters)
        .doc() = _("Image options.");

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

    timeline.def("time", &mrv::timeline::time, _("Current time in timeline."));
    timeline.def(
        "frame", &mrv::timeline::frame, _("Current frame in timeline."));
    timeline.def(
        "seconds", &mrv::timeline::seconds, _("Current seconds in timeline."));

    py::class_<timeline::CompareOptions>(timeline, "CompareOptions")
        .def(py::init<>())
        .def_readwrite("mode", &timeline::CompareOptions::mode)
        .def_readwrite("wipeCenter", &timeline::CompareOptions::wipeCenter)
        .def_readwrite("wipeRotation", &timeline::CompareOptions::wipeRotation)
        .def_readwrite("overlay", &timeline::CompareOptions::overlay)
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
}
