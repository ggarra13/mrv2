// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <vector>

#include <FL/Fl_RGB_Image.H>

#include <tlCore/Time.h>

#include "mrvOptions/mrvFilesPanelOptions.h"

#include "mrvPanels/mrvThumbnailPanel.h"

class ViewerUI;

namespace mrv
{

    class FileButton;
    class FilesPanelOptions;

    namespace panel
    {
        using namespace tl;
        class FilesPanel : public ThumbnailPanel
        {
        public:
            FilesPanel(ViewerUI* ui);

            void clear_controls();
            void add_controls() override;

            void setFilesPanelOptions(const FilesPanelOptions&);

            void redraw();

            void refresh();

        private:
            MRV2_PRIVATE();
        };

    } // namespace panel
} // namespace mrv
