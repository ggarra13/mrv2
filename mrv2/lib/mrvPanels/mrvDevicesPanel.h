// SPDX-License-Identifier: BSD-3-Clause
// mrv2 (mrViewer2)
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once


#include "mrvPanelWidget.h"

class ViewerUI;

namespace mrv
{
    class DevicesPanel : public PanelWidget
    {
    public:
        DevicesPanel( ViewerUI* ui );
        ~DevicesPanel();

        void add_controls() override;


    private:
        struct Private;
        std::unique_ptr<Private> _r;
    };

} // namespace mrv
