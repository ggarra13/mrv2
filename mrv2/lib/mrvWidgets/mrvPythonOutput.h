// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.


#pragma once

#include "mrvWidgets/mrvLogDisplay.h"

namespace mrv
{
    class PythonOutput : public LogDisplay
    {
    public:
        PythonOutput( int X, int Y, int W, int H, const char* L = 0 );

        int handle( int e ) override;
    };
}
