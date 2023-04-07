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

void mrv2_media(pybind11::module& m)
{
    using namespace tl;

    py::module media = m.def_submodule("media");
    media.doc() = _(R"PYTHON(
Media module.

Contains all classes and enums related to media. 
)PYTHON");

    py::class_<timeline::CompareOptions>(media, "CompareOptions")
        .def(py::init<>())
        .def_readwrite(
            "mode", &timeline::CompareOptions::mode,
            _("Compare mode :class:`mrv2.media.CompareMode`."))
        .def_readwrite(
            "wipeCenter", &timeline::CompareOptions::wipeCenter,
            _("Wipe center in X and Y :class:`mrv2.math.Vector2f`."))
        .def_readwrite(
            "wipeRotation", &timeline::CompareOptions::wipeRotation,
            _("Wipe Rotation."))
        .def_readwrite(
            "overlay", &timeline::CompareOptions::overlay,
            _("Overlay ( A over B )"))
        .def(
            "__repr__",
            [](const timeline::CompareOptions& o)
            {
                std::stringstream s;
                s << "<mrv2.media.CompareOptions mode=" << o.mode
                  << " wipeCenter=" << o.wipeCenter
                  << " wipeRotation=" << o.wipeRotation
                  << " overlay=" << o.overlay << ">";
                return s.str();
            })
        .doc() = "Comparison options.";
}
