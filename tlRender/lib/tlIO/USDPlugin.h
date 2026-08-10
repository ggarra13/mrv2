// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlIO/Read.h>
#include <tlIO/USD.h>

namespace tl
{
    //! USD image I/O.
    namespace usd
    {
        class Render;

        //! USD reader.
        class Read : public io::IVideoRead
        {
        protected:
            void _init(
                int64_t id, const std::shared_ptr<Render>&, const file::Path&,
                const std::vector<file::MemoryRead>&, const io::Options&,
                const std::shared_ptr<log::System>&);

            Read();

        public:
            ~Read() override;

            //! Create a new reader.
            static std::shared_ptr<Read> create(
                int64_t id, const std::shared_ptr<Render>&, const file::Path&,
                const io::Options&, const std::shared_ptr<log::System>&);

            std::future<io::Info> getInfo() override;
            std::future<io::VideoData>
            readVideo(const otime::RationalTime&, const io::Options&) override;
            void cancelRequests() override;

        private:
            TLRENDER_PRIVATE();
        };

        //! USD read plugin.
        class ReadPlugin : public io::IReadPlugin
        {
        protected:
            void _init(const std::shared_ptr<log::System>&);

            ReadPlugin();

        public:
            virtual ~ReadPlugin();

            //! Create a new plugin.
            static std::shared_ptr<ReadPlugin>
            create(const std::shared_ptr<log::System>&);

            std::shared_ptr<io::IVideoRead> videoRead(
                const file::Path&, const io::Options& = io::Options()) override;
            std::shared_ptr<io::IVideoRead> videoRead(
                const file::Path&, const std::vector<file::MemoryRead>&,
                const io::Options& = io::Options()) override;
        private:
            TLRENDER_PRIVATE();
        };
    } // namespace usd
} // namespace tl
