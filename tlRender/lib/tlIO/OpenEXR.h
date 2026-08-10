// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <OpenEXR/ImfCompression.h>
#include <OpenEXR/ImfChromaticities.h>

#include <tlIO/SequenceIO.h>

namespace tl
{
    //! OpenEXR image I/O.
    namespace exr
    {
        //! Channel grouping.
        enum class ChannelGrouping {
            kNone,
            Known,
            All,

            Count,
            First = kNone
        };
        TLRENDER_ENUM(ChannelGrouping);
        TLRENDER_ENUM_SERIALIZE(ChannelGrouping);

        //! Get default channels.
        std::set<std::string> getDefaultChannels(const std::set<std::string>&);

        //! Reorder channels.
        void reorderChannels(std::vector<std::string>&);

        //! OpenEXR decoder.
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
            double getSpeed(const io::Info&, double defaultSpeed) const override;

        private:
            ChannelGrouping _channelGrouping = ChannelGrouping::Known;
            bool _useRGBOnly = false;
            bool _ignoreDisplayWindow = false;
            bool _ignoreChromaticities = false;
            bool _autoNormalize = false;
            int  _xLevel = -1;
            int  _yLevel = -1;
        };


        //! OpenEXR writer.
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

            void _writeLayer(
                const std::shared_ptr<image::Image>& image, int layerId = 0);

        private:
            TLRENDER_PRIVATE();

            bool _hasChromaticities = false;
            Imf::Chromaticities _chromaticities;
            Imf::Compression _compression = Imf::ZIP_COMPRESSION;
            float _dwaCompressionLevel = 45.F;
            int _zipCompressionLevel = 4;
            double _speed = io::SeqOptions().defaultSpeed;
        };

        //! OpenEXR read plugin.
        class ReadPlugin : public io::IReadPlugin
        {
        protected:
            void _init(const std::shared_ptr<log::System>&);

            ReadPlugin();

        public:
            //! Create a new plugin.
            static std::shared_ptr<ReadPlugin> create(
                const std::shared_ptr<log::System>&);

            std::shared_ptr<io::IDecode> decode(
                const io::Options& = io::Options()) override;

            std::string getPluginInfo(
                const io::Options& = io::Options()) const override;
        };

        //! OpenEXR write plugin.
        class WritePlugin : public io::IWritePlugin
        {
        protected:
            void _init(const std::shared_ptr<log::System>&);

            WritePlugin();

        public:
            //! Create a new write plugin.
            static std::shared_ptr<WritePlugin> create(
                const std::shared_ptr<log::System>&);

            image::Info getInfo(
                const image::Info&,
                const io::Options& = io::Options()) const override;
            std::shared_ptr<io::IWrite> write(
                const file::Path&,
                const io::Info&,
                const io::Options& = io::Options()) override;

            std::string getPluginInfo(
                const io::Options& = io::Options()) const override;
        };
    } // namespace exr
} // namespace tl
