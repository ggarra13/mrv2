// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include "mrvCore/mrvBackend.h"

#include "mrvPanelWidget.h"

#ifdef OPENGL_BACKEND
#include <tlTimelineUI/ThumbnailSystem.h>
// #include <tlTimelineUI/TimelineWidget.h>
#endif

#ifdef VULKAN_BACKEND
#include <tlTimelineUIVk/ThumbnailSystem.h>
#endif

#include <tlCore/Time.h>
#include <tlCore/Path.h>

#include <map>

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

#ifdef OPENGL_BACKEND
            std::map<Fl_Widget*, timelineui::ThumbnailRequest> thumbnailRequests;
#endif
#ifdef VULKAN_BACKEND
            std::map<Fl_Widget*, timelineui_vk::ThumbnailRequest> thumbnailRequests;
#endif
        };

    } // namespace panel
} // namespace mrv
