// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <mrvGL/mrvThumbnailCreator.h>

#include <tlGlad/gl.h>

#include <tlGL/Mesh.h>
#include <tlGL/OffscreenBuffer.h>
#include <tlTimeline/GLRender.h>
#include <tlGL/Shader.h>
#include <tlGL/Init.h>

#include <tlTimeline/Player.h>

#include <tlCore/StringFormat.h>

#include <thread>
#include <atomic>
#include <mutex>

#include <mrvCore/mrvSequence.h>

// mrViewer includes
#include <mrvFl/mrvIO.h>

// For main fltk event loop
#include <FL/Fl_RGB_Image.H>
#include <FL/platform.H>
#include <FL/Fl.H>

#include "mrvGLOffscreenContext.h"

namespace
{
    const char* kModule = "thumb";
}

namespace mrv
{
    using namespace tl;

    struct ThumbnailCreator::Private
    {
        std::weak_ptr<system::Context> context;

        struct Request
        {
            int64_t id;
            std::string fileName;
            std::vector< otime::RationalTime > times;
            image::Size size;
            timeline::ColorConfigOptions colorConfigOptions;
            timeline::LUTOptions lutOptions;
            uint16_t layer = 0;

            std::shared_ptr<timeline::Timeline> timeline;
            std::vector<std::future<timeline::VideoData> > futures;

            callback_t callback = nullptr;
            void* callbackData = nullptr;
        };
        std::list<Request> requests;
        std::list<Request> requestsInProgress;

        struct Result
        {
            int64_t id;
            std::vector< std::pair<otime::RationalTime, Fl_RGB_Image*> >
                thumbnails;
            callback_t callback = nullptr;
            void* callbackData = nullptr;
        };
        std::vector<Result> results;

        int64_t id = 0;
        std::vector<int64_t> cancelRequests;
        size_t requestCount = 1;
        std::chrono::milliseconds requestTimeout =
            std::chrono::milliseconds(50);
        double timerInterval = 0.005;
        std::condition_variable cv;
        std::thread* thread = nullptr;
        std::mutex mutex;
        std::atomic<bool> running;

        OffscreenContext offscreenContext;
    };

    ThumbnailCreator::ThumbnailCreator(
        const std::shared_ptr<system::Context>& context) :
        _p(new Private)
    {
        TLRENDER_P();
        p.offscreenContext.init();

        p.context = context;
        p.running = false;
    }

    ThumbnailCreator::~ThumbnailCreator()
    {
        TLRENDER_P();
        p.running = false;

        Fl::remove_timeout((Fl_Timeout_Handler)timerEvent_cb, this);

        if (p.thread && p.thread->joinable())
        {
            p.thread->join();
        }

        delete p.thread;
    }

    void ThumbnailCreator::initThread()
    {
        TLRENDER_P();
        if (p.running)
            return;

        if (!p.thread)
        {
            p.running = true;
            p.thread = new std::thread(&ThumbnailCreator::run, this);
        }

        Fl::add_timeout(
            p.timerInterval, (Fl_Timeout_Handler)timerEvent_cb, this);
    }

    void ThumbnailCreator::stopThread()
    {
        TLRENDER_P();
        if (!p.running)
            return;

        p.running = false;
        if (p.thread)
        {
            if (p.thread->joinable())
                p.thread->join();
            p.thread = nullptr;
        }

        Fl::remove_timeout((Fl_Timeout_Handler)timerEvent_cb, this);
    }

    int64_t ThumbnailCreator::request(
        const std::string& fileName, const otime::RationalTime& time,
        const image::Size& size, const callback_t callback, void* callbackData,
        const uint16_t layer,
        const timeline::ColorConfigOptions& colorConfigOptions,
        const timeline::LUTOptions& lutOptions)
    {
        TLRENDER_P();

        int64_t out = 0;
        {
            std::unique_lock<std::mutex> lock(p.mutex);
            p.id = p.id + 1;
            Private::Request request;
            request.id = p.id;
            request.fileName = fileName;
            request.times.push_back(time);
            request.size = size;
            request.layer = layer;
            request.colorConfigOptions = colorConfigOptions;
            request.lutOptions = lutOptions;
            request.callback = callback;
            request.callbackData = callbackData;
            p.requests.push_back(std::move(request));
            out = p.id;
        }
        p.cv.notify_one();
        return out;
    }

    int64_t ThumbnailCreator::request(
        const std::string& fileName,
        const std::vector<otime::RationalTime>& times, const image::Size& size,
        const callback_t callback, void* callbackData, const uint16_t layer,
        const timeline::ColorConfigOptions& colorConfigOptions,
        const timeline::LUTOptions& lutOptions)
    {
        TLRENDER_P();
        int64_t out = 0;
        {
            std::unique_lock<std::mutex> lock(p.mutex);
            p.id = p.id + 1;
            Private::Request request;
            request.id = p.id;
            request.fileName = fileName;
            request.times = times;
            request.size = size;
            request.colorConfigOptions = colorConfigOptions;
            request.lutOptions = lutOptions;
            request.layer = layer;
            request.callback = callback;
            request.callbackData = callbackData;
            p.requests.push_back(std::move(request));
            out = p.id;
        }
        p.cv.notify_one();
        return out;
    }

    void ThumbnailCreator::cancelRequests(int64_t id)
    {
        TLRENDER_P();
        std::unique_lock<std::mutex> lock(p.mutex);
        auto requestIt = p.requests.begin();
        while (requestIt != p.requests.end())
        {
            if (id == requestIt->id)
            {
                requestIt = p.requests.erase(requestIt);
                continue;
            }
            ++requestIt;
        }
        auto resultIt = p.results.begin();
        while (resultIt != p.results.end())
        {
            if (id == resultIt->id)
            {
                resultIt = p.results.erase(resultIt);
                continue;
            }
            ++resultIt;
        }
        p.cancelRequests.push_back(id);
    }

    void ThumbnailCreator::setRequestCount(int value)
    {
        TLRENDER_P();
        std::unique_lock<std::mutex> lock(p.mutex);
        p.requestCount = value > 0 ? value : 0;
    }

    void ThumbnailCreator::setRequestTimeout(int value)
    {
        TLRENDER_P();
        std::unique_lock<std::mutex> lock(p.mutex);
        p.requestTimeout = std::chrono::milliseconds(value > 0 ? value : 0);
    }

    void ThumbnailCreator::setTimerInterval(double value)
    {
        TLRENDER_P();
        p.timerInterval = value;
        Fl::repeat_timeout(value, (Fl_Timeout_Handler)timerEvent_cb, this);
    }

    void ThumbnailCreator::run()
    {
        TLRENDER_P();

        p.offscreenContext.make_current();

        tl::gl::initGLAD();

        if (auto context = p.context.lock())
        {

            auto render = timeline::GLRender::create(context);

            std::shared_ptr<gl::OffscreenBuffer> offscreenBuffer;

            while (p.running)
            {
                // std::cout << this << " running: " << p.running << std::endl;
                // std::cout << "requests: " << p.requests.size() << std::endl;
                // std::cout << "requests in progress: " <<
                // p.requestsInProgress.size() << std::endl; std::cout <<
                // "results: " << p.results.size() << std::endl;

                // Gather requests.

                std::list<Private::Request> newRequests;
                {
                    std::unique_lock<std::mutex> lock(p.mutex);
                    if (p.cv.wait_for(
                            lock, p.requestTimeout,
                            [this]
                            {
                                return !_p->requests.empty() ||
                                       !_p->requestsInProgress.empty() ||
                                       !_p->cancelRequests.empty();
                            }))
                    {
                        for (auto i : p.cancelRequests)
                        {
                            auto j = p.requestsInProgress.begin();
                            while (j != p.requestsInProgress.end())
                            {
                                if (i == j->id)
                                {
                                    j = p.requestsInProgress.erase(j);
                                    continue;
                                }
                                ++j;
                            }
                        }
                        p.cancelRequests.clear();
                        while (!p.requests.empty() &&
                               (p.requestsInProgress.size() +
                                newRequests.size()) < p.requestCount)
                        {
                            newRequests.push_back(
                                std::move(p.requests.front()));
                            p.requests.pop_front();
                        }
                    }
                }

                // Initialize new requests.
                for (auto& request : newRequests)
                {
                    timeline::Options options;
                    options.videoRequestCount = 1;
                    options.audioRequestCount = 1;
                    options.requestTimeout = std::chrono::milliseconds(25);
                    options.ioOptions["SequenceIO/ThreadCount"] =
                        string::Format("{0}").arg(1);
                    options.ioOptions["FFmpeg/ThreadCount"] =
                        string::Format("{0}").arg(1);
                    try
                    {
                        request.timeline = timeline::Timeline::create(
                            request.fileName, context, options);
                        for (const auto& i : request.times)
                        {
                            request.futures.push_back(
                                request.timeline->getVideo(
                                    time::isValid(i)
                                        ? i
                                        : request.timeline->getTimeRange()
                                              .start_time(),
                                    request.layer));
                        }
                        p.requestsInProgress.push_back(std::move(request));
                    }
                    catch (const std::exception& e)
                    {
                        LOG_ERROR(e.what());
                    }
                }

                // Check for finished requests.
                std::vector<Private::Result> results;
                auto requestIt = p.requestsInProgress.begin();
                while (requestIt != p.requestsInProgress.end())
                {
                    auto futureIt = requestIt->futures.begin();
                    while (futureIt != requestIt->futures.end())
                    {
                        if (futureIt->valid() &&
                            futureIt->wait_for(std::chrono::seconds(0)) ==
                                std::future_status::ready)
                        {
                            const int depth = 4;
                            const auto videoData = futureIt->get();
                            const image::Info info(
                                requestIt->size.w, requestIt->size.h,
                                image::PixelType::RGBA_U8);
                            uint8_t* pixelData = new uint8_t
                                [static_cast<size_t>(info.size.w) *
                                 static_cast<size_t>(info.size.h) * depth];

                            try
                            {
                                math::Size2i offscreenBufferSize(
                                    info.size.w, info.size.h);
                                gl::OffscreenBufferOptions
                                    offscreenBufferOptions;

                                offscreenBufferOptions.colorType =
                                    image::PixelType::RGBA_U8;

                                if (gl::doCreate(
                                        offscreenBuffer, offscreenBufferSize,
                                        offscreenBufferOptions))
                                {
                                    offscreenBuffer =
                                        gl::OffscreenBuffer::create(
                                            offscreenBufferSize,
                                            offscreenBufferOptions);
                                }

                                timeline::ImageOptions i;
                                timeline::DisplayOptions d;
                                d.mirror.y = true; // images in GL are flipped

                                gl::OffscreenBufferBinding binding(
                                    offscreenBuffer);

                                char* saved_locale =
                                    strdup(setlocale(LC_NUMERIC, NULL));
                                setlocale(LC_NUMERIC, "C");
                                render->begin(
                                    offscreenBufferSize,
                                    requestIt->colorConfigOptions,
                                    requestIt->lutOptions);
                                render->drawVideo(
                                    {videoData},
                                    {math::Box2i(
                                        0, 0, info.size.w, info.size.h)},
                                    {i}, {d});
                                render->end();
                                setlocale(LC_NUMERIC, saved_locale);
                                free(saved_locale);

                                glPixelStorei(GL_PACK_ALIGNMENT, 1);
                                glReadPixels(
                                    0, 0, info.size.w, info.size.h, GL_RGBA,
                                    GL_UNSIGNED_BYTE, pixelData);
                            }
                            catch (const std::exception& e)
                            {
                                LOG_ERROR(e.what());
                            }

                            const auto rgbImage = new Fl_RGB_Image(
                                pixelData, info.size.w, info.size.h, depth);
                            rgbImage->alloc_array = true;
                            {
                                const auto i = std::find_if(
                                    results.begin(), results.end(),
                                    [&requestIt](const Private::Result& value)
                                    { return requestIt->id == value.id; });
                                if (i == results.end())
                                {
                                    Private::Result result;
                                    result.id = requestIt->id;
                                    result.thumbnails = {std::make_pair(
                                        videoData.time, rgbImage)};
                                    result.callback = requestIt->callback;
                                    result.callbackData =
                                        requestIt->callbackData;
                                    results.push_back(result);
                                }
                                else
                                {
                                    i->thumbnails.push_back(std::make_pair(
                                        videoData.time, rgbImage));
                                }
                            }

                            futureIt = requestIt->futures.erase(futureIt);
                            continue;
                        }
                        ++futureIt;
                    }
                    if (requestIt->futures.empty())
                    {
                        requestIt = p.requestsInProgress.erase(requestIt);
                        continue;
                    }
                    ++requestIt;
                }

                {
                    std::unique_lock<std::mutex> lock(p.mutex);
                    p.results.insert(
                        p.results.end(), results.begin(), results.end());
                }
            } // p.running
        }

        p.offscreenContext.release();
    }

    void ThumbnailCreator::timerEvent()
    {
        TLRENDER_P();
        std::vector<Private::Result> results;
        {
            std::unique_lock<std::mutex> lock(p.mutex);
            results.swap(p.results);
        }
        for (auto& i : results)
        {
            i.callback(i.id, i.thumbnails, i.callbackData);
        }
        if (p.running)
        {
            Fl::repeat_timeout(
                p.timerInterval, (Fl_Timeout_Handler)timerEvent_cb, this);
        }
    }

    void ThumbnailCreator::timerEvent_cb(void* d)
    {
        ThumbnailCreator* t = static_cast< ThumbnailCreator* >(d);
        t->timerEvent();
    }

} // namespace mrv
