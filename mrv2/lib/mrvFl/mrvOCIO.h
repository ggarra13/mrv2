// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <string>

namespace mrv
{
    namespace image
    {
        std::string ocioIcs();
        void setOcioIcs(const std::string&);

        std::string ocioLook();
        void setOcioLook(const std::string&);

        std::string ocioView();
        void setOcioView(const std::string&);

    } // namespace image
} // namespace mrv
