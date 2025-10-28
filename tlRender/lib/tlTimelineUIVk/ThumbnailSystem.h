// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlIO/IO.h>

#include <tlCore/Context.h>
#include <tlCore/FileIO.h>
#include <tlCore/ISystem.h>
#include <tlCore/Image.h>
#include <tlCore/Mesh.h>
#include <tlCore/Path.h>

#include <FL/Fl_Vk_Context.H>

#include <future>

namespace tl
{

    namespace timelineui_vk
    {
        //! Information request.
        struct InfoRequest
        {
            uint64_t id = 0;
            std::future<io::Info> future;
        };

        //! Video thumbnail request.
        struct ThumbnailRequest
        {
            uint64_t id = 0;
            int height = 0;
            otime::RationalTime time = time::invalidTime;
            std::future<std::shared_ptr<image::Image> > future;
        };

        //! Audio waveform request.
        struct WaveformRequest
        {
            uint64_t id = 0;
            math::Size2i size;
            otime::TimeRange timeRange = time::invalidTimeRange;
            std::future<std::shared_ptr<geom::TriangleMesh2> > future;
        };

        //! Thumbnail cache.
        class ThumbnailCache
            : public std::enable_shared_from_this<ThumbnailCache>
        {
        protected:
            void _init(const std::shared_ptr<system::Context>&);

            ThumbnailCache();

        public:
            ~ThumbnailCache();

            //! Create a new thumbnail cache.
            static std::shared_ptr<ThumbnailCache>
            create(const std::shared_ptr<system::Context>&);

            //! Get the maximum cache size.
            size_t getMax() const;

            //! Set the maximum cache size.
            void setMax(size_t);

            //! Get the current cache size.
            size_t getSize() const;

            //! Get the current cache size as a percentage.
            float getPercentage() const;

            //! Get an I/O information cache key.
            static std::string
            getInfoKey(const file::Path&, const io::Options&);

            //! Add I/O information to the cache.
            void addInfo(const std::string& key, const io::Info&);

            //! Get whether the cache contains I/O information.
            bool containsInfo(const std::string& key);

            //! Get I/O information from the cache.
            bool getInfo(const std::string& key, io::Info&) const;

            //! Get a thumbnail cache key.
            static std::string getThumbnailKey(
                int height, const file::Path&, const otime::RationalTime&,
                const io::Options&);

            //! Add a thumbnail to the cache.
            void addThumbnail(
                const std::string& key, const std::shared_ptr<image::Image>&);

            //! Get whether the cache contains a thumbnail.
            bool containsThumbnail(const std::string& key);

            //! Get a thumbnail from the cache.
            bool getThumbnail(
                const std::string& key, std::shared_ptr<image::Image>&) const;

            //! Get a waveform cache key.
            static std::string getWaveformKey(
                const math::Size2i&, const file::Path&, const otime::TimeRange&,
                const io::Options&);

            //! Add a waveform to the cache.
            void addWaveform(
                const std::string& key,
                const std::shared_ptr<geom::TriangleMesh2>&);

            //! Get whether the cache contains a waveform.
            bool containsWaveform(const std::string& key);

            //! Get a waveform from the cache.
            bool getWaveform(
                const std::string& key,
                std::shared_ptr<geom::TriangleMesh2>&) const;

        private:
            void _maxUpdate();

            TLRENDER_PRIVATE();
        };

        //! Thumbnail generator.
        class ThumbnailGenerator
            : public std::enable_shared_from_this<ThumbnailGenerator>
        {
        protected:
            void _init(
                const std::shared_ptr<ThumbnailCache>&,
                const std::shared_ptr<system::Context>&);

            ThumbnailGenerator(Fl_Vk_Context& ctx);

        public:
            ~ThumbnailGenerator();

            //! Create a new thumbnail generator.
            static std::shared_ptr<ThumbnailGenerator> create(
                const std::shared_ptr<ThumbnailCache>&,
                const std::shared_ptr<system::Context>&,
                Fl_Vk_Context&);

            //! Get information.
            InfoRequest
            getInfo(const file::Path&, const io::Options& = io::Options());

            //! Get information.
            InfoRequest getInfo(
                const file::Path&, const std::vector<file::MemoryRead>&,
                const io::Options& = io::Options());

            //! Get a video thumbnail.
            ThumbnailRequest getThumbnail(
                const file::Path&, int height,
                const otime::RationalTime& = time::invalidTime,
                const io::Options& = io::Options());

            //! Get a video thumbnail.
            ThumbnailRequest getThumbnail(
                const file::Path&, const std::vector<file::MemoryRead>&,
                int height, const otime::RationalTime& = time::invalidTime,
                const io::Options& = io::Options());

            //! Get an audio waveform.
            WaveformRequest getWaveform(
                const file::Path&, const math::Size2i&,
                const otime::TimeRange& = time::invalidTimeRange,
                const io::Options& = io::Options());

            //! Get an audio waveform.
            WaveformRequest getWaveform(
                const file::Path&, const std::vector<file::MemoryRead>&,
                const math::Size2i&,
                const otime::TimeRange& = time::invalidTimeRange,
                const io::Options& = io::Options());

            //! Cancel pending requests.
            void cancelRequests(const std::vector<uint64_t>&);

            //! Start the threads.
            void startThreads();
            
            //! Exit the threads.
            void exitThreads();
            
        private:
            void _infoRun();
            void _thumbnailRun();
            void _waveformRun();
            void _infoCancel();
            void _thumbnailCancel();
            void _waveformCancel();

            Fl_Vk_Context& ctx;
            
            TLRENDER_PRIVATE();
        };

        //! Thumbnail system.
        class ThumbnailSystem : public system::ISystem
        {
        protected:
            void _init(const std::shared_ptr<system::Context>&);

            ThumbnailSystem(Fl_Vk_Context&);

        public:
            ~ThumbnailSystem();

            //! Create a new system.
            static std::shared_ptr<ThumbnailSystem>
            create(const std::shared_ptr<system::Context>&,
                   Fl_Vk_Context& ctx);

            //! Get information.
            InfoRequest
            getInfo(const file::Path&, const io::Options& = io::Options());

            //! Get a video thumbnail.
            ThumbnailRequest getThumbnail(
                const file::Path&, int height,
                const otime::RationalTime& = time::invalidTime,
                const io::Options& = io::Options());

            //! Get an audio waveform.
            WaveformRequest getWaveform(
                const file::Path&, const math::Size2i&,
                const otime::TimeRange& = time::invalidTimeRange,
                const io::Options& = io::Options());

            //! Cancel pending requests.
            void cancelRequests(const std::vector<uint64_t>&);

            //! Get the thumbnail cache.
            const std::shared_ptr<ThumbnailCache>& getCache() const;

        private:
            Fl_Vk_Context& ctx;
            
            TLRENDER_PRIVATE();
        };
    } // namespace timelineui_vk
} // namespace tl
