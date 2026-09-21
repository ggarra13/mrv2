// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include "Namespace.h"

#include <tlIO/IO.h>

#include <tlCore/Context.h>
#include <tlCore/FileIO.h>
#include <tlCore/ISystem.h>
#include <tlCore/Image.h>
#include <tlCore/Mesh.h>
#include <tlCore/Path.h>
#include <tlCore/ValueObserver.h>

#include <future>

namespace tl
{

#ifdef OPENGL_BACKEND
    namespace gl
    {
        class GLFWWindow;
    }
#endif

    namespace TIMELINEUI
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
            std::optional<otio::RationalTime> time;
            std::future<std::shared_ptr<image::Image> > future;
        };

        //! Audio waveform request.
        struct WaveformRequest
        {
            uint64_t id = 0;
            math::Size2i size;
            std::optional<otio::TimeRange> timeRange;
            std::future<std::shared_ptr<geom::TriangleMesh2> > future;
        };

        //! Thumbnails cache options.
        struct ThumbnailCacheOptions
        {
            //! Video cache size in megabytes.
            float thumbnailMB = 16.F;

            //! Audio cache size in megabytes.
            float waveformMB = 16.F;

            bool operator == (const ThumbnailCacheOptions&) const;
            bool operator != (const ThumbnailCacheOptions&) const;
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
                int height, const file::Path&, const OTIO_NS::RationalTime&,
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
                const math::Size2i&, const file::Path&, const OTIO_NS::TimeRange&,
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

        //! Thumbnail System.
        class ThumbnailSystem : public system::ISystem
        {
        protected:
#ifdef OPENGL_BACKEND
            ThumbnailSystem(
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<gl::GLFWWindow>&);
#endif

#ifdef VULKAN_BACKEND
            ThumbnailSystem(
                const std::shared_ptr<system::Context>&,
                Fl_Vk_Context& ctx);
#endif

        public:
            ~ThumbnailSystem();

#ifdef OPENGL_BACKEND
            //! Create a new thumbnail System.
            static std::shared_ptr<ThumbnailSystem> create(
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<gl::GLFWWindow>& = nullptr);
#endif

#ifdef VULKAN_BACKEND
            //! Create a new thumbnail System.
            static std::shared_ptr<ThumbnailSystem> create(
                const std::shared_ptr<system::Context>&,
                Fl_Vk_Context&);
#endif

            //! Get information.
            InfoRequest
            getInfo(const file::Path&, const io::Options& = io::Options());

            //! Get information.
            InfoRequest getInfo(
                const file::Path&, const file::Path&,
                const io::Options& = io::Options());

            //! Get a video thumbnail.
            ThumbnailRequest getThumbnail(
                const file::Path&, int height,
                const std::optional<otio::RationalTime>& time,
                const std::string& mediaReferenceKey = "",
                const io::Options& = io::Options());

            //! Get a video thumbnail.
            ThumbnailRequest getThumbnail(
                const file::Path& timelinePath,
                const file::Path& mediaPath,
                int height,
                const std::optional<otio::RationalTime>& time,
                const std::string& mediaReferenceKey = "",
                const io::Options& = io::Options());

            //! Get an audio waveform.
            WaveformRequest getWaveform(
                const file::Path&, const math::Size2i&,
                const std::optional<otime::TimeRange >& timeRange,
                const std::string& mediaReferenceKey = "",
                const io::Options& = io::Options());

            //! Get an audio waveform.
            WaveformRequest getWaveform(
                const file::Path& timelinePath,
                const file::Path& mediaPath,
                const math::Size2i&,
                const std::optional<otime::TimeRange >& timeRange,
                const std::string& mediaReferenceKey = "",
                const io::Options& = io::Options());

            //! Cancel pending requests.
            void cancelRequests(const std::vector<uint64_t>&);

            //! Get the cache options.
            const ThumbnailCacheOptions& getCacheOptions() const;

            //! Observe the cache options.
            std::shared_ptr<observer::IValue<ThumbnailCacheOptions> > observeCacheOptions() const;

            //! Set the cache options.
            void setCacheOptions(const ThumbnailCacheOptions&);

            //! Clear the cache.
            void clearCache();

            void shutdown();

        private:
            void _infoRun();
            void _thumbnailRun();
            void _waveformRun();
            void _infoCancel();
            void _thumbnailCancel();
            void _waveformCancel();
            void _startThreads();

#ifdef VULKAN_BACKEND
            Fl_Vk_Context& ctx;
#endif
            TLRENDER_PRIVATE();
        };

        //! \name Serialize
        ///@{

        void to_json(nlohmann::json&, const ThumbnailCacheOptions&);

        void from_json(const nlohmann::json&, ThumbnailCacheOptions&);

        ///@}
    } // namespace TIMELINEUI
} // namespace tl
