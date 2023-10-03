// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <string>
#include <map>

namespace mrv
{
    class Browser;

    void fill_path_mappings(mrv::Browser* b);

    void add_path_mapping(mrv::Browser* b);

    void change_path_mapping(mrv::Browser* b, int idx);

    void remove_path_mapping(mrv::Browser* b);

    std::map< std::string, std::string > path_mappings();

    bool replace_path(std::string& inOutFile);
} // namespace mrv
