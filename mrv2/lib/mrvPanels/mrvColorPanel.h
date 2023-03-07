// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <tlTimeline/IRender.h>

#include "mrvPanelWidget.h"

class ViewerUI;

namespace mrv
{

    class ColorPanel : public PanelWidget
    {
    public:
        ColorPanel(ViewerUI* ui);
        ~ColorPanel();


        void setLUTOptions(const timeline::LUTOptions&);

        void setColor(const timeline::Color&);
        
        void setLevels(const timeline::Levels&);
        
        void setDisplayOptions(const timeline::DisplayOptions&);
        
        void refresh() noexcept;

        void add_controls() override;

    private:
        struct Private;
        std::unique_ptr<Private> _r;
    };

} // namespace mrv
