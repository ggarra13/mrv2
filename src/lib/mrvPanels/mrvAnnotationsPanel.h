// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include "mrvPanels/mrvPanelWidget.h"

class ViewerUI;
class Fl_Button;

namespace mrv
{
    class Button;

    namespace panel
    {
        class AnnotationsPanel : public PanelWidget
        {
            Fl_Button* penColor = nullptr;
            Button* hardBrush = nullptr;
            Button* softBrush = nullptr;

            int openedNote = -1;

        public:
            AnnotationsPanel(ViewerUI* ui);
            virtual ~AnnotationsPanel() {};

            void add_controls() override;

            void redraw();
        };
    } // namespace panel

} // namespace mrv
