// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <sstream>

#include <pybind11/pybind11.h>

namespace py = pybind11;

#include <tlIO/USD.h>

#include "mrvCore/mrvI8N.h"

#include "mrvApp/mrvSettingsObject.h"
#include "mrvApp/App.h"

namespace mrv
{
    namespace usd
    {
        tl::usd::RenderOptions renderOptions()
        {
            App* app = App::application();
            auto settingsObject = app->settingsObject();
            tl::usd::RenderOptions o;
            o.renderWidth =
                std_any_cast<int>(settingsObject->value("usd/renderWidth"));
            o.complexity =
                std_any_cast<float>(settingsObject->value("usd/complexity"));
            o.drawMode = static_cast<tl::usd::DrawMode>(
                std_any_cast<int>(settingsObject->value("usd/drawMode")));
            o.enableLighting = static_cast<bool>(
                std_any_cast<int>(settingsObject->value("usd/enableLighting")));
            o.stageCacheCount =
                std_any_cast<int>(settingsObject->value("usd/stageCacheCount"));
            o.diskCacheByteCount = std_any_cast<int>(
                settingsObject->value("usd/diskCacheByteCount"));
            return o;
        }

        bool setRenderOptions(const tl::usd::RenderOptions& o)
        {
            App* app = App::application();
            auto settingsObject = app->settingsObject();
            settingsObject->setValue("usd/renderWidth", o.renderWidth);
            settingsObject->setValue("usd/complexity", o.complexity);
            settingsObject->setValue("usd/drawMode", o.drawMode);
            settingsObject->setValue("usd/enableLighting", o.enableLighting);
            settingsObject->setValue("usd/stageCacheCount", o.stageCacheCount);
            settingsObject->setValue(
                "usd/diskCacheByteCount", o.diskCacheByteCount);
            return true;
        }
    } // namespace usd
} // namespace mrv

void mrv2_usd(pybind11::module& m)
{
    using namespace tl;

    py::module usd = m.def_submodule("usd");
    usd.doc() = _(R"PYTHON(
USD module.

Contains all classes and enums related to USD (Universal Scene Description). 
)PYTHON");

    py::class_<usd::RenderOptions>(usd, "RenderOptions")
        .def(py::init<>())
        .def_readwrite(
            "renderWidth", &usd::RenderOptions::renderWidth, _("Render Width"))
        .def_readwrite(
            "complexity", &usd::RenderOptions::complexity,
            _("Models' complexity."))
        .def_readwrite(
            "drawMode", &usd::RenderOptions::drawMode,
            _("Draw mode  :class:`mrv2.usd.DrawMode`."))
        .def_readwrite(
            "enableLighting", &usd::RenderOptions::enableLighting,
            _("Enable Lighting"))
        .def_readwrite(
            "stageCacheCount", &usd::RenderOptions::stageCacheCount,
            _("Stage Cache Count"))
        .def_readwrite(
            "diskCacheByteCount", &usd::RenderOptions::diskCacheByteCount,
            _("Disk Cache Byte Count"))
        .def(
            "__repr__",
            [](const usd::RenderOptions& o)
            {
                std::stringstream s;
                s << "<mrv2.usd.RenderOptions renderWidth=" << o.renderWidth
                  << " complexity=" << o.complexity
                  << " drawMode=" << o.drawMode
                  << " enableLighting=" << (o.enableLighting ? "True" : "False")
                  << " stageCacheCount=" << o.stageCacheCount
                  << " diskCacheByteCount=" << o.diskCacheByteCount << ">";
                return s.str();
            })
        .doc() = _("USD Render Options.");

    usd.def(
        "renderOptions", &mrv::usd::renderOptions, _("Get Render Options."));

    usd.def(
        "setRenderOptions", &mrv::usd::setRenderOptions,
        _("Set Render Options."), py::arg("renderOptions"));
}
