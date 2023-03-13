// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.


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

    bool setA(const std::shared_ptr<FilesModelItem>& item)
    {
        auto model = filesModel();
        auto files = model->observeFiles()->get();
        int idx = 0;
        for (const auto& file : files)
        {
            if (file == item)
            {
                model->setA(idx);
                return true;
            }
            ++idx;
        }
        return false;
    }

    void close(const std::shared_ptr<FilesModelItem>& item)
    {
        auto model = filesModel();
        bool ok = setA(item);
        if (!ok)
            return;
        model->close();
    }

    //! Compare itemA with itemB with the mode set (default Wipe)
    bool setB(const std::shared_ptr<FilesModelItem>& item)
    {
        auto model = filesModel();
        auto files = model->observeFiles()->get();
        int idx = 0;
        for (const auto& file : files)
        {
            if (file == item)
            {
                const auto bIndexes = model->observeBIndexes()->get();
                const auto i = std::find(bIndexes.begin(), bIndexes.end(), idx);
                model->setB(idx, i == bIndexes.end());
                return true;
            }
            ++idx;
        }
        return false;
    }

    //! Compare itemA with itemB with the mode set (default Wipe)
    bool compare(
        const std::shared_ptr<FilesModelItem>& itemA,
        const std::shared_ptr<FilesModelItem>& itemB,
        timeline::CompareMode mode)
    {

        if (!setB(itemB))
            return false;
        if (!setA(itemA))
            return false;

        auto model = filesModel();
        auto o = model->observeCompareOptions()->get();
        o.mode = mode;
        model->setCompareOptions(o);

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

    cmds.def("setA", &cmd::setA, _("Set the A file item."), py::arg("item"));

    cmds.def(
        "compare", &cmd::compare,
        _("Compare two file items with a compare mode."), py::arg("itemA"),
        py::arg("itemB"), py::arg("mode") = tl::timeline::CompareMode::Wipe);

    cmds.def(
        "setB", &cmd::setB, _("Set the B file item for comparisons."),
        py::arg("item"));

    cmds.def("close", &cmd::close, _("Close the file item."), py::arg("item"));

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

    cmds.def("update", &cmd::update, _("Call Fl::check to update the GUI."));

}
