// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;

#include "mrvPy/CmdsAux.h"

#include "mrvCore/mrvI8N.h"

#include "mrvFl/mrvCallbacks.h"

#include "mrvApp/App.h"

namespace mrv2
{
    namespace media
    {
        using namespace mrv;

        /**
         * @brief Lists all the file media loaded.
         *
         *
         * @return a list of file media items.
         */
        std::vector< std::shared_ptr< FilesModelItem >> list()
        {
            return filesModel()->observeFiles()->get();
        }

        /**
         * @brief Lists all active file media.
         *
         *
         * @return a list of file media items.
         */
        std::vector< std::shared_ptr< FilesModelItem >> activeFiles()
        {
            return filesModel()->observeActive()->get();
        }

        /**
         * @brief Returns the current A file media.
         *
         *
         * @return a file media.
         */
        std::shared_ptr< FilesModelItem > Afile()
        {
            return filesModel()->observeA()->get();
        }

        /**
         * @brief Returns the index of the current A file media.
         *
         *
         * @return an integer or -1 if no file media.
         */
        int Aindex()
        {
            return filesModel()->observeAIndex()->get();
        }

        /**
         * @brief Returns the current B file media items.
         *
         *
         * @return a list of file media.
         */
        std::vector< std::shared_ptr< FilesModelItem >> Bfiles()
        {
            return filesModel()->observeB()->get();
        }

        /**
         * @brief Returns the indexes of the B file mesia.
         *
         *
         * @return a list of integers.
         */
        std::vector< int > BIndexes()
        {
            return filesModel()->observeBIndexes()->get();
        }

        /**
         * @brief Close an item.
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
         * @brief Close all file media opened.
         *
         */
        void closeAll()
        {
            return filesModel()->closeAll();
        }

        /**
         * @brief Set the current A file media item.
         *
         * @param index a value > 0 and less than the number or loaded file
         *        media.
         */
        void setA(int index)
        {
            return filesModel()->setA(index);
        }

        /**
         * @brief Set or add a B file media item at index "index" and optional
         *        removal of other B file items.
         *
         * @param index Index to set the B file media.
         * @param value If True, and a valid compare option is set, the B
         *              item is added to the list.
         *              If False, the item replaces any previous B media
         *              file items.
         */
        void setB(int index, bool value)
        {
            return filesModel()->setB(index, value);
        }

        /**
         * @brief Set the current stereo file media item.
         *
         * @param index a value of -1, 0 and less than the number or loaded file
         *        media.
         */
        void setStereo(int index)
        {
            return filesModel()->setStereo(index);
        }

        /**
         * @brief Toggle a B file media at a certain index.
         *
         * @param index Index of the file media to toggle.
         */
        void toggleB(int index)
        {
            return filesModel()->toggleB(index);
        }

        /**
         * @brief Clears all B file media.
         *
         */
        void clearB()
        {
            return filesModel()->clearB();
        }

        /**
         * @brief Gets the list of valid layer indices for the current A file
         *        media.
         *
         *
         * @return a list of integers.
         */
        std::vector< int > layers()
        {
            return filesModel()->observeLayers()->get();
        }

        /**
         * @brief Sets the layer for a file media.
         *
         * @param item  file media to set layer for.
         * @param layer layer index to set.
         */
        void setLayer(const std::shared_ptr<FilesModelItem>& item, int layer)
        {
            return filesModel()->setLayer(item, layer);
        }

        /**
         * @brief Goes to the first version of the current A file media item.
         *
         */
        void firstVersion()
        {
            first_image_version_cb(nullptr, App::ui);
        }

        /**
         * @brief Goes to the previous version of the current A file media item.
         *
         */
        void previousVersion()
        {
            previous_image_version_cb(nullptr, App::ui);
        }

        /**
         * @brief Goes to the next version of the current A file media item.
         *
         */
        void nextVersion()
        {
            next_image_version_cb(nullptr, App::ui);
        }

        /**
         * @brief Goes to the last version of the current A file media item.
         *
         */
        void lastVersion()
        {
            last_image_version_cb(nullptr, App::ui);
        }

    } // namespace media
} // namespace mrv2

void mrv2_filesmodel(py::module& m)
{
    using namespace mrv;

    py::module media = m.def_submodule("media");

    media.def("list", &mrv2::media::list, _("Return all the files."));

    media.def(
        "activeFiles", &mrv2::media::activeFiles,
        _("Return all the active files."));

    media.def("Afile", &mrv2::media::Afile, _("Return the A file item."));

    media.def("Aindex", &mrv2::media::Aindex, _("Return the A file index."));

    media.def("Bfiles", &mrv2::media::Bfiles, _("Return the list of B files."));

    media.def(
        "BIndexes", &mrv2::media::BIndexes, _("Return the list of B indexes."));

    media.def(
        "close", &mrv2::media::close, _("Close the current A file."),
        py::arg("index"));

    media.def("closeAll", &mrv2::media::closeAll, _("Close all files."));

    media.def(
        "setA", &mrv2::media::setA, _("Set the A file index."),
        py::arg("index"));

    media.def(
        "setB", &mrv2::media::setB, _("Set a new B file index."),
        py::arg("index"), py::arg("value"));

    media.def(
        "setStereo", &mrv2::media::setStereo, _("Set a new stereo file index."),
        py::arg("index"));

    media.def(
        "toggleB", &mrv2::media::toggleB, _("Toggle the B file index."),
        py::arg("index"));

    media.def("clearB", &mrv2::media::clearB, _("Clear the B indexes."));

    media.def("layers", &mrv2::media::layers, _("Return the list of layers."));

    media.def(
        "setLayer", &mrv2::media::setLayer, _("Set layer for file item."),
        py::arg("item"), py::arg("layer"));

    media.def(
        "firstVersion", &mrv2::media::firstVersion,
        _("Set the first version for current media."));

    media.def(
        "previousVersion", &mrv2::media::previousVersion,
        _("Set the previous version for current media."));

    media.def(
        "nextVersion", &mrv2::media::nextVersion,
        _("Set the next version for current media."));

    media.def(
        "lastVersion", &mrv2::media::lastVersion,
        _("Set the last version for current media."));
}
