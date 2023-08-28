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

    class PlaylistButton : public ClipButton
    {
    public:
        PlaylistButton(int X, int Y, int W, int H, const char* L = 0);
        ~PlaylistButton();
        int handle(int event) override;

        void setIndex(size_t value);

    protected:
        TLRENDER_PRIVATE();
    };
} // namespace mrv
