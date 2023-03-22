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
            auto model = playlistModel();
            if ( index < 0 || index >= model->observePlaylists()->getSize() )
                throw std::out_of_range(_("Index is invalid"));
            model->set( index );
        }

        void set( const std::string& name )
        {
            auto model = playlistModel();
            auto playlists = model->observePlaylists()->get();
            int index = 0;
            for ( const auto& playlist : playlists )
            {
                if ( playlist->name == name )
                {
                    set( index );
                    return;
                }
                ++index;
            }
            throw std::invalid_argument(_("Playlist name not found."));
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

        void add( const std::shared_ptr< Playlist >& value )
        {
            playlistModel()->add( value );
        }
        
        void create(const std::string& fileName)
        {
            auto playlist = current();
            if ( playlist->clips.size() < 2 )
                throw std::runtime_error(_("Playlist has less than 2 elements"));
            create_playlist( Preferences::ui, playlist, false );
        }
        
        void save(const std::string& fileName)
        {
            auto playlist = current();
            if ( playlist->clips.size() < 2 )
                throw std::runtime_error(_("Playlist has less than 2 elements"));
            create_playlist( Preferences::ui, playlist, fileName, false );
        }
    }
}

void mrv2_playlist(py::module& m)
{
    using namespace mrv;

    py::class_<Playlist, std::shared_ptr<Playlist> >(m, "Playlist")
        .def(py::init<>())
        .def(py::init<const std::string&>())
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
    
    playlist.def( "add", &mrv::playlist::add, _("Add a playlist."),
                  py::arg("playlist"));
    
    playlist.def( "set", py::overload_cast<const int>(&mrv::playlist::set),
                  _("Select a playlist with the given index."),
                  py::arg("index"));
    
    playlist.def( "set",
                  py::overload_cast<const std::string&>(&mrv::playlist::set),
                  _("Select a playlist with the given name."),
                  py::arg("name"));
    
    playlist.def( "create", &mrv::playlist::create,
                  _("Create a temporary .otio file with absolute paths."));
    
    playlist.def( "current", &mrv::playlist::current,
                  _("Return the current playlist."));
    
    playlist.def( "close", &mrv::playlist::close,
                  _("Close the current playlist."));

    playlist.def( "save", &mrv::playlist::save,
                  _("Save an .otio file with relative paths."),
                  py::arg("fileName") );
}
