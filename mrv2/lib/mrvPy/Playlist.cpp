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
    namespace playlist
    {
        void set( int index )
        {
            playlistModel()->set( index );
        }

        void close()
        {
            playlistModel()->close();
        }
        
        void create(
            const std::string& fileName,
            const std::vector< std::shared_ptr< FilesModelItem >> items)
        {
            std::shared_ptr<Playlist> playlist = std::make_shared<Playlist>();
            playlist->clips = items;

            playlistModel()->add( playlist );
            
            create_playlist( Preferences::ui, playlist, fileName, true );
        }
    }
}

void mrv2_playlist(py::module& m)
{
    using namespace mrv;
    
    py::module playlist = m.def_submodule("playlist");

    playlist.def( "create", &mrv::playlist::create,
                  _("Create a new playlist with the given name and items."),
                  py::arg("fileName"), py::arg("items") );
    
    playlist.def( "set", &mrv::playlist::set,
                  _("Select a playlist with the given index."),
                  py::arg("index"));
    
    playlist.def( "close", &mrv::playlist::close,
                  _("Close the crurent playlist."));
}
