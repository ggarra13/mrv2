// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <vector>

#include <FL/Fl_RGB_Image.H>

#include <tlCore/Time.h>

#include "mrvOptions/mrvFilesPanelOptions.h"

#include "mrvPanelWidget.h"

class ViewerUI;

namespace mrv
{

    class FileButton;
    class FilesPanelOptions;

    namespace panel
    {
        using namespace tl;
        class FilesPanel : public PanelWidget
        {
        public:
            FilesPanel(ViewerUI* ui);
            ~FilesPanel();

            void clear_controls();
            void add_controls() override;

            void setFilesPanelOptions(const FilesPanelOptions&);

            void redraw();

            void refresh();
            void filesThumbnail(
                const int64_t id,
                const std::vector<
                    std::pair<otime::RationalTime, Fl_RGB_Image*> >& thumbnails,
                FileButton* w);

        protected:
            void cancel_thumbnails();

        private:
            MRV2_PRIVATE();
        };

    } // namespace panel
} // namespace mrv
