// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <string>

namespace mrv
{
    //! Returns true on success, false on failure
    bool save_session(const std::string& file);

    //! Returns true on success, false on failure
    bool load_session(const std::string& file);
} // namespace mrv
