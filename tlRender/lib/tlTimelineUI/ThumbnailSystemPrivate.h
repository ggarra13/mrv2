// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// Copyright (c) 2025-Present Gonzalo Garramuño
// All rights reserved.

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

#include <tlTimeline/Timeline.h>

#include <tlIO/System.h>

#include <tlCore/LRUCache.h>
#include <tlCore/Timer.h>

namespace tl
{
    namespace TIMELINEUI
    {
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
        }

        std::shared_ptr<timeline::Timeline> getTimeline(
            const std::shared_ptr<system::Context>& context,
            memory::LRUCache<std::string, std::shared_ptr<timeline::Timeline> >& cache,
            std::mutex& mutex,
            const file::Path& path);

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
                file::Path audioPath;
                io::Options options;
                std::promise<io::Info> promise;
            };

            struct ThumbnailRequest
            {
                uint64_t id = 0;
                file::Path path;
                file::Path mediaPath;
                file::Path audioPath;
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

    } // namespace TIMELINEUI
} // namespace tl
