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
    
} // namespace mrv
