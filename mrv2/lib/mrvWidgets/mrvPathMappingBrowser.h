// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include "mrvWidgets/mrvBrowser.h"

namespace mrv
{

    class PathMappingBrowser : public Browser
    {
    public:
        PathMappingBrowser(int X, int Y, int W, int H, const char* L = 0) :
            Browser(X, Y, W, H, L)
        {
        }

    protected:
        int handle(int e) override;
    };

} // namespace mrv
