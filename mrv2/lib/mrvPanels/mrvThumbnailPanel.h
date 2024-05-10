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
    namespace panel
    {
        using namespace tl;
        class ThumbnailPanel : public PanelWidget
        {
        public:
            ThumbnailPanel(ViewerUI* ui);
            virtual ~ThumbnailPanel();

            //! Clear our thumbnail cache
            void clearCache();
            
            //! FLTK callback
            static void timerEvent_cb(void*);
            void timerEvent();
            
        protected:
            void _tickEvent();
            
            void _updateThumbnail(
                Fl_Widget* widget, const std::shared_ptr<image::Image>& image);
            void _thumbnailEvent();

            void _createThumbnail(Fl_Widget* widget,
                                  const file::Path& path,
                                  const otime::RationalTime& time,
                                  const int layerId = 0,
                                  const int height = 64,
                                  const bool isNDI = false);
            
            void _cancelRequests();

        private:
            
            std::weak_ptr<system::Context> context;

            // New thumbnail classes
            std::vector<tl::file::MemoryRead> memoryRead;
            std::shared_ptr<ui::ThumbnailGenerator> thumbnailGenerator;
            std::shared_ptr<gl::GLFWWindow> window;
            static std::map<std::string, std::shared_ptr<image::Image> > thumbnails;

            // Requests classes
            io::Options ioOptions;
            ui::InfoRequest infoRequest;
            std::shared_ptr<io::Info> ioInfo;
            std::map<otime::RationalTime, ui::ThumbnailRequest>
            thumbnailRequests;

            // Mapping of ids to panel data.
            struct CallbackData
            {
                Fl_Widget* widget = nullptr;
                file::Path path;
            };
            std::map<int64_t, std::shared_ptr<CallbackData>> callbackData;
        };

    } // namespace panel
} // namespace mrv
