// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <pybind11/pybind11.h>
namespace py = pybind11;

namespace mrv
{

    //! Class used to redirect stdout and stderr to two python strings
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
            auto stringio = py::module::import("io").attr("StringIO");
            _stdout_buffer =
                stringio(); // Other filelike object can be used here as well,
            // such as objects created by pybind11
            _stderr_buffer = stringio();
            sysm.attr("stdout") = _stdout_buffer;
            sysm.attr("stderr") = _stderr_buffer;
        }
        std::string stdoutString()
        {
            _stdout_buffer.attr("seek")(0);
            return py::str(_stdout_buffer.attr("read")());
        }
        std::string stderrString()
        {
            _stderr_buffer.attr("seek")(0);
            return py::str(_stderr_buffer.attr("read")());
        }
        ~PyStdErrOutStreamRedirect()
        {
            auto sysm = py::module::import("sys");
            sysm.attr("stdout") = _stdout;
            sysm.attr("stderr") = _stderr;
        }
    };

} // namespace mrv
