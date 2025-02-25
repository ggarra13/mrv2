// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#ifdef TLRENDER_NDI

#include "mrvPanelWidget.h"

class ViewerUI;
class Fl_Menu_Item;
class Fl_Button;

namespace mrv
{

    class ToggleButton;
    
    namespace panel
    {
        class NDIPanel : public PanelWidget
        {
        public:
            NDIPanel(ViewerUI* ui);
            ~NDIPanel();

            void add_controls() override;

            static void refresh_sources_cb(void* v);
            void refresh_sources();

        protected:
            void _ndi_input(const Fl_Menu_Item*);
            void _ndi_output(ToggleButton*);

        private:
            MRV2_PRIVATE();
        };

    } // namespace panel
} // namespace mrv

#endif // TLRENDER_NDI
