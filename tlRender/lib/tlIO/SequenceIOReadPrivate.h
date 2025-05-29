// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlIO/SequenceIO.h>

#include <atomic>
#include <condition_variable>
#include <list>
#include <mutex>
#include <thread>

namespace tl
{
    namespace io
    {
        struct ISequenceRead::Private
        {
            void addTags(Info&);

            size_t threadCount = sequenceThreadCount;

            Info info;

            struct InfoRequest
            {
                InfoRequest() {}
                InfoRequest(InfoRequest&&) = default;

                std::promise<Info> promise;
            };

            struct VideoRequest
            {
                VideoRequest() {}
                VideoRequest(VideoRequest&&) = default;

                otime::RationalTime time = time::invalidTime;
                Options options;
                std::promise<VideoData> promise;
                std::future<VideoData> future;
            };

            struct Mutex
            {
                std::list<std::shared_ptr<InfoRequest> > infoRequests;
                std::list<std::shared_ptr<VideoRequest> > videoRequests;
                bool stopped = false;
                std::mutex mutex;
            };
            Mutex mutex;

            struct Thread
            {
                std::list<std::shared_ptr<VideoRequest> >
                    videoRequestsInProgress;
                std::chrono::steady_clock::time_point logTimer;
                std::condition_variable cv;
                std::thread thread;
                std::atomic<bool> running;
            };
            Thread thread;
        };
    } // namespace io
} // namespace tl
