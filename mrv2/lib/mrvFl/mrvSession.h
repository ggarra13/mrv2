// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <string>

namespace mrv
{
    namespace session
    {
        //! Returns user provided metadata in the last saved session.
        const std::string& metadata();

        //! Stores some user provided metadata.
        void setMetadata(const std::string& metadata);

        //! Returns the current session file name.
        std::string current();

        //! Sets the current session to file.
        void setCurrent(const std::string& file);

        //! Returns true on success, false on failure
        bool save(const std::string& file);

        //! Returns true on success, false on failure
        bool load(const std::string& file);
    } // namespace session

} // namespace mrv
