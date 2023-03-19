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
    class PlaylistModel : public std::enable_shared_from_this<PlaylistModel>
    {
        TLRENDER_NON_COPYABLE(PlaylistModel);

    protected:
        void _init(const std::shared_ptr<system::Context>&);
        PlaylistModel();

    public:
        ~PlaylistModel();

        //! Create a new files model.
        static std::shared_ptr<PlaylistModel>
        create(const std::shared_ptr<system::Context>&);

        //! Observe the files.
        std::shared_ptr<observer::IList<std::shared_ptr<Playlist> > >
        observePlaylists() const;

    private:
        TLRENDER_PRIVATE();
    };
    
} // namespace mrv
