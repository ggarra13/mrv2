// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include "mrvPanelWidget.h"

namespace mrv
{
#ifdef MRV2_NETWORK
    namespace panel
    {
        class WebRTCPanel : public PanelWidget
        {
        public:
            WebRTCPanel(ViewerUI* ui);
            ~WebRTCPanel();

            void add_controls() override;

            void shutdown();

        private:
            void deactivate();

            MRV2_PRIVATE();
        };

    } // namespace panel
#endif

} // namespace mrv
