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
        class VectorscopePanel : public PanelWidget
        {
        public:
            VectorscopePanel(ViewerUI* ui);
            ~VectorscopePanel();

            void add_controls() override;

            void update(const area::Info& info);

        private:
            MRV2_PRIVATE();
        };

    } // namespace panel
} // namespace mrv
