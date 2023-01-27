// SPDX-License-Identifier: BSD-3-Clause
// mrv2 (mrViewer2)
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include "mrvToolWidget.h"

namespace mrv
{
    class HorSlider;
    
    class LatLongTool : public ToolWidget
    {
    public:
      HorSlider* hAperture;
      HorSlider* vAperture;
      HorSlider* focalLength;
      HorSlider* rotateX;
      HorSlider* rotateY;
    public:
        LatLongTool( ViewerUI* ui );
        ~LatLongTool();

        void clear_controls();
        void add_controls() override;
    };


} // namespace mrv
