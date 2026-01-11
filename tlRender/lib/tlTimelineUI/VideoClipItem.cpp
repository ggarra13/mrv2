// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include "VideoClipItem.h"
#include "ThumbnailSystem.h"

#include <tlUI/DrawUtil.h>

#ifdef VULKAN_BACKEND
#    include <tlTimelineVk/Render.h>
#endif

#include <tlTimeline/RenderUtil.h>
#include <tlTimeline/Util.h>

#include <tlIO/Cache.h>

#include <tlCore/Assert.h>
#include <tlCore/StringFormat.h>

namespace tl
{
    namespace TIMELINEUI
    {
        struct VideoClipItem::Private
        {
            std::string clipName;
            file::Path path;
            std::vector<file::MemoryRead> memoryRead;
            std::shared_ptr<ThumbnailGenerator> thumbnailGenerator;

            struct SizeData
            {
                bool sizeInit = true;
                int dragLength = 0;

                math::Box2i clipRect;
            };
            SizeData size;

            io::Options ioOptions;
            TIMELINEUI::InfoRequest infoRequest;
            std::shared_ptr<io::Info> ioInfo;
            std::map<otime::RationalTime, ThumbnailRequest>
                thumbnailRequests;
        };

        void VideoClipItem::_init(
            const otio::SerializableObject::Retainer<otio::Clip>& clip,
            double scale, const ItemOptions& options,
            const DisplayOptions& displayOptions,
            const std::shared_ptr<ItemData>& itemData,
            const std::shared_ptr<ThumbnailGenerator> thumbnailGenerator,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            const auto path = timeline::getPath(
                clip->media_reference(), itemData->directory,
                itemData->options.pathOptions);
            IBasicItem::_init(
                !clip->name().empty() ? clip->name()
                                      : path.getFileName(false),
                ui::ColorRole::VideoClip, "tl::TIMELINEUI::VideoClipItem",
                clip.value, scale, options, displayOptions, itemData, context,
                parent);
            
            TLRENDER_P();

            p.clipName = clip->name();
            p.path = path;
            p.memoryRead = timeline::getMemoryRead(clip->media_reference());
            p.thumbnailGenerator = thumbnailGenerator;

            p.ioOptions = _data->options.ioOptions;
            p.ioOptions["USD/cameraName"] = p.clipName;
            const std::string infoCacheKey =
                io::getInfoCacheKey(path, p.ioOptions);
            const auto i = itemData->info.find(infoCacheKey);
            if (i != itemData->info.end())
            {
                p.ioInfo = i->second;
            }
        }

        VideoClipItem::VideoClipItem() :
            _p(new Private)
        {
        }

        VideoClipItem::~VideoClipItem()
        {
            TLRENDER_P();
            _cancelRequests();
        }

        std::shared_ptr<VideoClipItem> VideoClipItem::create(
            const otio::SerializableObject::Retainer<otio::Clip>& clip,
            double scale, const ItemOptions& options,
            const DisplayOptions& displayOptions,
            const std::shared_ptr<ItemData>& itemData,
            const std::shared_ptr<TIMELINEUI::ThumbnailGenerator> thumbnailGenerator,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<VideoClipItem>(new VideoClipItem);
            out->_init(
                clip, scale, options, displayOptions, itemData,
                thumbnailGenerator, context, parent);
            return out;
        }

        void VideoClipItem::setScale(double value)
        {
            const bool changed = value != _scale;
            IBasicItem::setScale(value);
            TLRENDER_P();
            if (changed)
            {
                _cancelRequests();
                _updates |= ui::Update::Draw;
            }
        }

        void VideoClipItem::setDisplayOptions(const DisplayOptions& value)
        {
            const bool thumbnailsChanged =
                value.thumbnails != _displayOptions.thumbnails ||
                value.thumbnailHeight != _displayOptions.thumbnailHeight;
            IBasicItem::setDisplayOptions(value);
            TLRENDER_P();
            if (thumbnailsChanged)
            {
                _cancelRequests();
                _updates |= ui::Update::Draw;
            }
        }

        void VideoClipItem::tickEvent(
            bool parentsVisible, bool parentsEnabled,
            const ui::TickEvent& event)
        {
            IWidget::tickEvent(parentsVisible, parentsEnabled, event);
            TLRENDER_P();

            // Check if the I/O information is finished.
            if (p.infoRequest.future.valid() &&
                p.infoRequest.future.wait_for(std::chrono::seconds(0)) ==
                    std::future_status::ready)
            {
                p.ioInfo =
                    std::make_shared<io::Info>(p.infoRequest.future.get());
                const std::string infoCacheKey =
                    io::getInfoCacheKey(p.path, p.ioOptions);
                _data->info[infoCacheKey] = p.ioInfo;
                _updates |= ui::Update::Size;
                _updates |= ui::Update::Draw;
            }

            // Check if any thumbnails are finished.
            auto i = p.thumbnailRequests.begin();
            while (i != p.thumbnailRequests.end())
            {
                if (i->second.future.valid() &&
                    i->second.future.wait_for(std::chrono::seconds(0)) ==
                        std::future_status::ready)
                {
                    const auto image = i->second.future.get();
                    const std::string cacheKey =
                        io::getVideoCacheKey(p.path, i->first, p.ioOptions, {});
                    _data->thumbnails[cacheKey] = image;
                    i = p.thumbnailRequests.erase(i);
                    _updates |= ui::Update::Draw;
                }
                else
                {
                    ++i;
                }
            }
        }

        void VideoClipItem::sizeHintEvent(const ui::SizeHintEvent& event)
        {
            const bool displayScaleChanged =
                event.displayScale != _displayScale;
            IBasicItem::sizeHintEvent(event);
            TLRENDER_P();

            if (displayScaleChanged || p.size.sizeInit)
            {
                p.size.dragLength = event.style->getSizeRole(
                    ui::SizeRole::DragLength, _displayScale);
            }
            p.size.sizeInit = false;

            if (_displayOptions.thumbnails)
            {
                _sizeHint.h += _displayOptions.thumbnailHeight;
            }
        }

        void VideoClipItem::clipEvent(const math::Box2i& clipRect, bool clipped)
        {
            IBasicItem::clipEvent(clipRect, clipped);
            TLRENDER_P();
            if (clipRect == p.size.clipRect)
                return;
            p.size.clipRect = clipRect;
            if (clipped)
            {
                _cancelRequests();
                _updates |= ui::Update::Draw;
            }
        }

        void VideoClipItem::drawEvent(
            const math::Box2i& drawRect, const ui::DrawEvent& event)
        {
            IBasicItem::drawEvent(drawRect, event);
            
            if (_displayOptions.thumbnails)
            {
                _drawThumbnails(drawRect, event);
            }
        }

        void VideoClipItem::_drawThumbnails(
            const math::Box2i& drawRect, const ui::DrawEvent& event)
        {
            TLRENDER_P();

            const math::Box2i g = _getInsideGeometry();
            const int m = _getMargin();
            const int lineHeight = _getLineHeight();

            const math::Box2i box(
                g.min.x,
                g.min.y + (_displayOptions.clipInfo ? (lineHeight + m * 2) : 0),
                g.w(), _displayOptions.thumbnailHeight);
            event.render->drawRect(box, image::Color4f(0.F, 0.F, 0.F));
            const timeline::ClipRectEnabledState clipRectEnabledState(
                event.render);
            const timeline::ClipRectState clipRectState(event.render);
            event.render->setClipRectEnabled(true);
            event.render->setClipRect(
                box.intersect(clipRectState.getClipRect()));
            event.render->setOCIOOptions(_displayOptions.ocio);
            event.render->setLUTOptions(_displayOptions.lut);
            event.render->setHDROptions(_displayOptions.hdr);

            const math::Box2i clipRect =
                _getClipRect(drawRect, _displayOptions.clipRectScale);
            if (g.intersects(clipRect))
            {
                if (!p.ioInfo && !p.infoRequest.future.valid())
                {
                    p.infoRequest = p.thumbnailGenerator->getInfo(
                        p.path, p.memoryRead, p.ioOptions);
                }
            }
 
            const int thumbnailWidth =
                (_displayOptions.thumbnails && p.ioInfo &&
                 !p.ioInfo->video.empty())
                    ? static_cast<int>(
                          _displayOptions.thumbnailHeight *
                          p.ioInfo->video[0].size.getAspect())
                    : 0;
            if (thumbnailWidth > 0)
            {
                timeline::BackgroundOptions background;
                background.type = timeline::Background::Transparent;
                const int w = g.w();
                for (int x = 0; x < w; x += thumbnailWidth)
                {
                    const math::Box2i box(
                        g.min.x + x,
                        g.min.y + (_displayOptions.clipInfo
                                       ? (lineHeight + m * 2)
                                       : 0),
                        thumbnailWidth, _displayOptions.thumbnailHeight);
                    if (box.intersects(clipRect))
                    {
                        const otime::RationalTime time =
                            otime::RationalTime(
                                _timeRange.start_time().value() +
                                    (w > 1 ? (x / static_cast<double>(w - 1))
                                           : 0) *
                                        _timeRange.duration().value(),
                                _timeRange.duration().rate())
                                .floor();
                        const otime::RationalTime mediaTime = 
                            timeline::toVideoMediaTime(
                                time, _timeRange, _trimmedRange,
                                p.ioInfo->videoTime.duration().rate());

                        const std::string cacheKey = io::getVideoCacheKey(
                            p.path, mediaTime, p.ioOptions, {});
                        const auto i = _data->thumbnails.find(cacheKey);
                        if (i != _data->thumbnails.end())
                        {
                            if (i->second)
                            {
                                
                                timeline::VideoData videoData;
                                videoData.size = i->second->getSize();
                                videoData.layers.push_back({i->second});

#ifdef VULKAN_BACKEND
                                event.render->endRenderPass();
#endif

                                const timeline::VideoLayer& layer = videoData.layers[0];

                                if ((!_displayOptions.ocio.enabled ||
                                     _displayOptions.ocio == timeline::OCIOOptions()) &&
                                    _displayOptions.lut == timeline::LUTOptions() &&
                                    _displayOptions.hdr == timeline::HDROptions())
                                {
                                    if (!layer.imageB && layer.image)
                                    {
                                        event.render->drawImage(layer.image, box);
                                    }
                                    else if (!layer.image && layer.imageB)
                                    {
                                        event.render->drawImage(layer.imageB, box);
                                    }
                                    else
                                    {
                                        event.render->drawVideo({videoData}, {box},
                                                                {}, {}, {},
                                                                background);
                                    }
                                }
                                else
                                {
                                    event.render->drawVideo({videoData}, {box},
                                                            {}, {}, {},
                                                            background);
                                }
#ifdef VULKAN_BACKEND
                                event.render->beginLoadRenderPass();
#endif
                            }
                        }
                        else if (p.ioInfo && !p.ioInfo->video.empty())
                        {
                            const auto k = p.thumbnailRequests.find(mediaTime);
                            if (k == p.thumbnailRequests.end())
                            {
                                p.thumbnailRequests[mediaTime] =
                                    p.thumbnailGenerator->getThumbnail(
                                        p.path, p.memoryRead,
                                        _displayOptions.thumbnailHeight,
                                        mediaTime, p.ioOptions);
                            }
                        }
                    }
                }
            }
        }

        void VideoClipItem::_cancelRequests()
        {
            TLRENDER_P();
            std::vector<uint64_t> ids;
            if (p.infoRequest.future.valid())
            {
                ids.push_back(p.infoRequest.id);
                p.infoRequest = TIMELINEUI::InfoRequest();
            }
            for (const auto& i : p.thumbnailRequests)
            {
                ids.push_back(i.second.id);
            }
            p.thumbnailRequests.clear();
            p.thumbnailGenerator->cancelRequests(ids);
        }
    } // namespace TIMELINEUI
} // namespace tl
