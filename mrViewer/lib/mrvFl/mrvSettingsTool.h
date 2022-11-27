// SPDX-License-Identifier: BSD-3-Clause
// mrv2 (mrViewer2)
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once


#include "mrvToolWidget.h"

class ViewerUI;

namespace mrv
{
    using namespace tl;
    
    class SettingsTool : public ToolWidget
    {
    public:
        SettingsTool( ViewerUI* ui );
        virtual ~SettingsTool() {};

        void add_controls() override;
        
    };


} // namespace mrv
