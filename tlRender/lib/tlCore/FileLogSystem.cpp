// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlCore/FileLogSystem.h>

#include <tlCore/Context.h>
#include <tlCore/FileIO.h>
#include <tlCore/Time.h>

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <thread>

namespace tl
{
    namespace file
    {
        namespace
        {
            const std::chrono::milliseconds timeout(1000);
        }

        struct FileLogSystem::Private
        {
            std::filesystem::path path;

            std::shared_ptr<observer::ListObserver<log::Item> > logObserver;

            struct Mutex
            {
                std::vector<log::Item> items;
                std::mutex mutex;
            };
            Mutex mutex;

            struct Thread
            {
                std::thread thread;
                std::atomic<bool> running;
                std::condition_variable stop;
                std::mutex stopMutex;
            };
            Thread thread;
        };

        void FileLogSystem::_init(
            const std::shared_ptr<system::Context>& context,
            const std::filesystem::path& path)
        {
            ICoreSystem::_init(context, "tl::file:::FileLogSystem");
            TLRENDER_P();

            p.path = path;

            p.logObserver = observer::ListObserver<log::Item>::create(
                context->getSystem<log::System>()->observeLog(),
                [this](const std::vector<log::Item>& value)
                    {
                        std::unique_lock<std::mutex> lock(_p->mutex.mutex);
                        _p->mutex.items.insert(
                            _p->mutex.items.end(), value.begin(), value.end());
                    });

            p.thread.running = true;
            p.thread.thread = std::thread(
                [this]
                    {
                        TLRENDER_P();
                        // Writing the log must not be able to take the application
                        // down with it. The file can be locked or replaced underneath
                        // us at any moment -- a folder synced to a cloud drive does
                        // exactly that -- and an exception leaving this thread would
                        // call std::terminate. Reported in #549, where the
                        // application aborted every few seconds because the documents
                        // folder was inside a synced folder.
                        //
                        // A batch that cannot be written is dropped rather than kept
                        // to try again, so that a folder that never becomes writable
                        // does not grow the queue without bound. How many were lost
                        // is said in the file once one can be written again, since a
                        // gap in a log otherwise looks like nothing happened.
                        size_t dropped = 0;
                        const auto write =
                            [&p, &dropped](const std::vector<log::Item>& items)
                                {
                                    if (items.empty() && 0 == dropped)
                                        return;
                                    try
                                    {
                                        auto io = FileIO::create(p.path, file::Mode::Append);
                                        if (dropped > 0)
                                        {
                                            io->write(
                                                std::to_string(dropped) +
                                                " log messages could not be written\n");
                                            dropped = 0;
                                        }
                                        for (const auto& item : items)
                                        {
                                            io->write(getLabel(item) + "\n");
                                        }
                                    }
                                    catch (const std::exception&)
                                    {
                                        dropped += items.size();
                                    }
                                };

                        try
                        {
                            auto io = FileIO::create(p.path, file::Mode::Write);
                        }
                        catch (const std::exception&)
                        {
                            // No log file; the application carries on without one.
                        }
                        while (p.thread.running)
                        {
                            const auto t0 = std::chrono::steady_clock::now();

                            std::vector<log::Item> items;
                            {
                                std::unique_lock<std::mutex> lock(p.mutex.mutex);
                                std::swap(p.mutex.items, items);
                            }
                            write(items);

                            // Wait out the rest of the tick, but wake as soon as the
                            // destructor clears the flag. Sleeping instead made
                            // shutdown wait for the current tick to expire.
                            const auto t1 = std::chrono::steady_clock::now();
                            const auto period =
                                std::chrono::duration_cast<std::chrono::microseconds>(timeout);
                            const auto elapsed =
                                std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0);
                            std::unique_lock<std::mutex> lock(p.thread.stopMutex);
                            p.thread.stop.wait_for(
                                lock,
                                period > elapsed ? period - elapsed : std::chrono::microseconds(0),
                                [&p] { return !p.thread.running; });
                        }
                        std::vector<log::Item> items;
                        {
                            std::unique_lock<std::mutex> lock(p.mutex.mutex);
                            std::swap(p.mutex.items, items);
                        }
                        write(items);
                    });
        }

        FileLogSystem::FileLogSystem() :
            _p(new Private)
        {
        }

        FileLogSystem::~FileLogSystem()
        {
            TLRENDER_P();
            {
                // Under the mutex so the thread cannot test the flag and begin
                // waiting between the store and the notify.
                std::unique_lock<std::mutex> lock(p.thread.stopMutex);
                p.thread.running = false;
            }
            p.thread.stop.notify_one();
            if (p.thread.thread.joinable())
            {
                p.thread.thread.join();
            }
        }

        std::shared_ptr<FileLogSystem> FileLogSystem::create(
            const std::shared_ptr<system::Context>& context,
            const std::filesystem::path& path)
        {
            auto out = context->getSystem<FileLogSystem>();
            if (!out)
            {
                out = std::shared_ptr<FileLogSystem>(new FileLogSystem);
                out->_init(context, path);
            }
            return out;
        }
    } // namespace file
} // namespace tl
