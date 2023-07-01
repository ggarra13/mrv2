// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <pybind11/pybind11.h>
namespace py = pybind11;

#include "mrvCore/mrvOrderedMap.h"

namespace mrv
{
    extern OrderedMap<std::string, py::handle > pythonMenus;
    extern float kCrops[];

    void discover_python_plugins();

} // namespace mrv
