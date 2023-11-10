
#pragma once

#include <string>
#include <vector>

namespace mrv
{
    namespace fonts
    {
        //! List all fonts in the system.
        std::vector<std::string> list();

        //! Compare a fontName to the list in the system and return its
        //! index.  If fontName is not found, returns FL_HELVETICA.
        int compare(const std::string& fontName);
    } // namespace fonts
} // namespace mrv
