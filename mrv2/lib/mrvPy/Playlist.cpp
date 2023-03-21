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

        std::shared_ptr<Playlist> current()
        {
            auto playlists = playlistModel()->observePlaylists()->get();
            auto index = playlistModel()->observeIndex()->get();
            if ( index < 0 )
            {
                std::shared_ptr< Playlist > playlist =
                    std::make_shared<Playlist>();
                playlist->name = _("Playlist");
                return playlist;
            }
            return playlists[index];
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

    py::class_<Playlist, std::shared_ptr<Playlist> >(m, "Playlist")
        .def(py::init<>())
        .def_readwrite("name", &Playlist::name)
        .def_readwrite("clips", &Playlist::clips)
        .def(
            "__str__",
            [](const Playlist& a)
            {
                std::ostringstream s;
                s << "<mrv2.Playlist " << a.name << ">";
                return s.str();
            })
        .def(
            "__repr__",
            [](const Playlist& a)
            {
                std::ostringstream s;
                s << "<mrv2.Playlist name=" << a.name
                  << " clips=[";
                for ( size_t  i = 0; i < a.clips.size(); ++i )
                {
                    if ( i > 0 ) s << ", ";
                    const mrv::FilesModelItem* item = a.clips[i].get();
                    s << *item;
                }
                s << "]>";
                return s.str();
            })
        .doc() = _("Class used to hold a playlist");
    
    py::module playlist = m.def_submodule("playlist");

    playlist.def( "create", &mrv::playlist::create,
                  _("Create a new playlist with the given name and items."),
                  py::arg("fileName"), py::arg("items") );
    
    playlist.def( "set", &mrv::playlist::set,
                  _("Select a playlist with the given index."),
                  py::arg("index"));
    
    playlist.def( "current", &mrv::playlist::current,
                  _("Return the crurent playlist."));
    
    playlist.def( "close", &mrv::playlist::close,
                  _("Close the crurent playlist."));
}
