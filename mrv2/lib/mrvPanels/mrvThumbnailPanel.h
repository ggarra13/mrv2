// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <map>

#include <tlCore/Time.h>
#include <tlCore/Path.h>

#include <tlUI/ThumbnailSystem.h>

#include <tlTimelineUI/TimelineWidget.h>

#include "mrvPanelWidget.h"

class ViewerUI;
class Fl_Widget;

namespace mrv
{
    class ThumbnailCreator;

    namespace panel
    {
        using namespace tl;
        class ThumbnailPanel : public PanelWidget
        {
        public:
            ThumbnailPanel(ViewerUI* ui);
            virtual ~ThumbnailPanel();

            //! FLTK callbacks
            static void timerEvent_cb(void* data);
            void timerEvent();

            void clearCache();

        protected:
            void _createThumbnail(
                Fl_Widget* widget, const file::Path& path,
                const otime::RationalTime& time, const int layerId = 0,
                const bool isNDI = false);

            void _cancelRequests();

            //! Default Thumbnail Size.
            image::Size size = image::Size(128, 64);

        private:
            //! Whether to clear the cache for the thumbnails.
            bool _clearCache = false;

            std::map<Fl_Widget*, ui::ThumbnailRequest> thumbnailRequests;
        };

    } // namespace panel
} // namespace mrv
