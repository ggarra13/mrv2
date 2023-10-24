// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include "mrvPanelWidget.h"

namespace mrv
{
    class Button;

    namespace panel
    {
        class BackgroundPanel : public PanelWidget
        {
        public:
            BackgroundPanel(ViewerUI* ui);
            ~BackgroundPanel();

            void add_controls() override;

        private:
            MRV2_PRIVATE();
        };
    } // namespace panel

} // namespace mrv
