// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <string>
#include <map>

namespace mrv
{
    class Browser;

    /**
     * Fill the path mapping browser from the preferences.
     *
     * @param b Path Mapping Browser.
     */
    void fill_path_mappings(mrv::Browser* b);

} // namespace mrv
