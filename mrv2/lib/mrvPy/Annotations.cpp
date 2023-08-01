// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
namespace py = pybind11;

#include "mrvPy/CmdsAux.h"

#include "mrvCore/mrvI8N.h"

#include "mrvFl/mrvCallbacks.h"

#include "mrViewer.h"

namespace mrv2
{
    namespace annotations
    {
        using namespace mrv;

        void add(const otime::RationalTime& time, const std::string& note)
        {
            ViewerUI* ui = App::ui;
            Viewport* view = ui->uiView;
            if (!view)
                return;
            auto player = view->getTimelinePlayer();
            if (!player)
                return;
            player->seek(time);
            add_note_annotation_cb(ui, note);
        }

        void add(const int64_t& frame, const std::string& note)
        {
            ViewerUI* ui = App::ui;
            Viewport* view = ui->uiView;
            if (!view)
                return;
            auto player = view->getTimelinePlayer();
            if (!player)
                return;
            const auto& currentTime = player->currentTime();
            const otime::RationalTime& time =
                otime::RationalTime(frame, currentTime.rate());
            add(time, note);
        }

        void add(const double& seconds, const std::string& note)
        {
            ViewerUI* ui = App::ui;
            Viewport* view = ui->uiView;
            if (!view)
                return;
            auto player = view->getTimelinePlayer();
            if (!player)
                return;
            const otime::RationalTime& time = otime::RationalTime(seconds, 1.0);
            add(time, note);
        }

    } // namespace annotations
} // namespace mrv2

void mrv2_annotations(py::module& m)
{
    using namespace mrv;

    py::module annotations = m.def_submodule("annotations");
    annotations.doc() = _(R"PYTHON(
Annotations module.

Contains all functions and classes related to the annotationss.
)PYTHON");

    annotations.def(
        "add",
        py::overload_cast<const otime::RationalTime&, const std::string&>(
            &mrv2::annotations::add),
        _("Add notes annotations to current clip at a certain time."),
        py::arg("time"), py::arg("notes"));
    annotations.def(
        "add",
        py::overload_cast<const int64_t&, const std::string&>(
            &mrv2::annotations::add),
        _("Add notes annotations to current clip at a certain frame."),
        py::arg("frame"), py::arg("notes"));
    annotations.def(
        "add",
        py::overload_cast<const double&, const std::string&>(
            &mrv2::annotations::add),
        _("Add notes annotations to current clip at certain seconds."),
        py::arg("seconds"), py::arg("notes"));
}
