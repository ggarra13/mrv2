// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#pragma once

#include <tlIO/Decode.h>
#include <tlIO/Plugin.h>

namespace tl
{
    namespace io
    {
        class Cache;

        //! Base class for readers.
        //!
        //! A reader is stateful -- a movie carries a demuxer position -- which
        //! is what separates it from a decoder. What it reads is video or
        //! audio, one of the two classes below: they share nothing but the
        //! file, and plenty of files have only one of them.
        class IRead : public IIO
        {
        protected:
            void _init(
                const file::Path&,
                const std::vector<file::MemoryRead>&,
                const Options&,
                const std::shared_ptr<log::System>&);

            IRead();

        public:
            virtual ~IRead();

            //! Get the information.
            virtual std::future<Info> getInfo() = 0;

            //! Cancel pending requests.
            virtual void cancelRequests() = 0;

            //! Get the first error encountered while reading, or an empty
            //! string. Errors are also sent to the log. The default
            //! implementation returns an empty string.
            virtual std::string getError() const;

            //! Get the number of errors encountered while reading. The
            //! default implementation returns zero.
            virtual size_t getErrorCount() const;

        protected:
            std::vector<file::MemoryRead> _mem;
        };

        //! Base class for video readers.
        class IVideoRead : public IRead
        {
        public:
            virtual ~IVideoRead();

            //! Read video data.
            virtual std::future<VideoData> readVideo(
                const OTIO_NS::RationalTime&,
                const Options& = Options()) = 0;

            //! Give the reader a cache to use for decoded frames, in place of
            //! any private one it would otherwise keep. Called at most once,
            //! before the first readVideo() request is issued. The default
            //! implementation does nothing -- most readers are stateless and
            //! have no use for one.
            virtual void setCache(const std::shared_ptr<Cache>&) {}
        };

        //! Base class for audio readers.
        class IAudioRead : public IRead
        {
        public:
            virtual ~IAudioRead();

            //! Read audio data.
            virtual std::future<AudioData> readAudio(
                const OTIO_NS::TimeRange&,
                const Options& = Options()) = 0;

            //! Give the reader a cache to use for decoded frames, in place of
            //! any private one it would otherwise keep. Called at most once,
            //! before the first readVideo() request is issued. The default
            //! implementation does nothing -- most readers are stateless and
            //! have no use for one.
            virtual void setCache(const std::shared_ptr<Cache>&) {}
        };

        //! Base class for read plugins.
        class IReadPlugin : public IIOPlugin
        {
            TLRENDER_NON_COPYABLE(IReadPlugin);

        protected:
            void _init(
                const std::string& name,
                const std::map<std::string, FileType>& extensions,
                const std::shared_ptr<log::System>&);

            IReadPlugin();

        public:
            virtual ~IReadPlugin() = 0;

            //! Create a video reader for the given path, or null when this
            //! format has no video, or is decoded rather than read: a sequence
            //! of stateless files needs no reader, and decode() returns one for
            //! it instead.
            virtual std::shared_ptr<IVideoRead> videoRead(
                const file::Path&,
                const Options& = Options());

            //! Create a video reader for the given path and memory locations.
            virtual std::shared_ptr<IVideoRead> videoRead(
                const file::Path&,
                const std::vector<file::MemoryRead>&,
                const Options& = Options());

            //! Create an audio reader for the given path, or null when this
            //! format has no audio.
            virtual std::shared_ptr<IAudioRead> audioRead(
                const file::Path&,
                const Options& = Options());

            //! Create an audio reader for the given path and memory locations.
            virtual std::shared_ptr<IAudioRead> audioRead(
                const file::Path&,
                const std::vector<file::MemoryRead>&,
                const Options& = Options());

            //! Create a decoder, or null when this format has to be read
            //! statefully.
            virtual std::shared_ptr<IDecode> decode(
                const Options& = Options());

        private:
            TLRENDER_PRIVATE();
        };
    }
}
