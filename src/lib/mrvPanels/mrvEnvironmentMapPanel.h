// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <FL/Fl_Radio_Round_Button.H>

#include "mrvOptions/mrvEnvironmentMapOptions.h"

#include "mrvPanels/mrvPanelWidget.h"

namespace mrv
{
    namespace panel
    {

        class EnvironmentMapPanel : public PanelWidget
        {
        public:
            EnvironmentMapPanel(ViewerUI* ui);
            ~EnvironmentMapPanel();

            void setEnvironmentMapOptions(const EnvironmentMapOptions& value);

            void add_controls() override;

        private:
            MRV2_PRIVATE();
        };

    } // namespace panel

} // namespace mrv
