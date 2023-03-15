// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <vector>
#include <string>

#include "mrvApp/mrvFilesModel.h"
#include "mrvApp/App.h"

#include "mrvCore/mrvHome.h"

#include "mrvPy/CmdsAux.h"
#include "mrvPy/Cmds.h"

#include <pybind11/embed.h>
#include <pybind11/stl.h>
namespace py = pybind11;

#include "mrViewer.h"

namespace cmd
{
    using namespace mrv;

    void open(const std::string& file, const std::string& audioFile)
    {
        App* app = App::application();
        std::string filename = file;
        if (file[0] == '~')
        {
            filename = mrv::homepath() + file.substr(1, file.size());
        }
        app->open(filename, audioFile);
    }

    void close(const int item)
    {
        auto model = filesModel();
        if (item >= 0)
            model->setA(item);
        model->close();
    }

    //! Compare itemA with itemB with the mode set (default Wipe)
    bool compare(const int itemA, const int itemB, timeline::CompareMode mode)
    {

        auto model = filesModel();

        auto o = model->observeCompareOptions()->get();
        o.mode = mode;
        model->setCompareOptions(o);

        model->setA(itemA);
        model->setB(itemB, true);

        return true;
    }

    timeline::ImageOptions imageOptions()
    {
        App* app = App::application();
        return app->imageOptions();
    }

    void setImageOptions(const timeline::ImageOptions& value)
    {
        App* app = App::application();
        app->setImageOptions(value);
    }

    timeline::DisplayOptions displayOptions()
    {
        App* app = App::application();
        return app->displayOptions();
    }

    void setDisplayOptions(const timeline::DisplayOptions& value)
    {
        App* app = App::application();
        app->setDisplayOptions(value);
    }

    timeline::LUTOptions lutOptions()
    {
        App* app = App::application();
        return app->lutOptions();
    }

    void setLUTOptions(const timeline::LUTOptions& value)
    {
        App* app = App::application();
        app->setLUTOptions(value);
    }

    timeline::CompareOptions compareOptions()
    {
        return filesModel()->observeCompareOptions()->get();
    }

    void setCompareOptions(const timeline::CompareOptions& options)
    {
        filesModel()->setCompareOptions(options);
    }

    std::vector< std::string > getLayers()
    {
        std::vector< std::string > out;
        ViewerUI* ui = Preferences::ui;
        if (!ui)
            return out;
        const auto& player = ui->uiView->getTimelinePlayer();
        if (!player)
            return out;

        const auto& info = player->timelinePlayer()->getIOInfo();
        const auto& videos = info.video;

        std::string name;
        for (const auto& video : videos)
        {
            if (video.name == "A,B,G,R" || video.name == "B,G,R")
                name = "Color";
            else
                name = video.name;
            out.push_back(name);
        }
        return out;
    }

    void update()
    {
        Fl::check();
    }

} // namespace cmd

void mrv2_commands(py::module& m)
{
    py::module cmds = m.def_submodule("cmd");

    cmds.def(
        "open", &cmd::open, _("Open file with optional audio."),
        py::arg("filename"), py::arg("audioFilename") = std::string());

    cmds.def(
        "compare", &cmd::compare,
        _("Compare two file items with a compare mode."), py::arg("itemA"),
        py::arg("itemB"), py::arg("mode") = tl::timeline::CompareMode::Wipe);

    cmds.def(
        "close", &cmd::close, _("Close the file item."), py::arg("item") = -1);

    cmds.def(
        "displayOptions", &cmd::displayOptions,
        _("Return the display options."));

    cmds.def(
        "setDisplayOptions", &cmd::setDisplayOptions,
        _("Set the display options."), py::arg("options"));

    cmds.def("lutOptions", &cmd::lutOptions, _("Return the LUT options."));

    cmds.def(
        "setLUTOptions", &cmd::setLUTOptions, _("Set the LUT options."),
        py::arg("options"));

    cmds.def(
        "imageOptions", &cmd::imageOptions, _("Return the image options."));

    cmds.def(
        "setImageOptions", &cmd::setImageOptions, _("Set the image options."),
        py::arg("options"));

    cmds.def(
        "compareOptions", &cmd::compareOptions,
        "Return the current compare options.");

    cmds.def(
        "setCompareOptions", &cmd::setCompareOptions,
        _("Set the compare options."), py::arg("options"));

    cmds.def(
        "getLayers", &cmd::getLayers,
        _("Get the layers of the timeline (GUI)."));

    cmds.def("update", &cmd::update, _("Call Fl::check to update the GUI."));
}
