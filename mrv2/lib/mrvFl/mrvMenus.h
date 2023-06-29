// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <map>

#include <pybind11/pybind11.h>
namespace py = pybind11;

namespace mrv
{
    extern std::map<std::string, py::handle > pythonMenus;
    extern float kCrops[];

    void discover_python_plugins();

} // namespace mrv
