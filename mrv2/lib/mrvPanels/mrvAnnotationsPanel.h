// SPDX-License-Identifier: BSD-3-Clause
// mrv2 (mrViewer2)
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once


#include "mrvPanelWidget.h"

class ViewerUI;
class Fl_Button;

namespace mrv
{
    class AnnotationsPanel : public PanelWidget
    {
        Fl_Button* penColor = nullptr;
    public:
        AnnotationsPanel( ViewerUI* ui );
        virtual ~AnnotationsPanel() {};

        void add_controls() override;

        void redraw();
    };


} // namespace mrv
