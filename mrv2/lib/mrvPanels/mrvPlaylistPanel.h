// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <vector>

#include <tlCore/Time.h>

#include "mrvPanelWidget.h"

class ViewerUI;
class Fl_RGB_Image;

namespace mrv
{
    using namespace tl;

    class ClipButton;

    class PlaylistPanel : public PanelWidget
    {
    public:
        PlaylistPanel(ViewerUI* ui);
        ~PlaylistPanel();

        void clear_controls();
        void add_controls() override;

        void refresh();
        void playlistThumbnail(
            const int64_t id,
            const std::vector< std::pair<otime::RationalTime, Fl_RGB_Image*> >&
                thumbnails,
            ClipButton* w);

    protected:
        void cancel_thumbnails();

    private:
        struct Private;
        std::unique_ptr<Private> _r;
    };

} // namespace mrv
