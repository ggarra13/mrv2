// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlIO/Cache.h>

#include <tlCore/LRUCache.h>
#include <tlCore/String.h>
#include <tlCore/StringFormat.h>

#include <mutex>
#include <string>

namespace tl
{
    namespace io
    {
        namespace
        {
            //! Append ";key:value" for each option. Produces the same text as
            //! joining "key:value" strings with ';', without a temporary
            //! string and vector per option.
            void appendOptions(
                std::string& out, const Options& options,
                const char* skip = nullptr)
            {
                for (const auto& i : options)
                {
                    if (skip && i.first == skip)
                        continue;
                    out += ';';
                    out += i.first;
                    out += ':';
                    out += i.second;
                }
            }
        } // namespace

        std::string
        getInfoCacheKey(const file::Path& path, const Options& options)
        {
            std::string out = path.get();
            out += ';';
            out += path.getNumber();
            appendOptions(out, options);
            return out;
        }

        std::string getVideoCacheKey(
            const file::Path& path, const OTIO_NS::RationalTime& time,
            const Options& initOptions, const Options& frameOptions)
        {
            std::string out = path.get();
            out += ';';
            out += path.getNumber();
            out += ';';
            out += string::Format("{0}").arg(time);
            appendOptions(out, initOptions);
            // Do not add ClearFrame frame option if present
            appendOptions(out, frameOptions, "ClearFrame");
            return out;
        }

        std::string getAudioCacheKey(
            const file::Path& path, const OTIO_NS::TimeRange& timeRange,
            const Options& initOptions, const Options& frameOptions)
        {
            std::string out = path.get();
            out += ';';
            out += path.getNumber();
            out += ';';
            out += string::Format("{0}").arg(timeRange);
            appendOptions(out, initOptions);
            appendOptions(out, frameOptions);
            return out;
        }

        struct Cache::Private
        {
            size_t max = memory::gigabyte;
            memory::LRUCache<std::string, VideoData> video;
            memory::LRUCache<std::string, AudioData> audio;
            std::mutex mutex;
        };

        void Cache::_init()
        {
            _maxUpdate();
        }

        Cache::Cache() :
            _p(new Private)
        {
        }

        Cache::~Cache() {}

        std::shared_ptr<Cache> Cache::create()
        {
            auto out = std::shared_ptr<Cache>(new Cache);
            out->_init();
            return out;
        }

        size_t Cache::getMax() const
        {
            TLRENDER_P();
            std::unique_lock<std::mutex> lock(p.mutex);
            return p.max;
        }

        void Cache::setMax(size_t value)
        {
            TLRENDER_P();
            {
                std::unique_lock<std::mutex> lock(p.mutex);
                if (value == p.max)
                    return;
                p.max = value;
            }
            _maxUpdate(); // Takes the lock itself.
        }

        size_t Cache::getSize() const
        {
            TLRENDER_P();
            std::unique_lock<std::mutex> lock(p.mutex);
            return p.video.getSize() + p.audio.getSize();
        }

        float Cache::getPercentage() const
        {
            TLRENDER_P();
            std::unique_lock<std::mutex> lock(p.mutex);
            const size_t max = p.video.getMax() + p.audio.getMax();
            return max ? (p.video.getSize() + p.audio.getSize()) /
                             static_cast<float>(max) * 100.F
                       : 0.F;
        }

        void Cache::addVideo(const std::string& key, const VideoData& videoData)
        {
            TLRENDER_P();
            std::unique_lock<std::mutex> lock(p.mutex);
            p.video.add(
                key, videoData,
                videoData.image ? videoData.image->getDataByteCount() : 1);
        }

        void Cache::removeVideo(const std::string& key)
        {
            TLRENDER_P();
            std::unique_lock<std::mutex> lock(p.mutex);
            p.video.remove(key);
        }

        bool Cache::containsVideo(const std::string& key) const
        {
            TLRENDER_P();
            std::unique_lock<std::mutex> lock(p.mutex);
            return p.video.contains(key);
        }

        bool Cache::getVideo(const std::string& key, VideoData& videoData) const
        {
            TLRENDER_P();
            std::unique_lock<std::mutex> lock(p.mutex);
            return p.video.get(key, videoData);
        }

        void Cache::addAudio(const std::string& key, const AudioData& audioData)
        {
            TLRENDER_P();
            std::unique_lock<std::mutex> lock(p.mutex);
            p.audio.add(
                key, audioData,
                audioData.audio ? audioData.audio->getByteCount() : 1);
        }

        bool Cache::containsAudio(const std::string& key) const
        {
            TLRENDER_P();
            std::unique_lock<std::mutex> lock(p.mutex);
            return p.audio.contains(key);
        }

        bool Cache::getAudio(const std::string& key, AudioData& audioData) const
        {
            TLRENDER_P();
            std::unique_lock<std::mutex> lock(p.mutex);
            return p.audio.get(key, audioData);
        }

        void Cache::clear()
        {
            TLRENDER_P();
            std::unique_lock<std::mutex> lock(p.mutex);
            p.video.clear();
            p.audio.clear();
        }

        void Cache::_maxUpdate()
        {
            TLRENDER_P();
            std::unique_lock<std::mutex> lock(p.mutex);
            p.video.setMax(p.max * .9F);
            p.audio.setMax(p.max * .1F);
        }
    } // namespace io
} // namespace tl
