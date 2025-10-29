// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include "mrViewer.h"

#include "mrvPanels/mrvThumbnailPanel.h"

#include "mrvIcons/NDI.h"

#include "mrvCore/mrvFile.h"
#include "mrvCore/mrvWait.h"

#include <tlCore/StringFormat.h>

#include <FL/Fl_Widget.H>
#include <FL/Fl.H>

#ifdef MRV2_PYBIND11
#    include <pybind11/embed.h>
namespace py = pybind11;
#endif

namespace
{
    const float kTimeout = 0.01;
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

        void ThumbnailPanel::timerEvent_cb(ThumbnailPanel* panel)
        {
            panel->timerEvent();
        }

        void ThumbnailPanel::timerEvent()
        {
            auto i = thumbnailRequests.begin();
            while (i != thumbnailRequests.end())
            {
                if (i->second.future.valid() &&
                    i->second.future.wait_for(std::chrono::seconds(0)) ==
                        std::future_status::ready)
                {
                    if (auto image = i->second.future.get())
                    {
                        if (image::PixelType::RGBA_U8 == image->getPixelType())
                        {
                            const int w = image->getWidth();
                            const int h = image->getHeight();
                            const int depth = 4;

                            uint8_t* pixelData = new uint8_t[w * h * depth];

                            auto rgbImage =
                                new Fl_RGB_Image(pixelData, w, h, depth);
                            rgbImage->alloc_array = true;
                            uint8_t* d = pixelData;
                            const uint8_t* s = image->getData();

#ifdef OPENGL_BACKEND
                            for (int y = 0; y < h; ++y)
                            {
                                std::memcpy(
                                    d + (h - 1 - y) * w * 4, s + y * w * 4,
                                    w * 4);
                            }
#endif

#ifdef VULKAN_BACKEND
                            std::memcpy(d, s, w * h * depth);
#endif
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
            Fl::repeat_timeout(
                kTimeout, (Fl_Timeout_Handler)timerEvent_cb, this);
        }
        
        void ThumbnailPanel::_createThumbnail(
            Fl_Widget* widget, const file::Path& path,
            const otime::RationalTime& currentTime, const int layerId)
        {
            TLRENDER_P();

            static Fl_SVG_Image* NDIimage = MRV2_LOAD_SVG(NDI);

            if (!p.ui->uiPrefs->uiPrefsPanelThumbnails->value())
            {
                widget->bind_image(nullptr);
                return;
            }

            if (file::isTemporaryNDI(path))
            {
                widget->bind_image(NDIimage->copy());
                return;
            }

            try
            {
                const auto context = App::app->getContext();
#ifdef OPENGL_BACKEND
                auto thumbnailSystem =
                    context->getSystem<timelineui::ThumbnailSystem>();
#endif
#ifdef VULKAN_BACKEND
                auto thumbnailSystem =
                    context->getSystem<timelineui_vk::ThumbnailSystem>();
#endif

#ifdef MRV2_PYBIND11
                py::gil_scoped_release release;
#endif
                const auto& timeline =
                    timeline::Timeline::create(path, context);
                const auto& timeRange = timeline->getTimeRange();

                auto time = currentTime;

                if (file::isMovie(path))
                {
                    double start = p.ui->uiPrefs->uiStartTimeOffset->value();
                    time -= otime::RationalTime(start, time.rate());
                }

                if (time::isValid(timeRange))
                {
                    auto startTime = timeRange.start_time();
                    auto endTime = timeRange.end_time_inclusive();

                    if (time < startTime)
                        time = startTime;
                    else if (time > endTime)
                        time = endTime;
                }

                auto it = thumbnailRequests.find(widget);
                if (it != thumbnailRequests.end())
                {
                    const auto& request = it->second;
                    thumbnailSystem->cancelRequests({request.id});
                    thumbnailRequests.erase(it);
                }

                io::Options options;
                if (_clearCache)
                {
                    options["ClearCache"] = string::Format("{0}").arg(rand());
                    _clearCache = false;
                }

                options["Layer"] = string::Format("{0}").arg(layerId);
                
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
#ifdef OPENGL_BACKEND
            auto thumbnailSystem = context->getSystem<timelineui::ThumbnailSystem>();
#endif

#ifdef VULKAN_BACKEND
            auto thumbnailSystem = context->getSystem<timelineui_vk::ThumbnailSystem>();
#endif

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
