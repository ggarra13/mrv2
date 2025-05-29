// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlIO/SequenceIO.h>

#include <tlCore/FileIO.h>

namespace tl
{
    //! DPX image I/O.
    //!
    //! References:
    //! - SMPTE, "SMPTE 268M-2003"
    //!   http://www.smpte.org
    //! - Cinesite, "Conversion of 10-bit Log Film Data To 8-bit Linear or Video
    //! Data"
    namespace dpx
    {
        //! DPX file format versions.
        enum class Version {
            _1_0,
            _2_0,

            Count,
            First = _1_0
        };
        TLRENDER_ENUM(Version);
        TLRENDER_ENUM_SERIALIZE(Version);

        //! DPX endian options.
        enum class Endian {
            Auto, //!< Use the endian of the machine
            MSB,  //!< Most-significant byte first
            LSB,  //!< Least-significant byte first

            Count,
            First = Auto
        };
        TLRENDER_ENUM(Endian);
        TLRENDER_ENUM_SERIALIZE(Endian);

        //! DPX header magic numbers.
        static const char magic[][5] = {"SDPX", "XPDS"};

        //! DPX image orientations.
        enum class Orient {
            LeftRightTopBottom,
            RightLeftTopBottom,
            LeftRightBottomTop,
            RightLeftBottomTop,
            TopBottomLeftRight,
            TopBottomRightLeft,
            BottomTopLeftRight,
            BottomTopRightLeft,

            Count,
            First = LeftRightTopBottom
        };
        TLRENDER_ENUM(Orient);
        TLRENDER_ENUM_SERIALIZE(Orient);

        //! DPX channel descriptors.
        enum class Descriptor {
            User = 0,
            R = 1,
            G = 2,
            B = 3,
            A = 4,
            L = 6,
            Chroma = 7,
            Z = 8,
            CompositeVideo = 9,
            RGB = 50,
            RGBA = 51,
            ABGR = 52,
            CBYCRY = 100,
            CBYACRYA = 101,
            CBYCR = 102,
            CBYCRA = 103,
            User2 = 150,
            User3 = 151,
            User4 = 152,
            User5 = 153,
            User6 = 154,
            User7 = 155,
            User8 = 156
        };

        //! DPX color transfer information.
        enum class Transfer {
            User,
            FilmPrint,
            Linear,
            Log,
            Video,
            SMPTE_274M,
            ITU_R_709_4,
            ITU_R_601_5_B_OR_G,
            ITU_R_601_5_M,
            NTSC,
            PAL,
            Z,
            ZHomogeneous,

            Count,
            First = User
        };
        TLRENDER_ENUM(Transfer);
        TLRENDER_ENUM_SERIALIZE(Transfer);

        //! DPX colorimetric information for version 1.0.
        enum class Colorimetric_1_0 {
            User = 0,
            FilmPrint = 1,
            Video = 4,
            SMPTE_240M = 5,
            ITU_R_709_1 = 6,
            ITU_R_601_2_B_OR_G = 7,
            ITU_R_601_2_M = 8,
            NTSC = 9,
            PAL = 10
        };

        //! DPX colorimetric information for version 2.0.
        enum Colorimetric_2_0 {
            User = 0,
            FilmPrint = 1,
            Video = 4,
            SMPTE_274M = 5,
            ITU_R_709_4 = 6,
            ITU_R_601_5_B_OR_G = 7,
            ITU_R_601_5_M = 8,
            NTSC = 9,
            PAL = 10
        };

        //! DPX data component ordering.
        enum class Components {
            Pack,
            TypeA,
            TypeB,

            Count,
            First = Pack
        };
        TLRENDER_ENUM(Components);
        TLRENDER_ENUM_SERIALIZE(Components);

        //! DPX header.
        struct Header
        {
            Header();

            //! File header.
            struct File
            {
                uint32_t magic;
                uint32_t imageOffset;
                char version[8];
                uint32_t size;
                uint32_t dittoKey;
                uint32_t headerSize;
                uint32_t industryHeaderSize;
                uint32_t userHeaderSize;
                char name[100];
                char time[24];
                char creator[100];
                char project[200];
                char copyright[200];
                uint32_t encryptionKey;
                uint8_t pad[104];
            };
            File file;

            //! Image header.
            struct Image
            {
                uint16_t orient;
                uint16_t elemSize;
                uint32_t size[2];

                //! Element header.
                struct Elem
                {
                    uint32_t dataSign;
                    uint32_t lowData;
                    float lowQuantity;
                    uint32_t highData;
                    float highQuantity;
                    uint8_t descriptor;
                    uint8_t transfer;
                    uint8_t colorimetric;
                    uint8_t bitDepth;
                    uint16_t packing;
                    uint16_t encoding;
                    uint32_t dataOffset;
                    uint32_t linePadding;
                    uint32_t elemPadding;
                    char description[32];
                };

                Elem elem[8];
                uint8_t pad[52];
            };
            Image image;

            //! Source header.
            struct Source
            {
                uint32_t offset[2];
                float center[2];
                uint32_t size[2];
                char file[100];
                char time[24];
                char inputDevice[32];
                char inputSerial[32];
                uint16_t border[4];
                uint32_t pixelAspect[2];
                float scanSize[2]; // V2.0
                uint8_t pad[20];
            };
            Source source;

            //! Film header.
            struct Film
            {
                char id[2];
                char type[2];
                char offset[2];
                char prefix[6];
                char count[4];
                char format[32];
                uint32_t frame;
                uint32_t sequence;
                uint32_t hold;
                float frameRate;
                float shutter;
                char frameId[32];
                char slate[100];
                uint8_t pad[56];
            };
            Film film;

            //! TV header.
            struct TV
            {
                uint32_t timecode;
                uint32_t userBits;
                uint8_t interlace;
                uint8_t field;
                uint8_t videoSignal;
                uint8_t pad;
                float sampleRate[2];
                float frameRate;
                float timeOffset;
                float gamma;
                float blackLevel;
                float blackGain;
                float breakpoint;
                float whiteLevel;
                float integrationTimes;
                uint8_t pad2[76];
            };
            TV tv;
        };

        //! Read a header.
        Header read(const std::shared_ptr<file::FileIO>&, io::Info&, Transfer&);

        //! Write a header.
        void write(
            const std::shared_ptr<file::FileIO>&, const io::Info&, Version,
            Endian, Transfer);

        //! Finish writing the header.
        void finishWrite(const std::shared_ptr<file::FileIO>&);

        //! DPX reader.
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

            bool _autoNormalize = false;
        };

        //! DPX writer.
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

        //! DPX plugin.
        class Plugin : public io::IPlugin
        {
        protected:
            void _init(
                const std::shared_ptr<io::Cache>&,
                const std::weak_ptr<log::System>&);

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
    } // namespace dpx
} // namespace tl
