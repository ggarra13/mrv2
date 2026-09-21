
#include "ThumbnailSystem.h"
#include "ThumbnailSystemPrivate.h"

#include <tlCore/AudioResample.h>

#define DBG std::cerr << __FUNCTION__ << " " << __LINE__ << std::endl;

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


            std::string getWaveformKey(
                const file::Path& path,
                const file::Path& mediaPath,
                const std::string& mediaReferenceKey,
                const math::Size2i& size,
                const std::optional<otio::TimeRange>& timeRange,
                const io::Options& options)
            {
                std::stringstream ss;
                ss << path.get() << ";" << mediaPath.get()  << ";"
                   << mediaReferenceKey << ";" <<
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
        } // namespace

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
            // options.threaded = false;  // \@bug: \@note: was false, now true
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

        ThumbnailSystem::~ThumbnailSystem()
        {
            DBG;
            shutdown();
            DBG;
        }

        void ThumbnailSystem::shutdown()
        {
            TLRENDER_P();
            DBG;

            p.infoThread.running = false;
            if (p.infoThread.thread.joinable())
            {
                p.infoThread.thread.join();
            }
            DBG;

            p.thumbnailThread.running = false;
            if (p.thumbnailThread.thread.joinable())
            {
                p.thumbnailThread.thread.join();
            }
            DBG;

            p.waveformThread.running = false;
            if (p.waveformThread.thread.joinable())
            {
                p.waveformThread.thread.join();
            }
            DBG;
        }

        void
        ThumbnailSystem::cancelRequests(const std::vector<uint64_t>& ids)
        {
            TLRENDER_P();

            //
            // Fill up the promises.
            //
            _infoCancel();
            _thumbnailCancel();
            _waveformCancel();

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
            {
                // Acquire the cache lock to safely iterate over the shared
                // Timeline instances.
                std::unique_lock<std::mutex> lock(p.ioCacheMutex);

                // p.ioCache.getValues() is a list of timelines.
                for (const auto& timeline : p.ioCache.getValues())
                {
                    timeline->cancelRequests(ids);
                }
            }
        }

        ThumbnailRequest ThumbnailSystem::getThumbnail(
            const file::Path& path, int height,
            const std::optional<otio::RationalTime>& time,
            const std::string& mediaReferenceKey, const io::Options& options)
        {
            return getThumbnail(path, path, height, time, mediaReferenceKey,
                                options);
        }


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
                mediaReferenceKey,
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
                        request->mediaReferenceKey,
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

    }
}
