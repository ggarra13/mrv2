// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
namespace py = pybind11;

#include "mrvPy/CmdsAux.h"

#include "mrvCore/mrvI8N.h"

#include "mrvFl/mrvCallbacks.h"

#include "mrvEdit/mrvEditUtil.h"

#include "mrvApp/mrvFilesModel.h"

#include "mrViewer.h"

namespace mrv2
{
    namespace playlist
    {
        using namespace mrv;

        std::vector<std::shared_ptr<FilesModelItem>> list()
        {
            ViewerUI* ui = App::ui;

            auto model = ui->app->filesModel();
            auto items = model->observeFiles()->get();

            std::vector<std::shared_ptr<FilesModelItem>> out;
            for (unsigned i = 0; i < items.size(); ++i)
            {
                auto path = items[i]->path;
                if (!isTemporaryEDL(path))
                    continue;
                out.push_back(items[i]);
            }
            return out;
        }

        void select(const std::shared_ptr<FilesModelItem>& item)
        {
            ViewerUI* ui = App::ui;

            auto model = ui->app->filesModel();
            auto items = model->observeFiles()->get();

            int Aindex = -1;
            for (unsigned i = 0; i < items.size(); ++i)
            {
                auto path = items[i]->path;
                if (!isTemporaryEDL(path))
                    continue;
                if (item == items[i])
                {
                    Aindex = i;
                    break;
                }
            }
            if (Aindex < 0)
                throw std::runtime_error(_("No playlist matched item."));

            model->setA(Aindex);
        }

        void select(const std::string& fileName)
        {
            ViewerUI* ui = App::ui;

            auto model = ui->app->filesModel();
            auto items = model->observeFiles()->get();

            int Aindex = -1;
            for (unsigned i = 0; i < items.size(); ++i)
            {
                auto path = items[i]->path;
                if (!isTemporaryEDL(path))
                    continue;
                if (path.get() == fileName)
                {
                    Aindex = i;
                    break;
                }
            }
            if (Aindex < 0)
                throw std::runtime_error(_("No playlist matched file name."));

            model->setA(Aindex);
        }

        void select(const unsigned playlistIndex)
        {
            ViewerUI* ui = App::ui;

            auto model = ui->app->filesModel();
            auto items = model->observeFiles()->get();

            int Aindex = -1;
            unsigned idx = 0;
            for (unsigned i = 0; i < items.size(); ++i)
            {
                auto path = items[i]->path;
                if (!isTemporaryEDL(path))
                    continue;
                if (playlistIndex == idx)
                {
                    Aindex = i;
                    break;
                }
                ++idx;
            }
            if (Aindex < 0)
                throw std::runtime_error(_("No playlist loaded."));

            model->setA(Aindex);
        }

        void add_clip(const std::shared_ptr<FilesModelItem>& clip)
        {
            ViewerUI* ui = App::ui;

            auto model = ui->app->filesModel();
            auto items = model->observeFiles()->get();
            int playlistIndex = model->observeAIndex()->get();
            if (playlistIndex < 0)
                throw std::runtime_error(_("No playlist selected."));
            auto path = items[playlistIndex]->path;
            if (!isTemporaryEDL(path))
                throw std::runtime_error(
                    _("Not an EDL playlist to add clips to."));
            int Aindex = -1;
            for (int i = 0; i < items.size(); ++i)
            {
                if (clip == items[i])
                {
                    Aindex = i;
                    break;
                }
            }
            if (Aindex == -1)
                throw std::runtime_error(
                    _("Could not find clip in loaded clips."));

            const std::string Afile = items[Aindex]->path.get();
            add_clip_to_timeline(Afile, Aindex, ui);
        }

        /**
         * @brief Save the .otio playlist to disk with relative paths
         *        and loads it right after.
         *
         * @param fileName file name of the playlist to save on disk.
         */
        void save(const std::string& fileName)
        {
            ViewerUI* ui = App::ui;

            auto model = ui->app->filesModel();
            auto Aitem = model->observeA()->get();
            file::Path path = Aitem->path;
            if (!isTemporaryEDL(path))
                throw std::runtime_error(_("Not an EDL file to save."));

            auto player = ui->uiView->getTimelinePlayer();
            if (!player)
                return;
            auto timeline = player->getTimeline();
            if (timeline->duration().value() <= 0.0)
                throw std::runtime_error(_("Empty EDL file.  Not saving."));

            std::string otioFile = fileName;
            file::Path otioPath(otioFile);
            if (otioPath.getExtension() != ".otio")
                otioFile += ".otio";

            save_timeline_to_disk(timeline, otioFile);
        }

    } // namespace playlist
} // namespace mrv2

void mrv2_playlist(py::module& m)
{
    using namespace mrv;

    py::module playlist = m.def_submodule("playlist");
    playlist.doc() = _(R"PYTHON(
Playlist module.

Contains all functions and classes related to the playlists.
)PYTHON");

    playlist.def("list", &mrv2::playlist::list, _("List playlist items."));

    playlist.def(
        "select",
        py::overload_cast<const std::shared_ptr<FilesModelItem>&>(
            &mrv2::playlist::select),
        _("Select a playlist by FileModelItem."), py::arg("item"));

    playlist.def(
        "select",
        py::overload_cast<const std::string&>(&mrv2::playlist::select),
        _("Select a playlist by fileName."), py::arg("fileName"));

    playlist.def(
        "select", py::overload_cast<const unsigned>(&mrv2::playlist::select),
        _("Select a playlist by index."), py::arg("playlistIndex"));

    playlist.def(
        "add_clip", &mrv2::playlist::add_clip,
        _("Add a clip to currently selected Playlist EDL."), py::arg("clip"));

    playlist.def(
        "save", &mrv2::playlist::save,
        _("Save current .otio file with relative paths."), py::arg("fileName"));
}
