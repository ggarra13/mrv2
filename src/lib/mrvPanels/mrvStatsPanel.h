// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include "mrvPanelWidget.h"

namespace mrv
{
    namespace panel
    {
        class StatsPanel : public PanelWidget
        {
        public:
            StatsPanel(ViewerUI* ui);
            ~StatsPanel();

            void add_controls() override;

            void tick();
            
        private:
            MRV2_PRIVATE();
        };

    } // namespace panel
} // namespace mrv
