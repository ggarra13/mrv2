// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlIO/SequenceIO.h>

extern "C"
{
#include <jpeglib.h>
}

#include <setjmp.h>
#include <tlCore/FileIO.h>

namespace tl
{
    //! JPEG image I/O.
    namespace jpeg
    {
        //! JPEG error.
        struct ErrorStruct
        {
            struct jpeg_error_mgr pub;
            std::vector<std::string> messages;
            jmp_buf jump;
        };

        //! JPEG error function.
        void errorFunc(j_common_ptr);

        //! JPEG warning function.
        void warningFunc(j_common_ptr, int level);

        //! JPEG decoder.
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
            bool _autoNormalize;
        };

        //! PPM writer.
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
                const std::string& fileName, const otime::RationalTime&,
                const std::shared_ptr<image::Image>&,
                const io::Options&) override;

            float _quality = 90;
        };

        //! JPEG read plugin.
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

        //! JPEG write plugin.
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

    } // namespace jpeg
} // namespace tl
