// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include "mrvPanelWidget.h"

class ViewerUI;

namespace mrv
{
    class NetworkPanel : public PanelWidget
    {
    public:
        NetworkPanel(ViewerUI* ui);
        ~NetworkPanel();

        void add_controls() override;

    private:
        struct Private;
        std::unique_ptr<Private> _r;
    };

} // namespace mrv
