// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlCore/FileLogSystem.h>

#include <tlCore/Context.h>
#include <tlCore/FileIO.h>
#include <tlCore/Time.h>

#include <atomic>
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
            std::string fileName;

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
            };
            Thread thread;
        };

        void FileLogSystem::_init(
            const std::string& fileName,
            const std::shared_ptr<system::Context>& context)
        {
            ICoreSystem::_init("tl::file:::FileLogSystem", context);
            TLRENDER_P();

            p.fileName = fileName;

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
                    {
                        auto io =
                            file::FileIO::create(p.fileName, file::Mode::Write);
                    }
                    while (p.thread.running)
                    {
                        const auto t0 = std::chrono::steady_clock::now();

                        std::vector<log::Item> items;
                        {
                            std::unique_lock<std::mutex> lock(p.mutex.mutex);
                            std::swap(p.mutex.items, items);
                        }
                        {
                            auto io = file::FileIO::create(
                                p.fileName, file::Mode::Append);
                            io->seek(io->getSize());
                            const size_t options =
                                static_cast<size_t>(log::StringConvert::Time) |
                                static_cast<size_t>(log::StringConvert::Prefix);
                            for (const auto& item : items)
                            {
                                io->write(log::toString(item, options) + "\n");
                            }
                        }

                        const auto t1 = std::chrono::steady_clock::now();
                        time::sleep(timeout, t0, t1);
                    }
                    std::vector<log::Item> items;
                    {
                        std::unique_lock<std::mutex> lock(p.mutex.mutex);
                        std::swap(p.mutex.items, items);
                    }
                    {
                        auto io = file::FileIO::create(
                            p.fileName, file::Mode::Append);
                        io->seek(io->getSize());
                        const size_t options =
                            static_cast<size_t>(log::StringConvert::Time) |
                            static_cast<size_t>(log::StringConvert::Prefix);
                        for (const auto& item : items)
                        {
                            io->write(log::toString(item, options) + "\n");
                        }
                    }
                });
        }

        FileLogSystem::FileLogSystem() :
            _p(new Private)
        {
        }

        FileLogSystem::~FileLogSystem()
        {
            TLRENDER_P();
            p.thread.running = false;
            if (p.thread.thread.joinable())
            {
                p.thread.thread.join();
            }
        }

        std::shared_ptr<FileLogSystem> FileLogSystem::create(
            const std::string& fileName,
            const std::shared_ptr<system::Context>& context)
        {
            auto out = context->getSystem<FileLogSystem>();
            if (!out)
            {
                out = std::shared_ptr<FileLogSystem>(new FileLogSystem);
                out->_init(fileName, context);
            }
            return out;
        }
    } // namespace file
} // namespace tl
