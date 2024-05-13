// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <vector>

#include <tlCore/Time.h>

#include "mrvPanels/mrvThumbnailPanel.h"

namespace mrv
{
    class PlaylistButton;

    namespace panel
    {
        class PlaylistPanel : public ThumbnailPanel
        {
        public:
            PlaylistPanel(ViewerUI* ui);
            ~PlaylistPanel();

            void add_controls() override;

            void
            add(const math::Vector2i& pos, const size_t index, ViewerUI* ui);

            void redraw();
            void refresh();

        private:
            MRV2_PRIVATE();
        };

    } // namespace panel
} // namespace mrv
