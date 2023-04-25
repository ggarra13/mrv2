// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <vector>
#include <string>
#include <chrono>

#include "mrvApp/mrvFilesModel.h"
#include "mrvApp/App.h"

#include "mrvCore/mrvHome.h"

#include "mrvFl/mrvSaving.h"

#include "mrvPy/CmdsAux.h"
#include "mrvPy/Cmds.h"

#include <pybind11/embed.h>
#include <pybind11/stl.h>
namespace py = pybind11;

#include "mrViewer.h"

namespace mrv2
{
    namespace cmd
    {
        using namespace mrv;

        /**
         *  \brief Open a file with optiona audio.
         *
         * @param file        str holding the path to the file.
         * @param audioFile   optional str holding the path to the audio file.
         */
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

        /**
         * \brief Close an item.
         *
         * @param index -1 will close the current active item.  Else, it will
         *              close the set item.
         */
        void close(const int index = -1)
        {
            auto model = filesModel();
            if (index >= 0)
                model->setA(index);
            model->close();
        }

        /**
         * \brief Close all items.
         *
         */
        void closeAll()
        {
            auto model = filesModel();
            model->closeAll();
        }

        /**
         * \brief Compare two clips with a comparison mode.
         *
         * @param itemA first clip.
         * @param itemB second clip.
         * @param mode Compare mode.
         */
        void
        compare(const int itemA, const int itemB, timeline::CompareMode mode)
        {

            auto model = filesModel();

            auto o = model->observeCompareOptions()->get();
            o.mode = mode;
            model->setCompareOptions(o);

            model->setA(itemA);
            model->setB(itemB, true);
        }

        /**
         * \brief Return the image options.
         *
         *
         * @return image.ImageOptions.
         */
        timeline::ImageOptions imageOptions()
        {
            App* app = App::application();
            return app->imageOptions();
        }

        /**
         * \brief Return the image options.
         *
         *
         * @return image.ImageOptions.
         */
        EnvironmentMapOptions environmentMapOptions()
        {
            ViewerUI* ui = Preferences::ui;
            return ui->uiView->getEnvironmentMapOptions();
        }

        /**
         * \brief Set the image options.
         *
         * @param value A valid image.ImageOptions.
         */
        void setEnvironmentMapOptions(const EnvironmentMapOptions& value)
        {
            ViewerUI* ui = Preferences::ui;
            return ui->uiView->setEnvironmentMapOptions(value);
        }

        /**
         * \brief Set the image options.
         *
         * @param value A valid image.ImageOptions.
         */
        void setImageOptions(const timeline::ImageOptions& value)
        {
            App* app = App::application();
            app->setImageOptions(value);
        }

        /**
         * \brief Return the display options.
         *
         *
         * @return image.DisplayOptions.
         */
        timeline::DisplayOptions displayOptions()
        {
            App* app = App::application();
            return app->displayOptions();
        }

        /**
         * \brief Set the display options.
         *
         * @param value a valide image.DisplayOptions.
         */
        void setDisplayOptions(const timeline::DisplayOptions& value)
        {
            App* app = App::application();
            app->setDisplayOptions(value);
        }

        /**
         * \brief Return the LUT options.
         *
         *
         * @return image.LUTOptions.
         */
        timeline::LUTOptions lutOptions()
        {
            App* app = App::application();
            return app->lutOptions();
        }

        /**
         * \brief Set the LUT options.
         *
         * @param value a valid image.LUTOptions.
         */
        void setLUTOptions(const timeline::LUTOptions& value)
        {
            App* app = App::application();
            app->setLUTOptions(value);
        }

        /**
         * \brief Return the compare options.
         *
         *
         * @return media.CompareOptions
         */
        timeline::CompareOptions compareOptions()
        {
            return filesModel()->observeCompareOptions()->get();
        }

        /**
         * \brief Set the compare options.
         *
         * @param options a valid media.CompareOptions
         */
        void setCompareOptions(const timeline::CompareOptions& options)
        {
            filesModel()->setCompareOptions(options);
        }

        /**
         * \brief Return the layers in current clip being played.
         *
         *
         * @return a list of str.
         */
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

        /**
         * \brief Refresh the UI.
         *
         * This function calls Fl::check() internally.  Use this on long
         * computations, to keep the mrv2 UI updating.
         *
         */
        double update()
        {
            auto start_time = std::chrono::steady_clock::now();
            Fl::check();
            auto end_time = std::chrono::steady_clock::now();
            std::chrono::duration<double> diff = end_time - start_time;
            return diff.count();
        }

        /**
         * \brief Save a movie or sequence.
         *
         * @param file The path to the movie file or to the sequence, like:
         *        bunny.0001.exr
         */
        void save(const std::string& file)
        {
            save_movie(file, Preferences::ui);
        }

    } // namespace cmd
} // namespace mrv2

/**
 * \cond
 *
 */
void mrv2_commands(py::module& m)
{
    py::module cmds = m.def_submodule("cmd");
    cmds.doc() = _(R"PYTHON(
Command module.

Used to run main commands and get and set the display, image, compare, LUT options.
)PYTHON");

    cmds.def(
        "open", &mrv2::cmd::open, _("Open file with optional audio."),
        py::arg("filename"), py::arg("audioFilename") = std::string());

    cmds.def(
        "compare", &mrv2::cmd::compare,
        _("Compare two file items with a compare mode."), py::arg("itemA"),
        py::arg("itemB"), py::arg("mode") = tl::timeline::CompareMode::Wipe);

    cmds.def(
        "close", &mrv2::cmd::close, _("Close the file item."),
        py::arg("item") = -1);

    cmds.def("closeAll", &mrv2::cmd::closeAll, _("Close all file items."));

    cmds.def(
        "displayOptions", &mrv2::cmd::displayOptions,
        _("Return the display options."));

    cmds.def(
        "setDisplayOptions", &mrv2::cmd::setDisplayOptions,
        _("Set the display options."), py::arg("options"));

    cmds.def(
        "lutOptions", &mrv2::cmd::lutOptions, _("Return the LUT options."));

    cmds.def(
        "setLUTOptions", &mrv2::cmd::setLUTOptions, _("Set the LUT options."),
        py::arg("options"));

    cmds.def(
        "imageOptions", &mrv2::cmd::imageOptions,
        _("Return the image options."));

    cmds.def(
        "setImageOptions", &mrv2::cmd::setImageOptions,
        _("Set the image options."), py::arg("options"));

    cmds.def(
        "environmentMapOptions", &mrv2::cmd::environmentMapOptions,
        _("Return the environment map options."));

    cmds.def(
        "setEnvironmentMapOptions", &mrv2::cmd::setEnvironmentMapOptions,
        _("Set the environment map options."), py::arg("options"));

    cmds.def(
        "compareOptions", &mrv2::cmd::compareOptions,
        "Return the current compare options.");

    cmds.def(
        "setCompareOptions", &mrv2::cmd::setCompareOptions,
        _("Set the compare options."), py::arg("options"));

    cmds.def(
        "getLayers", &mrv2::cmd::getLayers,
        _("Get the layers of the timeline (GUI)."));

    cmds.def(
        "update", &mrv2::cmd::update,
        _("Call Fl::check to update the GUI and return the number of seconds "
          "that elapsed."));

    cmds.def(
        "save", &mrv2::cmd::save,
        _("Save a movie or sequence from the front layer."), py::arg("file"));
}
/**
 * \endcond
 *
 */
