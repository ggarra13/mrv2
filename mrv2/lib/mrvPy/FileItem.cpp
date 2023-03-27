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
        .def_readwrite(
            "path", &FilesModelItem::path, _("Path :class:`mrv2.Path` to the File Media."))
        .def_readwrite(
            "audioPath", &FilesModelItem::path,
            _("Audio path :class:`mrv2.Path` to the File Media if any."))
        .def_readwrite(
            "timeRange", &FilesModelItem::timeRange,
            _("Time range :class:`mrv2.TimeRange` of the File Media."))
        .def_readwrite(
            "speed", &FilesModelItem::speed,
            _("Speed (FPS) of the File Media."))
        .def_readwrite(
            "playback", &FilesModelItem::playback,
            _("Playback state :class:`mrv2.timeline.Playbacks` of the File Media."))
        .def_readwrite(
            "loop", &FilesModelItem::loop, _("Loop state :class:`mrv2.timeline.Loop` of the File Media."))
        .def_readwrite(
            "currentTime", &FilesModelItem::currentTime,
            _("Current time :class:`mrv2.RationalTime` of the File Media."))
        .def_readwrite(
            "inOutRange", &FilesModelItem::inOutRange,
            _("In/Out range :class:`mrv2.TimeRange` of the File Media."))
        .def_readwrite(
            "videoLayer", &FilesModelItem::videoLayer,
            _("Video layer of the File Media."))
        .def_readwrite(
            "volume", &FilesModelItem::volume, _("Volume of the File Media."))
        .def_readwrite(
            "mute", &FilesModelItem::mute, _("Mute state of the File Media."))
        .def_readwrite(
            "audioOffset", &FilesModelItem::audioOffset,
            _("Audio offset of the File Media."))
        .def(
            "__repr__",
            [](const FilesModelItem& a)
            {
                std::ostringstream s;
                s << a;
                return s.str();
            })
        .doc() = _("Class used to hold a media item.");
}
