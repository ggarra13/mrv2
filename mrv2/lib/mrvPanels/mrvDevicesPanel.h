// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include "mrvPanelWidget.h"

class ViewerUI;

namespace mrv
{
    namespace panel
    {

        class DevicesPanel : public PanelWidget
        {
        public:
            DevicesPanel(ViewerUI* ui);
            ~DevicesPanel();

            void add_controls() override;

        private:
            MRV2_PRIVATE();
        };

    } // namespace panel

} // namespace mrv
