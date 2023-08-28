// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <memory>

#include <tlCore/Util.h>
#include <tlCore/Context.h>

#include "mrvWidgets/mrvClipButton.h"

namespace mrv
{
    using namespace tl;

    class PlaylistButton : public ClipButton
    {
    public:
        PlaylistButton(int X, int Y, int W, int H, const char* L = 0);
        ~PlaylistButton();

        int handle(int event) override;
        void draw() override;

        void setIndex(size_t value);

        void createTimeline(const std::shared_ptr<system::Context>&);

    protected:
        void _countVideoAndAudioClips();

        TLRENDER_PRIVATE();
    };
} // namespace mrv
