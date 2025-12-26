// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#pragma once

#include <string>

namespace tl
{
    //! URLs
    namespace url
    {
        //! Get the URL scheme.
        std::string scheme(const std::string&);

        //! Encode a URL.
        std::string encode(const std::string&);

        //! Decode a URL.
        std::string decode(const std::string&);
    }
}
