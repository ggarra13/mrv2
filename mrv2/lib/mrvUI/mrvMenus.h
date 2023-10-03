// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#ifdef MRV2_PYBIND11
#    include <pybind11/pybind11.h>
namespace py = pybind11;
#endif

#include "mrvCore/mrvOrderedMap.h"

namespace mrv
{

#ifdef MRV2_PYBIND11
    extern OrderedMap<std::string, py::handle > pythonMenus;
#endif

    extern float kCrops[];

    void discover_python_plugins();

} // namespace mrv
