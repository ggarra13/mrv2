// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include "mrvPanelWidget.h"

class ViewerUI;

namespace mrv
{
    using namespace tl;

    class SettingsPanel : public PanelWidget
    {
    public:
        SettingsPanel(ViewerUI* ui);
        virtual ~SettingsPanel(){};

        void add_controls() override;

        void refresh();
    };

} // namespace mrv
