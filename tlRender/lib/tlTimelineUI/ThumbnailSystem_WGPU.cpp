// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// Copyright (c) 2025-Present Gonzalo Garramuño
// All rights reserved.

#include "ThumbnailSystem.h"
#include "ThumbnailSystemPrivate.h"

#include <tlTimeline/Timeline.h>

#include <tlIO/System.h>

#include <tlCore/AudioResample.h>
#include <tlCore/LRUCache.h>
#include <tlCore/StringFormat.h>
#include <tlCore/Timer.h>

#include <chrono>
#include <cstring>
#include <functional>
#include <sstream>
#include <thread>

#if defined(__EMSCRIPTEN__)
#    include <emscripten/proxying.h>
#    include <emscripten/threading.h>
#endif

namespace tl
{
    namespace timelineui
    {
        namespace
        {
            std::string getThumbnailKey(
                const file::Path& path,
                const file::Path& mediaPath,
                const std::string& mediaReferenceKey,
                int height,
                const std::optional<OTIO_NS::RationalTime>& time,
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

            //! Configure the thumbnail renderer for an image: HDR sources
            //! are tone-mapped to SDR using their static metadata, SDR
            //! sources go through untouched.  Must be called on the
            //! thumbnail thread, after Render::begin() and before drawing.
            void setThumbnailHDR(
                const std::shared_ptr<timeline_wgpu::Render>& render,
                const std::shared_ptr<image::Image>& image)
            {
                timeline::HDROptions hdr;
                if (image && image->getHDR())
                {
                    const auto data = *(image->getHDR());
                    const bool isHDR =
                        data.eotf == image::EOTFType::EOTF_BT2100_PQ ||
                        data.eotf == image::EOTFType::EOTF_BT2020 ||
                        data.eotf == image::EOTFType::EOTF_BT2100_HLG;
                    hdr.hdrData = data;
                    hdr.tonemap = isHDR;
                }
                // Static metadata only: no per thumbnail peak detection
                // compute pass, and no state carried between unrelated
                // frames.
                hdr.peak_detection = false;
                hdr.algorithm = timeline::HDRTonemapAlgorithm::Spline;
                render->setHDROptions(hdr);
            }

            //! A WebGPU command encoder is single use: it is created per
            //! thumbnail and finished by OffscreenBuffer::submitReadback().
            //! This releases our reference whichever way the scope is left
            //! (replaces the Vulkan command pool + reusable command buffer).
            class CommandEncoderGuard
            {
            public:
                explicit CommandEncoderGuard(WGPUDevice device)
                {
                    WGPUCommandEncoderDescriptor desc = {};
                    desc.label = {"Thumbnail command encoder", WGPU_STRLEN};
                    encoder = wgpuDeviceCreateCommandEncoder(device, &desc);
                }

                ~CommandEncoderGuard()
                {
                    if (encoder)
                        wgpuCommandEncoderRelease(encoder);
                }

                CommandEncoderGuard(const CommandEncoderGuard&) = delete;
                CommandEncoderGuard& operator=(const CommandEncoderGuard&) =
                    delete;

                WGPUCommandEncoder encoder = nullptr;
            };

            //! How long to wait for a read-back before giving up and
            //! returning a black thumbnail.
            constexpr std::chrono::seconds readbackTimeout(5);

            //! Run a function on the thread that owns the WebGPU device and
            //! wait for it to finish.
            //!
            //! Under Emscripten, WebGPU handles are indices into a
            //! per-thread JavaScript object table, so a WGPUDevice (or any
            //! object made from it) is only valid on the thread that created
            //! the context.  std::thread is a Web Worker there, so every
            //! WebGPU call made by the thumbnail thread has to be proxied.
            //! The function must not block waiting on a WebGPU callback
            //! (buffer map): that thread has to return to its event loop for
            //! the callback to fire.
            //!
            //! \todo If the context is created on a thread other than the
            //! browser's main thread (PROXY_TO_PTHREAD), target that thread
            //! instead of emscripten_main_runtime_thread_id().
#if defined(__EMSCRIPTEN__)
            void gpuTrampoline(void* arg)
            {
                (*static_cast<std::function<void()>*>(arg))();
            }

            void runOnGPUThread(std::function<void()> func)
            {
                const pthread_t target = emscripten_main_runtime_thread_id();
                if (pthread_equal(pthread_self(), target))
                {
                    func();
                    return;
                }
                emscripten_proxy_sync(
                    emscripten_proxy_get_system_queue(), target, gpuTrampoline,
                    &func);
            }
#else
            inline void runOnGPUThread(std::function<void()> func)
            {
                func();
            }
#endif
        }

        ThumbnailSystem::ThumbnailSystem(
            const std::shared_ptr<system::Context>& context,
            Fl_WebGPU_Context& wgpu_ctx) :
            ISystem::ISystem(context, "tl::timelineui::ThumbnailSystem"),
            ctx(wgpu_ctx),
            _p(new Private)
        {
            TLRENDER_P();

            p.context = context;

            p.cacheOptions = observer::Value<ThumbnailCacheOptions>::create();

            _startThreads();
        }

        ThumbnailRequest ThumbnailSystem::getThumbnail(
            const file::Path& path,
            const file::Path& mediaPath,
            int height,
            const std::optional<OTIO_NS::RationalTime>& time,
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
                {
                    //true; // left blank on purpose
                }
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
                    if (auto context = p.context.lock())
                    {
                        // Wait for the WebGPU context (device, queue and
                        // instance) to be created by the main window.
                        while ((ctx.queue() == nullptr ||
                                ctx.device == nullptr ||
                                ctx.instance == nullptr) &&
                               p.thumbnailThread.running)
                        {
                            std::this_thread::sleep_for(
                                std::chrono::milliseconds(1));
                        }

                        // No command pool / command buffer to create: WebGPU
                        // command encoders are created per thumbnail.
                        if (p.thumbnailThread.running)
                        {
                            runOnGPUThread(
                                [&]
                                {
                                    p.thumbnailThread.render =
                                        timeline_wgpu::Render::create(
                                            ctx, context);
                                });

                            // The thumbnail target is 8-bit SDR whatever the
                            // main window's surface is.  The offscreen buffer
                            // below is always RGBA_U8, so this is not needed
                            // unless Render grows an SDR-output switch.
                            // p.thumbnailThread.render->setSDROutput(true);
                        }
                    }
                    if (p.thumbnailThread.render)
                    {
                        _thumbnailRun();
                    }
                    {
                        std::unique_lock<std::mutex> lock(p.thumbnailMutex.mutex);
                        p.thumbnailMutex.stopped = true;
                    }
                    _thumbnailCancel();

                    // No vkDeviceWaitIdle() equivalent is needed: every
                    // request waits for its own read-back before it
                    // completes, and WebGPU keeps resources alive until the
                    // GPU has finished with them.
                    // Destroy the WebGPU objects on the thread that owns them.
                    runOnGPUThread(
                        [&]
                        {
                            p.thumbnailThread.buffer.reset();
                            p.thumbnailThread.render.reset();
                        });
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

        std::shared_ptr<ThumbnailSystem> ThumbnailSystem::create(
            const std::shared_ptr<system::Context>& context,
            Fl_WebGPU_Context& ctx)
        {
            auto out = context->getSystem<ThumbnailSystem>();
            if (!out)
            {
                out = std::shared_ptr<ThumbnailSystem>(new ThumbnailSystem(context, ctx));
                context->addSystem(out);
            }
            return out;
        }


        void ThumbnailSystem::_thumbnailRun()
        {
            TLRENDER_P();

            //! Render one frame into the thumbnail buffer and read it back.
            //! Returns nullptr if the thread was stopped while waiting.
            //! Both branches below (media in the timeline, or the timeline
            //! itself) used to repeat this sequence.
            auto renderThumbnail =
                [&](const math::Size2i& size,
                    const timeline::VideoFrame& frame,
                    const std::shared_ptr<image::Image>& hdrImage)
                -> std::shared_ptr<image::Image>
            {
                if (!p.thumbnailThread.running)
                    return nullptr;

                auto image = image::Image::create(
                    size.w, size.h, image::PixelType::RGBA_U8);

                // Part 1 (GPU thread, does not block): create the buffer if
                // needed, render, record the copy and submit.
                bool submitted = false;
                runOnGPUThread(
                    [&]
                    {
                        wgpu::OffscreenBufferOptions options;
                        options.colorType = image::PixelType::RGBA_U8;
                        options.pbo = true;
                        if (wgpu::doCreate(
                                p.thumbnailThread.buffer, size, options))
                        {
                            p.thumbnailThread.buffer =
                                wgpu::OffscreenBuffer::create(
                                    ctx, size, options);
                        }
                        if (!p.thumbnailThread.render ||
                            !p.thumbnailThread.buffer)
                        {
                            return;
                        }

                        CommandEncoderGuard guard(ctx.device);
                        WGPUCommandEncoder& encoder = guard.encoder;
                        if (!encoder)
                            return;

                        timeline::RenderOptions renderOptions;
                        renderOptions.clear = true;
                        // Float intermediate buffer so HDR highlights survive
                        // until tone mapping.
                        renderOptions.colorBuffer = image::PixelType::RGBA_F16;
                        p.thumbnailThread.render->begin(
                            encoder, p.thumbnailThread.buffer,
                            p.thumbnailThread.frameIndex, size,
                            renderOptions);

                        const math::Matrix4x4f ortho = math::ortho(
                            0.F, static_cast<float>(size.w),
                            0.F, static_cast<float>(size.h),
                            -1.F, 1.F);
                        p.thumbnailThread.render->setTransform(ortho);

                        // Go through the display shader (drawVideo), as
                        // drawImage skips tone mapping.
                        setThumbnailHDR(p.thumbnailThread.render, hdrImage);
                        p.thumbnailThread.render->drawVideo(
                            {frame}, {math::Box2i(0, 0, size.w, size.h)});
                        p.thumbnailThread.render->end();

                        p.thumbnailThread.buffer->readPixels(
                            encoder, 0, 0, size.w, size.h);
                        p.thumbnailThread.buffer->submitReadback(encoder);
                        submitted = true;
                    });

                if (!submitted)
                {
                    fprintf(stderr, "not submitted\n");
                    return nullptr;
                }

                // Part 2: poll for the mapped pixels.  Each poll runs on the
                // GPU thread (getLatestReadPixels() pumps the instance events
                // there), then returns so its event loop can deliver the map
                // callback.  The pixels are copied out inside the call.
                bool ready = false;
                const auto start = std::chrono::steady_clock::now();
                while (p.thumbnailThread.running)
                {
                    runOnGPUThread(
                        [&]
                        {
                            void* imageData = nullptr;
                            if (p.thumbnailThread.buffer->getLatestReadPixels(
                                    imageData) &&
                                imageData)
                            {
                                std::memcpy(
                                    image->getData(), imageData,
                                    image->getDataByteCount());
                                ready = true;
                            }
                        });
                    if (ready)
                        break;
                    if (std::chrono::steady_clock::now() - start >
                        readbackTimeout)
                    {
                        break;
                    }
                    std::this_thread::sleep_for(
                        std::chrono::milliseconds(1));
                }

                // Safely bail if the thread is dying.
                if (!p.thumbnailThread.running)
                    return nullptr;

                if (!ready)
                {
                    std::memset(
                        image->getData(), 0, image->getDataByteCount());
                    fprintf(stderr, "not ready\n");
                    return nullptr;
                }

                p.thumbnailThread.frameIndex++;
                return image;
            };

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
                                const math::Size2i
                                    tmp(info.video[0].size.w *
                                        info.video[0].size.pixelAspectRatio,
                                        info.video[0].size.h);
                                size.w = request->height * math::aspectRatio(tmp);
                                size.h = request->height;
                            }

                            if (size.isValid())
                            {
                                const OTIO_NS::RationalTime time =
                                    request->time.value_or(info.videoTime->start_time());
                                auto videoFuture = timeline->readMedia(request->mediaPath,
                                                                       time, request->options);
                                if (videoFuture.valid())
                                {
                                    io::VideoData videoData;
                                    while (p.thumbnailThread.running)
                                    {
                                        std::future_status status = videoFuture.wait_for(std::chrono::milliseconds(5));
                                        if (status == std::future_status::ready)
                                        {
                                            videoData = videoFuture.get();
                                            break;
                                        }
                                    }
                                    if (!p.thumbnailThread.running)
                                    {
                                        request->promise.set_value(nullptr);
                                        break; // unwind without touching the buffer/render objects
                                    }

                                    if (videoData.image)
                                    {
                                        timeline::VideoLayer layer;
                                        layer.image = videoData.image;
                                        timeline::VideoFrame frame;
                                        frame.layers.push_back(layer);

                                        image = renderThumbnail(
                                            size, frame, videoData.image);

                                        if (!p.thumbnailThread.running)
                                        {
                                            request->promise.set_value(nullptr);
                                            break;
                                        }
                                    }
                                }  // videoFuture valid
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
                            const bool mediaAsked =
                                request->mediaPath.get() != request->path.get();
                            auto logSystem = context->getLogSystem();
                            if (mediaAsked && logSystem)
                            {
                                logSystem->print("tl::ui::ThumbnailSystem",
                                                 string::Format("Media not found in timeline, "
                                                                "using \"{0}\" in timeline "
                                                                "in \"{1}\"").
                                                 arg(request->mediaPath.get()).
                                                 arg(request->path.get()),
                                                 log::Type::Warning);
                            }

                            const auto info = timeline->getIOInfo();
                            auto videoFuture = timeline->getVideo(
                                request->time.value_or(
                                    timeline->getTimeRange().start_time())).future;
                            timeline::VideoFrame videoFrame;
                            while (p.thumbnailThread.running)
                            {
                                std::future_status status = videoFuture.wait_for(std::chrono::milliseconds(5));

                                if (status == std::future_status::ready)
                                {
                                    videoFrame = videoFuture.get();
                                    break;
                                }
                            }

                            math::Size2i size;
                            if (!info.video.empty())
                            {
                                const math::Size2i
                                    tmp(info.video[0].size.w *
                                        info.video[0].size.pixelAspectRatio,
                                        info.video[0].size.h);
                                size.w = request->height * math::aspectRatio(tmp);
                                size.h = request->height;
                            }
                            if (size.isValid() && p.thumbnailThread.running)
                            {
                                std::shared_ptr<image::Image> hdrImage;
                                if (!videoFrame.layers.empty())
                                    hdrImage = videoFrame.layers[0].image;

                                image = renderThumbnail(
                                    size, videoFrame, hdrImage);
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

    } // namespace timelineui
} // namespace tl
