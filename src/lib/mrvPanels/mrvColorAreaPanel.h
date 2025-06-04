// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include "mrvPanelWidget.h"

#include "mrvCore/mrvColorAreaInfo.h"

class ViewerUI;

namespace mrv
{
    namespace area
    {
        class Info;
    }

    namespace panel
    {
        class ColorAreaPanel : public PanelWidget
        {
        public:
            ColorAreaPanel(ViewerUI* ui);
            ~ColorAreaPanel();

            void add_controls() override;

            void update(const area::Info& info);

        private:
            MRV2_PRIVATE();
        };

    } // namespace panel

} // namespace mrv
