// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include "mrViewer.h"

#include "mrvPy/CmdsAux.h"

#include "mrvVoice/mrvVoiceOver.h"
#include "mrvVoice/mrvAnnotation.h"

#include "mrvFl/mrvCallbacks.h"

#include "mrvCore/mrvI8N.h"

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
namespace py = pybind11;

namespace mrv2
{
    namespace annotations
    {
        using namespace mrv;

        void add(const otime::RationalTime& time, const std::string& note)
        {
            ViewerUI* ui = App::ui;
            MyViewport* view = ui->uiView;
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
            MyViewport* view = ui->uiView;
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
            MyViewport* view = ui->uiView;
            if (!view)
                return;
            auto player = view->getTimelinePlayer();
            if (!player)
                return;
            const otime::RationalTime& time = otime::RationalTime(seconds, 1.0);
            add(time, note);
        }

        std::vector< otime::RationalTime >
        getTimes()
        {
            std::vector< otime::RationalTime > out;
            
            ViewerUI* ui = App::ui;
            MyViewport* view = ui->uiView;
            if (!view)
                return out;
            auto player = view->getTimelinePlayer();
            if (!player)
                return out;
            
            out = player->getAnnotationTimes();
            return out;
        }
        
        std::vector<std::string >
        getVoiceAnnotations(const otime::RationalTime& time)
        {
            std::vector<std::string > out;
        
            auto player = App::ui->uiView->getTimelinePlayer();
            if (!player) return out;

            auto annotations = player->getVoiceAnnotations();
            for (auto annotation : annotations)
            {
                for (auto voice : annotation->voices )
                {
                    out.push_back(voice->getFileName());
                }
            }
            return out;
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
    annotations.def(
        "getTimes", &mrv2::annotations::getTimes,
        _("Get all times for annotations."));
    annotations.def(
        "getVoiceAnnotations", &mrv2::annotations::getVoiceAnnotations,
        _("Get all voice annotations for current frame."));
}
