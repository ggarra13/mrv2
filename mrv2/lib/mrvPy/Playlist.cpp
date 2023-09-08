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
    namespace playlist
    {
        using namespace mrv;

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
            if (path.getBaseName() != "EDL." || path.getExtension() != ".otio")
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

    playlist.def(
        "save", &mrv2::playlist::save,
        _("Save current .otio file with relative paths."), py::arg("fileName"));
}
