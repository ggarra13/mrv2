// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlIO/SequenceIO.h>


namespace tl
{
    //! SVG image I/O.
    namespace svg
    {

        //! SVG decoder.
        //!
        //! An SVG has no resolution of its own beyond the size the document
        //! asks for, so the size to rasterize at is settled when the decoder
        //! is made rather than per frame: the information and the image have
        //! to agree, and the information is asked for without options.
        //!
        //! Options:
        //! - SVG/Width: the width to rasterize at, in pixels.
        //! - SVG/Height: the height to rasterize at, in pixels.
        //!
        //! Giving one keeps the document's aspect ratio; giving neither uses
        //! the size the document asks for.
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
            math::Size2i _requestedSize;
        };

        //! SVG read plugin.
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
    } // namespace svg
} // namespace tl
