// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <iostream>
#include <sstream>

#include <pybind11/pybind11.h>

namespace py = pybind11;

#include "mrvFl/mrvSaveOptions.h"

#include "mrvCore/mrvI8N.h"

void mrv2_io(py::module& m)
{
    py::module io = m.def_submodule("io");

    py::class_<mrv::SaveOptions>(io, "SaveOptions")
        .def(
            py::init<
            bool,
            mrv::SaveResolution
#ifdef TLRENDER_FFMPEG
                ,
                tl::ffmpeg::Profile, std::string, std::string,
                tl::ffmpeg::AudioCodec, bool, bool, std::string, std::string,
                std::string, std::string
#endif
#ifdef TLRENDER_EXR
                ,
                tl::exr::Compression, tl::image::PixelType, int, float
#endif
                >(),
            py::arg("annotations") = false,
            py::arg("resolution") = mrv::SaveResolution::kSameSize
#ifdef TLRENDER_FFMPEG
            ,
            py::arg("ffmpegProfile") = tl::ffmpeg::Profile::None,
            py::arg("ffmpegPreset") = "",
            py::arg("ffmpegPixelFormat") = "YUV420P",
            py::arg("ffmpegHardwareEncode") = false,
            py::arg("ffmpegOverride") = false,
            py::arg("ffmpegColorRange") = "PC",
            py::arg("ffmpegColorSpace") = "bt709",
            py::arg("ffmpegColorPrimaries") = "bt709",
            py::arg("ffmpegColorTRC") = "bt709",
            py::arg("ffmpegAudioCodec") = tl::ffmpeg::AudioCodec::None
#endif
#ifdef TLRENDER_EXR
            ,
            py::arg("exrCompression") = tl::exr::Compression::ZIP,
            py::arg("exrPixelType") = tl::image::PixelType::RGBA_F16,
            py::arg("zipCompressionLevel") = 4,
            py::arg("dwaCompressionLevel") = 45.0F
#endif
            )
        .def_readwrite(
            "annotations", &mrv::SaveOptions::annotations,
            _("Save annotations."))
        .def_readwrite(
            "resolution", &mrv::SaveOptions::resolution,
            _("Save resolution."))
#ifdef TLRENDER_FFMPEG
        .def_readwrite(
            "ffmpegProfile", &mrv::SaveOptions::ffmpegProfile,
            _("FFmpeg Profile."))
        .def_readwrite(
            "ffmpegPreset", &mrv::SaveOptions::ffmpegPreset,
            _("FFmpeg Preset."))
        .def_readwrite(
            "ffmpegPixelFormat", &mrv::SaveOptions::ffmpegPixelFormat,
            _("FFmpeg Pixel Format."))
        .def_readwrite(
            "ffmpegHardwareEncode", &mrv::SaveOptions::ffmpegHardwareEncode,
            _("FFmpeg video encoding with hardware if possible."))
        .def_readwrite(
            "ffmpegOverride", &mrv::SaveOptions::ffmpegOverride,
            _("FFmpeg Override color characteristics."))
        .def_readwrite(
            "ffmpegColorRange", &mrv::SaveOptions::ffmpegColorRange,
            _("FFmpeg Color Range."))
        .def_readwrite(
            "ffmpegColorSpace", &mrv::SaveOptions::ffmpegColorSpace,
            _("FFmpeg Color Space."))
        .def_readwrite(
            "ffmpegColorPrimaries", &mrv::SaveOptions::ffmpegColorPrimaries,
            _("FFmpeg Color Primaries."))
        .def_readwrite(
            "ffmpegColorTRC", &mrv::SaveOptions::ffmpegColorTRC,
            _("FFmpeg Color Transfer Characteristics."))
        .def_readwrite(
            "ffmpegAudioCodec", &mrv::SaveOptions::ffmpegAudioCodec,
            _("FFmpeg Audio Codec."))
#endif
#ifdef TLRENDER_EXR
        .def_readwrite(
            "exrCompression", &mrv::SaveOptions::exrCompression,
            _("OpenEXR's Compression."))
        .def_readwrite(
            "exrPixelType", &mrv::SaveOptions::exrPixelType,
            _("OpenEXR's Pixel Type."))
        .def_readwrite(
            "zipCompressionLevel", &mrv::SaveOptions::zipCompressionLevel,
            _("OpenEXR's Zip Compression Level."))
        .def_readwrite(
            "dwaCompressionLevel", &mrv::SaveOptions::dwaCompressionLevel,
            _("OpenEXR's DWA Compression Level."))
#endif
        .def(
            "__repr__",
            [](const mrv::SaveOptions& o)
            {
                std::stringstream s;
                s << "<mrv2.io.SaveOptions "
                  << " annotations=" << (o.annotations ? "True" : "False")
#ifdef TLRENDER_FFMPEG
                  << " ffmpegProfile=" << getLabel(o.ffmpegProfile)
                  << " ffmpegAudioCodec=" << getLabel(o.ffmpegAudioCodec)
#endif
#ifdef TLRENDER_EXR
                  << " exrCompression=" << getLabel(o.exrCompression)
                  << " zipCompressionLevel=" << o.zipCompressionLevel
                  << " dwaCompressionLevel=" << o.dwaCompressionLevel
#endif
                  << ">";
                return s.str();
            })
        .doc() = "Saving options.";
}
