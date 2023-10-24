// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <sstream>

#include <pybind11/pybind11.h>

namespace py = pybind11;

#include <tlCore/Vector.h>
#include <tlCore/Size.h>

#include "mrvCore/mrvI8N.h"

void mrv2_vectors(pybind11::module& m)
{
    using namespace tl;

    py::module math = m.def_submodule("math");
    math.doc() = _(R"PYTHON(
Math module.

Contains all math classes.
)PYTHON");

    py::class_<math::Vector2i>(math, "Vector2i")
        .def(py::init<>())
        .def(py::init<int, int>(), py::arg("x"), py::arg("y"))
        .def_readwrite("x", &math::Vector2i::x)
        .def_readwrite("y", &math::Vector2i::y)
        .def(
            "__repr__",
            [](const math::Vector2i& o)
            {
                std::stringstream s;
                s << "<mrv2.math.Vector2i x=" << o.x << " y=" << o.y << ">";
                return s.str();
            })
        .doc() = _("Vector of 2 integers.");

    py::class_<math::Vector2f>(math, "Vector2f")
        .def(py::init<>())
        .def(py::init<float, float>(), py::arg("x"), py::arg("y"))
        .def_readwrite("x", &math::Vector2f::x)
        .def_readwrite("y", &math::Vector2f::y)
        .def(
            "__repr__",
            [](const math::Vector2f& o)
            {
                std::stringstream s;
                s << "<mrv2.math.Vector2f x=" << o.x << " y=" << o.y << ">";
                return s.str();
            })
        .doc() = _("Vector of 2 floats.");

    py::class_<math::Vector3f>(math, "Vector3f")
        .def(py::init<>())
        .def(
            py::init<float, float, float>(), py::arg("x"), py::arg("y"),
            py::arg("z"))
        .def_readwrite("x", &math::Vector3f::x)
        .def_readwrite("y", &math::Vector3f::y)
        .def_readwrite("z", &math::Vector3f::z)
        .def(
            "__repr__",
            [](const math::Vector3f& o)
            {
                std::stringstream s;
                s << "<mrv2.math.Vector3f x=" << o.x << " y=" << o.y
                  << " z=" << o.z << ">";
                return s.str();
            })
        .doc() = _("Vector of 3 floats.");

    py::class_<math::Vector4f>(math, "Vector4f")
        .def(py::init<>())
        .def(
            py::init<float, float, float, float>(), py::arg("x"), py::arg("y"),
            py::arg("z"), py::arg("w"))
        .def_readwrite("x", &math::Vector4f::x)
        .def_readwrite("y", &math::Vector4f::y)
        .def_readwrite("z", &math::Vector4f::z)
        .def_readwrite("w", &math::Vector4f::w)
        .def(
            "__repr__",
            [](const math::Vector4f& o)
            {
                std::stringstream s;
                s << "<mrv2.math.Vector4f x=" << o.x << " y=" << o.y
                  << " z=" << o.z << " w=" << o.w << ">";
                return s.str();
            })
        .doc() = _("Vector of 4 floats.");

    py::class_<math::Size2i>(math, "Size2i")
        .def(py::init<>())
        .def(py::init<int, int>(), py::arg("w"), py::arg("h"))
        .def_readwrite("w", &math::Size2i::w)
        .def_readwrite("h", &math::Size2i::h)
        .def(
            "__repr__",
            [](const math::Size2i& o)
            {
                std::stringstream s;
                s << "<mrv2.math.Size2i w=" << o.w << " h=" << o.h << ">";
                return s.str();
            })
        .doc() = _("Size of 2 integers.");
}
