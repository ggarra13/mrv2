// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <tlCore/Path.h>

#include <pybind11/pybind11.h>

namespace py = pybind11;

void mrv2_filepath(py::module& m)
{

    py::class_<tl::file::PathOptions>(m, "PathOptions")
        .def(py::init<>())
        .def_readwrite(
            "maxNumberDigits", &tl::file::PathOptions::maxNumberDigits);

    py::class_<tl::file::Path>(m, "Path")
        .def(
            py::init<const std::string&, const tl::file::PathOptions&>(),
            py::arg("path"), py::arg("options") = tl::file::PathOptions())
        .def(
            py::init<
                const std::string&, const std::string&,
                const tl::file::PathOptions&>(),
            py::arg("path"), py::arg("value"),
            py::arg("options") = tl::file::PathOptions())
        .def(
            py::init<
                const std::string&, const std::string&, const std::string&,
                uint8_t, const std::string&>(),
            py::arg("directory"), py::arg("baseName"), py::arg("number"),
            py::arg("padding"), py::arg("exetension"))
        .def(
            "get", &tl::file::Path::get, "Returns the path",
            py::arg("idx") = -1, py::arg("directoru") = true)
        .def("getDirectory", &tl::file::Path::getDirectory)
        .def("getBaseName", &tl::file::Path::getBaseName)
        .def("getNumber", &tl::file::Path::getNumber)
        .def("getPadding", &tl::file::Path::getPadding)
        .def("getExtension", &tl::file::Path::getExtension)
        .def("isEmpty", &tl::file::Path::isEmpty)
        .def("isAbsolute", &tl::file::Path::isAbsolute)
        .def(
            "__repr__", [](const tl::file::Path& a)
            { return "<mrv2.Path '" + a.get() + "'>"; })
        .doc() = "Class used to hold a file path";
    ;
}
