// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <pybind11/pybind11.h>
namespace py = pybind11;

namespace mrv
{

    //! Class used to redirect stdout and stderr to FLTK widget
    class PyStdErrOutStreamRedirect
    {
        py::object _stdout;
        py::object _stderr;
        py::object _stdout_buffer;
        py::object _stderr_buffer;

    public:
        PyStdErrOutStreamRedirect()
        {
            auto sysm = py::module::import("sys");
            _stdout = sysm.attr("stdout");
            _stderr = sysm.attr("stderr");
            auto stdout = py::module::import("mrv2").attr("FLTKRedirectOutput");
            auto stderr = py::module::import("mrv2").attr("FLTKRedirectError");
            _stdout_buffer = stdout();
            // such as objects created by pybind11
            _stderr_buffer = stderr();
            sysm.attr("stdout") = _stdout_buffer;
            sysm.attr("stderr") = _stderr_buffer;
        }
        ~PyStdErrOutStreamRedirect()
        {
            auto sysm = py::module::import("sys");
            sysm.attr("stdout") = _stdout;
            sysm.attr("stderr") = _stderr;
        }
    };

} // namespace mrv
