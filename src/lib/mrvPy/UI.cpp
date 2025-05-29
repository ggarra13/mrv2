// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include "mrViewer.h"

#include "mrvApp/mrvApp.h"

#include "mrvCore/mrvI8N.h"


#include <pybind11/pybind11.h>
namespace py = pybind11;

#include <sstream>

namespace mrv
{
    namespace ui
    {
        void refreshMenus()
        {
            ViewerUI* ui = App::ui;
            ui->uiMain->fill_menu(ui->uiMenuBar);
        }
    } // namespace ui
} // namespace mrv

void mrv2_ui(pybind11::module& m)
{

    py::module ui = m.def_submodule("ui");
    ui.doc() = _(R"PYTHON(
UI module.

Contains all classes and enums related to UI (User Interface). 
)PYTHON");
    using namespace mrv;

    ui.def("refreshMenus", &mrv::ui::refreshMenus, _("Refresh menus."));
}
