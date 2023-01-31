// SPDX-License-Identifier: BSD-3-Clause
// mrv2 (mrViewer2)
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <FL/Fl_Radio_Round_Button.H>

#include "mrvToolWidget.h"


namespace mrv
{
    class HorSlider;
    
    class EnvironmentMapTool : public ToolWidget
    {
    public:
        Fl_Radio_Round_Button* sphericalMap;
        Fl_Radio_Round_Button* cubicMap;
        HorSlider* hAperture;
        HorSlider* vAperture;
        HorSlider* focalLength;
        HorSlider* rotateX;
        HorSlider* rotateY;
    public:
        EnvironmentMapTool( ViewerUI* ui );
        ~EnvironmentMapTool();

        void clear_controls();
        void add_controls() override;
    };


} // namespace mrv
