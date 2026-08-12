// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// Copyright (c) 2025-Present Gonzalo Garramuño
// All rights reserved.

#include "ThumbnailSystem.h"
#include "ThumbnailSystemPrivate.h"

#include <tlCore/StringFormat.h>

namespace tl
{
    namespace TIMELINEUI
    {
        namespace
        {
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

                    p.window->makeCurrent();

                    if (auto context = p.context.lock())
                    {
                        p.thumbnailThread.render = timeline_gl::Render::create(
                            context);
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
                    p.window->doneCurrent();
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
                                auto videoFuture = timeline->readMedia(request->mediaPath,
                                                                       time, request->options);
                                if (videoFuture.valid())
                                {
                                    if (p.thumbnailThread.running)
                                    {
                                        const auto videoFrame = videoFuture.get();
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
                                            p.thumbnailThread.buffer && videoFrame.image &&
                                            p.thumbnailThread.running)
                                        {
                                            gl::OffscreenBufferBinding binding(
                                                p.thumbnailThread.buffer);
                                            p.thumbnailThread.render->begin(size);
                                            const math::Matrix4x4f ortho = math::ortho(
                                                0.F, static_cast<float>(size.w),
                                                static_cast<float>(size.h), 0.F,
                                                -1.F, 1.F);
                                            p.thumbnailThread.render->setTransform(ortho);
                                            p.thumbnailThread.render->drawImage(
                                                videoFrame.image,
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
                                    }  // thread.running
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

                            // //! Video request is this:
                            // struct VideoRequest
                            // {
                            //     uint64_t id = 0;
                            //     std::future<VideoFrame> future;
                            // };
                            auto future = timeline->getVideo(
                                request->time.value_or(
                                    timeline->getTimeRange().start_time())).future;
                            timeline::VideoFrame videoFrame;
                            while (p.thumbnailThread.running)
                            {
                                std::future_status status = future.wait_for(std::chrono::milliseconds(5));

                                if (status == std::future_status::ready)
                                {
                                    videoFrame = future.get();
                                }
                            }

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
                                    const math::Matrix4x4f ortho = math::ortho(
                                        0.F, static_cast<float>(size.w),
                                        static_cast<float>(size.h), 0.F,
                                        -1.F, 1.F);
                                    p.thumbnailThread.render->setTransform(ortho);
                                    p.thumbnailThread.render->drawVideo(
                                        { videoFrame },
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

    } // namespace TIMELINEUI
} // namespace tl
