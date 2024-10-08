// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <vector>

#include <tlCore/Time.h>

#include <tlTimeline/CompareOptions.h>

#include "mrvPanels/mrvThumbnailPanel.h"

class ViewerUI;

namespace mrv
{

    class ClipButton;
    class HorSlider;
    class PopupMenu;

    namespace panel
    {
        using namespace tl;

        class ComparePanel : public ThumbnailPanel
        {
        public:
            HorSlider* wipeX;
            HorSlider* wipeY;
            HorSlider* wipeRotation;
            HorSlider* overlay;
            PopupMenu* compareTimeW;

        public:
            ComparePanel(ViewerUI* ui);
            ~ComparePanel();

            void add_controls() override;

            void redraw();

            void setCompareOptions(const tl::timeline::CompareOptions&);
            void setCompareTime(const timeline::CompareTimeMode&);

            void refresh();

        private:
            MRV2_PRIVATE();
        };

    } // namespace panel
} // namespace mrv
