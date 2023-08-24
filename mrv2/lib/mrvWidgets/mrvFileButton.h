// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include "mrvWidgets/mrvClipButton.h"

namespace mrv
{
    class FileDragger;

    class FileButton : public ClipButton
    {
    public:
        FileButton(int X, int Y, int W, int H, const char* L = 0);
        int handle(int event) override;

        void setIndex(size_t value) { index = value; };

    protected:
        size_t index = 0;
        FileDragger* drag = nullptr;
    };
} // namespace mrv
