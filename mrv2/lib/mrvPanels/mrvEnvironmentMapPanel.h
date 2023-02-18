// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <FL/Fl_Radio_Round_Button.H>

#include "mrvPanelWidget.h"

namespace mrv
{
    class HorSlider;

    class EnvironmentMapPanel : public PanelWidget
    {
    public:
        HorSlider* focalLength;
        HorSlider* rotateX;
        HorSlider* rotateY;

    public:
        EnvironmentMapPanel(ViewerUI* ui);
        ~EnvironmentMapPanel();

        void clear_controls();
        void add_controls() override;
    };

} // namespace mrv
