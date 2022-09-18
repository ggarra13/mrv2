// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <mrvFl/mrvThumbnailProvider.h>

#include <tlGL/Mesh.h>
#include <tlGL/OffscreenBuffer.h>
#include <tlGL/Render.h>
#include <tlGL/Shader.h>
#include <tlGL/Util.h>

#include <tlTimeline/TimelinePlayer.h>

#include <tlCore/StringFormat.h>


#include <thread>
#include <atomic>
#include <mutex>


#include <mrvCore/mrvSequence.h>

// mrViewer includes
#include <mrvFl/mrvIO.h>


// For main fltk event loop
#include <FL/Fl_RGB_Image.H>
#include <FL/Fl.H>

namespace {
    const char* kModule = "thumb";
}

namespace mrv
{
    using namespace tl;

    struct ThumbnailProvider::Private
    {
        std::weak_ptr<system::Context> context;

        struct Request
        {
            int64_t id;
            std::string fileName;
            std::vector< otime::RationalTime > times;
            imaging::Size size;
            timeline::ColorConfigOptions colorConfigOptions;
            timeline::LUTOptions lutOptions;

            std::shared_ptr<timeline::Timeline> timeline;
            std::vector<std::future<timeline::VideoData> > futures;

            callback_t callback = nullptr;
            void*      callbackData = nullptr;
        };
        std::list<Request> requests;
        std::list<Request> requestsInProgress;

        struct Result
        {
            int64_t id;
            std::vector< std::pair<otime::RationalTime, Fl_RGB_Image*> > thumbnails;
            callback_t callback = nullptr;
            void*      callbackData = nullptr;
        };
        std::vector<Result> results;

        callback_t callback = nullptr;
        void*      callbackData = nullptr;

        int64_t id = 0;
        std::vector<int64_t> cancelRequests;
        size_t requestCount = 1;
        std::chrono::milliseconds requestTimeout = std::chrono::milliseconds(50);
        double timerInterval = 0.005;
        std::condition_variable cv;
        std::thread* thread = nullptr;
        std::mutex mutex;
        std::atomic<bool> running;
    };


    ThumbnailProvider::ThumbnailProvider(
        const std::shared_ptr<system::Context>& context ) :
        Fl_Gl_Window( 0, 0 ),
        _p( new Private )
    {
        mode( FL_RGB | FL_ALPHA | FL_OPENGL3 );
        TLRENDER_P();

        border(0);
        p.context = context;
        p.running = false;

        end();

        // We create a window but we never show it, as we just need the
        // GL context, which we copy from the main window in the run() method
    }


    ThumbnailProvider::~ThumbnailProvider()
    {
        TLRENDER_P();
        DBGM1( this );
        p.running = false;
        Fl::remove_timeout( (Fl_Timeout_Handler) timerEvent_cb, this );
        DBGM1( this );
        p.thread->join();
        DBGM1( this );
        delete p.thread;
        DBGM1( this );
    }


    void
    ThumbnailProvider::initThread()
    {
        TLRENDER_P();
        if ( p.running ) return;


        p.running = true;
        p.thread  = new std::thread( &ThumbnailProvider::run, this );

        Fl::add_timeout(p.timerInterval,
                        (Fl_Timeout_Handler) timerEvent_cb, this );
    }


    int64_t ThumbnailProvider::request(
        const std::string& fileName,
        const otime::RationalTime& time,
        const imaging::Size& size,
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
            request.colorConfigOptions = colorConfigOptions;
            request.lutOptions = lutOptions;
            request.callback   = p.callback;
            request.callbackData = p.callbackData;
            p.requests.push_back(std::move(request));
            out = p.id;
        }
        p.cv.notify_one();
        return out;
    }

    int64_t ThumbnailProvider::request(
        const std::string& fileName,
        const std::vector<otime::RationalTime>& times,
        const imaging::Size& size,
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
            request.callback   = p.callback;
            request.callbackData = p.callbackData;
            p.requests.push_back(std::move(request));
            out = p.id;
        }
        p.cv.notify_one();
        return out;
    }

    void ThumbnailProvider::cancelRequests(int64_t id)
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

    void ThumbnailProvider::setCallback( callback_t func, void* data )
    {
        TLRENDER_P();
        p.callback = func;
        p.callbackData = data;
    }

    void ThumbnailProvider::setRequestCount(int value)
    {
        TLRENDER_P();
        std::unique_lock<std::mutex> lock(p.mutex);
        p.requestCount = value > 0 ? value : 0;
    }

    void ThumbnailProvider::setRequestTimeout(int value)
    {
        TLRENDER_P();
        std::unique_lock<std::mutex> lock(p.mutex);
        p.requestTimeout = std::chrono::milliseconds(value > 0 ? value : 0);
    }

    void ThumbnailProvider::setTimerInterval(double value)
    {
        TLRENDER_P();
        p.timerInterval = value;
        Fl::repeat_timeout(value, (Fl_Timeout_Handler) timerEvent_cb, this );
    }

    void ThumbnailProvider::run()
    {
        TLRENDER_P();


        Fl_Window* w = nullptr;
        for ( w = Fl::first_window(); w; w = Fl::next_window(w) )
        {
            Fl_Gl_Window* gl = dynamic_cast< Fl_Gl_Window* >(w);
            if ( !gl || !gl->context() ) continue;
            // Share GL context with a main window
            this->context( gl->context(), false );
        }

        make_current();

        if ( ! valid() )
        {
            gladLoaderLoadGL();
            valid(1);
        }

        if (auto context = p.context.lock())
        {
            auto render = gl::Render::create(context);

            std::shared_ptr<gl::OffscreenBuffer> offscreenBuffer;

            while (p.running)
            {
                // std::cout << "running: " << p.running << std::endl;
                // std::cout << "requests: " << p.requests.size() << std::endl;
                // std::cout << "requests in progress: " << p.requestsInProgress.size() << std::endl;
                // std::cout << "results: " << p.results.size() << std::endl;

                // Gather requests.

                std::list<Private::Request> newRequests;
                {
                    std::unique_lock<std::mutex> lock(p.mutex);
                    if (p.cv.wait_for(
                            lock,
                            p.requestTimeout,
                            [this]
                                {
                                    return
                                        !_p->requests.empty() ||
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
                               (p.requestsInProgress.size() + newRequests.size()) < p.requestCount)
                        {
                            newRequests.push_back(std::move(p.requests.front()));
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
                    options.requestTimeout = std::chrono::milliseconds(100);
                    options.ioOptions["SequenceIO/ThreadCount"] = string::Format("{0}").arg(1);
                    options.ioOptions["ffmpeg/ThreadCount"] = string::Format("{0}").arg(1);
                    try
                    {
                        request.timeline =
                            timeline::Timeline::create(request.fileName,
                                                       context, options);
                    }
                    catch( const std::runtime_error& e )
                    {
                        std::string what = request.fileName + ": " + e.what();
                        context->log( kModule, what, log::Type::Error);
                        continue;
                    }



                    otime::TimeRange timeRange;
                    if (!request.times.empty())
                    {
                        otio::RationalTime start =
                            request.timeline->getGlobalStartTime();
                        otio::RationalTime duration =
                            request.timeline->getDuration();
                        if ( request.times[0] < start ||
                             request.times[0] > start + duration)
                            request.times[0] = start;
                        timeRange = otime::TimeRange(request.times[0],
                                                     otime::RationalTime(1.0, request.times[0].rate()));
                        for (size_t i = 1; i < request.times.size(); ++i)
                        {
                            timeRange = timeRange.extended_by(
                                otime::TimeRange(request.times[i], otime::RationalTime(1.0, request.times[i].rate())));
                        }
                        request.timeline->setActiveRanges({ timeRange });
                    }
                    for (const auto& i : request.times)
                    {
                        request.futures.push_back(request.timeline->getVideo(i));
                    }
                    p.requestsInProgress.push_back(std::move(request));
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
                            futureIt->wait_for(std::chrono::seconds(0)) == std::future_status::ready)
                        {
                            const int depth = 4;
                            const auto videoData = futureIt->get();
                            const imaging::Info info(
                                requestIt->size.w,
                                requestIt->size.h,
                                imaging::PixelType::RGBA_U8);
                            uint8_t* pixelData = new uint8_t[
                                static_cast<size_t>(info.size.w) *
                                static_cast<size_t>(info.size.h) * depth];

                            try
                            {
                                gl::OffscreenBufferOptions offscreenBufferOptions;

                                offscreenBufferOptions.colorType = imaging::PixelType::RGBA_U8;

                                if (gl::doCreate(offscreenBuffer, info.size, offscreenBufferOptions))
                                {
                                    DBGM1( this );


                                    offscreenBuffer = gl::OffscreenBuffer::create(info.size, offscreenBufferOptions);
                                    DBGM1( this );
                                }

                                timeline::ImageOptions i;
                                timeline::DisplayOptions d;
                                d.mirror.y = true;  // images in GL are flipped
                                render->setColorConfig(requestIt->colorConfigOptions);
                                render->setLUT(requestIt->lutOptions);

                                gl::OffscreenBufferBinding binding(offscreenBuffer);

                                render->begin(info.size);
                                render->drawVideo(
                                    { videoData },
                                    { math::BBox2i(0, 0,
                                                   info.size.w, info.size.h) },
                                    { i }, { d });
                                render->end();

                                glPixelStorei(GL_PACK_ALIGNMENT, 1);
                                glReadPixels(
                                    0,
                                    0,
                                    info.size.w,
                                    info.size.h,
                                    GL_RGBA,
                                    GL_UNSIGNED_BYTE,
                                    pixelData);


                            }
                            catch (const std::exception& e)
                            {
                                context->log( kModule, e.what(),
                                              log::Type::Error );
                            }


                            const auto flImage = new Fl_RGB_Image(
                                pixelData,
                                info.size.w,
                                info.size.h,
                                depth );
                            flImage->alloc_array = true;
                            {
                                const auto i = std::find_if(
                                    results.begin(),
                                    results.end(),
                                    [&requestIt](const Private::Result& value)
                                        {
                                            return requestIt->id == value.id;
                                        });
                                if (i == results.end())
                                {
                                    Private::Result result;
                                    result.id = requestIt->id;
                                    result.thumbnails = { std::make_pair(videoData.time, flImage) };
                                    result.callback = requestIt->callback;
                                    result.callbackData = requestIt->callbackData;
                                    results.push_back( result );
                                }
                                else
                                {
                                    i->thumbnails.push_back(
                                        std::make_pair(videoData.time, flImage));
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
                    p.results.insert(p.results.end(), results.begin(), results.end());
                }
            }  // p.running
        }

    }


    void ThumbnailProvider::timerEvent()
    {
        TLRENDER_P();
        std::vector<Private::Result> results;
        {
            std::unique_lock<std::mutex> lock(p.mutex);
            results.swap(p.results);
        }
        for (auto& i : results)
        {
            if ( ! i.callback )
            {
                DBGM1( "CALLBACK IS EMPTY!!!!" );
                continue;
            }
            i.callback(i.id, i.thumbnails, i.callbackData);
        }
        if ( p.running )
        {
            Fl::repeat_timeout( p.timerInterval,
                                (Fl_Timeout_Handler) timerEvent_cb, this );
        }
    }

    void ThumbnailProvider::timerEvent_cb( void* d )
    {
        ThumbnailProvider* t = static_cast< ThumbnailProvider* >( d );
        t->timerEvent();
    }

}
