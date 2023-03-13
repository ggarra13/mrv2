// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <tlCore/Image.h>

#include <tlTimeline/LUTOptions.h>
#include <tlTimeline/ImageOptions.h>
#include <tlTimeline/CompareOptions.h>
#include <tlTimeline/TimelinePlayer.h>

#include <pybind11/pybind11.h>

namespace py = pybind11;

void mrv2_enums(pybind11::module& m)
{
    using namespace tl;

    py::module imaging = m.def_submodule("imaging");

    py::enum_<imaging::VideoLevels>(imaging, "VideoLevels")
        .value("FullRange", imaging::VideoLevels::FullRange)
        .value("LegalRange", imaging::VideoLevels::LegalRange)
        .export_values();

    py::enum_<imaging::YUVCoefficients>(imaging, "YUVCoefficients")
        .value("REC709", imaging::YUVCoefficients::REC709)
        .value("BT2020", imaging::YUVCoefficients::BT2020)
        .export_values();

    py::module timeline = m.def_submodule("timeline");

    py::enum_<timeline::CompareMode>(timeline, "CompareMode")
        .value("A", timeline::CompareMode::A)
        .value("B", timeline::CompareMode::B)
        .value("Wipe", timeline::CompareMode::Wipe)
        .value("Overlay", timeline::CompareMode::Overlay)
        .value("Difference", timeline::CompareMode::Difference)
        .value("Horizontal", timeline::CompareMode::Horizontal)
        .value("Vertical", timeline::CompareMode::Vertical)
        .value("Tile", timeline::CompareMode::Tile)
        .export_values();

    py::enum_<timeline::Playback>(timeline, "Playback")
        .value("Stop", timeline::Playback::Stop)
        .value("Forward", timeline::Playback::Forward)
        .value("Reverse", timeline::Playback::Reverse)
        .export_values();

    py::enum_<timeline::Loop>(timeline, "Loop")
        .value("Loop", timeline::Loop::Loop)
        .value("Once", timeline::Loop::Once)
        .value("PingPong", timeline::Loop::PingPong)
        .export_values();

    py::enum_<timeline::Channels>(timeline, "Channels")
        .value("Color", timeline::Channels::Color)
        .value("Red", timeline::Channels::Red)
        .value("Green", timeline::Channels::Green)
        .value("Blue", timeline::Channels::Blue)
        .value("Alpha", timeline::Channels::Alpha)
        .export_values();

    py::enum_<timeline::InputVideoLevels>(timeline, "InputVideoLevels")
        .value("FromFile", timeline::InputVideoLevels::FromFile)
        .value("FullRange", timeline::InputVideoLevels::FullRange)
        .value("LegalRange", timeline::InputVideoLevels::LegalRange)
        .export_values();

    py::enum_<timeline::AlphaBlend>(timeline, "AlphaBlend")
        .value("None", timeline::AlphaBlend::None)
        .value("Straight", timeline::AlphaBlend::Straight)
        .value("Premultiplied", timeline::AlphaBlend::Premultiplied)
        .export_values();

    py::enum_<timeline::ImageFilter>(timeline, "ImageFilter")
        .value("Nearest", timeline::ImageFilter::Nearest)
        .value("Linear", timeline::ImageFilter::Linear)
        .export_values();

    py::enum_<timeline::LUTOrder>(timeline, "LUTOrder")
        .value("PostColorConfig", timeline::LUTOrder::PostColorConfig)
        .value("PreColorConfig", timeline::LUTOrder::PreColorConfig)
        .export_values();
}
