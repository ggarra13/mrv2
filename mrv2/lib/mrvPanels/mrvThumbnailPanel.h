// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <tlIO/Cache.h>

#include <tlCore/Time.h>
#include <tlCore/Path.h>

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
            static void updateThumbnail_cb(
                const int64_t id,
                const std::vector<
                    std::pair<otime::RationalTime, Fl_RGB_Image*> >& thumbnails,
                void* opaque);

            void clearCache();

            void updateThumbnail(
                const int64_t id,
                const std::vector<
                    std::pair<otime::RationalTime, Fl_RGB_Image*> >& thumbnails,
                Fl_Widget* w);

        protected:
            void _createThumbnail(
                Fl_Widget* widget, const file::Path& path,
                const otime::RationalTime& time, const int layerId = 0,
                const bool isNDI = false);

            void _cancelRequests();

            //! Default Thumbnail Size.
            image::Size size = image::Size(128, 64);

        private:
            typedef std::map< Fl_Widget*, int64_t > WidgetIds;
            WidgetIds ids;

            //! Whether to clear the cache for the thumbnails.
            bool _clearCache = false;

            //! Thumbnail creation class.
            ThumbnailCreator* thumbnailCreator;

            //! Thumbnails callback data.
            struct ThumbnailData
            {
                Fl_Widget* widget = nullptr;
                ThumbnailPanel* panel = nullptr;
            };
        };

    } // namespace panel
} // namespace mrv
