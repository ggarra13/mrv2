// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <pybind11/pybind11.h>
namespace py = pybind11;

extern void mrv2_enums(py::module& m);
extern void mrv2_vectors(py::module& m);
extern void mrv2_otio(py::module& m);
extern void mrv2_filepath(py::module& m);
extern void mrv2_fileitem(py::module& m);
extern void mrv2_image(py::module& m);
extern void mrv2_media(py::module& m);
extern void mrv2_timeline(py::module& m);
extern void mrv2_settings(py::module& m);
extern void mrv2_filesmodel(py::module& m);
extern void mrv2_playlist(py::module& m);
extern void mrv2_io(py::module& m);
extern void mrv2_annotations(py::module& m);
extern void mrv2_usd(py::module& m);
extern void mrv2_commands(py::module& m);
extern void mrv2_python_plugins(py::module& m);
extern void mrv2_python_redirect(py::module& m);
extern void mrv2_discover_python_plugins();
