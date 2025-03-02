// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#ifdef TLRENDER_NDI

#include "mrvPanels/mrvPanelWidget.h"

#include <tlDevice/OutputData.h>

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
            
        protected:
            void _ndi_input(const Fl_Menu_Item*);
            void _ndi_output(ToggleButton*);
            device::PixelType  _ndi_fourCC(int fltk_value);
        private:
            MRV2_PRIVATE();
        };

    } // namespace panel
} // namespace mrv

#endif // TLRENDER_NDI
