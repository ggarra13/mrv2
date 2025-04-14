// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <tlCore/Image.h>

#ifdef TLRENDER_FFMPEG
#    include <tlIO/FFmpeg.h>
#endif

#ifdef TLRENDER_EXR
#    include <tlIO/OpenEXR.h>
#endif

#ifdef TLRENDER_USD
#    include <tlIO/USD.h>
#endif

#include <tlTimeline/BackgroundOptions.h>
#include <tlTimeline/LUTOptions.h>
#include <tlTimeline/ImageOptions.h>
#include <tlTimeline/CompareOptions.h>
#include <tlTimeline/DisplayOptions.h>
#include <tlTimeline/Timeline.h>
#include <tlTimeline/Player.h>

#include "mrvOptions/mrvStereo3DOptions.h"
#include "mrvOptions/mrvEnvironmentMapOptions.h"

#include "mrvFl/mrvSaveOptions.h"

#include <pybind11/pybind11.h>

namespace py = pybind11;

void mrv2_enums(py::module& m)
{
    using namespace tl;

    py::module io = m.def_submodule("io");

    py::enum_<mrv::SaveResolution>(io, "Resolution")
        .value("Same_Size", mrv::SaveResolution::kSameSize)
        .value("Half_Size", mrv::SaveResolution::kHalfSize)
        .value("Quarter_Size", mrv::SaveResolution::kQuarterSize);

#ifdef TLRENDER_FFMPEG
    py::enum_<ffmpeg::Profile>(io, "Profile")
        .value("None", ffmpeg::Profile::kNone)
#    ifdef TLRENDER_X264
        .value("H264", ffmpeg::Profile::H264)
#    endif
        .value("ProRes", ffmpeg::Profile::ProRes)
        .value("ProRes_Proxy", ffmpeg::Profile::ProRes_Proxy)
        .value("ProRes_LT", ffmpeg::Profile::ProRes_LT)
        .value("ProRes_HQ", ffmpeg::Profile::ProRes_HQ)
        .value("ProRes_4444", ffmpeg::Profile::ProRes_4444)
        .value("ProRes_XQ", ffmpeg::Profile::ProRes_XQ)
        .value("VP9", ffmpeg::Profile::VP9)
        .value("Cineform", ffmpeg::Profile::Cineform)
        .value("AV1", ffmpeg::Profile::AV1);

    py::enum_<ffmpeg::AudioCodec>(io, "AudioCodec")
        .value("None", ffmpeg::AudioCodec::kNone)
        .value("AAC", ffmpeg::AudioCodec::AAC)
        .value("AC3", ffmpeg::AudioCodec::AC3)
        .value("True_HD", ffmpeg::AudioCodec::True_HD)
        .value("MP2", ffmpeg::AudioCodec::MP2)
        .value("MP3", ffmpeg::AudioCodec::MP3)
        .value("OPUS", ffmpeg::AudioCodec::OPUS)
        .value("VORBIS", ffmpeg::AudioCodec::VORBIS)
        .value("PCM_S16LE", ffmpeg::AudioCodec::PCM_S16LE);
#endif

#ifdef TLRENDER_EXR
    py::enum_<Imf::Compression>(io, "Compression")
        .value("None", Imf::NO_COMPRESSION)
        .value("RLE", Imf::RLE_COMPRESSION)
        .value("ZIPS", Imf::ZIPS_COMPRESSION)
        .value("ZIP", Imf::ZIP_COMPRESSION)
        .value("PIZ", Imf::PIZ_COMPRESSION)
        .value("PXR24", Imf::PXR24_COMPRESSION)
        .value("B44", Imf::B44_COMPRESSION)
        .value("B44A", Imf::B44A_COMPRESSION)
        .value("DWAA", Imf::DWAA_COMPRESSION)
        .value("DWAB", Imf::DWAB_COMPRESSION)
        //.value("HT256", Imf::HT256_COMPRESSION)
        ;
    
    py::enum_<mrv::SaveContents>(io, "Contents")
        .value("Display_Window", mrv::SaveContents::kDisplayWindow)
        .value("Data_Window", mrv::SaveContents::kDataWindow);
#endif

    py::module image = m.def_submodule("image");

    py::enum_<image::PixelType>(image, "PixelType")
        .value("None", image::PixelType::kNone)
        .value("L_U8", image::PixelType::L_U8)
        .value("L_U16", image::PixelType::L_U16)
        .value("L_U32", image::PixelType::L_U32)
        .value("L_F16", image::PixelType::L_F16)
        .value("L_F32", image::PixelType::L_F32)

        .value("LA_U8", image::PixelType::LA_U8)
        .value("LA_U16", image::PixelType::LA_U16)
        .value("LA_U32", image::PixelType::LA_U32)
        .value("LA_F16", image::PixelType::LA_F16)
        .value("LA_F32", image::PixelType::LA_F32)

        .value("RGB_U8", image::PixelType::RGB_U8)
        .value("RGB_U16", image::PixelType::RGB_U16)
        .value("RGB_U32", image::PixelType::RGB_U32)
        .value("RGB_F16", image::PixelType::RGB_F16)
        .value("RGB_F32", image::PixelType::RGB_F32)

        .value("RGBA_U8", image::PixelType::RGBA_U8)
        .value("RGBA_U16", image::PixelType::RGBA_U16)
        .value("RGBA_U32", image::PixelType::RGBA_U32)
        .value("RGBA_F16", image::PixelType::RGBA_F16)
        .value("RGBA_F32", image::PixelType::RGBA_F32)

        .value("YUV_420P_U8", image::PixelType::YUV_420P_U8)
        .value("YUV_422P_U8", image::PixelType::YUV_422P_U8)
        .value("YUV_444P_U8", image::PixelType::YUV_444P_U8)

        .value("YUV_420P_U16", image::PixelType::YUV_420P_U16)
        .value("YUV_422P_U16", image::PixelType::YUV_422P_U16)
        .value("YUV_444P_U16", image::PixelType::YUV_444P_U16);

    py::enum_<image::VideoLevels>(image, "VideoLevels")
        .value("FullRange", image::VideoLevels::FullRange)
        .value("LegalRange", image::VideoLevels::LegalRange)
        .export_values();

    py::enum_<image::YUVCoefficients>(image, "YUVCoefficients")
        .value("REC709", image::YUVCoefficients::REC709)
        .value("BT2020", image::YUVCoefficients::BT2020)
        .export_values();

    py::enum_<timeline::Background>(image, "Background")
        .value("Transparent", timeline::Background::Transparent)
        .value("Solid", timeline::Background::Solid)
        .value("Checkers", timeline::Background::Checkers)
        .value("Gradient", timeline::Background::Gradient)
        .export_values();

    // We cannot export this one as Color conflicts with Color class
    py::enum_<timeline::Channels>(image, "Channels")
        .value("Color", timeline::Channels::Color)
        .value("Red", timeline::Channels::Red)
        .value("Green", timeline::Channels::Green)
        .value("Blue", timeline::Channels::Blue)
        .value("Alpha", timeline::Channels::Alpha)
        .value("Lumma", timeline::Channels::Lumma);
    // .export_values();
    //

    py::enum_<timeline::InputVideoLevels>(image, "InputVideoLevels")
        .value("FromFile", timeline::InputVideoLevels::FromFile)
        .value("FullRange", timeline::InputVideoLevels::FullRange)
        .value("LegalRange", timeline::InputVideoLevels::LegalRange)
        .export_values();

    py::enum_<timeline::AlphaBlend>(image, "AlphaBlend")
        .value("None", timeline::AlphaBlend::kNone)
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
        .value("None", mrv::Stereo3DInput::kNone)
        .value("Image", mrv::Stereo3DInput::Image)
        .export_values();

    py::enum_<mrv::Stereo3DOutput>(image, "Stereo3DOutput")
        .value("Anaglyph", mrv::Stereo3DOutput::Anaglyph)
        .value("Scanlines", mrv::Stereo3DOutput::Scanlines)
        .value("Columns", mrv::Stereo3DOutput::Columns)
        .value("Checkerboard", mrv::Stereo3DOutput::Checkerboard)
#ifdef TLRENDER_VK
        .value("OpenGL", mrv::Stereo3DOutput::Vulkan)
#else
        .value("OpenGL", mrv::Stereo3DOutput::OpenGL)
#endif
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
    
    py::enum_<timeline::CompareTimeMode>(media, "CompareTimeMode")
        .value("Relative", timeline::CompareTimeMode::Relative)
        .value("Absolute", timeline::CompareTimeMode::Absolute)
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
        .value("None", timeline::FileSequenceAudio::kNone)
        .value("BaseName", timeline::FileSequenceAudio::BaseName)
        .value("FileName", timeline::FileSequenceAudio::FileName)
        .value("Directory", timeline::FileSequenceAudio::Directory)
        .export_values();

#ifdef TLRENDER_USD
    py::module usd = m.def_submodule("usd");

    py::enum_<usd::DrawMode>(usd, "DrawMode")
        .value("Points", usd::DrawMode::Points)
        .value("Wireframe", usd::DrawMode::Wireframe)
        .value("WireframeOnSurface", usd::DrawMode::WireframeOnSurface)
        .value("ShadedFlat", usd::DrawMode::ShadedFlat)
        .value("ShadedSmooth", usd::DrawMode::ShadedSmooth)
        .value("GeomOnly", usd::DrawMode::GeomOnly)
        .value("GeomFlat", usd::DrawMode::GeomFlat)
        .value("GeomSmooth", usd::DrawMode::GeomSmooth)
        .export_values();
#endif
}
