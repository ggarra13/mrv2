// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include "mrvOptions/mrvStereo3DOptions.h"

#include "mrvPanels/mrvThumbnailPanel.h"

namespace mrv
{
    class HorSlider;

    namespace panel
    {

        class Stereo3DPanel : public ThumbnailPanel
        {
        public:
            Stereo3DPanel(ViewerUI* ui);
            virtual ~Stereo3DPanel();

            void setStereo3DOptions(const Stereo3DOptions& value);

            void add_controls() override;

            void refresh();

            void redraw();

        private:
            MRV2_PRIVATE();
        };

    } // namespace panel

} // namespace mrv
