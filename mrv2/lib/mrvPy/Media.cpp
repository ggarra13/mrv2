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
        .doc() = _("Comparison options.");
}
