// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <map>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
namespace py = pybind11;

#include "mrvCore/mrvHome.h"
#include "mrvCore/mrvI8N.h"

namespace mrv
{
    struct Plugin
    {
        std::map<std::string, std::string> get_menu_entries() const
        {
            std::map<std::string, std::string> out;
            return out;
        }
    };
} // namespace mrv

void mrv2_python_plugins(pybind11::module& m)
{
    using namespace mrv;

    py::class_<Plugin>(m, "Plugin")
        .def(py::init<>())
        .def(
            "get_menu_entries", &Plugin::get_menu_entries,
            _("Add menu entries as a python dictionary."))
        .doc() = _("Base class for python plug-ins.");
}
