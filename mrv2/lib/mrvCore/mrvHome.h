// SPDX-License-Identifier: BSD-3-Clause
// mrv2 (mrViewer2)
// Copyright Contributors to the mrv2 Project. All rights reserved.


#pragma once

#include <string>

namespace mrv
{
    //! Path to a temporary directory
    std::string rootpath();
    
    //! Path to a temporary directory
    std::string tmppath();

    //! Path to a studio defined directory
    std::string studiopath();

    //! Path to user's home directory
    std::string homepath();

    //! Path to preference's file
    std::string prefspath();

    //! Path to lockfile (currently unused in mrv2)
    std::string lockfile();
    
    //! Path to glsl shaders
    std::string shaderpath();
}
