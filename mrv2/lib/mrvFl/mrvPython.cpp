#include <pybind11/embed.h> // everything needed for embedding
namespace py = pybind11;

#include "mrvFl/mrvCallbacks.h"

namespace mrv
{

    void python_run_cb(Fl_Menu_* w, ViewerUI* ui)
    {
        py::scoped_interpreter guard{}; // start the interpreter and keep it alive

        py::print("Hello, World!"); // use the Python API
    }

}
