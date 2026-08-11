// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// Copyright (c) 2025-Present Gonzalo Garramuño
// All rights reserved.

#include "ThumbnailSystem.h"

#include <tlTimeline/Timeline.h>

#include <tlIO/System.h>

#ifdef OPENGL_BACKEND
#    include <tlTimelineGL/Render.h>
#    include <tlGL/GL.h>
#    include <tlGL/GLFWWindow.h>
#    include <tlGL/OffscreenBuffer.h>
#endif

#ifdef VULKAN_BACKEND
#    include <tlTimelineVk/Render.h>
#    include <tlVk/Vk.h>
#    include <tlVk/OffscreenBuffer.h>
#    include <FL/Fl_Vk_Utils.H>
#    include <FL/Fl.H>
#endif

#include <tlCore/AudioResample.h>
#include <tlCore/LRUCache.h>
#include <tlCore/StringFormat.h>
#include <tlCore/Timer.h>

#include <sstream>

namespace tl
{
    namespace TIMELINEUI
    {
        bool ThumbnailCacheOptions::operator == (const ThumbnailCacheOptions& other) const
        {
            return
                thumbnailMB == other.thumbnailMB &&
                waveformMB == other.waveformMB;
        }

        bool ThumbnailCacheOptions::operator != (const ThumbnailCacheOptions& other) const
        {
            return !(*this == other);
        }

        namespace
        {
            // Timelines, which hold no thread now that they are opened
            // without one, so a file browser listing can keep the ones it is
            // showing rather than reopening two at a time. Idle entries are
            // dropped after ioCacheTimeout regardless.
            const size_t ioCacheMax   = 100;
            // How long a thread holds its open timelines once it goes idle.
            // The point is to let go of readers, and the decode subprocesses
            // they keep alive, after a file is closed. It has to be long
            // compared to how long opening costs: a bundle of 25,000 entries
            // takes seconds to open, and dropping it between two thumbnails
            // meant most of the time went into opening it again.
            const std::chrono::seconds ioCacheTimeout(60);
            const size_t infoCacheMax = 1000;

            std::string getInfoKey(
                const file::Path& path,
                const file::Path& mediaPath,
                const io::Options& options)
            {
                std::stringstream ss;
                ss << path.get() << ";" << mediaPath.get() << ";";
                for (const auto& i : options)
                {
                    ss << i.first << ":" << i.second << ";";
                }
                return ss.str();
            }

            // Every thumbnail is a frame of a timeline. A plain file is a
            // timeline of one clip, so one path covers files, bundles and
            // timelines alike, and nothing needs a syntax for naming what is
            // inside a bundle.
            std::shared_ptr<timeline::Timeline> getTimeline(
                const std::shared_ptr<system::Context>& context,
                memory::LRUCache<std::string, std::shared_ptr<timeline::Timeline> >& cache,
                std::mutex& mutex,
                const file::Path& path)
            {
                // One timeline per file, shared by the three threads rather
                // than one each. Opening a bundle of 25,000 entries takes
                // seconds, so opening it three times is three times too
                // many. The timeline has no thread of its own and guards its
                // caches, so the threads can read it at once.
                std::shared_ptr<timeline::Timeline> out;
                {
                    std::unique_lock<std::mutex> lock(mutex);
                    if (cache.get(path.get(), out))
                    {
                        return out;
                    }
                }

                // Opened without the lock: a bundle takes seconds and a movie
                // needs a probe, and holding the lock across that would stall
                // every request for every other file. Two threads opening the
                // same file at once is cheaper than that, and only one of the
                // two ends up in the cache.
                timeline::Options options;
                options.threaded = false;
                file::Path inOutPath = path;
                out = timeline::Timeline::create(context, inOutPath, options);
                {
                    std::unique_lock<std::mutex> lock(mutex);
                    std::shared_ptr<timeline::Timeline> other;
                    if (cache.get(path.get(), other))
                    {
                        return other;
                    }
                    cache.add(path.get(), out);
                }
                return out;
            }

            std::string getThumbnailKey(
                const file::Path& path,
                const file::Path& mediaPath,
                const std::string& mediaReferenceKey,
                int height,
                const std::optional<otio::RationalTime>& time,
                const io::Options& options)
            {
                std::stringstream ss;
                ss << path.get() << ";" << mediaPath.get() << ";" <<
                    height << ";" << mediaReferenceKey << ";";
                if (time.has_value())
                {
                    ss << time.value();
                }
                ss << ";";
                for (const auto& i : options)
                {
                    ss << i.first << ":" << i.second << ";";
                }
                return ss.str();
            }

            std::string getWaveformKey(
                const file::Path& path,
                const file::Path& mediaPath,
                const math::Size2i& size,
                const std::optional<otio::TimeRange>& timeRange,
                const io::Options& options)
            {
                std::stringstream ss;
                ss << path.get() << ";" << mediaPath.get() << ";" <<
                    size << ";";
                if (timeRange.has_value())
                {
                    ss << timeRange.value();
                }
                ss << ";";
                for (const auto& i : options)
                {
                    ss << i.first << ":" << i.second << ";";
                }
                return ss.str();
            }
        }

        struct ThumbnailSystem::Private
        {
            std::weak_ptr<system::Context> context;
#ifdef OPENGL_BACKEND
            std::shared_ptr<gl::GLFWWindow> window;
#endif
            uint64_t requestId = 0;;
            std::shared_ptr<observer::Value<ThumbnailCacheOptions> > cacheOptions;


            struct InfoRequest
            {
                uint64_t id = 0;
                file::Path path;
                file::Path mediaPath;
                io::Options options;
                std::promise<io::Info> promise;
            };

            struct ThumbnailRequest
            {
                uint64_t id = 0;
                file::Path path;
                file::Path mediaPath;
                int height = 0;
                std::optional<otio::RationalTime> time;
                std::string mediaReferenceKey;
                io::Options options;
                std::promise<std::shared_ptr<image::Image> > promise;
            };

            struct WaveformRequest
            {
                uint64_t id = 0;
                file::Path path;
                file::Path mediaPath;
                math::Size2i size;
                std::optional<otio::TimeRange> timeRange;
                std::string mediaReferenceKey;
                io::Options options;
                std::promise<std::shared_ptr<geom::TriangleMesh2> > promise;
            };

            struct InfoMutex
            {
                std::list<std::shared_ptr<InfoRequest> > requests;
                memory::LRUCache<std::string, io::Info> cache;
                bool stopped = false;
                std::mutex mutex;
            };
            InfoMutex infoMutex;

            struct ThumbnailMutex
            {
                std::list<std::shared_ptr<ThumbnailRequest> > requests;
                memory::LRUCache<std::string, std::shared_ptr<image::Image> > cache;
                bool stopped = false;
                std::mutex mutex;
            };
            ThumbnailMutex thumbnailMutex;

            struct WaveformMutex
            {
                std::list<std::shared_ptr<WaveformRequest> > requests;
                memory::LRUCache<std::string,
                                 std::shared_ptr<geom::TriangleMesh2> > cache;
                bool stopped = false;
                std::mutex mutex;
            };
            WaveformMutex waveformMutex;

            // Shared by the three threads below.
            memory::LRUCache<std::string, std::shared_ptr<timeline::Timeline> > ioCache;
            std::mutex ioCacheMutex;

            struct InfoThread
            {
                std::atomic<bool> ioCacheClear = false;
                std::condition_variable cv;
                std::thread thread;
                std::atomic<bool> running;
            };
            InfoThread infoThread;

            struct ThumbnailThread
            {
#ifdef OPENGL_BACKEND
                std::shared_ptr<timeline_gl::Render> render;
                std::shared_ptr<gl::OffscreenBuffer> buffer;
#endif
#ifdef VULKAN_BACKEND
                std::shared_ptr<timeline_vlk::Render> render;
                std::shared_ptr<vlk::OffscreenBuffer> buffer;
                VkCommandBuffer cmd = VK_NULL_HANDLE;
                VkCommandPool commandPool = VK_NULL_HANDLE;
                uint32_t frameIndex = 0;
#endif
                std::atomic<bool> ioCacheClear = false;
                std::condition_variable cv;
                std::thread thread;
                std::atomic<bool> running;
            };
            ThumbnailThread thumbnailThread;

            struct WaveformThread
            {
                std::atomic<bool> ioCacheClear = false;
                std::condition_variable cv;
                std::thread thread;
                std::atomic<bool> running;
            };
            WaveformThread waveformThread;

            std::shared_ptr<time::Timer> logTimer;


            // When any of the three threads last had work. The cache is
            // shared, so one thread going quiet must not drop the timelines
            // another is still using.
            std::atomic<int64_t> ioCacheActive{ 0 };
            void ioCacheTouch()
            {
                ioCacheActive = std::chrono::steady_clock::now()
                    .time_since_epoch().count();
            }
            bool ioCacheIdle(const std::chrono::seconds& timeout)
            {
                const auto last = std::chrono::steady_clock::time_point(
                    std::chrono::steady_clock::duration(ioCacheActive.load()));
                return std::chrono::steady_clock::now() - last > timeout;
            }
            void clearIOCache()
            {
                std::unique_lock<std::mutex> lock(ioCacheMutex);
                ioCache.clear();
            }
            size_t ioCacheCount()
            {
                std::unique_lock<std::mutex> lock(ioCacheMutex);
                return ioCache.getCount();
            }
        };

        void ThumbnailSystem::_startThreads()
        {
            TLRENDER_P();

            p.infoMutex.cache.setMax(infoCacheMax);
            p.infoThread.running = true;
            p.infoThread.thread = std::thread(
                [this]
                {
                    TLRENDER_P();
                    _infoRun();
                    {
                        std::unique_lock<std::mutex> lock(p.infoMutex.mutex);
                        p.infoMutex.stopped = true;
                    }
                    _infoCancel();
                });

            p.thumbnailMutex.cache.setMax(p.cacheOptions->get().thumbnailMB * memory::megabyte);
            p.ioCache.setMax(ioCacheMax);
            p.thumbnailThread.running = true;
            p.thumbnailThread.thread = std::thread(
                [this]
                {
                    TLRENDER_P();
#ifdef OPENGL_BACKEND
                    p.window->makeCurrent();
#endif
                    if (auto context = p.context.lock())
                    {
#ifdef VULKAN_BACKEND
                        // Check Vulkan context
                        while ((ctx.queue() == VK_NULL_HANDLE ||
                                ctx.device == VK_NULL_HANDLE ||
                                ctx.instance == VK_NULL_HANDLE) &&
                               p.thumbnailThread.running)
                        {
                            continue;
                        }

                        VkDevice device = ctx.device;
                        VkResult result = VK_SUCCESS;

                        // Create command pool
                        if (p.thumbnailThread.running &&
                            p.thumbnailThread.commandPool == VK_NULL_HANDLE)
                        {
                            VkCommandPoolCreateInfo cmd_pool_info = {};
                            cmd_pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
                            cmd_pool_info.queueFamilyIndex = ctx.queueFamilyIndex;
                            cmd_pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
                            result = vkCreateCommandPool(device, &cmd_pool_info,
                                                         nullptr,
                                                         &p.thumbnailThread.commandPool);
                        }

                        if (p.thumbnailThread.running && result == VK_SUCCESS)
                        {
                            VkCommandBufferAllocateInfo allocInfo = {};
                            allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
                            allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
                            allocInfo.commandPool = p.thumbnailThread.commandPool;
                            allocInfo.commandBufferCount = 1;

                            result = vkAllocateCommandBuffers(device, &allocInfo, &p.thumbnailThread.cmd);
                        }

                        p.thumbnailThread.render =
                            timeline_vlk::Render::create(ctx, context);

#endif

#ifdef OPENGL_BACKEND
                        p.thumbnailThread.render = timeline_gl::Render::create(
                            context);
#endif
                    }
                    if (p.thumbnailThread.render)
                    {
                        _thumbnailRun();
                    }
                    {
                        std::unique_lock<std::mutex> lock(p.thumbnailMutex.mutex);
                        p.thumbnailMutex.stopped = true;
                    }
                    p.thumbnailThread.buffer.reset();
                    p.thumbnailThread.render.reset();
                    _thumbnailCancel();
#ifdef OPENGL_BACKEND
                    p.window->doneCurrent();
#endif
#ifdef VULKAN_BACKEND
                    if (p.thumbnailThread.commandPool != VK_NULL_HANDLE)
                    {
                        vkDestroyCommandPool(ctx.device,
                                             p.thumbnailThread.commandPool,
                                             nullptr);
                    }
#endif
                });

            p.waveformMutex.cache.setMax(p.cacheOptions->get().waveformMB * memory::megabyte);

            p.waveformThread.running = true;
            p.waveformThread.thread = std::thread(
                [this]
                {
                    TLRENDER_P();
                    _waveformRun();
                    {
                        std::unique_lock<std::mutex> lock(p.waveformMutex.mutex);
                        p.waveformMutex.stopped = true;
                    }
                    _waveformCancel();
                });

            p.logTimer = time::Timer::create(p.context.lock());
            p.logTimer->setRepeating(true);
            p.logTimer->start(
                std::chrono::seconds(10),
                [this]
                {
                    TLRENDER_P();
                    if (auto context = p.context.lock())
                    {
                        size_t infoCacheSize = 0;
                        size_t thumbnailCacheSize = 0;
                        size_t waveformCacheSize = 0;
                        {
                            std::unique_lock<std::mutex> lock(p.infoMutex.mutex);
                            infoCacheSize = p.infoMutex.cache.getSize();
                        }
                        {
                            std::unique_lock<std::mutex> lock(p.thumbnailMutex.mutex);
                            thumbnailCacheSize = p.thumbnailMutex.cache.getSize();
                        }
                        {
                            std::unique_lock<std::mutex> lock(p.waveformMutex.mutex);
                            waveformCacheSize = p.waveformMutex.cache.getSize();
                        }
                        auto logSystem = context->getLogSystem();
                        logSystem->print(
                            "tl::ui::ThumbnailSystem",
                            string::Format(
                                "\n"
                                "    * Information: {0}/{1}\n"
                                "    * Thumbnails: {2}/{3}MB\n"
                                "    * Waveforms: {4}/{5}MB"
                            ).
                            arg(infoCacheSize).
                            arg(infoCacheMax).
                            arg(thumbnailCacheSize / memory::megabyte).
                            arg(p.cacheOptions->get().thumbnailMB).
                            arg(waveformCacheSize / memory::megabyte).
                            arg(p.cacheOptions->get().waveformMB));
                    }
                });

        }

#ifdef OPENGL_BACKEND

        ThumbnailSystem::ThumbnailSystem(
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<gl::GLFWWindow>& window) :
            ISystem::ISystem(context, "tl::timelineui::ThumbnailSystem"),
            _p(new Private)
        {
            TLRENDER_P();

            p.context = context;

            p.window = window;
            if (!p.window)
            {
                p.window = gl::GLFWWindow::create(
                    "tl::timelineui::ThumbnailSystem", math::Size2i(1, 1), context,
                    static_cast<int>(gl::GLFWWindowOptions::kNone));
            }

            p.cacheOptions = observer::Value<ThumbnailCacheOptions>::create();

            _startThreads();
        }

#endif

#ifdef VULKAN_BACKEND


        ThumbnailSystem::ThumbnailSystem(
            const std::shared_ptr<system::Context>& context,
            Fl_Vk_Context& vlk_ctx) :
            ISystem::ISystem(context, "tl::timelineui::ThumbnailSystem"),
            ctx(vlk_ctx),
            _p(new Private)
        {
            TLRENDER_P();

            p.context = context;

            p.cacheOptions = observer::Value<ThumbnailCacheOptions>::create();

            _startThreads();
        }

#endif
        ThumbnailSystem::~ThumbnailSystem()
        {
            shutdown();
        }

        void ThumbnailSystem::shutdown()
        {
            TLRENDER_P();

            p.infoThread.running = false;
            p.thumbnailThread.running = false;
            p.waveformThread.running = false;
            if (p.infoThread.thread.joinable())
            {
                p.infoThread.thread.join();
            }
            if (p.thumbnailThread.thread.joinable())
            {
                p.thumbnailThread.thread.join();
            }
            if (p.waveformThread.thread.joinable())
            {
                p.waveformThread.thread.join();
            }
        }

#ifdef OPENGL_BACKEND
        std::shared_ptr<ThumbnailSystem> ThumbnailSystem::create(
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<gl::GLFWWindow>& window)
        {
            auto out = context->getSystem<ThumbnailSystem>();
            if (!out)
            {
                out = std::shared_ptr<ThumbnailSystem>(new ThumbnailSystem(context, window));
                context->addSystem(out);
            }
            return out;
        }
#endif

#ifdef VULKAN_BACKEND
        std::shared_ptr<ThumbnailSystem> ThumbnailSystem::create(
            const std::shared_ptr<system::Context>& context,
            Fl_Vk_Context& ctx)
        {
            auto out = context->getSystem<ThumbnailSystem>();
            if (!out)
            {
                out = std::shared_ptr<ThumbnailSystem>(new ThumbnailSystem(context, ctx));
                context->addSystem(out);
            }
            return out;
        }
#endif

        InfoRequest ThumbnailSystem::getInfo(
            const file::Path& path, const io::Options& options)
        {
            return getInfo(path, path, options);
        }

        InfoRequest ThumbnailSystem::getInfo(
            const file::Path& timelinePath,
            const file::Path& mediaPath,
            const io::Options& options)
        {
            TLRENDER_P();
            (p.requestId)++;
            auto request = std::make_shared<Private::InfoRequest>();
            request->id = p.requestId;
            request->path = timelinePath;
            request->mediaPath = mediaPath;
            request->options = options;

            const std::string key = getInfoKey(timelinePath, mediaPath, options);
            io::Info info;
            bool notify = false;
            {
                std::unique_lock<std::mutex> lock(p.infoMutex.mutex);
                if (p.infoMutex.cache.get(key, info))
                    ;
                else if (!p.infoMutex.stopped)
                {
                    notify = true;
                    p.infoMutex.requests.push_back(request);
                }
            }
            if (notify)
            {
                p.infoThread.cv.notify_one();
            }
            else
            {
                request->promise.set_value(info);
            }

            InfoRequest out;
            out.id = request->id;
            out.future = request->promise.get_future();
            return out;
        }

        ThumbnailRequest ThumbnailSystem::getThumbnail(
            const file::Path& path, int height,
            const std::optional<otio::RationalTime>& time,
            const std::string& mediaReferenceKey, const io::Options& options)
        {
            return getThumbnail(path, path, height, time, mediaReferenceKey,
                                options);
        }

        ThumbnailRequest ThumbnailSystem::getThumbnail(
            const file::Path& path,
            const file::Path& mediaPath,
            int height,
            const std::optional<otio::RationalTime>& time,
            const std::string& mediaReferenceKey, const io::Options& options)
        {
            TLRENDER_P();
            (p.requestId)++;
            auto request = std::make_shared<Private::ThumbnailRequest>();
            request->id = p.requestId;
            request->path = path;
            request->mediaPath = mediaPath;
            request->height = height;
            request->time = time;
            request->mediaReferenceKey = mediaReferenceKey;
            request->options = options;

            const std::string key = getThumbnailKey(
                path,
                mediaPath,
                mediaReferenceKey,
                height,
                time,
                options);
            std::shared_ptr<image::Image> thumbnail;
            bool notify = false;
            {
                std::unique_lock<std::mutex> lock(p.thumbnailMutex.mutex);
                if (p.thumbnailMutex.cache.get(key, thumbnail))
                    ;
                else if (!p.thumbnailMutex.stopped)
                {
                    notify = true;
                    p.thumbnailMutex.requests.push_back(request);
                }
            }
            if (notify)
            {
                p.thumbnailThread.cv.notify_one();
            }
            else
            {
                request->promise.set_value(thumbnail);
            }

            ThumbnailRequest out;
            out.id = request->id;
            out.height = height;
            out.time = time;
            out.future = request->promise.get_future();
            return out;
        }

        WaveformRequest ThumbnailSystem::getWaveform(
            const file::Path& path, const math::Size2i& size,
            const std::optional<otime::TimeRange >& timeRange,
            const std::string& mediaReferenceKey, const io::Options& options)
        {
            return getWaveform(path, {}, size, timeRange, mediaReferenceKey,
                               options);
        }

        WaveformRequest ThumbnailSystem::getWaveform(
            const file::Path& path,
            const file::Path& mediaPath,
            const math::Size2i& size,
            const std::optional<otime::TimeRange >& timeRange,
            const std::string& mediaReferenceKey,
            const io::Options& options)
        {
            TLRENDER_P();
            (p.requestId)++;

            auto request = std::make_shared<Private::WaveformRequest>();
            request->id = p.requestId;
            request->path = path;
            request->mediaPath = mediaPath;
            request->size = size;
            request->timeRange = timeRange;
            request->mediaReferenceKey = mediaReferenceKey;
            request->options = options;

            const std::string key = getWaveformKey(
                path,
                mediaPath,
                size,
                timeRange,
                options);

            std::shared_ptr<geom::TriangleMesh2> mesh;
            bool notify = false;
            {
                std::unique_lock<std::mutex> lock(p.waveformMutex.mutex);
                if (p.waveformMutex.cache.get(key, mesh))
                    ;
                else if (!p.waveformMutex.stopped)
                {
                    notify = true;
                    p.waveformMutex.requests.push_back(request);
                }
            }
            if (notify)
            {
                p.waveformThread.cv.notify_one();
            }
            else
            {
                request->promise.set_value(mesh);
            }

            WaveformRequest out;
            out.id = request->id;
            out.size = size;
            out.timeRange = timeRange;
            out.future = request->promise.get_future();
            return out;
        }

        void
        ThumbnailSystem::cancelRequests(const std::vector<uint64_t>& ids)
        {
            TLRENDER_P();
            // Looked up as a set: this is called with the requests of a whole
            // timeline's worth of items, and searching the list of ids for each
            // pending request made cancelling cost the product of the two.
            const std::set<uint64_t> idSet(ids.begin(), ids.end());
            {
                std::unique_lock<std::mutex> lock(p.infoMutex.mutex);
                auto i = p.infoMutex.requests.begin();
                while (i != p.infoMutex.requests.end())
                {
                    if (idSet.find((*i)->id) != idSet.end())
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
                    if (idSet.find((*i)->id) != idSet.end())
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
                    if (idSet.find((*i)->id) != idSet.end())
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

        std::shared_ptr<observer::IValue<ThumbnailCacheOptions> > ThumbnailSystem::observeCacheOptions() const
        {
             return _p->cacheOptions;
        }

        const ThumbnailCacheOptions& ThumbnailSystem::getCacheOptions() const
        {
            return _p->cacheOptions->get();
        }

        void ThumbnailSystem::setCacheOptions(const ThumbnailCacheOptions& value)
        {
            TLRENDER_P();
            if (p.cacheOptions->setIfChanged(value))
            {
                {
                    std::unique_lock<std::mutex> lock(p.thumbnailMutex.mutex);
                    p.thumbnailMutex.cache.setMax(value.thumbnailMB * memory::megabyte);
                }
                {
                    std::unique_lock<std::mutex> lock(p.waveformMutex.mutex);
                    p.waveformMutex.cache.setMax(value.waveformMB * memory::megabyte);
                }
            }
        }

        void ThumbnailSystem::clearCache()
        {
            TLRENDER_P();
            {
                std::unique_lock<std::mutex> lock(p.infoMutex.mutex);
                p.infoMutex.cache.clear();
            }
            {
                std::unique_lock<std::mutex> lock(p.thumbnailMutex.mutex);
                p.thumbnailMutex.cache.clear();
            }
            {
                std::unique_lock<std::mutex> lock(p.waveformMutex.mutex);
                p.waveformMutex.cache.clear();
            }

            // Signal the worker threads to drop their cached open readers so a
            // reloaded file is re-opened rather than served from a reader that
            // points at the previous file contents. The ioCache is owned by its
            // worker thread, so it is cleared there rather than under a lock.
            p.infoThread.ioCacheClear = true;
            p.infoThread.cv.notify_one();
            p.thumbnailThread.ioCacheClear = true;
            p.thumbnailThread.cv.notify_one();
            p.waveformThread.ioCacheClear = true;
            p.waveformThread.cv.notify_one();
        }

        void ThumbnailSystem::_infoRun()
        {
            TLRENDER_P();
            while (p.infoThread.running)
            {
                if (p.infoThread.ioCacheClear.exchange(false))
                {
                    p.clearIOCache();
                }
                std::shared_ptr<Private::InfoRequest> request;
                {
                    std::unique_lock<std::mutex> lock(p.infoMutex.mutex);
                    if (p.infoThread.cv.wait_for(
                        lock,
                        std::chrono::milliseconds(5),
                        [this]
                        {
                            return !_p->infoMutex.requests.empty();
                        }))
                    {
                        request = p.infoMutex.requests.front();
                        p.infoMutex.requests.pop_front();
                    }
                }
                if (request)
                {
                    p.ioCacheTouch();
                    io::Info info;
                    try
                    {
                        //std::cout << "info request: " << request->path.get() << std::endl;
                        auto context = p.context.lock();
                        if (auto timeline = getTimeline(
                            context, p.ioCache, p.ioCacheMutex, request->path))
                        {
                            timeline->getMediaInfo(request->mediaPath, info);
                        }
                    }
                    catch (const std::exception&)
                    {}
                    request->promise.set_value(info);

                    const std::string key = getInfoKey(
                        request->path, request->mediaPath, request->options);
                    std::unique_lock<std::mutex> lock(p.infoMutex.mutex);
                    p.infoMutex.cache.add(key, info);
                }
            }
        }

        void ThumbnailSystem::_thumbnailRun()
        {
            TLRENDER_P();

            io::Options ioOptions;
            while (p.thumbnailThread.running)
            {
                if (p.thumbnailThread.ioCacheClear.exchange(false))
                {
                    p.clearIOCache();
                }

                std::shared_ptr<Private::ThumbnailRequest> request;
                {
                    std::unique_lock<std::mutex> lock(p.thumbnailMutex.mutex);
                    if (p.thumbnailThread.cv.wait_for(
                        lock,
                        std::chrono::milliseconds(5),
                        [this]
                        {
                            return !_p->thumbnailMutex.requests.empty();
                        }))
                    {
                        request = p.thumbnailMutex.requests.front();
                        p.thumbnailMutex.requests.pop_front();
                    }
                }
                if (request)
                {
                    // The options belong to the read, not to the timeline
                    // that is read from: a per clip option such as a camera
                    // name changes with every request, and dropping the
                    // timeline for it meant reopening the file each time.
                    p.ioCacheTouch();

                    std::shared_ptr<image::Image> image;
                    try
                    {
                        //std::cout << "thumbnail request: " <<
                        //    request->path.get() << " " <<
                        //    request->time << std::endl;
                        auto context = p.context.lock();
                        auto timeline = getTimeline(
                            context, p.ioCache, p.ioCacheMutex, request->path);
                        io::Info info;
                        if (timeline &&
                            timeline->getMediaInfo(request->mediaPath, info, request->options))
                        {
                            math::Size2i size;
                            if (!info.video.empty())
                            {
                                math::Size2i tmp(info.video[0].size.w,
                                                 info.video[0].size.h);
                                size.w = request->height * math::aspectRatio(tmp);
                                size.h = request->height;
                            }

                            if (size.isValid())
                            {
                                const otime::RationalTime time =
                                    request->time.value_or(info.videoTime->start_time());
                                auto videoRequest = timeline->readMedia(request->mediaPath,
                                                                        time, request->options);
                                if (videoRequest.valid())
                                {
                                    const auto videoData = videoRequest.get();
#ifdef OPENGL_BACKEND
                                    gl::OffscreenBufferOptions options;
                                    options.colorType = image::PixelType::RGBA_U8;
                                    if (gl::doCreate(
                                            p.thumbnailThread.buffer, size,
                                            options))
                                    {
                                        p.thumbnailThread.buffer =
                                            gl::OffscreenBuffer::create(
                                                size, options);
                                    }
                                    if (p.thumbnailThread.render &&
                                        p.thumbnailThread.buffer && videoData.image &&
                                        p.thumbnailThread.running)
                                    {
                                        gl::OffscreenBufferBinding binding(
                                            p.thumbnailThread.buffer);
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
#endif
#ifdef VULKAN_BACKEND
                                    vlk::OffscreenBufferOptions options;
                                    options.colorType = image::PixelType::RGBA_U8;
                                    options.pbo = true;
                                    if (vlk::doCreate(p.thumbnailThread.buffer, size, options))
                                    {
                                        p.thumbnailThread.buffer = vlk::OffscreenBuffer::create(ctx,
                                                                                                size,
                                                                                                options);
                                    }
                                    if (p.thumbnailThread.render &&
                                        p.thumbnailThread.buffer && videoData.image &&
                                        p.thumbnailThread.running)
                                    {
                                        image = image::Image::create(
                                            size.w, size.h,
                                            image::PixelType::RGBA_U8);

                                        VkCommandBuffer& cmd = p.thumbnailThread.cmd;
                                        vkResetCommandBuffer(cmd, 0);

                                        VkCommandBufferBeginInfo beginInfo = {};
                                        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
                                        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

                                        vkBeginCommandBuffer(cmd, &beginInfo);

                                        p.thumbnailThread.buffer->transitionToColorAttachment(cmd);

                                        timeline::RenderOptions renderOptions;
                                        renderOptions.clear = true;
                                        p.thumbnailThread.render->begin(cmd, p.thumbnailThread.buffer,
                                                                        p.thumbnailThread.frameIndex, size,
                                                                        renderOptions);

                                        const math::Matrix4x4f ortho = math::ortho(
                                            0.F, static_cast<float>(size.w),
                                            0.F, static_cast<float>(size.h),
                                            -1.F, 1.F);
                                        p.thumbnailThread.render->setTransform(ortho);

                                        p.thumbnailThread.render->drawImage(
                                            videoData.image,
                                            {math::Box2i(0, 0, size.w, size.h)});

                                        p.thumbnailThread.render->end();


                                        p.thumbnailThread.buffer->readPixels(cmd, 0, 0, size.w,
                                                                             size.h);

                                        vkEndCommandBuffer(cmd);

                                        p.thumbnailThread.buffer->submitReadback(cmd);

                                        VkResult result = VK_NOT_READY;
                                        void* imageData = nullptr;
                                        while (result == VK_NOT_READY)
                                        {
                                            result = p.thumbnailThread.buffer->getLatestReadPixels(imageData);
                                        }

                                        if (imageData)
                                            std::memcpy(image->getData(), imageData, image->getDataByteCount());
                                        else
                                            std::memset(image->getData(), 0, image->getDataByteCount());

                                        p.thumbnailThread.frameIndex = (p.thumbnailThread.frameIndex + 1) % vlk::MAX_FRAMES_IN_FLIGHT;
                                    }
#endif
                                }  // videoRequest valid
                            } // size valid
                        }
                        else if (timeline)
                        {
                            // The request does not name media inside the
                            // timeline, so it wants a picture of the timeline
                            // itself. Use the one already open: creating
                            // another here read the file again for every
                            // request, which on a bundle of 25,000 entries
                            // meant a thumbnail took as long as an open.
                            if (auto logSystem = context->getLogSystem())
                            {
                                logSystem->print("tl::ui::ThumbnailSystem",
                                                 string::Format("Media not found in timeline, "
                                                                "using the timeline itself: \"{0}\" "
                                                                "in \"{1}\"").
                                                 arg(request->mediaPath.get()).
                                                 arg(request->path.get()),
                                                 log::Type::Warning);
                            }
                            const auto info = timeline->getIOInfo();
                            const auto videoData = timeline->getVideo(
                                request->time.value_or(
                                    timeline->getTimeRange().start_time())).future.get();
                            math::Size2i size;
                            if (!info.video.empty())
                            {
                                math::Size2i tmp(info.video.front().size.w,
                                                 info.video.front().size.h);
                                size.w = request->height * math::aspectRatio(tmp);
                                size.h = request->height;
                            }
                            if (size.isValid())
                            {
#ifdef OPENGL_BACKEND
                                gl::OffscreenBufferOptions options;
                                options.colorType = image::PixelType::RGBA_U8;
                                if (gl::doCreate(
                                    p.thumbnailThread.buffer,
                                    size,
                                    options))
                                {
                                    p.thumbnailThread.buffer = gl::OffscreenBuffer::create(
                                        size,
                                        options);
                                }
                                if (p.thumbnailThread.render && p.thumbnailThread.buffer)
                                {
                                    gl::OffscreenBufferBinding binding(p.thumbnailThread.buffer);
                                    p.thumbnailThread.render->begin(size);
                                    p.thumbnailThread.render->drawVideo(
                                        { videoData },
                                        { math::Box2i(0, 0, size.w, size.h) });
                                    p.thumbnailThread.render->end();
                                    image::Info info(size.w,
                                                     size.h,
                                                     image::PixelType::RGBA_U8);
                                    image = image::Image::create(info);
                                    glPixelStorei(GL_PACK_ALIGNMENT, 1);
                                    glReadPixels(
                                        0,
                                        0,
                                        size.w,
                                        size.h,
                                        GL_RGBA,
                                        GL_UNSIGNED_BYTE,
                                        image->getData());
                                }
#endif

#ifdef VULKAN_BACKEND
                                vlk::OffscreenBufferOptions options;
                                options.colorType =
                                    image::PixelType::RGBA_U8;
                                options.pbo = true;
                                if (vlk::doCreate(
                                        p.thumbnailThread.buffer, size,
                                        options))
                                {
                                    p.thumbnailThread.buffer =
                                        vlk::OffscreenBuffer::create(ctx,
                                                                     size, options);
                                }
                                if (p.thumbnailThread.render &&
                                    p.thumbnailThread.buffer)
                                {
                                    image = image::Image::create(
                                        size.w, size.h,
                                        image::PixelType::RGBA_U8);

                                    VkCommandBuffer& cmd = p.thumbnailThread.cmd;
                                    vkResetCommandBuffer(cmd, 0);

                                    VkCommandBufferBeginInfo beginInfo = {};
                                    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
                                    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

                                    vkBeginCommandBuffer(cmd, &beginInfo);

                                    p.thumbnailThread.buffer->transitionToColorAttachment(cmd);

                                    timeline::RenderOptions renderOptions;
                                    renderOptions.clear = true;
                                    p.thumbnailThread.render->begin(cmd,
                                                                    p.thumbnailThread.buffer,
                                                                    p.thumbnailThread.frameIndex,
                                                                    size, renderOptions);

                                    const math::Matrix4x4f ortho = math::ortho(
                                        0.F, static_cast<float>(size.w),
                                        0.F, static_cast<float>(size.h),
                                        -1.F, 1.F);
                                    p.thumbnailThread.render->setTransform(ortho);
                                    p.thumbnailThread.render->drawVideo(
                                        {videoData},
                                        {math::Box2i(
                                                0, 0, size.w, size.h)});
                                    p.thumbnailThread.render->end();

                                    p.thumbnailThread.buffer->readPixels(cmd, 0, 0, size.w,
                                                                         size.h);

                                    vkEndCommandBuffer(cmd);

                                    p.thumbnailThread.buffer->submitReadback(cmd);


                                    VkResult result = VK_NOT_READY;
                                    void* imageData = nullptr;
                                    while (result == VK_NOT_READY)
                                    {
                                        result = p.thumbnailThread.buffer->getLatestReadPixels(imageData);
                                    }

                                    if (imageData)
                                        std::memcpy(image->getData(), imageData, image->getDataByteCount());
                                    else
                                        std::memset(image->getData(), 0, image->getDataByteCount());


                                    p.thumbnailThread.frameIndex = (p.thumbnailThread.frameIndex + 1) % vlk::MAX_FRAMES_IN_FLIGHT;
                                }  // if (p.thumbnailThread.buffer
#endif
                            }  // if (size.isValid())
                        }  // if timeline
                    }  // try
                    catch (const std::exception&)
                    {}
                    request->promise.set_value(image);

                    const std::string key = getThumbnailKey(
                        request->path,
                        request->mediaPath,
                        request->mediaReferenceKey,
                        request->height,
                        request->time,
                        request->options);
                    std::unique_lock<std::mutex> lock(p.thumbnailMutex.mutex);
                    p.thumbnailMutex.cache.add(key, image, image ? image->getDataByteCount() : 0);
                }
                else if (p.ioCacheCount() > 0 && p.ioCacheIdle(ioCacheTimeout))
                {
                    // Release cached readers (and the decode subprocesses they
                    // keep alive) once this thread has gone idle. Otherwise a
                    // file's readers linger until a different file's readers
                    // push them out of the LRU caches, so closing a file leaves
                    // its ffmpeg process running until the next file is opened.
                    p.clearIOCache();
                }
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
                                min = audio::F32Range.max();
                                max = audio::F32Range.min();
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
                                min = audio::F32Range.max();
                                max = audio::F32Range.min();
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

        void ThumbnailSystem::_waveformRun()
        {
            TLRENDER_P();
            io::Options ioOptions;
            while (p.waveformThread.running)
            {
                if (p.waveformThread.ioCacheClear.exchange(false))
                {
                    p.clearIOCache();
                }

                std::shared_ptr<Private::WaveformRequest> request;
                {
                    std::unique_lock<std::mutex> lock(p.waveformMutex.mutex);
                    if (p.waveformThread.cv.wait_for(
                        lock,
                        std::chrono::milliseconds(5),
                        [this]
                        {
                            return !_p->waveformMutex.requests.empty();
                        }))
                    {
                        request = p.waveformMutex.requests.front();
                        p.waveformMutex.requests.pop_front();
                    }
                }
                if (request)
                {
                    // The options belong to the read, not to the timeline
                    // that is read from: a per clip option such as a camera
                    // name changes with every request, and dropping the
                    // timeline for it meant reopening the file each time.
                    p.ioCacheTouch();

                    std::shared_ptr<geom::TriangleMesh2> mesh;
                    try
                    {
                        auto context = p.context.lock();
                        auto timeline = getTimeline(
                            context, p.ioCache, p.ioCacheMutex, request->path);
                        io::Info info;
                        if (timeline &&
                            timeline->getMediaInfo(
                                request->mediaPath, info, request->options))
                        {
                            const otio::TimeRange timeRange =
                                request->timeRange.value_or(
                                    otio::TimeRange(
                                        otio::RationalTime(0.0, 1.0),
                                        otio::RationalTime(1.0, 1.0)));
                            auto audioRequest = timeline->readMediaAudio(
                                request->mediaPath, timeRange, request->options);
                            if (audioRequest.valid())
                            {
                                const auto audioData = audioRequest.get();
                                if (audioData.audio && p.waveformThread.running)
                                {
                                    auto resample = audio::AudioResample::create(
                                        audioData.audio->getInfo(),
                                        audio::Info(1, audio::DataType::F32, audioData.audio->getSampleRate()));
                                    if (auto resampledAudio = resample->process(audioData.audio))
                                    {
                                        mesh = audioMesh(resampledAudio, request->size);
                                    }
                                }
                            }
                        }
                    }
                    catch (const std::exception&)
                    {}
                    request->promise.set_value(mesh);

                    const std::string key = getWaveformKey(
                        request->path,
                        request->mediaPath,
                        request->size,
                        request->timeRange,
                        request->options);
                    std::unique_lock<std::mutex> lock(p.waveformMutex.mutex);
                    p.waveformMutex.cache.add(key, mesh, mesh ? mesh->getByteCount() : 0);
                }
                else if (p.ioCacheCount() > 0 && p.ioCacheIdle(ioCacheTimeout))
                {
                    // Release cached readers (and the decode subprocesses they
                    // keep alive) once this thread has gone idle; see the note in
                    // _thumbnailRun().
                    p.clearIOCache();
                }
            }
        }

        void ThumbnailSystem::_infoCancel()
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

        void ThumbnailSystem::_thumbnailCancel()
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

        void ThumbnailSystem::_waveformCancel()
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

    } // namespace TIMELINEUI
} // namespace tl
