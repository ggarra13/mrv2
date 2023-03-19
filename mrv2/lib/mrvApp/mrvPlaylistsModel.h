// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <vector>
#include "mrvApp/mrvFilesModel.h"

namespace mrv
{
    struct Playlist
    {
        std::string name;
        std::vector<std::shared_ptr< FilesModelItem > > clips;
    };

    //! Files model.
    class PlaylistsModel : public std::enable_shared_from_this<PlaylistsModel>
    {
        TLRENDER_NON_COPYABLE(PlaylistsModel);

    protected:
        void _init(const std::shared_ptr<system::Context>&);
        PlaylistsModel();

    public:
        ~PlaylistsModel();

        //! Create a new files model.
        static std::shared_ptr<PlaylistsModel>
        create(const std::shared_ptr<system::Context>&);

        //! Observe the files.
        std::shared_ptr<observer::IList<std::shared_ptr<Playlist> > >
        observePlaylists() const;

        //! Observe the playlist index.
        std::shared_ptr<observer::IValue<int> > observeIndex() const;
        
        //! Add a playlist
        void add(const std::shared_ptr<Playlist>&);

        //! Set the current playlist
        void set(int index);
        
        //! Close the current playlist
        void close();

    private:
        int _index(const std::shared_ptr<Playlist>&) const;
        
        TLRENDER_PRIVATE();
    };
    
} // namespace mrv
