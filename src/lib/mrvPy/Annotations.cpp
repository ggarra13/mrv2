// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include "mrViewer.h"

#include "mrvPy/CmdsAux.h"

#include "mrvVoice/mrvVoiceOver.h"
#include "mrvVoice/mrvAnnotation.h"

#include "mrvFl/mrvCallbacks.h"

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

        /** 
         * Get a list of .wav voice annotations for the current frame.
         * 
         * @return list of .wav files
         */
        std::vector<std::string >
        getVoiceAnnotations()
        {
            std::vector<std::string > out;
        
            auto player = App::ui->uiView->getTimelinePlayer();
            if (!player) return out;

            auto annotations = player->getVoiceAnnotations();
            for (auto annotation : annotations)
            {
                for (auto voice : annotation->voices)
                {
                    out.push_back(voice->getFileName());
                }
            }
            return out;
        }

        /** 
         * Return a JSON string with the list of all voice annotations for the
         * current time.
         * 
         * @return JSON output.
         */
        std::string
        getVoiceAnnotationsJSON()
        {
            std::string out;
        
            auto player = App::ui->uiView->getTimelinePlayer();
            if (!player) return out;

            const auto& viewportSize = App::ui->uiView->getViewportSize();
            const auto& renderSize = App::ui->uiView->getRenderSize();
            const float viewZoom = App::ui->uiView->viewZoom();
            const math::Matrix4x4f& mvp = App::ui->uiView->projectionMatrix();

            // Calculate resolution multiplier.
            float resolutionMultiplier = renderSize.w * 6 / 4096.0 / viewZoom;
            resolutionMultiplier = std::clamp(resolutionMultiplier, 1.F, 10.F);
            
            nlohmann::json j;
            j["viewportSize"] = viewportSize;
            j["renderSize"] = renderSize;
            j["viewZoom"] = viewZoom;
            j["mvp"] = mvp;

            auto annotations = player->getVoiceAnnotations();
            std::vector<nlohmann::json > voiceAnnotations;
            for (auto annotation : annotations)
            {
                nlohmann::json a;
                a["time"] = annotation->time.floor();
                a["allFrames"] = annotation->allFrames;

                std::vector<nlohmann::json > voices;
                for (auto voice : annotation->voices)
                {
                    nlohmann::json v;
                    v["center"] = voice->getCenter();
                    v["voice"] = voice->getFileName();
                    v["mouse"] = voice->mouse;
                    v["multiplier"] = resolutionMultiplier;
                    voices.push_back(v);
                }
                a["voices"] = voices;
                voiceAnnotations.push_back(a);
            }
            j["voiceAnnotations"] = voiceAnnotations;

            // Voice over recordings are always done at 30 FPS.
            // Maybe this should use player's default speed instead?
            j["FPS"] = 30.F;  

            out = j.dump(4);
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
        _("Get all .wav voice annotations for current frame."));
    annotations.def(
        "getVoiceAnnotationsJSON", &mrv2::annotations::getVoiceAnnotationsJSON,
        _("Get all voice and mouse directions for current frame as a JSON file."));
}
