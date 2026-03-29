// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include "mrvCore/mrvColorAreaInfo.h"

#include "mrvPanelWidget.h"

namespace mrv
{
    namespace area
    {
        class Info;
    }

    namespace panel
    {
        class WaveformPanel : public PanelWidget
        {
        public:
            WaveformPanel(ViewerUI* ui);
            ~WaveformPanel();

            void add_controls() override;

            void update(const area::Info& info);

        private:
            MRV2_PRIVATE();
        };

    } // namespace panel
} // namespace mrv
