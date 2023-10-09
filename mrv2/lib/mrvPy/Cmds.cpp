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
#include "mrvFl/mrvSession.h"
#include "mrvFl/mrvIO.h"

#include "mrvPDF/mrvSavePDF.h"

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
            App* app = App::app;
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

        std::string rootPath()
        {
            return mrv::rootpath();
        }

        std::string prefsPath()
        {
            return mrv::prefspath();
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
            return App::app->imageOptions();
        }

        /**
         * \brief Return the image options.
         *
         *
         * @return image.ImageOptions.
         */
        EnvironmentMapOptions environmentMapOptions()
        {
            ViewerUI* ui = App::ui;
            return ui->uiView->getEnvironmentMapOptions();
        }

        /**
         * \brief Set the image options.
         *
         * @param value A valid image.ImageOptions.
         */
        void setEnvironmentMapOptions(const EnvironmentMapOptions& value)
        {
            ViewerUI* ui = App::ui;
            return ui->uiView->setEnvironmentMapOptions(value);
        }

        /**
         * \brief Set the image options.
         *
         * @param value A valid image.ImageOptions.
         */
        void setImageOptions(const timeline::ImageOptions& value)
        {
            App::app->setImageOptions(value);
        }

        /**
         * \brief Return the display options.
         *
         *
         * @return image.DisplayOptions.
         */
        timeline::DisplayOptions displayOptions()
        {
            return App::app->displayOptions();
        }

        /**
         * \brief Set the display options.
         *
         * @param value a valide image.DisplayOptions.
         */
        void setDisplayOptions(const timeline::DisplayOptions& value)
        {
            App::app->setDisplayOptions(value);
        }

        /**
         * \brief Return the LUT options.
         *
         *
         * @return image.LUTOptions.
         */
        timeline::LUTOptions lutOptions()
        {
            return App::app->lutOptions();
        }

        /**
         * \brief Return the audio volume
         *
         *
         * @return volume between 0 and 1.
         */
        float volume()
        {
            App* app = App::app;
            return app->volume();
        }

        /**
         * \brief Set the audio volume.
         *
         */
        void setVolume(const float value)
        {
            App* app = App::app;
            app->setVolume(value);
        }

        /**
         * \brief Returns if the audio is muted.
         *
         * @return true or false.
         */
        bool isMuted()
        {
            App* app = App::app;
            return app->isMuted();
        }

        /**
         * \brief Set the audio mute.
         *
         */
        void setMute(const float value)
        {
            App::app->setMute(value);
        }

        /**
         * \brief Set the LUT options.
         *
         * @param value a valid image.LUTOptions.
         */
        void setLUTOptions(const timeline::LUTOptions& value)
        {
            App::app->setLUTOptions(value);
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
         * \brief Return the stereo 3D options.
         *
         *
         * @return image.Stereo3DOptions
         */
        Stereo3DOptions stereo3DOptions()
        {
            return filesModel()->observeStereo3DOptions()->get();
        }

        /**
         * \brief Set the stereo 3D Options
         *
         * @param options a valid image.Stereo3DOptions
         */
        void setStereo3DOptions(const Stereo3DOptions& options)
        {
            filesModel()->setStereo3DOptions(options);
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
            ViewerUI* ui = App::ui;
            if (!ui)
                return out;
            const auto& player = ui->uiView->getTimelinePlayer();
            if (!player)
                return out;

            const auto& info = player->player()->getIOInfo();
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
         * @param options (annotations, ffmpeg and openexr options)
         */
        void
        save(const std::string& file, const SaveOptions opts = SaveOptions())
        {
            save_movie(file, App::ui, opts);
        }

#ifdef MRV2_PDF
        /**
         * \brief Save a PDF document.
         *
         * @param file The path to the PDF document, like: document.pdf
         */
        bool savePDF(std::string file)
        {
            const ViewerUI* ui = App::ui;

            return save_pdf(file, ui);
        }
#endif

        /**
         * \brief Returns the current session file.
         *
         * @param file The path to the session file, like: test.mrv2s
         */
        std::string currentSession()
        {
            return current_session();
        }

        /**
         * \brief Open a session file.
         *
         * @param file The path to the session file, like: test.mrv2s
         */
        bool openSession(std::string file)
        {
            return load_session(file);
        }

        /**
         * \brief Save a session file.
         *
         * @param file The path to the session file, like: test.mrv2s
         */
        bool saveSession()
        {
            const std::string& file = current_session();
            if (file.empty())
            {
                throw std::runtime_error(
                    _("No session name established, cannot save."));
                return false;
            }
            return save_session(file);
        }

        /**
         * \brief Save a session file.
         *
         * @param file The path to the session file, like: test.mrv2s
         */
        bool saveSessionAs(std::string file)
        {
            if (file.substr(file.size() - 6, file.size()) != ".mrv2s")
            {
                file += ".mrv2s";
            }
            return save_session(file);
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
        "rootPath", &mrv2::cmd::rootPath,
        _("Return the root path to the insallation of mrv2."));

    cmds.def(
        "prefsPath", &mrv2::cmd::prefsPath,
        _("Return the path to preferences of mrv2."));

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
        "stereo3DOptions", &mrv2::cmd::stereo3DOptions,
        "Return the current stereo 3D options.");

    cmds.def(
        "setStereo3DOptions", &mrv2::cmd::setStereo3DOptions,
        _("Set the stereo 3D options."), py::arg("options"));
    cmds.def(
        "getLayers", &mrv2::cmd::getLayers,
        _("Get the layers of the timeline (GUI)."));

    cmds.def(
        "update", &mrv2::cmd::update,
        _("Call Fl::check to update the GUI and return the number of seconds "
          "that elapsed."));

    cmds.def(
        "isMuted", &mrv2::cmd::isMuted, _("Returns true if audio is muted."));

    cmds.def(
        "setMute", &mrv2::cmd::setMute, _("Set the muting of the audio."),
        py::arg("mute"));

    cmds.def("volume", &mrv2::cmd::volume, _("Get the playback volume."));

    cmds.def(
        "setVolume", &mrv2::cmd::setVolume, _("Set the playback volume."),
        py::arg("volume"));

    cmds.def(
        "save", &mrv2::cmd::save,
        _("Save a movie or sequence from the front layer."), py::arg("file"),
        py::arg("options") = mrv::SaveOptions());

#ifdef MRV2_PDF
    cmds.def(
        "savePDF", &mrv2::cmd::savePDF,
        _("Save a PDF document with all annotations and notes."),
        py::arg("file"));
#endif

    cmds.def(
        "currentSession", &mrv2::cmd::currentSession,
        _("Returns current session file."));

    cmds.def(
        "openSession", &mrv2::cmd::openSession, _("Open a session file."),
        py::arg("file"));

    cmds.def("saveSession", &mrv2::cmd::saveSession, _("Save a session file."));

    cmds.def(
        "saveSessionAs", &mrv2::cmd::saveSessionAs, _("Save a session file."),
        py::arg("file"));
}
/**
 * \endcond
 *
 */
