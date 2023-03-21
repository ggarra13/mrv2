#pragma once

#include <string>

#include "mrvCore/mrvI8N.h"

namespace mrv
{
    inline bool isPanelWithHeight( const std::string& label )
    {
        if ( label != _("Files") && label != _("Compare") &&
             label != _("Playlist") )
            return true;
        return false;
    }
}
