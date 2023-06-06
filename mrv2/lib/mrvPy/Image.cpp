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
#include "mrvCore/mrvStereo3DOptions.h"
#include "mrvCore/mrvEnvironmentMapOptions.h"

namespace tl
{
    namespace imaging
    {
        inline std::ostream& operator<<(std::ostream& o, const Mirror& a)
        {
            o << "<mrv2.image.Mirror x=" << (a.x ? "True" : "False")
              << " y=" << (a.y ? "True" : "False") << ">";
            return o;
        }
    } // namespace imaging

    namespace timeline
    {
        inline std::ostream& operator<<(std::ostream& o, const ImageFilters& a)
        {
            o << "<mrv2.image.ImageFilters minify=" << a.minify
              << " magnify=" << a.magnify << ">";
            return o;
        }

        inline std::ostream& operator<<(std::ostream& o, const Color& a)
        {
            o << "<mrv2.image.Color add=" << a.add
              << " brightness=" << a.brightness << " contrast=" << a.contrast
              << " saturation=" << a.saturation << " tint=" << a.tint
              << " invert=" << (a.invert ? "True" : "False") << ">";
            return o;
        }

        inline std::ostream& operator<<(std::ostream& o, const Levels& a)
        {
            o << "<mrv2.image.Levels inLow=" << a.inLow
              << " inHigh=" << a.inHigh << " gamma=" << a.gamma
              << " outLow=" << a.outLow << " outHigh=" << a.outHigh << ">";
            return o;
        }

        inline std::ostream&
        operator<<(std::ostream& o, const DisplayOptions& a)
        {
            o << "<mrv2.image.DisplayOptions channels" << a.channels
              << " mirror=" << a.mirror << " colorEnabled=" << a.colorEnabled
              << " color=" << a.color << " levelsEnabled=" << a.levelsEnabled
              << " levels=" << a.levels
              << " softClipEnabled=" << a.softClipEnabled
              << " softClip=" << a.softClip
              << " imageFilters=" << a.imageFilters
              << " videoLevels=" << a.videoLevels << ">";
            return o;
        }

        inline std::ostream& operator<<(std::ostream& s, const ImageOptions& o)
        {
            s << "<mrv2.image.inageOptions videoLevels=" << o.videoLevels
              << " alphaBlend=" << o.alphaBlend
              << " imageFilters=" << o.imageFilters << ">";
            return s;
        }

        inline std::ostream& operator<<(std::ostream& s, const LUTOptions& o)
        {
            s << "<mrv2.image.LUTOptions fileName=" << o.fileName
              << " order=" << o.order << ">";
            return s;
        }

    } // namespace timeline
} // namespace tl

namespace mrv
{
    inline std::ostream&
    operator<<(std::ostream& s, const EnvironmentMapOptions& o)
    {
        s << "<mrv2.image.EnvironmentMapOptions type=" << o.type
          << " horizonalAperture=" << o.horizontalAperture
          << " verticalAperture=" << o.verticalAperture
          << " focalLength=" << o.focalLength << " rotateX=" << o.rotateX
          << " rotateY=" << o.rotateY << " subdivisionX=" << o.subdivisionX
          << " subdivisionY=" << o.subdivisionY
          << " spin=" << (o.spin ? "True" : "False") << ">";
        return s;
    }

    inline std::ostream& operator<<(std::ostream& s, const Stereo3DOptions& o)
    {
        s << "<mrv2.image.Stereo3DOptions input=" << o.input
          << " output=" << o.output << " eyeSeparation=" << o.eyeSeparation
          << " swapEyes=" << (o.swapEyes ? "True" : "False") << ">";
        return s;
    }
} // namespace mrv

void mrv2_image(pybind11::module& m)
{
    using namespace tl;

    py::module image = m.def_submodule("image");
    image.doc() = _(R"PYTHON(
Image module.

Contains all classes and enums related to image controls. 
)PYTHON");

    py::class_<imaging::Mirror>(image, "Mirror")
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

    // Cannot be timeline as it clashes with timeline::Color class
    py::class_<timeline::Color>(image, "Color")
        .def(py::init<>())
        .def_readwrite(
            "add", &timeline::Color::add,
            _("Add a :class:`mrv2.math.Vector3f` to image."))
        .def_readwrite(
            "brightness", &timeline::Color::brightness,
            _("Change a :class:`mrv2.math.Vector3f` of brightness"
              " to image."))
        .def_readwrite(
            "contrast", &timeline::Color::contrast,
            _("Change a :class:`mrv2.math.Vector3f` of contrast"
              " to image."))
        .def_readwrite(
            "saturation", &timeline::Color::saturation,
            _("Change a :class:`mrv2.math.Vector3f` of saturation"
              " to image."))
        .def_readwrite(
            "tint", &timeline::Color::tint,
            _("Change tint of image to image between 0 and 1."))
        .def_readwrite(
            "invert", &timeline::Color::invert, _("Invert the color values."))
        .def(
            "__repr__",
            [](const timeline::Color& o)
            {
                std::ostringstream s;
                s << o;
                return s.str();
            })
        .doc() = _("Color values.");

    py::class_<timeline::Levels>(image, "Levels")
        .def(py::init<>())
        .def_readwrite(
            "inLow", &timeline::Levels::inLow, _("In Low Level value."))
        .def_readwrite(
            "inHigh", &timeline::Levels::inHigh, _("In High Level value."))
        .def_readwrite(
            "gamma", &timeline::Levels::gamma, _("Gamma Level value."))
        .def_readwrite(
            "outLow", &timeline::Levels::outLow, _("Out Low Level value."))
        .def_readwrite(
            "outHigh", &timeline::Levels::outHigh, _("Out High Level value."))
        .def(
            "__repr__",
            [](const timeline::Levels& o)
            {
                std::ostringstream s;
                s << o;
                return s.str();
            })
        .doc() = _("Levels values.");

    py::class_<timeline::ImageFilters>(image, "ImageFilters")
        .def(py::init<>())
        .def_readwrite(
            "minify", &timeline::ImageFilters::minify,
            _("Minify filter :class:`mrv2.image.ImageFilter`."))
        .def_readwrite(
            "magnify", &timeline::ImageFilters::magnify,
            _("Magnify filter :class:`mrv2.image.ImageFilter`."))
        .def(
            "__repr__",
            [](const timeline::Levels& o)
            {
                std::ostringstream s;
                s << o;
                return s.str();
            })
        .doc() = _("Image filters.");

    py::class_<timeline::DisplayOptions>(image, "DisplayOptions")
        .def(py::init<>())
        .def_readwrite(
            "channels", &timeline::DisplayOptions::channels,
            _("Color channels :class:`mrv2.image.Channels`."))
        .def_readwrite(
            "mirror", &timeline::DisplayOptions::mirror,
            _("Mirror on X, Y or both :class:`mrv2.image.Mirror`."))
        .def_readwrite(
            "colorEnabled", &timeline::DisplayOptions::colorEnabled,
            _("Enable color transforms."))
        .def_readwrite(
            "color", &timeline::DisplayOptions::color,
            _("Color options :class:`mrv2.image.Color`."))
        .def_readwrite(
            "levelsEnabled", &timeline::DisplayOptions::levelsEnabled,
            _("Enable levels transforms."))
        .def_readwrite(
            "levels", &timeline::DisplayOptions::levels,
            _("Levels options :class:`mrv2.image.Levels`."))
        .def_readwrite(
            "softClipEnabled", &timeline::DisplayOptions::softClipEnabled,
            _("Enable soft clip."))
        .def_readwrite(
            "softClip", &timeline::DisplayOptions::softClip,
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

    py::class_<timeline::LUTOptions>(image, "LUTOptions")
        .def(py::init<>())
        .def_readwrite(
            "fileName", &timeline::LUTOptions::fileName, _("LUT filename."))
        .def_readwrite(
            "order", &timeline::LUTOptions::order,
            _("LUT transformation order."))
        .def(
            "__repr__",
            [](const timeline::LUTOptions& o)
            {
                std::stringstream s;
                s << o;
                return s.str();
            })
        .doc() = _("LUT options.");

    py::class_<timeline::ImageOptions>(image, "ImageOptions")
        .def(py::init<>())
        .def_readwrite(
            "videoLevels", &timeline::ImageOptions::videoLevels,
            _("Video Levels."))
        .def_readwrite(
            "alphaBlend", &timeline::ImageOptions::alphaBlend,
            _("Alpha blending algorithm"))
        .def_readwrite(
            "imageFilters", &timeline::ImageOptions::imageFilters,
            _("Image Filters"))
        .def(
            "__repr__",
            [](const timeline::ImageOptions& o)
            {
                std::stringstream s;
                s << o;
                return s.str();
            })
        .doc() = _("Image options.");

    py::class_<mrv::EnvironmentMapOptions>(image, "EnvironmentMapOptions")
        .def(py::init<>())
        .def_readwrite(
            "type", &mrv::EnvironmentMapOptions::type,
            _("Environment Map type."))
        .def_readwrite(
            "horizontalAperture",
            &mrv::EnvironmentMapOptions::horizontalAperture,
            _("Horizontal aperture"))
        .def_readwrite(
            "verticalAperture", &mrv::EnvironmentMapOptions::verticalAperture,
            _("Vertical aperture"))
        .def_readwrite(
            "focalLength", &mrv::EnvironmentMapOptions::focalLength,
            _("Focal Length"))
        .def_readwrite(
            "rotateX", &mrv::EnvironmentMapOptions::rotateX, _("Rotation on X"))
        .def_readwrite(
            "rotateY", &mrv::EnvironmentMapOptions::rotateY, _("Rotation on Y"))
        .def_readwrite(
            "subdivisionX", &mrv::EnvironmentMapOptions::subdivisionX,
            _("Subdivision on X"))
        .def_readwrite(
            "subdivisionY", &mrv::EnvironmentMapOptions::subdivisionY,
            _("Subdivision on Y"))
        .def_readwrite("spin", &mrv::EnvironmentMapOptions::spin, _("Spin"))
        .def(
            "__repr__",
            [](const mrv::EnvironmentMapOptions& o)
            {
                std::stringstream s;
                s << o;
                return s.str();
            })
        .doc() = _("EnvironmentMap options.");

    py::class_<mrv::Stereo3DOptions>(image, "Stereo3DOptions")
        .def(py::init<>())
        .def_readwrite(
            "input", &mrv::Stereo3DOptions::input,
            _("Stereo 3d input :class:`mrv2.image.StereoInput`.."))
        .def_readwrite(
            "output", &mrv::Stereo3DOptions::input,
            _("Stereo 3d output :class:`mrv2.image.StereoOutput`.."))
        .def_readwrite(
            "eyeSeparation", &mrv::Stereo3DOptions::eyeSeparation,
            _("Separation between left and right eye."))
        .def_readwrite(
            "swapEyes", &mrv::Stereo3DOptions::swapEyes,
            _("Swap left and right eye"))
        .def(
            "__repr__",
            [](const mrv::Stereo3DOptions& o)
            {
                std::stringstream s;
                s << o;
                return s.str();
            })
        .doc() = _("Stereo3D options.");
}
