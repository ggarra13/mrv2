// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <memory>

#include <tlCore/Util.h>

#include "mrvWidgets/mrvClipButton.h"

namespace mrv
{
    class FileDragger;

    class FileButton : public ClipButton
    {
    public:
        FileButton(int X, int Y, int W, int H, const char* L = 0);
        ~FileButton();
        int handle(int event) override;

        void setIndex(size_t value);

    protected:
        TLRENDER_PRIVATE();
    };
} // namespace mrv
