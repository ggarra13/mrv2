// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include "mrvPanelWidget.h"

class ViewerUI;

namespace mrv
{
#ifdef TLRENDER_NDI
    namespace panel
    {
        class NDIPanel : public PanelWidget
        {
        public:
            NDIPanel(ViewerUI* ui);
            ~NDIPanel();

            void add_controls() override;

        protected:
            void _update();
            void _open_ndi();
            
        private:
            MRV2_PRIVATE();
        };

    }  // namespace panel
#endif // TLRENDER_NDI
} // namespace mrv
