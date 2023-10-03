// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include "mrvPanelWidget.h"

class ViewerUI;

namespace mrv
{
#ifdef TLRENDER_USD
    class USDPanel : public PanelWidget
    {
    public:
        USDPanel(ViewerUI* ui);
        virtual ~USDPanel(){};

        void add_controls() override;
    };

#endif // TLRENDER_USD
} // namespace mrv
