// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// Copyright (c) 2025-Present Gonzalo Garramuño
// All rights reserved.

#include <tlUI/ThumbnailSystem.h>

#include <tlTimelineVk/Render.h>

#include <tlTimeline/Timeline.h>

#include <tlIO/System.h>

#include <tlVk/Vk.h>
#include <tlVk/OffscreenBuffer.h>

#include <tlCore/AudioResample.h>
#include <tlCore/LRUCache.h>
#include <tlCore/StringFormat.h>

#include <sstream>

namespace tl
{
    namespace timeline_vk
    {
        namespace
        {
            const size_t ioCacheMax = 16;
        }

        struct ThumbnailCache::Private
        {
            size_t max = 1000;
            memory::LRUCache<std::string, io::Info> info;
            memory::LRUCache<std::string, std::shared_ptr<image::Image> >
                thumbnails;
            memory::LRUCache<std::string, std::shared_ptr<geom::TriangleMesh2> >
                waveforms;
            std::mutex mutex;
        };

        void
        ThumbnailCache::_init(const std::shared_ptr<system::Context>& context)
        {
            _maxUpdate();
        }

        ThumbnailCache::ThumbnailCache() :
            _p(new Private)
        {
        }

        ThumbnailCache::~ThumbnailCache() {}

        std::shared_ptr<ThumbnailCache>
        ThumbnailCache::create(const std::shared_ptr<system::Context>& context)
        {
            auto out = std::shared_ptr<ThumbnailCache>(new ThumbnailCache);
            out->_init(context);
            return out;
        }

        size_t ThumbnailCache::getMax() const
        {
            return _p->max;
        }

        void ThumbnailCache::setMax(size_t value)
        {
            TLRENDER_P();
            if (value == p.max)
                return;
            p.max = value;
            _maxUpdate();
        }

        size_t ThumbnailCache::getSize() const
        {
            TLRENDER_P();
            std::unique_lock<std::mutex> lock(p.mutex);
            return p.info.getSize() + p.thumbnails.getSize() +
                   p.waveforms.getSize();
        }

        float ThumbnailCache::getPercentage() const
        {
            TLRENDER_P();
            std::unique_lock<std::mutex> lock(p.mutex);
            return (p.info.getSize() + p.thumbnails.getSize() +
                    p.waveforms.getSize()) /
                   static_cast<float>(
                       p.info.getMax() + p.thumbnails.getMax() +
                       p.waveforms.getMax()) *
                   100.F;
        }

        std::string ThumbnailCache::getInfoKey(
            const file::Path& path, const io::Options& options)
        {
            std::vector<std::string> s;
            s.push_back(path.get());
            for (const auto& i : options)
            {
                s.push_back(
                    string::Format("{0}:{1}").arg(i.first).arg(i.second));
            }
            return string::join(s, ';');
        }

        void
        ThumbnailCache::addInfo(const std::string& key, const io::Info& info)
        {
            TLRENDER_P();
            std::unique_lock<std::mutex> lock(p.mutex);
            p.info.add(key, info);
        }

        bool ThumbnailCache::containsInfo(const std::string& key)
        {
            TLRENDER_P();
            std::unique_lock<std::mutex> lock(p.mutex);
            return p.info.contains(key);
        }

        bool
        ThumbnailCache::getInfo(const std::string& key, io::Info& info) const
        {
            TLRENDER_P();
            std::unique_lock<std::mutex> lock(p.mutex);
            return p.info.get(key, info);
        }

        std::string ThumbnailCache::getThumbnailKey(
            int height, const file::Path& path, const otime::RationalTime& time,
            const io::Options& options)
        {
            std::vector<std::string> s;
            s.push_back(string::Format("{0}").arg(height));
            s.push_back(path.get());
            s.push_back(string::Format("{0}").arg(time));
            for (const auto& i : options)
            {
                s.push_back(
                    string::Format("{0}:{1}").arg(i.first).arg(i.second));
            }
            return string::join(s, ';');
        }

        void ThumbnailCache::addThumbnail(
            const std::string& key,
            const std::shared_ptr<image::Image>& thumbnail)
        {
            TLRENDER_P();
            std::unique_lock<std::mutex> lock(p.mutex);
            p.thumbnails.add(key, thumbnail);
        }

        bool ThumbnailCache::containsThumbnail(const std::string& key)
        {
            TLRENDER_P();
            std::unique_lock<std::mutex> lock(p.mutex);
            return p.thumbnails.contains(key);
        }

        bool ThumbnailCache::getThumbnail(
            const std::string& key,
            std::shared_ptr<image::Image>& thumbnail) const
        {
            TLRENDER_P();
            std::unique_lock<std::mutex> lock(p.mutex);
            return p.thumbnails.get(key, thumbnail);
        }

        std::string ThumbnailCache::getWaveformKey(
            const math::Size2i& size, const file::Path& path,
            const otime::TimeRange& timeRange, const io::Options& options)
        {
            std::vector<std::string> s;
            s.push_back(string::Format("{0}").arg(size));
            s.push_back(path.get());
            s.push_back(string::Format("{0}").arg(timeRange));
            for (const auto& i : options)
            {
                s.push_back(
                    string::Format("{0}:{1}").arg(i.first).arg(i.second));
            }
            return string::join(s, ';');
        }

        void ThumbnailCache::addWaveform(
            const std::string& key,
            const std::shared_ptr<geom::TriangleMesh2>& waveform)
        {
            TLRENDER_P();
            std::unique_lock<std::mutex> lock(p.mutex);
            p.waveforms.add(key, waveform);
        }

        bool ThumbnailCache::containsWaveform(const std::string& key)
        {
            TLRENDER_P();
            std::unique_lock<std::mutex> lock(p.mutex);
            return p.waveforms.contains(key);
        }

        bool ThumbnailCache::getWaveform(
            const std::string& key,
            std::shared_ptr<geom::TriangleMesh2>& waveform) const
        {
            TLRENDER_P();
            std::unique_lock<std::mutex> lock(p.mutex);
            return p.waveforms.get(key, waveform);
        }

        void ThumbnailCache::_maxUpdate()
        {
            TLRENDER_P();
            std::unique_lock<std::mutex> lock(p.mutex);
            p.info.setMax(p.max);
            p.thumbnails.setMax(p.max);
            p.waveforms.setMax(p.max);
        }

        struct ThumbnailGenerator::Private
        {
            std::weak_ptr<system::Context> context;
            std::shared_ptr<ThumbnailCache> cache;
            uint64_t requestId = 0;

            struct InfoRequest
            {
                uint64_t id = 0;
                file::Path path;
                std::vector<file::MemoryRead> memoryRead;
                io::Options options;
                std::promise<io::Info> promise;
            };

            struct ThumbnailRequest
            {
                uint64_t id = 0;
                file::Path path;
                std::vector<file::MemoryRead> memoryRead;
                int height = 0;
                otime::RationalTime time = time::invalidTime;
                io::Options options;
                std::promise<std::shared_ptr<image::Image> > promise;
            };

            struct WaveformRequest
            {
                uint64_t id = 0;
                file::Path path;
                std::vector<file::MemoryRead> memoryRead;
                math::Size2i size;
                otime::TimeRange timeRange = time::invalidTimeRange;
                io::Options options;
                std::promise<std::shared_ptr<geom::TriangleMesh2> > promise;
            };

            struct InfoMutex
            {
                std::list<std::shared_ptr<InfoRequest> > requests;
                bool stopped = false;
                std::mutex mutex;
            };
            InfoMutex infoMutex;

            struct ThumbnailMutex
            {
                std::list<std::shared_ptr<ThumbnailRequest> > requests;
                bool stopped = false;
                std::mutex mutex;
            };
            ThumbnailMutex thumbnailMutex;

            struct WaveformMutex
            {
                std::list<std::shared_ptr<WaveformRequest> > requests;
                bool stopped = false;
                std::mutex mutex;
            };
            WaveformMutex waveformMutex;

            struct InfoThread
            {
                std::condition_variable cv;
                std::thread thread;
                std::atomic<bool> running;
            };
            InfoThread infoThread;

            struct ThumbnailThread
            {
                std::shared_ptr<timeline_vk::Render> render;
                std::shared_ptr<vlk::OffscreenBuffer> buffer;
                memory::LRUCache<std::string, std::shared_ptr<io::IRead> >
                    ioCache;
                std::condition_variable cv;
                std::thread thread;
                std::atomic<bool> running;
            };
            ThumbnailThread thumbnailThread;

            struct WaveformThread
            {
                memory::LRUCache<std::string, std::shared_ptr<io::IRead> >
                    ioCache;
                std::condition_variable cv;
                std::thread thread;
                std::atomic<bool> running;
            };
            WaveformThread waveformThread;
        };

        void ThumbnailGenerator::_init(
            const std::shared_ptr<ThumbnailCache>& cache,
            const std::shared_ptr<system::Context>& context)
        {
            TLRENDER_P();

            p.context = context;

            p.cache = cache;

            p.infoThread.running = true;
            p.infoThread.thread = std::thread(
                [this]
                {
                    TLRENDER_P();
                    while (p.infoThread.running)
                    {
                        _infoRun();
                    }
                    {
                        std::unique_lock<std::mutex> lock(p.infoMutex.mutex);
                        p.infoMutex.stopped = true;
                    }
                    _infoCancel();
                });

            p.thumbnailThread.ioCache.setMax(ioCacheMax);
            p.thumbnailThread.running = true;
            p.thumbnailThread.thread = std::thread(
                [this]
                {
                    TLRENDER_P();
                    // p.window->makeCurrent();
                    if (auto context = p.context.lock())
                    {
                        p.thumbnailThread.render =
                            timeline_vk::Render::create(context);
                    }
                    while (p.thumbnailThread.running)
                    {
                        _thumbnailRun();
                    }
                    {
                        std::unique_lock<std::mutex> lock(
                            p.thumbnailMutex.mutex);
                        p.thumbnailMutex.stopped = true;
                    }
                    p.thumbnailThread.buffer.reset();
                    p.thumbnailThread.render.reset();
                    //p.window->doneCurrent();
                    _thumbnailCancel();
                });

            p.waveformThread.ioCache.setMax(ioCacheMax);
            p.waveformThread.running = true;
            p.waveformThread.thread = std::thread(
                [this]
                {
                    TLRENDER_P();
                    while (p.waveformThread.running)
                    {
                        _waveformRun();
                    }
                    {
                        std::unique_lock<std::mutex> lock(
                            p.waveformMutex.mutex);
                        p.waveformMutex.stopped = true;
                    }
                    _waveformCancel();
                });
        }

        ThumbnailGenerator::ThumbnailGenerator() :
            _p(new Private)
        {
        }

        ThumbnailGenerator::~ThumbnailGenerator()
        {
            TLRENDER_P();
            p.infoThread.running = false;
            if (p.infoThread.thread.joinable())
            {
                p.infoThread.thread.join();
            }
            p.thumbnailThread.running = false;
            if (p.thumbnailThread.thread.joinable())
            {
                p.thumbnailThread.thread.join();
            }
            p.waveformThread.running = false;
            if (p.waveformThread.thread.joinable())
            {
                p.waveformThread.thread.join();
            }
        }

        std::shared_ptr<ThumbnailGenerator> ThumbnailGenerator::create(
            const std::shared_ptr<ThumbnailCache>& cache,
            const std::shared_ptr<system::Context>& context)
        {
            auto out =
                std::shared_ptr<ThumbnailGenerator>(new ThumbnailGenerator);
            out->_init(cache, context);
            return out;
        }

        InfoRequest ThumbnailGenerator::getInfo(
            const file::Path& path, const io::Options& options)
        {
            return getInfo(path, {}, options);
        }

        InfoRequest ThumbnailGenerator::getInfo(
            const file::Path& path,
            const std::vector<file::MemoryRead>& memoryRead,
            const io::Options& options)
        {
            TLRENDER_P();
            (p.requestId)++;
            auto request = std::make_shared<Private::InfoRequest>();
            request->id = p.requestId;
            request->path = path;
            request->memoryRead = memoryRead;
            request->options = options;
            InfoRequest out;
            out.id = p.requestId;
            out.future = request->promise.get_future();
            bool valid = false;
            {
                std::unique_lock<std::mutex> lock(p.infoMutex.mutex);
                if (!p.infoMutex.stopped)
                {
                    valid = true;
                    p.infoMutex.requests.push_back(request);
                }
            }
            if (valid)
            {
                p.infoThread.cv.notify_one();
            }
            else
            {
                request->promise.set_value(io::Info());
            }
            return out;
        }

        ThumbnailRequest ThumbnailGenerator::getThumbnail(
            const file::Path& path, int height, const otime::RationalTime& time,
            const io::Options& options)
        {
            return getThumbnail(path, {}, height, time, options);
        }

        ThumbnailRequest ThumbnailGenerator::getThumbnail(
            const file::Path& path,
            const std::vector<file::MemoryRead>& memoryRead, int height,
            const otime::RationalTime& time, const io::Options& options)
        {
            TLRENDER_P();
            (p.requestId)++;
            auto request = std::make_shared<Private::ThumbnailRequest>();
            request->id = p.requestId;
            request->path = path;
            request->memoryRead = memoryRead;
            request->height = height;
            request->time = time;
            request->options = options;
            ThumbnailRequest out;
            out.id = p.requestId;
            out.height = height;
            out.time = time;
            out.future = request->promise.get_future();
            bool valid = false;
            {
                std::unique_lock<std::mutex> lock(p.thumbnailMutex.mutex);
                if (!p.thumbnailMutex.stopped)
                {
                    valid = true;
                    p.thumbnailMutex.requests.push_back(request);
                }
            }
            if (valid)
            {
                p.thumbnailThread.cv.notify_one();
            }
            else
            {
                request->promise.set_value(nullptr);
            }
            return out;
        }

        WaveformRequest ThumbnailGenerator::getWaveform(
            const file::Path& path, const math::Size2i& size,
            const otime::TimeRange& range, const io::Options& options)
        {
            return getWaveform(path, {}, size, range, options);
        }

        WaveformRequest ThumbnailGenerator::getWaveform(
            const file::Path& path,
            const std::vector<file::MemoryRead>& memoryRead,
            const math::Size2i& size, const otime::TimeRange& timeRange,
            const io::Options& options)
        {
            TLRENDER_P();
            (p.requestId)++;
            auto request = std::make_shared<Private::WaveformRequest>();
            request->id = p.requestId;
            request->path = path;
            request->memoryRead = memoryRead;
            request->size = size;
            request->timeRange = timeRange;
            request->options = options;
            WaveformRequest out;
            out.id = p.requestId;
            out.size = size;
            out.timeRange = timeRange;
            out.future = request->promise.get_future();
            bool valid = false;
            {
                std::unique_lock<std::mutex> lock(p.waveformMutex.mutex);
                if (!p.waveformMutex.stopped)
                {
                    valid = true;
                    p.waveformMutex.requests.push_back(request);
                }
            }
            if (valid)
            {
                p.waveformThread.cv.notify_one();
            }
            else
            {
                request->promise.set_value(nullptr);
            }
            return out;
        }

        void
        ThumbnailGenerator::cancelRequests(const std::vector<uint64_t>& ids)
        {
            TLRENDER_P();
            {
                std::unique_lock<std::mutex> lock(p.infoMutex.mutex);
                auto i = p.infoMutex.requests.begin();
                while (i != p.infoMutex.requests.end())
                {
                    const auto j = std::find(ids.begin(), ids.end(), (*i)->id);
                    if (j != ids.end())
                    {
                        i = p.infoMutex.requests.erase(i);
                    }
                    else
                    {
                        ++i;
                    }
                }
            }
            {
                std::unique_lock<std::mutex> lock(p.thumbnailMutex.mutex);
                auto i = p.thumbnailMutex.requests.begin();
                while (i != p.thumbnailMutex.requests.end())
                {
                    const auto j = std::find(ids.begin(), ids.end(), (*i)->id);
                    if (j != ids.end())
                    {
                        i = p.thumbnailMutex.requests.erase(i);
                    }
                    else
                    {
                        ++i;
                    }
                }
            }
            {
                std::unique_lock<std::mutex> lock(p.waveformMutex.mutex);
                auto i = p.waveformMutex.requests.begin();
                while (i != p.waveformMutex.requests.end())
                {
                    const auto j = std::find(ids.begin(), ids.end(), (*i)->id);
                    if (j != ids.end())
                    {
                        i = p.waveformMutex.requests.erase(i);
                    }
                    else
                    {
                        ++i;
                    }
                }
            }
        }

        void ThumbnailGenerator::_infoRun()
        {
            TLRENDER_P();
            std::shared_ptr<Private::InfoRequest> request;
            {
                std::unique_lock<std::mutex> lock(p.infoMutex.mutex);
                if (p.infoThread.cv.wait_for(
                        lock, std::chrono::milliseconds(5),
                        [this] { return !_p->infoMutex.requests.empty(); }))
                {
                    request = p.infoMutex.requests.front();
                    p.infoMutex.requests.pop_front();
                }
            }
            if (request)
            {
                io::Info info;
                const std::string key =
                    ThumbnailCache::getInfoKey(request->path, request->options);
                if (!p.cache->getInfo(key, info))
                {
                    if (auto context = p.context.lock())
                    {
                        auto ioSystem = context->getSystem<io::System>();
                        try
                        {
                            const std::string& fileName = request->path.get();
                            // std::cout << "info request: " <<
                            // request->path.get() << std::endl;
                            std::shared_ptr<io::IRead> read = ioSystem->read(
                                request->path, request->memoryRead,
                                request->options);
                            if (read)
                            {
                                info = read->getInfo().get();
                            }
                        }
                        catch (const std::exception&)
                        {
                        }
                    }
                }
                request->promise.set_value(info);
                p.cache->addInfo(key, info);
            }
        }

        void ThumbnailGenerator::_thumbnailRun()
        {
            TLRENDER_P();
            std::shared_ptr<Private::ThumbnailRequest> request;
            {
                std::unique_lock<std::mutex> lock(p.thumbnailMutex.mutex);
                if (p.thumbnailThread.cv.wait_for(
                        lock, std::chrono::milliseconds(5), [this]
                        { return !_p->thumbnailMutex.requests.empty(); }))
                {
                    request = p.thumbnailMutex.requests.front();
                    p.thumbnailMutex.requests.pop_front();
                }
            }
            if (request)
            {
                std::shared_ptr<image::Image> image;
                const std::string key = ThumbnailCache::getThumbnailKey(
                    request->height, request->path, request->time,
                    request->options);
                if (!p.cache->getThumbnail(key, image))
                {
                    if (auto context = p.context.lock())
                    {
                        auto ioSystem = context->getSystem<io::System>();
                        try
                        {
                            const std::string& fileName = request->path.get();
                            // std::cout << "thumbnail request: " << fileName <<
                            // " " <<
                            //     request->time << std::endl;
                            std::shared_ptr<io::IRead> read;
                            if (!p.thumbnailThread.ioCache.get(fileName, read))
                            {
                                auto ioSystem =
                                    context->getSystem<io::System>();
                                read = ioSystem->read(
                                    request->path, request->memoryRead,
                                    request->options);
                                p.thumbnailThread.ioCache.add(fileName, read);
                            }
                            if (read)
                            {
                                const io::Info info = read->getInfo().get();
                                math::Size2i size;
                                if (!info.video.empty())
                                {
                                    size.w = request->height *
                                             info.video[0].size.getAspect();
                                    size.h = request->height;
                                }
                                vlk::OffscreenBufferOptions options;
                                options.colorType = image::PixelType::RGBA_U8;
                                if (vlk::doCreate(
                                        p.thumbnailThread.buffer, size,
                                        options))
                                {
                                    p.thumbnailThread.buffer =
                                        vlk::OffscreenBuffer::create(
                                            size, options);
                                }
                                const otime::RationalTime time =
                                    request->time != time::invalidTime
                                        ? request->time
                                        : info.videoTime.start_time();
                                const auto videoData =
                                    read->readVideo(time, request->options)
                                        .get();
                                if (p.thumbnailThread.render &&
                                    p.thumbnailThread.buffer && videoData.image)
                                {
                                    p.thumbnailThread.render->begin(size);
                                    p.thumbnailThread.render->drawImage(
                                        videoData.image,
                                        {math::Box2i(0, 0, size.w, size.h)});
                                    p.thumbnailThread.render->end();
                                    image = image::Image::create(
                                        size.w, size.h,
                                        image::PixelType::RGBA_U8);
                                    glPixelStorei(GL_PACK_ALIGNMENT, 1);
                                    glReadPixels(
                                        0, 0, size.w, size.h, GL_RGBA,
                                        GL_UNSIGNED_BYTE, image->getData());
                                }
                            }
                            else if (
                                string::compare(
                                    ".otio", request->path.getExtension(),
                                    string::Compare::CaseInsensitive) ||
                                string::compare(
                                    ".otioz", request->path.getExtension(),
                                    string::Compare::CaseInsensitive))
                            {
                                otime::RationalTime offsetTime =
                                    time::invalidTime;
                                timeline::Options timelineOptions;
                                timelineOptions.ioOptions = request->options;
                                auto timeline = timeline::Timeline::create(
                                    request->path, context, offsetTime,
                                    timelineOptions);
                                const auto info = timeline->getIOInfo();
                                // const auto videoData = timeline->getVideo(
                                //     timeline->getTimeRange().start_time()).future.get();
                                const auto videoData =
                                    timeline->getVideo(request->time)
                                        .future.get();
                                math::Size2i size;
                                if (!info.video.empty())
                                {
                                    size.w =
                                        request->height *
                                        info.video.front().size.getAspect();
                                    size.h = request->height;
                                }
                                if (size.isValid())
                                {
                                    vlk::OffscreenBufferOptions options;
                                    options.colorType =
                                        image::PixelType::RGBA_U8;
                                    if (vlk::doCreate(
                                            p.thumbnailThread.buffer, size,
                                            options))
                                    {
                                        p.thumbnailThread.buffer =
                                            vlk::OffscreenBuffer::create(
                                                size, options);
                                    }
                                    if (p.thumbnailThread.render &&
                                        p.thumbnailThread.buffer)
                                    {
                                        p.thumbnailThread.render->begin(size);
                                        p.thumbnailThread.render->drawVideo(
                                            {videoData},
                                            {math::Box2i(
                                                0, 0, size.w, size.h)});
                                        p.thumbnailThread.render->end();
                                        image = image::Image::create(
                                            size.w, size.h,
                                            image::PixelType::RGBA_U8);
                                        glPixelStorei(GL_PACK_ALIGNMENT, 1);
                                        glReadPixels(
                                            0, 0, size.w, size.h, GL_RGBA,
                                            GL_UNSIGNED_BYTE, image->getData());
                                    }
                                }
                            }
                        }
                        catch (const std::exception&)
                        {
                        }
                    }
                }
                request->promise.set_value(image);
                p.cache->addThumbnail(key, image);
            }
        }

        namespace
        {
            std::shared_ptr<geom::TriangleMesh2> audioMesh(
                const std::shared_ptr<audio::Audio>& audio,
                const math::Size2i& size)
            {
                auto out = std::shared_ptr<geom::TriangleMesh2>(
                    new geom::TriangleMesh2);
                const auto& info = audio->getInfo();
                const size_t sampleCount = audio->getSampleCount();
                if (sampleCount > 0)
                {
                    switch (info.dataType)
                    {
                    case audio::DataType::F32:
                    {
                        const audio::F32_T* data =
                            reinterpret_cast<const audio::F32_T*>(
                                audio->getData());
                        for (int x = 0; x < size.w; ++x)
                        {
                            const int x0 = std::min(
                                static_cast<size_t>(
                                    (x + 0) / static_cast<double>(size.w - 1) *
                                    (sampleCount - 1)),
                                sampleCount - 1);
                            const int x1 = std::min(
                                static_cast<size_t>(
                                    (x + 1) / static_cast<double>(size.w - 1) *
                                    (sampleCount - 1)),
                                sampleCount - 1);
                            // std::cout << x << ": " << x0 << " " << x1 <<
                            // std::endl;
                            audio::F32_T min = 0.F;
                            audio::F32_T max = 0.F;
                            if (x0 <= x1)
                            {
                                min = audio::F32Range.getMax();
                                max = audio::F32Range.getMin();
                                for (int i = x0; i <= x1 && i < sampleCount;
                                     ++i)
                                {
                                    const audio::F32_T v =
                                        *(data + i * info.channelCount);
                                    min = std::min(min, v);
                                    max = std::max(max, v);
                                }
                            }
                            const int h2 = size.h / 2;
                            const math::Box2i box(
                                math::Vector2i(x, h2 - h2 * max),
                                math::Vector2i(x + 1, h2 - h2 * min));
                            if (box.isValid())
                            {
                                const size_t j = 1 + out->v.size();
                                out->v.push_back(
                                    math::Vector2f(box.x(), box.y()));
                                out->v.push_back(
                                    math::Vector2f(box.x() + box.w(), box.y()));
                                out->v.push_back(math::Vector2f(
                                    box.x() + box.w(), box.y() + box.h()));
                                out->v.push_back(
                                    math::Vector2f(box.x(), box.y() + box.h()));
                                out->triangles.push_back(
                                    geom::Triangle2({j + 0, j + 1, j + 2}));
                                out->triangles.push_back(
                                    geom::Triangle2({j + 2, j + 3, j + 0}));
                            }
                        }
                        break;
                    }
                    default:
                        break;
                    }
                }
                return out;
            }

            std::shared_ptr<image::Image> audioImage(
                const std::shared_ptr<audio::Audio>& audio,
                const math::Size2i& size)
            {
                auto out = image::Image::create(
                    size.w, size.h, image::PixelType::L_U8);
                const auto& info = audio->getInfo();
                const size_t sampleCount = audio->getSampleCount();
                if (sampleCount > 0)
                {
                    switch (info.dataType)
                    {
                    case audio::DataType::F32:
                    {
                        const audio::F32_T* data =
                            reinterpret_cast<const audio::F32_T*>(
                                audio->getData());
                        for (int x = 0; x < size.w; ++x)
                        {
                            const int x0 = std::min(
                                static_cast<size_t>(
                                    (x + 0) / static_cast<double>(size.w - 1) *
                                    (sampleCount - 1)),
                                sampleCount - 1);
                            const int x1 = std::min(
                                static_cast<size_t>(
                                    (x + 1) / static_cast<double>(size.w - 1) *
                                    (sampleCount - 1)),
                                sampleCount - 1);
                            // std::cout << x << ": " << x0 << " " << x1 <<
                            // std::endl;
                            audio::F32_T min = 0.F;
                            audio::F32_T max = 0.F;
                            if (x0 < x1)
                            {
                                min = audio::F32Range.getMax();
                                max = audio::F32Range.getMin();
                                for (int i = x0; i < x1; ++i)
                                {
                                    const audio::F32_T v =
                                        *(data + i * info.channelCount);
                                    min = std::min(min, v);
                                    max = std::max(max, v);
                                }
                            }
                            uint8_t* p = out->getData() + x;
                            for (int y = 0; y < size.h; ++y)
                            {
                                const float v =
                                    y / static_cast<float>(size.h - 1) * 2.F -
                                    1.F;
                                *p = (v > min && v < max) ? 255 : 0;
                                p += size.w;
                            }
                        }
                        break;
                    }
                    default:
                        break;
                    }
                }
                return out;
            }
        } // namespace

        void ThumbnailGenerator::_waveformRun()
        {
            TLRENDER_P();
            std::shared_ptr<Private::WaveformRequest> request;
            {
                std::unique_lock<std::mutex> lock(p.waveformMutex.mutex);
                if (p.waveformThread.cv.wait_for(
                        lock, std::chrono::milliseconds(5),
                        [this] { return !_p->waveformMutex.requests.empty(); }))
                {
                    request = p.waveformMutex.requests.front();
                    p.waveformMutex.requests.pop_front();
                }
            }
            if (request)
            {
                std::shared_ptr<geom::TriangleMesh2> mesh;
                const std::string key = ThumbnailCache::getWaveformKey(
                    request->size, request->path, request->timeRange,
                    request->options);
                if (!p.cache->getWaveform(key, mesh))
                {
                    if (auto context = p.context.lock())
                    {
                        try
                        {
                            const std::string& fileName = request->path.get();
                            std::shared_ptr<io::IRead> read;
                            if (!p.waveformThread.ioCache.get(fileName, read))
                            {
                                auto ioSystem =
                                    context->getSystem<io::System>();
                                read = ioSystem->read(
                                    request->path, request->memoryRead,
                                    request->options);
                                p.waveformThread.ioCache.add(fileName, read);
                            }
                            if (read)
                            {
                                const auto info = read->getInfo().get();
                                const otime::TimeRange timeRange =
                                    request->timeRange != time::invalidTimeRange
                                        ? request->timeRange
                                        : otime::TimeRange(
                                              otime::RationalTime(0.0, 1.0),
                                              otime::RationalTime(1.0, 1.0));
                                const auto audioData =
                                    read->readAudio(timeRange, request->options)
                                        .get();
                                if (audioData.audio)
                                {
                                    auto resample =
                                        audio::AudioResample::create(
                                            audioData.audio->getInfo(),
                                            audio::Info(
                                                1, audio::DataType::F32,
                                                audioData.audio
                                                    ->getSampleRate()));
                                    const auto resampledAudio =
                                        resample->process(audioData.audio);
                                    mesh = audioMesh(
                                        resampledAudio, request->size);
                                }
                            }
                        }
                        catch (const std::exception&)
                        {
                        }
                    }
                }
                request->promise.set_value(mesh);
                p.cache->addWaveform(key, mesh);
            }
        }

        void ThumbnailGenerator::_infoCancel()
        {
            TLRENDER_P();
            std::list<std::shared_ptr<Private::InfoRequest> > requests;
            {
                std::unique_lock<std::mutex> lock(p.infoMutex.mutex);
                requests = std::move(p.infoMutex.requests);
            }
            for (auto& request : requests)
            {
                request->promise.set_value(io::Info());
            }
        }

        void ThumbnailGenerator::_thumbnailCancel()
        {
            TLRENDER_P();
            std::list<std::shared_ptr<Private::ThumbnailRequest> > requests;
            {
                std::unique_lock<std::mutex> lock(p.thumbnailMutex.mutex);
                requests = std::move(p.thumbnailMutex.requests);
            }
            for (auto& request : requests)
            {
                request->promise.set_value(nullptr);
            }
        }

        void ThumbnailGenerator::_waveformCancel()
        {
            TLRENDER_P();
            std::list<std::shared_ptr<Private::WaveformRequest> > requests;
            {
                std::unique_lock<std::mutex> lock(p.waveformMutex.mutex);
                requests = std::move(p.waveformMutex.requests);
            }
            for (auto& request : requests)
            {
                request->promise.set_value(nullptr);
            }
        }

        struct ThumbnailSystem::Private
        {
            std::shared_ptr<ThumbnailCache> cache;
            std::shared_ptr<ThumbnailGenerator> generator;
        };

        void
        ThumbnailSystem::_init(const std::shared_ptr<system::Context>& context)
        {
            ISystem::_init("tl::timeline_vlk::ThumbnailSystem", context);
            TLRENDER_P();
            p.cache = ThumbnailCache::create(context);
            p.generator = ThumbnailGenerator::create(p.cache, context);
        }

        ThumbnailSystem::ThumbnailSystem() :
            _p(new Private)
        {
        }

        ThumbnailSystem::~ThumbnailSystem() {}

        std::shared_ptr<ThumbnailSystem>
        ThumbnailSystem::create(const std::shared_ptr<system::Context>& context)
        {
            auto out = std::shared_ptr<ThumbnailSystem>(new ThumbnailSystem);
            out->_init(context);
            return out;
        }

        InfoRequest ThumbnailSystem::getInfo(
            const file::Path& path, const io::Options& ioOptions)
        {
            return _p->generator->getInfo(path, ioOptions);
        }

        ThumbnailRequest ThumbnailSystem::getThumbnail(
            const file::Path& path, int height, const otime::RationalTime& time,
            const io::Options& ioOptions)
        {
            return _p->generator->getThumbnail(path, height, time, ioOptions);
        }

        WaveformRequest ThumbnailSystem::getWaveform(
            const file::Path& path, const math::Size2i& size,
            const otime::TimeRange& timeRange, const io::Options& ioOptions)
        {
            return _p->generator->getWaveform(path, size, timeRange, ioOptions);
        }

        void ThumbnailSystem::cancelRequests(const std::vector<uint64_t>& ids)
        {
            _p->generator->cancelRequests(ids);
        }

        const std::shared_ptr<ThumbnailCache>& ThumbnailSystem::getCache() const
        {
            return _p->cache;
        }
    } // namespace timeline_gl
} // namespace tl
