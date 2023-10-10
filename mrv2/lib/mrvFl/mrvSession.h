// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <string>

namespace mrv
{
    //! Returns user provided metadata in the last saved session.
    const std::string& session_metadata();

    //! Stores some user provided metadata.
    void set_session_metadata(const std::string& metadata);

    //! Returns the current session file name.
    std::string current_session();

    //! Sets the current session to file.
    void set_current_session(const std::string& file);

    //! Returns true on success, false on failure
    bool save_session(const std::string& file);

    //! Returns true on success, false on failure
    bool load_session(const std::string& file);
} // namespace mrv
