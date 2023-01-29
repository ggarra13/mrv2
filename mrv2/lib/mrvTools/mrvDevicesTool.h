// SPDX-License-Identifier: BSD-3-Clause
// mrv2 (mrViewer2)
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once


#include "mrvToolWidget.h"

class ViewerUI;

namespace mrv
{
    class DevicesTool : public ToolWidget
    {
    public:
        DevicesTool( ViewerUI* ui );
        ~DevicesTool();

        void add_controls() override;


    private:
        struct Private;
        std::unique_ptr<Private> _r;
    };

} // namespace mrv
