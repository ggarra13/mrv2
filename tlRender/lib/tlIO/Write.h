// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#pragma once

#include <tlIO/Plugin.h>

namespace tl
{
    namespace io
    {
        //! Base class for writers.
        class IWrite : public IIO
        {
        protected:
            void _init(
                const file::Path&,
                const io::Options&,
                const io::Info&,
                const std::shared_ptr<log::System>&);

            IWrite();

        public:
            virtual ~IWrite();

            //! Write video data.
            virtual void writeVideo(
                const otio::RationalTime&,
                const std::shared_ptr<image::Image>&,
                const io::Options& = io::Options()) = 0;

            //! Write audio data. The default implementation does nothing;
            //! writers that support audio should override this. Audio is
            //! expected to be written sequentially, starting at the beginning
            //! of the audio time range given in the io::Info.
            virtual void writeAudio(
                const otio::TimeRange&,
                const std::shared_ptr<audio::Audio>&,
                const io::Options& = io::Options());

            //! Finish writing, flushing any buffered data and finalizing the
            //! file. This is called automatically by the destructor, but since
            //! destructors cannot throw, errors that occur while finalizing
            //! are only logged. Call finish() explicitly to have those errors
            //! reported as exceptions.
            virtual void finish();

        protected:
            io::Info _info;
        };

        //! Base class for write plugins.
        class IWritePlugin : public IIOPlugin
        {
            TLRENDER_NON_COPYABLE(IWritePlugin);

        protected:
            void _init(
                const std::string& name,
                const std::map<std::string, FileType>& extensions,
                const std::shared_ptr<log::System>&);

            IWritePlugin();

        public:
            virtual ~IWritePlugin() = 0;

            //! Get information for writing.
            virtual image::Info getInfo(
                const image::Info&,
                const io::Options& = io::Options()) const = 0;

            //! Create a writer for the given path.
            virtual std::shared_ptr<IWrite> write(
                const file::Path&,
                const io::Info&,
                const io::Options& = io::Options()) = 0;

        protected:
            bool _isCompatible(const image::Info&, const io::Options&) const;

        private:
            TLRENDER_PRIVATE();
        };
    }
}
