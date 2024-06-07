// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <FL/Fl_Widget.H>
#include <FL/Fl.H>

#include "mrvGL/mrvThumbnailCreator.h"

#include "mrvPanels/mrvThumbnailPanel.h"

#include "mrViewer.h"

namespace mrv
{

    namespace panel
    {

        ThumbnailPanel::ThumbnailPanel(ViewerUI* ui) :
            PanelWidget(ui)
        {
            TLRENDER_P();

            thumbnailCreator = p.ui->uiTimeline->thumbnailCreator();
        }

        ThumbnailPanel::~ThumbnailPanel()
        {
            _cancelRequests();
        }

        void ThumbnailPanel::updateThumbnail_cb(
            const int64_t id,
            const std::vector< std::pair<otime::RationalTime, Fl_RGB_Image*> >&
                thumbnails,
            void* opaque)
        {
            ThumbnailData* data = static_cast< ThumbnailData* >(opaque);
            Fl_Widget* w = data->widget;
            ThumbnailPanel* panel = data->panel;
            panel->updateThumbnail(id, thumbnails, w);
            delete data;
        }

        void ThumbnailPanel::updateThumbnail(
            const int64_t id,
            const std::vector< std::pair<otime::RationalTime, Fl_RGB_Image*> >&
                thumbnails,
            Fl_Widget* w)
        {
            auto it = ids.find(w);
            if (it == ids.end())
                return;

            if (it->second == id)
            {
                for (const auto& i : thumbnails)
                {
                    w->bind_image(i.second);
                }
                w->redraw();
            }
            else
            {
                for (const auto& i : thumbnails)
                {
                    delete i.second;
                }
            }
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

                ThumbnailData* data = new ThumbnailData;
                data->widget = widget;
                data->panel = this;
            
                auto it = ids.find(widget);
                if (it != ids.end())
                {
                    thumbnailCreator->cancelRequests(it->second);
                    ids.erase(it);
                }
            
                if (_clearCache)
                    thumbnailCreator->clearCache();
                
                thumbnailCreator->initThread();
                
                int64_t id = thumbnailCreator->request(
                    path.get(), time, size, updateThumbnail_cb, (void*)data,
                    layerId);
                ids[widget] = id;
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
            for (const auto& it : ids)
            {
                thumbnailCreator->cancelRequests(it.second);
            }
            ids.clear();
        }

    } // namespace panel

} // namespace mrv
