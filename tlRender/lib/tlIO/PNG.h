// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlIO/SequenceIO.h>

#include <png.h>

namespace tl
{
    //! PNG image I/O.
    namespace png
    {
        //! PNG error.
        struct ErrorStruct
        {
            std::string message;
        };

        extern "C"
        {
            //! PNG error function.
            void errorFunc(png_structp in, png_const_charp msg);

            //! PNG warning function.
            void warningFunc(png_structp in, png_const_charp msg);
        }

        //! PNG reader.
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

        //! PNG writer.
        class Write : public io::ISequenceWrite
        {
        protected:
            void _init(
                const file::Path&, const io::Info&, const io::Options&,
                const std::weak_ptr<log::System>&);

            Write();

        public:
            virtual ~Write();

            //! Create a new writer.
            static std::shared_ptr<Write> create(
                const file::Path&, const io::Info&, const io::Options&,
                const std::weak_ptr<log::System>&);

        protected:
            void _writeVideo(
                const std::string& fileName, const otime::RationalTime&,
                const std::shared_ptr<image::Image>&,
                const io::Options&) override;
        };

        //! PNG plugin.
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
                const io::Options& = io::Options()) const override;
            std::shared_ptr<io::IWrite> write(
                const file::Path&, const io::Info&,
                const io::Options& = io::Options()) override;
        };
    } // namespace png
} // namespace tl
