// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include "mrvPanelWidget.h"

class ViewerUI;
class Fl_Button;

namespace mrv
{
    class Toggle_Button;

    class AnnotationsPanel : public PanelWidget
    {
        Fl_Button* penColor = nullptr;
        Toggle_Button* hardBrush = nullptr;
        Toggle_Button* softBrush = nullptr;

    public:
        AnnotationsPanel(ViewerUI* ui);
        virtual ~AnnotationsPanel(){};

        void add_controls() override;

        void redraw();
    };

} // namespace mrv
