// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <string>
#include <map>

namespace mrv
{
    namespace session
    {
        //! Returns all metadata.
        const std::map<std::string, std::string>& metadata();

        //! Given a key returns user provided metadata in the current session.
        const std::string& metadata(const std::string& key);

        //! Stores some user provided metadata.
        void setMetadata(const std::string& key, const std::string& value);

        //! Replaces all metadata.
        void setMetadata(const std::map<std::string, std::string>&);

        //! Clears the metadata for the current session.
        void clearMetadata();

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
