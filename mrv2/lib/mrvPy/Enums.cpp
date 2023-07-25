// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <tlCore/Image.h>

#include <tlIO/FFmpeg.h>
#include <tlIO/OpenEXR.h>

#include <tlTimeline/LUTOptions.h>
#include <tlTimeline/ImageOptions.h>
#include <tlTimeline/CompareOptions.h>
#include <tlTimeline/Timeline.h>
#include <tlTimeline/Player.h>

#include "mrvCore/mrvStereo3DOptions.h"
#include "mrvCore/mrvEnvironmentMapOptions.h"
#include "mrvFl/mrvSaveOptions.h"

#include <pybind11/pybind11.h>

namespace py = pybind11;

void mrv2_enums(py::module& m)
{
    using namespace tl;

    py::module io = m.def_submodule("io");

    py::enum_<ffmpeg::Profile>(io, "Profile")
        .value("None", ffmpeg::Profile::None)
        .value("H264", ffmpeg::Profile::H264)
        .value("ProRes", ffmpeg::Profile::ProRes)
        .value("ProRes_Proxy", ffmpeg::Profile::ProRes_Proxy)
        .value("ProRes_LT", ffmpeg::Profile::ProRes_LT)
        .value("ProRes_HQ", ffmpeg::Profile::ProRes_HQ)
        .value("ProRes_4444", ffmpeg::Profile::ProRes_4444)
        .value("ProRes_XQ", ffmpeg::Profile::ProRes_XQ);

    py::enum_<exr::Compression>(io, "Compression")
        .value("None", exr::Compression::None)
        .value("RLE", exr::Compression::RLE)
        .value("ZIPS", exr::Compression::ZIPS)
        .value("ZIP", exr::Compression::ZIP)
        .value("PIZ", exr::Compression::PIZ)
        .value("PXR24", exr::Compression::PXR24)
        .value("B44", exr::Compression::B44)
        .value("B44A", exr::Compression::B44A)
        .value("DWAA", exr::Compression::DWAA)
        .value("DWAB", exr::Compression::DWAB);

    py::module image = m.def_submodule("image");

    py::enum_<imaging::VideoLevels>(image, "VideoLevels")
        .value("FullRange", imaging::VideoLevels::FullRange)
        .value("LegalRange", imaging::VideoLevels::LegalRange)
        .export_values();

    py::enum_<imaging::YUVCoefficients>(image, "YUVCoefficients")
        .value("REC709", imaging::YUVCoefficients::REC709)
        .value("BT2020", imaging::YUVCoefficients::BT2020)
        .export_values();

    // We cannot export this one as Color conflicts with Color class
    py::enum_<timeline::Channels>(image, "Channels")
        .value("Color", timeline::Channels::Color)
        .value("Red", timeline::Channels::Red)
        .value("Green", timeline::Channels::Green)
        .value("Blue", timeline::Channels::Blue)
        .value("Alpha", timeline::Channels::Alpha);
    // .export_values();
    //

    py::enum_<timeline::InputVideoLevels>(image, "InputVideoLevels")
        .value("FromFile", timeline::InputVideoLevels::FromFile)
        .value("FullRange", timeline::InputVideoLevels::FullRange)
        .value("LegalRange", timeline::InputVideoLevels::LegalRange)
        .export_values();

    py::enum_<timeline::AlphaBlend>(image, "AlphaBlend")
        .value("None", timeline::AlphaBlend::None)
        .value("Straight", timeline::AlphaBlend::Straight)
        .value("Premultiplied", timeline::AlphaBlend::Premultiplied)
        .export_values();

    py::enum_<timeline::ImageFilter>(image, "ImageFilter")
        .value("Nearest", timeline::ImageFilter::Nearest)
        .value("Linear", timeline::ImageFilter::Linear)
        .export_values();

    py::enum_<timeline::LUTOrder>(image, "LUTOrder")
        .value("PostColorConfig", timeline::LUTOrder::PostColorConfig)
        .value("PreColorConfig", timeline::LUTOrder::PreColorConfig)
        .export_values();

    py::enum_<mrv::EnvironmentMapOptions::Type>(image, "EnvironmentMapType")
        .value("None", mrv::EnvironmentMapOptions::Type::kNone)
        .value("Spherical", mrv::EnvironmentMapOptions::Type::kSpherical)
        .value("Cubic", mrv::EnvironmentMapOptions::Type::kCubic);
    //.export_values();

    py::enum_<mrv::Stereo3DInput>(image, "Stereo3DInput")
        .value("None", mrv::Stereo3DInput::None)
        .value("Image", mrv::Stereo3DInput::Image)
        .export_values();

    py::enum_<mrv::Stereo3DOutput>(image, "Stereo3DOutput")
        .value("Anaglyph", mrv::Stereo3DOutput::Anaglyph)
        .value("Scanlines", mrv::Stereo3DOutput::Scanlines)
        .value("Columns", mrv::Stereo3DOutput::Columns)
        .value("Checkerboard", mrv::Stereo3DOutput::Checkerboard)
        .value("OpenGL", mrv::Stereo3DOutput::OpenGL)
        .export_values();

    py::module media = m.def_submodule("media");

    py::enum_<timeline::CompareMode>(media, "CompareMode")
        .value("A", timeline::CompareMode::A)
        .value("B", timeline::CompareMode::B)
        .value("Wipe", timeline::CompareMode::Wipe)
        .value("Overlay", timeline::CompareMode::Overlay)
        .value("Difference", timeline::CompareMode::Difference)
        .value("Horizontal", timeline::CompareMode::Horizontal)
        .value("Vertical", timeline::CompareMode::Vertical)
        .value("Tile", timeline::CompareMode::Tile)
        .export_values();

    py::module timeline = m.def_submodule("timeline");

    py::enum_<timeline::Playback>(timeline, "Playback")
        .value("Stop", timeline::Playback::Stop)
        .value("Forward", timeline::Playback::Forward)
        .value("Reverse", timeline::Playback::Reverse)
        .export_values();

    // We cannot export values of this class as Loop clashes with the class
    // name.
    py::enum_<timeline::Loop>(timeline, "Loop")
        .value("Loop", timeline::Loop::Loop)
        .value("Once", timeline::Loop::Once)
        .value("PingPong", timeline::Loop::PingPong);
    // .export_values();

    py::enum_<timeline::TimerMode>(timeline, "TimerMode")
        .value("System", timeline::TimerMode::System)
        .value("Audio", timeline::TimerMode::Audio)
        .export_values();

    py::enum_<timeline::FileSequenceAudio>(timeline, "FileSequenceAudio")
        .value("None", timeline::FileSequenceAudio::None)
        .value("BaseName", timeline::FileSequenceAudio::BaseName)
        .value("FileName", timeline::FileSequenceAudio::FileName)
        .value("Directory", timeline::FileSequenceAudio::Directory)
        .export_values();
}
