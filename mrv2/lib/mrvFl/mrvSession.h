#pragma once

#include <string>

namespace mrv
{
    //! Returns true on success, false on failure
    bool save_session(const std::string& file);

    //! Returns true on success, false on failure
    bool load_session(const std::string& file);
} // namespace mrv
