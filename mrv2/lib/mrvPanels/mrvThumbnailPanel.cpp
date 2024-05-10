// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <memory>

#include <FL/Fl_Widget.H>
#include <FL/Fl.H>

#include <tlCore/StringFormat.h>

#include <tlIO/Cache.h>

#include <tlGL/GL.h>
#include <tlGL/GLFWWindow.h>

#include "mrvCore/mrvImage.h"

#include "mrvPanels/mrvThumbnailPanel.h"

#include "mrViewer.h"


namespace
{
    const double kTimeout = 0.01;
}


namespace mrv
{

    namespace panel
    {

        std::map<std::string, std::shared_ptr<image::Image> >
        ThumbnailPanel::thumbnails;

        ThumbnailPanel::ThumbnailPanel(ViewerUI* ui) :
            PanelWidget(ui)
        {
            auto context = App::app->getContext();
            
            window = gl::GLFWWindow::create(
                "mrv::TimelineWidget::window",
                math::Size2i(1, 1),
                context,
                static_cast<int>(gl::GLFWWindowOptions::None));
            
            thumbnailGenerator = ui::ThumbnailGenerator::create(context,
                                                                window);
        
            Fl::add_timeout(
                kTimeout, (Fl_Timeout_Handler)timerEvent_cb, this);
        }

        ThumbnailPanel::~ThumbnailPanel()
        {
            _cancelRequests();
            Fl::remove_timeout((Fl_Timeout_Handler)timerEvent_cb, this);
        }

        void ThumbnailPanel::clearCache()
        {
            _cancelRequests();
            thumbnails.clear();
        }

        void ThumbnailPanel::timerEvent_cb(void* d)
        {
            ThumbnailPanel* o = static_cast<ThumbnailPanel*>(d);
            o->timerEvent();
        }

        void ThumbnailPanel::timerEvent()
        {
            _tickEvent();

            Fl::repeat_timeout(
                kTimeout, (Fl_Timeout_Handler)timerEvent_cb, this);
        }

        void ThumbnailPanel::_updateThumbnail(
            Fl_Widget* widget, const std::shared_ptr<image::Image>& image)
        {
            if (!image || !image->isValid())
            {
                widget->image(nullptr);
                widget->redraw();
                return;
            }
            const auto W = image->getWidth();
            const auto H = image->getHeight();
            const auto data = image->getData();
            const auto bytes = image->getDataByteCount();
            const int depth = bytes / W / H;

            // flipped is cleaned by FLTK (as we set it as bind_image not image).
            uint8_t* flipped = new uint8_t[ W * H * depth ];
            flipImageInY(flipped, data, W, H, depth);
            const auto rgbImage = new Fl_RGB_Image(flipped, W, H, depth);
            widget->bind_image(rgbImage);
            widget->redraw();
        }
        
        void ThumbnailPanel::_thumbnailEvent()
        {
            // Check if the I/O information is finished.
            if (infoRequest.future.valid() &&
                infoRequest.future.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
            {
                ioInfo = std::make_shared<io::Info>(infoRequest.future.get());
            }

            // Check if any thumbnails are finished.
            auto i = thumbnailRequests.begin();
            while (i != thumbnailRequests.end())
            {
                if (i->second.future.valid() &&
                    i->second.future.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
                {
                    const auto& data = callbackData[i->second.id];
                    auto widget  = data->widget;
                    const auto& path = data->path;
                    const auto image = i->second.future.get();
                    const auto time = i->second.time;

                    const auto W = image->getWidth();
                    const auto H = image->getHeight();
                    const auto bytes = image->getDataByteCount();
                    const int depth = bytes / W / H;
                    bool bound = false;
                    const auto& key = io::getCacheKey(path, time, ioOptions);
                    const auto& t = thumbnails.find(key);
                    if (t == thumbnails.end())
                    {
                        thumbnails[io::getCacheKey(path, time, ioOptions)] = image;
                    }
                    _updateThumbnail(widget, image);
                    
                    i = thumbnailRequests.erase(i);
                }
                else
                {
                    ++i;
                }
            }
        }

        void ThumbnailPanel::_tickEvent()
        {
            _thumbnailEvent();
        }

        void ThumbnailPanel::_createThumbnail(
            Fl_Widget* widget, const file::Path& path,
            const otime::RationalTime& currentTime, const int layerId,
            const int height, const bool isNDI)
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
                // We make a copy of the NDI image to not leak memory nor
                // de-allocate it several times on program exit
                widget->bind_image(NDIimage->copy());
                return;
            }

            if (auto context = App::app->getContext())
            {
                try
                {
                    const auto& timeline =
                        timeline::Timeline::create(path, context);
                    const auto& timeRange = timeline->getTimeRange();

                    auto time = currentTime;
                    if (time::isValid(timeRange))
                    {
                        const auto& startTime = timeRange.start_time();
                        const auto& endTime = timeRange.end_time_inclusive();
                    
                        if (time < startTime)
                            time = startTime;
                        else if (time > endTime)
                            time = endTime;
                    }
                
                    if (!ioInfo && !infoRequest.future.valid())
                    {
                        infoRequest = thumbnailGenerator->getInfo(path, memoryRead);
                    }
                
                    ioOptions["OpenEXR/IgnoreDisplayWindow"] =
                        string::Format("{0}").arg(
                            App::ui->uiView->getIgnoreDisplayWindow());
                    ioOptions["Layer"] =
                        string::Format("{0}").arg(layerId);
                    // @todo: ioOptions["USD/cameraName"] = clipName;
                    const auto& key = io::getCacheKey(path, time, ioOptions);
                    const auto i = thumbnails.find(key);
                    if (i != thumbnails.end())
                    {
                        _updateThumbnail(widget, i->second);
                    }
                    else // if (ioInfo && !ioInfo->video.empty())
                    {
                        const auto k = thumbnailRequests.find(time);
                        if (k == thumbnailRequests.end())
                        {
                            thumbnailRequests[time] = thumbnailGenerator->getThumbnail(
                                path, memoryRead, height, time, ioOptions);

                            const int64_t id = thumbnailRequests[time].id;
                            auto data = std::make_shared<CallbackData>();
                            data->widget = widget;
                            data->path   = path;
                            callbackData[id] = data;
                        }
                    }
                }
                catch(const std::exception& e)
                {
                    // ... no printing of errors to not saturate log.
                }
            }
        }
    
        void ThumbnailPanel::_cancelRequests()
        {
            std::vector<uint64_t> ids;
            if (infoRequest.future.valid())
            {
                ids.push_back(infoRequest.id);
                infoRequest = ui::InfoRequest();
            }
            for (const auto& i : thumbnailRequests)
            {
                ids.push_back(i.second.id);
            }
            thumbnailRequests.clear();
            thumbnailGenerator->cancelRequests(ids);

            callbackData.clear();
        }

    } // namespace panel
    
} // namespace mrv
