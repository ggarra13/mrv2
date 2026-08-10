// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <zlib.h>

#include <tlIO/SequenceIO.h>

#include <tlCore/FileIO.h>
#include <tlCore/Matrix.h>

namespace tl
{
    //! RAW image I/O.
    //!
    //! References:
    //! https://www.libraw.org/
    //!
    namespace raw
    {
        //! RAW decoder.
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
                const otio::RationalTime&,
                const io::Options& = io::Options()) override;

            private:
                image::Info _info;
        };

        //! RAW read plugin.
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

    } // namespace zfile
} // namespace tl
