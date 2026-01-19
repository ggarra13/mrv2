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
    //! ZFILE image I/O.
    //!
    //! References:
    //! https://github.com/nothings/zfile.git
    //!
    namespace zfile
    {
        //! ZFILE reader.
        class Read : public io::ISequenceRead
        {
        protected:
            void _init(
                const file::Path&, const std::vector<file::MemoryRead>&,
                const io::Options&, const std::shared_ptr<io::Cache>&,
                const std::weak_ptr<log::System>&);

            Read();

        public:
            virtual ~Read();

            //! Create a new reader.
            static std::shared_ptr<Read> create(
                const file::Path&, const io::Options&,
                const std::shared_ptr<io::Cache>&,
                const std::weak_ptr<log::System>&);

            //! Create a new reader.
            static std::shared_ptr<Read> create(
                const file::Path&, const std::vector<file::MemoryRead>&,
                const io::Options&, const std::shared_ptr<io::Cache>&,
                const std::weak_ptr<log::System>&);

        protected:
            io::Info _getInfo(
                const std::string& fileName, const file::MemoryRead*) override;
            io::VideoData _readVideo(
                const std::string& fileName, const file::MemoryRead*,
                const otime::RationalTime&, const io::Options&) override;
        };

        //! ZFILE plugin.
        class Plugin : public io::IPlugin
        {
        protected:
            Plugin();

        public:
            //! Create a new plugin.
            static std::shared_ptr<Plugin> create(
                const std::shared_ptr<io::Cache>&,
                const std::weak_ptr<log::System>&);

            std::shared_ptr<io::IRead> read(
                const file::Path&, const io::Options& = io::Options()) override;
            std::shared_ptr<io::IRead> read(
                const file::Path&, const std::vector<file::MemoryRead>&,
                const io::Options& = io::Options()) override;
            image::Info getWriteInfo(
                const image::Info&,
                const io::Options& = io::Options()) const override
            {
                image::Info out;
                return out;
            }
            std::shared_ptr<io::IWrite> write(
                const file::Path&, const io::Info&,
                const io::Options& = io::Options()) override
            {
                std::shared_ptr<io::IWrite> out;
                return out;
            }
        };
    } // namespace zfile
} // namespace tl
