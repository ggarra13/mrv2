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

namespace mrv
{
    namespace file
    {
        std::vector< std::shared_ptr< mrv::FilesModelItem >> fileList()
        {
            return filesModel()->observeFiles()->get();
        }

        std::vector< std::shared_ptr< mrv::FilesModelItem >> activeFiles()
        {
            return filesModel()->observeActive()->get();
        }

        std::shared_ptr< mrv::FilesModelItem > Afile()
        {
            return filesModel()->observeA()->get();
        }

        int Aindex()
        {
            return filesModel()->observeAIndex()->get();
        }

        std::vector< std::shared_ptr< mrv::FilesModelItem >> Bfiles()
        {
            return filesModel()->observeB()->get();
        }

        std::vector< int > BIndexes()
        {
            return filesModel()->observeBIndexes()->get();
        }

        void replace(
            const std::size_t idx, const std::shared_ptr<FilesModelItem>& item)
        {
            filesModel()->replace(idx, item);
        }

        void close()
        {
            return filesModel()->close();
        }

        void closeAll()
        {
            return filesModel()->closeAll();
        }

        void setA(int idx)
        {
            return filesModel()->setA(idx);
        }

        void setB(int idx, bool value)
        {
            return filesModel()->setB(idx, value);
        }

        void toggleB(int idx)
        {
            return filesModel()->toggleB(idx);
        }

        void clearB()
        {
            return filesModel()->clearB();
        }

        std::vector< int > layers()
        {
            return filesModel()->observeLayers()->get();
        }

        void setLayer(const std::shared_ptr<FilesModelItem>& item, int layer)
        {
            return filesModel()->setLayer(item, layer);
        }

        void firstVersion()
        {
            first_image_version_cb( nullptr, Preferences::ui );
        }
        
        void previousVersion()
        {
            previous_image_version_cb( nullptr, Preferences::ui );
        }
        
        void nextVersion()
        {
            next_image_version_cb( nullptr, Preferences::ui );
        }
        
        void lastVersion()
        {
            last_image_version_cb( nullptr, Preferences::ui );
        }
        
    } // namespace file
} // namespace mrv

void mrv2_filesmodel(pybind11::module& m)
{
    using namespace mrv;

    py::module media = m.def_submodule("media");

    media.def("fileList", &mrv::file::fileList, _("Return all the files."));

    media.def(
        "activeFiles", &mrv::file::activeFiles,
        _("Return all the active files."));

    media.def("Afile", &mrv::file::Afile, _("Return the A file item."));

    media.def("Aindex", &mrv::file::Aindex, _("Return the A file index."));

    media.def("Bfiles", &mrv::file::Bfiles, _("Return the list of B files."));

    media.def(
        "BIndexes", &mrv::file::BIndexes, _("Return the list of B indexes."));

    media.def(
        "replace", &mrv::file::replace,
        _("Replace a file at a certain index with another."), py::arg("index"),
        py::arg("item"));

    media.def("close", &mrv::file::close, _("Close the current A file."));

    media.def("closeAll", &mrv::file::closeAll, _("Close all files."));

    media.def(
        "setA", &mrv::file::setA, _("Set the A file index."), py::arg("index"));

    media.def(
        "setB", &mrv::file::setB, _("Set a new B file index."),
        py::arg("index"), py::arg("value"));

    media.def(
        "toggleB", &mrv::file::toggleB, _("Toggle the B file index."),
        py::arg("index"));

    media.def("clearB", &mrv::file::clearB, _("Clear the B indexes."));

    media.def("layers", &mrv::file::layers, _("Return the list of layers."));

    media.def(
        "setLayer", &mrv::file::setLayer, _("Set layer for file item."),
        py::arg("item"), py::arg("layer"));

    media.def(
        "firstVersion", &mrv::file::firstVersion, _("Set the first version for current media."));

    media.def(
        "previousVersion", &mrv::file::previousVersion, _("Set the previous version for current media."));

    media.def(
        "nextVersion", &mrv::file::nextVersion, _("Set the next version for current media."));

    media.def(
        "lastVersion", &mrv::file::lastVersion, _("Set the last version for current media."));

    
}
