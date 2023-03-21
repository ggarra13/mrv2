// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <sstream>

#include "mrvApp/mrvFilesModel.h"
#include <pybind11/pybind11.h>

#include "mrvCore/mrvI8N.h"

#include "mrvPy/CmdsAux.h"

namespace py = pybind11;

void mrv2_fileitem(py::module& m)
{
    using namespace mrv;

    py::class_<FilesModelItem, std::shared_ptr<FilesModelItem> >(m, "FileMedia")
        .def(py::init<>())
        .def_readwrite("path", &FilesModelItem::path)
        .def_readwrite("audioPath", &FilesModelItem::path)
        .def_readwrite("timeRange", &FilesModelItem::timeRange)
        .def_readwrite("speed", &FilesModelItem::speed)
        .def_readwrite("playback", &FilesModelItem::playback)
        .def_readwrite("loop", &FilesModelItem::loop)
        .def_readwrite("currentTime", &FilesModelItem::currentTime)
        .def_readwrite("cinOutRange", &FilesModelItem::inOutRange)
        .def_readwrite("videoLayer", &FilesModelItem::videoLayer)
        .def_readwrite("volume", &FilesModelItem::volume)
        .def_readwrite("mute", &FilesModelItem::mute)
        .def_readwrite("audioOffset", &FilesModelItem::audioOffset)
        .def(
            "__repr__",
            [](const FilesModelItem& a)
            {
                std::ostringstream s;
                s << a;
                return s.str();
            })
        .doc() = _("Class used to hold a file item");
}
