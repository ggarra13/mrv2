// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include "mrvPanels/mrvPanelWidget.h"

namespace mrv
{

    namespace panel
    {
        class NotesPanel : public PanelWidget
        {
        public:
            NotesPanel(ViewerUI* ui);
            virtual ~NotesPanel();

            void add_controls() override;
        };
    } // namespace panel

} // namespace mrv
