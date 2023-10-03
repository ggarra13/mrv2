// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include "mrvPanelWidget.h"

class ViewerUI;

namespace mrv
{
#ifdef MRV2_NETWORK
    class NetworkPanel : public PanelWidget
    {
        enum class Type { Client, Server };

    public:
        NetworkPanel(ViewerUI* ui);
        ~NetworkPanel();

        void add_controls() override;

        void shutdown();

    private:
        void deactivate();

        MRV2_PRIVATE();
    };
#endif

} // namespace mrv
