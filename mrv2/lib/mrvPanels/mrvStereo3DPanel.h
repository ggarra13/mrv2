// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include "mrvCore/mrvStereo3DOptions.h"

#include "mrvPanels/mrvPanelWidget.h"

namespace mrv
{
    class HorSlider;

    class Stereo3DPanel : public PanelWidget
    {
    public:
        Stereo3DPanel(ViewerUI* ui);
        ~Stereo3DPanel();

        void setStereo3DOptions(const Stereo3DOptions& value);

        void add_controls() override;

        void refresh();

        void redraw();

        void stereo3DThumbnail(
            const int64_t id,
            const std::vector< std::pair<otime::RationalTime, Fl_RGB_Image*> >&
                thumbnails,
            ClipButton* w);

    private:
        void cancel_thumbnails();
        void clear_controls();

        MRV2_PRIVATE();
    };

} // namespace mrv
