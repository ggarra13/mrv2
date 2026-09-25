// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlIO/SequenceIO.h>

#include <tlCore/FileIO.h>

namespace tl
{
    //! STB image I/O.
    //!
    //! References:
    //! https://github.com/nothings/stb.git
    //!
    namespace stb
    {

        //! STB decoder.
        class Decode : public io::IDecode
        {
        protected:
            Decode();

        public:
            virtual ~Decode();

            //! Create a new decoder.
            static std::shared_ptr<Decode> create();

            io::Info getInfo(
                const std::string& fileName,
                const file::MemoryRead* = nullptr) override;
            io::VideoData readVideo(
                const std::string& fileName,
                const file::MemoryRead*,
                const OTIO_NS::RationalTime&,
                const io::Options& = io::Options()) override;

        private:
            image::Info _info;
            bool _autoNormalize;
        };

        //! STB writer.
        class Write : public io::ISequenceWrite
        {
        protected:
            void _init(
                const file::Path&, const io::Info&, const io::Options&,
                const std::shared_ptr<log::System>&);

            Write();

        public:
            virtual ~Write();

            //! Create a new writer.
            static std::shared_ptr<Write> create(
                const file::Path&, const io::Info&, const io::Options&,
                const std::shared_ptr<log::System>&);

        protected:
            void _writeVideo(
                const std::string& fileName, const OTIO_NS::RationalTime&,
                const std::shared_ptr<image::Image>&,
                const io::Options&) override;
        };

        //! STB read plugin.
        class ReadPlugin : public io::IReadPlugin
        {
        protected:
            void _init(const std::shared_ptr<log::System>&);

            ReadPlugin() = default;

        public:
            //! Create a new plugin.
            static std::shared_ptr<ReadPlugin> create(
                const std::shared_ptr<log::System>&);

            std::shared_ptr<io::IDecode> decode(
                const io::Options& = io::Options()) override;

            std::string getPluginInfo(
                const io::Options& = io::Options()) const override;
        };

        //! STB write plugin.
        class WritePlugin : public io::IWritePlugin
        {
        protected:
            void _init(const std::shared_ptr<log::System>&);

            WritePlugin() = default;

        public:
            //! Create a new plugin.
            static std::shared_ptr<WritePlugin> create(
                const std::shared_ptr<log::System>&);

            image::Info getInfo(
                const image::Info&,
                const io::Options & = io::Options()) const override;
            std::shared_ptr<io::IWrite> write(
                const file::Path&,
                const io::Info&,
                const io::Options & = io::Options()) override;

            std::string getPluginInfo(
                const io::Options& = io::Options()) const override;
        };

    } // namespace stb
} // namespace tl
