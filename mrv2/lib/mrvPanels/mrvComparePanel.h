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

        public:
            ComparePanel(ViewerUI* ui);
            virtual ~ComparePanel();

            void redraw();
            
            void add_controls() override;

            void setCompareOptions(const tl::timeline::CompareOptions&);

            void refresh();

        private:
            MRV2_PRIVATE();
        };

    } // namespace panel
} // namespace mrv
