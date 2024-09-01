// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <vector>

#ifdef MRV2_PYBIND11
#    include <pybind11/pybind11.h>
namespace py = pybind11;
#endif

#ifdef MRV2_PYBIND11
namespace mrv
{
    extern std::vector<py::object> pythonOpenFileCallbacks;

    void run_python_open_file_cb(
        const py::object& method, const std::string& fileName,
        const std::string& audioFileName);
} // namespace mrv
#endif
