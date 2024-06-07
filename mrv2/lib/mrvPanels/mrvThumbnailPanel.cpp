// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <FL/Fl_Widget.H>
#include <FL/Fl.H>

#include <tlCore/StringFormat.h>

#include "mrvPanels/mrvThumbnailPanel.h"

#include "mrViewer.h"

namespace
{
    const float kTimeout = 0.05;
}

namespace mrv
{

    namespace panel
    {

        ThumbnailPanel::ThumbnailPanel(ViewerUI* ui) :
            PanelWidget(ui)
        {
            Fl::add_timeout(kTimeout, (Fl_Timeout_Handler)timerEvent_cb, this); 
        }

        ThumbnailPanel::~ThumbnailPanel()
        {
            Fl::remove_timeout((Fl_Timeout_Handler)timerEvent_cb, this); 
        }

        void ThumbnailPanel::timerEvent_cb(void* opaque)
        {
            ThumbnailPanel* panel = static_cast< ThumbnailPanel* >(opaque);
            panel->timerEvent();
        }

        void ThumbnailPanel::timerEvent()
        {
            auto i = thumbnailRequests.begin();
            while (i != thumbnailRequests.end())
            {
                if (i->second.future.valid() &&
                    i->second.future.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
                {
                    if (auto image = i->second.future.get())
                    {
                        if (image::PixelType::RGBA_U8 == image->getPixelType())
                        {
                            const int w = image->getWidth();
                            const int h = image->getHeight();
                            const int depth = 4;

                            uint8_t* pixelData = new uint8_t[w * h * depth];

                            auto rgbImage = new Fl_RGB_Image(
                                pixelData, w, h, depth);
                            rgbImage->alloc_array = true;

                            const uint8_t* d = pixelData;
                            const uint8_t* s = image->getData();
                            for (int y = 0; y < h; ++y)
                            {
                                memcpy(
                                    pixelData + (h - 1 - y) * w * 4,
                                    s + y * w * 4, w * 4);
                            }
                            i->first->bind_image(rgbImage);
                            i->first->redraw();
                        }
                    }
                    i = thumbnailRequests.erase(i);
                }
                else
                {
                    ++i;
                }
            }
            Fl::repeat_timeout(kTimeout, (Fl_Timeout_Handler)timerEvent_cb,
                               this);
        }

        void ThumbnailPanel::_createThumbnail(
            Fl_Widget* widget, const file::Path& path,
            const otime::RationalTime& currentTime, const int layerId,
            const bool isNDI)
        {
            TLRENDER_P();

            static Fl_SVG_Image* NDIimage = load_svg("NDI.svg");

            if (!p.ui->uiPrefs->uiPrefsPanelThumbnails->value())
            {
                widget->bind_image(nullptr);
                return;
            }

            if (isNDI)
            {
                widget->bind_image(NDIimage->copy());
                return;
            }


            try
            {
                const auto context = App::app->getContext();
                auto thumbnailSystem = context->getSystem<ui::ThumbnailSystem>();
                const auto& timeline = timeline::Timeline::create(path,
                                                                  context);
                const auto& timeRange = timeline->getTimeRange();

                auto time = currentTime;

                if (time::isValid(timeRange))
                {
                    auto startTime = timeRange.start_time();
                    auto endTime = timeRange.end_time_inclusive();

                    // If single frame and we have an icon, return.
                    if (startTime == endTime && widget->image())
                    {
                        return;
                    }
                    
                    if (time < startTime)
                        time = startTime;
                    else if (time > endTime)
                        time = endTime;
                }

                auto it = thumbnailRequests.find(widget);
                if (it != thumbnailRequests.end())
                {
                    std::vector<uint64_t> cancelIds;
                    const auto& request = it->second;
                    cancelIds.push_back(request.id);
                    thumbnailSystem->cancelRequests(cancelIds);
                    thumbnailRequests.erase(it);
                }

                io::Options options;
                if (_clearCache)
                {
                    auto cache = thumbnailSystem->getCache();
                    // @todo:
                    //cache->clear();
                    _clearCache = false;
                }
                
                thumbnailRequests[widget] =
                    thumbnailSystem->getThumbnail(path, size.h, time, options);
            }
            catch (const std::exception& e)
            {
                // We don't log errors on purpose
            }
        }

        void ThumbnailPanel::clearCache()
        {
            _clearCache = true;
        }

        void ThumbnailPanel::_cancelRequests()
        {
            const auto context = App::app->getContext();
            auto thumbnailSystem = context->getSystem<ui::ThumbnailSystem>();
                
            std::vector<uint64_t> ids;
            for (const auto& i : thumbnailRequests)
            {
                const auto& request = i.second;
                ids.push_back(request.id);
            }
            thumbnailSystem->cancelRequests(ids);
            thumbnailRequests.clear();
        }

    } // namespace panel

} // namespace mrv
