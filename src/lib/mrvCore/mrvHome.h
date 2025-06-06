// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <string>
#include <vector>

namespace mrv
{

    //! Name of user running mrv2
    std::string username();

    //! Path to a root of installed directory.
    std::string rootpath();

    //! Path to a temporary directory (without a trailing slash)
    std::string tmppath();

    //! Path to the mrv2 documentation.  If not installed, it will point to
    //! the online documentation.
    std::string docspath();

    //! Path to a studio defined directory
    std::string studiopath();

    //! Path to user's home directory (without a trailing slash)
    std::string homepath();

    //! Path to the mrv2's icon directory (with a trailing slash)
    std::string iconpath();
    
    //! Path to preference's file (with a trailing slash)
    std::string prefspath();

    //! Path to the built-in python script demos
    std::string pythonpath();

    //! Path to ffmpeg presets
    std::string presetspath();
    
    //! Paths to the built-in python plug-ins
    std::vector<std::string> python_plugin_paths();

} // namespace mrv
