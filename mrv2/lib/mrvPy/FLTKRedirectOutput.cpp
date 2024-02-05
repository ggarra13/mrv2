// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <pybind11/pybind11.h>
#include <iostream>

#include "mrvWidgets/mrvPythonOutput.h"

#include "mrvPanels/mrvPanelsCallbacks.h"

#include "mrvApp/mrvApp.h"

namespace py = pybind11;

namespace mrv
{

    class FLTKRedirectOutput
    {
    public:
        void write(const std::string& message)
        {
            if (!panel::pythonPanel)
                panel::python_panel_cb(nullptr, App::ui);
            PythonOutput* output = panel::PythonPanel::output();
            output->info(message.c_str());
            std::cout << message;
        }

        void flush() {}
    };

    class FLTKRedirectError
    {
    public:
        void write(const std::string& message)
        {
            if (!panel::pythonPanel)
                panel::python_panel_cb(nullptr, App::ui);
            PythonOutput* output = panel::PythonPanel::output();
            output->error(message.c_str());
            std::cerr << message;
        }
        void flush() {}
    };

} // namespace mrv

void mrv2_python_redirect(pybind11::module& m)
{
    py::class_<mrv::FLTKRedirectOutput>(m, "FLTKRedirectOutput")
        .def(py::init<>())
        .def("write", &mrv::FLTKRedirectOutput::write)
        .def("flush", &mrv::FLTKRedirectOutput::flush);

    py::class_<mrv::FLTKRedirectError>(m, "FLTKRedirectError")
        .def(py::init<>())
        .def("write", &mrv::FLTKRedirectError::write)
        .def("flush", &mrv::FLTKRedirectError::flush);
}
