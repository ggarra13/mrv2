// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlIO/Cineon.h>

#include <tlCore/Assert.h>
#include <tlCore/Error.h>
#include <tlCore/Memory.h>
#include <tlCore/StringFormat.h>

#include <array>
#include <cstring>
#include <sstream>

namespace tl
{
    namespace cineon
    {
        TLRENDER_ENUM_IMPL(
            Orient, "LeftRightTopBottom", "LeftRightBottomTop",
            "RightLeftTopBottom", "RightLeftBottomTop", "TopBottomLeftRight",
            "TopBottomRightLeft", "BottomTopLeftRight", "BottomTopRightLeft");
        TLRENDER_ENUM_SERIALIZE_IMPL(Orient);

        TLRENDER_ENUM_IMPL(
            Descriptor, "Luminance", "RedFilmPrint", "GreenFilmPrint",
            "BlueFilmPrint", "RedCCIRXA11", "GreenCCIRXA11", "BlueCCIRXA11");
        TLRENDER_ENUM_SERIALIZE_IMPL(Descriptor);

        namespace
        {
            void zero(int32_t* value)
            {
                *((uint32_t*)value) = 0x80000000;
            }

            void zero(float* value)
            {
                *((uint32_t*)value) = 0x7F800000;
            }

            void zero(char* value, size_t size)
            {
                std::memset(value, 0, size);
            }

        } // namespace

        Header::Header()
        {
            std::memset(&file, 0xff, sizeof(Header::File));
            zero(file.version, 8);
            zero(file.name, 100);
            zero(file.time, 24);

            std::memset(&image, 0xff, sizeof(Header::Image));

            for (uint8_t i = 0; i < 8; ++i)
            {
                zero(&image.channel[i].lowData);
                zero(&image.channel[i].lowQuantity);
                zero(&image.channel[i].highData);
                zero(&image.channel[i].highQuantity);
            }

            std::memset(&source, 0xff, sizeof(Header::Source));
            zero(&source.offset[0]);
            zero(&source.offset[1]);
            zero(source.file, 100);
            zero(source.time, 24);
            zero(source.inputDevice, 64);
            zero(source.inputModel, 32);
            zero(source.inputSerial, 32);
            zero(&source.inputPitch[0]);
            zero(&source.inputPitch[1]);
            zero(&source.gamma);

            std::memset(&film, 0xff, sizeof(Header::Film));
            zero(film.format, 32);
            zero(&film.frameRate);
            zero(film.frameId, 32);
            zero(film.slate, 200);
        }

        bool isValid(const char* in, size_t size)
        {
            const char _minChar = 32;
            const char _maxChar = 126;
            const char* p = in;
            const char* const end = p + size;
            for (; *p && p < end; ++p)
            {
                if (*p < _minChar || *p > _maxChar)
                {
                    return false;
                }
            }
            return size ? (in[0] != 0) : false;
        }

        std::string toString(const char* in, size_t size)
        {
            const char* p = in;
            const char* const end = p + size;
            for (; *p && p < end; ++p)
                ;
            return std::string(in, p - in);
        }

        size_t fromString(
            const std::string& string, char* out, size_t maxLen, bool terminate)
        {
            TLRENDER_ASSERT(maxLen >= 0);
            const char* c = string.c_str();
            const size_t length =
                std::min(string.length(), maxLen - static_cast<int>(terminate));
            size_t i = 0;
            for (; i < length; ++i)
            {
                out[i] = c[i];
            }
            if (terminate)
            {
                out[i++] = 0;
            }
            return i;
        }

        namespace
        {
            void convertEndian(Header& header)
            {
                memory::endian(&header.file.imageOffset, 1, 4);
                memory::endian(&header.file.headerSize, 1, 4);
                memory::endian(&header.file.industryHeaderSize, 1, 4);
                memory::endian(&header.file.userHeaderSize, 1, 4);
                memory::endian(&header.file.size, 1, 4);

                for (uint8_t i = 0; i < 8; ++i)
                {
                    memory::endian(&header.image.channel[i].size, 2, 4);
                    memory::endian(&header.image.channel[i].lowData, 1, 4);
                    memory::endian(&header.image.channel[i].lowQuantity, 1, 4);
                    memory::endian(&header.image.channel[i].highData, 1, 4);
                    memory::endian(&header.image.channel[i].highQuantity, 1, 4);
                }

                memory::endian(&header.image.white, 2, 4);
                memory::endian(&header.image.red, 2, 4);
                memory::endian(&header.image.green, 2, 4);
                memory::endian(&header.image.blue, 2, 4);
                memory::endian(&header.image.linePadding, 1, 4);
                memory::endian(&header.image.channelPadding, 1, 4);

                memory::endian(&header.source.offset, 2, 4);
                memory::endian(&header.source.inputPitch, 2, 4);
                memory::endian(&header.source.gamma, 1, 4);

                memory::endian(&header.film.prefix, 1, 4);
                memory::endian(&header.film.count, 1, 4);
                memory::endian(&header.film.frame, 1, 4);
                memory::endian(&header.film.frameRate, 1, 4);
            }

            bool isValid(const uint8_t* in)
            {
                return *in != 0xff;
            }

            // Constants to catch uninitialized values.
            const int32_t _intMax = 1000000;
            const float _floatMax = 1000000.F;
            const float _minSpeed = .000001F;

            bool isValid(const uint32_t* in)
            {
                return *in != 0xffffffff &&
                       *in < static_cast<uint32_t>(_intMax);
            }

            bool isValid(const int32_t* in)
            {
                return *in != static_cast<int32_t>(0x80000000) &&
                       *in > -_intMax && *in < _intMax;
            }

            bool isValid(const float* in)
            {
                return *(reinterpret_cast<const uint32_t*>(in)) != 0x7F800000 &&
                       *in > -_floatMax && *in < _floatMax;
            }

        } // namespace

        Header read(const std::shared_ptr<file::FileIO>& io, io::Info& info)
        {
            Header out;

            // Read the file section of the header.
            io->read(&out.file, sizeof(Header::File));

            // Check the magic number.
            bool convertEndian = false;
            if (magic[0] == out.file.magic)
                ;
            else if (magic[1] == out.file.magic)
            {
                convertEndian = true;
            }
            else
            {
                throw std::runtime_error(string::Format("{0}: {1}")
                                             .arg(io->getFileName())
                                             .arg("Bad magic number"));
            }

            // Read the rest of the header.
            io->read(&out.image, sizeof(Header::Image));
            io->read(&out.source, sizeof(Header::Source));
            io->read(&out.film, sizeof(Header::Film));

            // Convert the endian if necessary.
            image::Info imageInfo;
            if (convertEndian)
            {
                io->setEndianConversion(true);
                cineon::convertEndian(out);
                imageInfo.layout.endian = memory::opposite(memory::getEndian());
            }

            // Image information.
            imageInfo.size.w = out.image.channel[0].size[0];
            imageInfo.size.h = out.image.channel[0].size[1];

            if (!out.image.channels)
            {
                throw std::runtime_error(string::Format("{0}: {1}")
                                             .arg(io->getFileName())
                                             .arg("No image channels"));
            }
            uint8_t i = 1;
            for (; i < out.image.channels; ++i)
            {
                if ((out.image.channel[i].size[0] !=
                     out.image.channel[0].size[0]) ||
                    (out.image.channel[i].size[1] !=
                     out.image.channel[0].size[1]))
                {
                    break;
                }
                if (out.image.channel[i].bitDepth !=
                    out.image.channel[0].bitDepth)
                {
                    break;
                }
            }
            if (i < out.image.channels)
            {
                throw std::runtime_error(
                    string::Format("{0}: {1}")
                        .arg(io->getFileName())
                        .arg("Unsupported image channels"));
            }
            switch (out.image.channels)
            {
            case 3:
                switch (out.image.channel[0].bitDepth)
                {
                case 10:
                    imageInfo.pixelType = image::PixelType::RGB_U10;
                    imageInfo.layout.alignment = 4;
                    break;
                default:
                    break;
                }
                break;
            default:
                break;
            }
            if (image::PixelType::kNone == imageInfo.pixelType)
            {
                throw std::runtime_error(string::Format("{0}: {1}")
                                             .arg(io->getFileName())
                                             .arg("Unsupported bit depth"));
            }
            if (isValid(&out.image.linePadding) && out.image.linePadding)
            {
                throw std::runtime_error(string::Format("{0}: {1}")
                                             .arg(io->getFileName())
                                             .arg("Unsupported line padding"));
            }
            if (isValid(&out.image.channelPadding) && out.image.channelPadding)
            {
                throw std::runtime_error(
                    string::Format("{0}: {1}")
                        .arg(io->getFileName())
                        .arg("Unsupported channel padding"));
            }

            if (io->getSize() - out.file.imageOffset !=
                image::getDataByteCount(imageInfo))
            {
                throw std::runtime_error(string::Format("{0}: {1}")
                                             .arg(io->getFileName())
                                             .arg("Incomplete file"));
            }
            switch (static_cast<Orient>(out.image.orient))
            {
            case Orient::LeftRightTopBottom:
                imageInfo.layout.mirror.y = true;
                break;
            case Orient::RightLeftTopBottom:
                imageInfo.layout.mirror.x = true;
                imageInfo.layout.mirror.y = true;
                break;
            case Orient::RightLeftBottomTop:
                imageInfo.layout.mirror.x = true;
                break;
            case Orient::TopBottomLeftRight:
            case Orient::TopBottomRightLeft:
            case Orient::BottomTopLeftRight:
            case Orient::BottomTopRightLeft:
                //! \todo Implement these image orientations.
                break;
            default:
                break;
            }
            info.video.push_back(imageInfo);

            // Tags.
            if (isValid(out.file.time, 24))
            {
                info.tags["Time"] = toString(out.file.time, 24);
            }
            if (isValid(&out.source.offset[0]) &&
                isValid(&out.source.offset[1]))
            {
                std::stringstream ss;
                ss << out.source.offset[0] << " " << out.source.offset[1];
                info.tags["Source Offset"] = ss.str();
            }
            if (isValid(out.source.file, 100))
            {
                info.tags["Source File"] = toString(out.source.file, 100);
            }
            if (isValid(out.source.time, 24))
            {
                info.tags["Source Time"] = toString(out.source.time, 24);
            }
            if (isValid(out.source.inputDevice, 64))
            {
                info.tags["Source Input Device"] =
                    toString(out.source.inputDevice, 64);
            }
            if (isValid(out.source.inputModel, 32))
            {
                info.tags["Source Input Model"] =
                    toString(out.source.inputModel, 32);
            }
            if (isValid(out.source.inputSerial, 32))
            {
                info.tags["Source Input Serial"] =
                    toString(out.source.inputSerial, 32);
            }
            if (isValid(&out.source.inputPitch[0]) &&
                isValid(&out.source.inputPitch[1]))
            {
                std::stringstream ss;
                ss << out.source.inputPitch[0] << " "
                   << out.source.inputPitch[1];
                info.tags["Source Input Pitch"] = ss.str();
            }
            if (isValid(&out.source.gamma))
            {
                std::stringstream ss;
                ss << out.source.gamma;
                info.tags["Source Gamma"] = ss.str();
            }
            if (isValid(&out.film.id) && isValid(&out.film.type) &&
                isValid(&out.film.offset) && isValid(&out.film.prefix) &&
                isValid(&out.film.count))
            {
                info.tags["Keycode"] = time::keycodeToString(
                    out.film.id, out.film.type, out.film.prefix, out.film.count,
                    out.film.offset);
            }
            if (isValid(out.film.format, 32))
            {
                info.tags["Film Format"] = toString(out.film.format, 32);
            }
            if (isValid(&out.film.frame))
            {
                std::stringstream ss;
                ss << out.film.frame;
                info.tags["Film Frame"] = ss.str();
            }
            if (isValid(&out.film.frameRate) && out.film.frameRate >= _minSpeed)
            {
                std::stringstream ss;
                ss << out.film.frameRate;
                info.tags["Film Frame Rate"] = ss.str();
            }
            if (isValid(out.film.frameId, 32))
            {
                info.tags["Film Frame ID"] = toString(out.film.frameId, 32);
            }
            if (isValid(out.film.slate, 200))
            {
                info.tags["Film Slate"] = toString(out.film.slate, 200);
            }

            // Set the file position.
            if (out.file.imageOffset)
            {
                io->setPos(out.file.imageOffset);
            }

            return out;
        }

        void
        write(const std::shared_ptr<file::FileIO>& io, const io::Info& info)
        {
            Header header;

            // Set the file section.
            header.file.imageOffset = 2048;
            header.file.headerSize = 1024;
            header.file.industryHeaderSize = 1024;
            header.file.userHeaderSize = 0;

            // Set the image section.
            header.image.orient =
                static_cast<uint8_t>(Orient::LeftRightTopBottom);
            header.image.channels = 3;
            header.image.channel[0].descriptor[1] =
                static_cast<uint8_t>(Descriptor::RedFilmPrint);
            header.image.channel[1].descriptor[1] =
                static_cast<uint8_t>(Descriptor::GreenFilmPrint);
            header.image.channel[2].descriptor[1] =
                static_cast<uint8_t>(Descriptor::BlueFilmPrint);
            const uint8_t bitDepth = 10;
            for (uint8_t i = 0; i < header.image.channels; ++i)
            {
                header.image.channel[i].descriptor[0] = 0;
                header.image.channel[i].bitDepth = bitDepth;
                header.image.channel[i].size[0] = info.video[0].size.w;
                header.image.channel[i].size[1] = info.video[0].size.h;

                header.image.channel[i].lowData = 0;

                switch (bitDepth)
                {
                case 8:
                    header.image.channel[i].highData = image::U8Range.getMax();
                    break;
                case 10:
                    header.image.channel[i].highData = image::U10Range.getMax();
                    break;
                case 12:
                    header.image.channel[i].highData = image::U12Range.getMax();
                    break;
                case 16:
                    header.image.channel[i].highData = image::U16Range.getMax();
                    break;
                default:
                    break;
                }
            }
            header.image.interleave = 0;
            header.image.packing = 5;
            header.image.dataSign = 0;
            header.image.dataSense = 0;
            header.image.linePadding = 0;
            header.image.channelPadding = 0;

            // Set the tags.
            auto i = info.tags.find("Time");
            if (i != info.tags.end())
            {
                fromString(i->second, header.file.time, 24, false);
            }
            i = info.tags.find("Source Offset");
            if (i != info.tags.end())
            {
                std::stringstream ss(i->second);
                ss >> header.source.offset[0];
                ss >> header.source.offset[1];
            }
            i = info.tags.find("Source File");
            if (i != info.tags.end())
            {
                fromString(i->second, header.source.file, 100, false);
            }
            i = info.tags.find("Source Time");
            if (i != info.tags.end())
            {
                fromString(i->second, header.source.time, 24, false);
            }
            i = info.tags.find("Source Input Device");
            if (i != info.tags.end())
            {
                fromString(i->second, header.source.inputDevice, 64, false);
            }
            i = info.tags.find("Source Input Model");
            if (i != info.tags.end())
            {
                fromString(i->second, header.source.inputModel, 32, false);
            }
            i = info.tags.find("Source Input Serial");
            if (i != info.tags.end())
            {
                fromString(i->second, header.source.inputSerial, 32, false);
            }
            i = info.tags.find("Source Input Pitch");
            if (i != info.tags.end())
            {
                std::stringstream ss(i->second);
                ss >> header.source.inputPitch[0];
                ss >> header.source.inputPitch[1];
            }
            i = info.tags.find("Source Gamma");
            if (i != info.tags.end())
            {
                std::stringstream ss(i->second);
                ss >> header.source.gamma;
            }
            i = info.tags.find("Keycode");
            if (i != info.tags.end())
            {
                int id = 0;
                int type = 0;
                int prefix = 0;
                int count = 0;
                int offset = 0;
                time::stringToKeycode(
                    i->second, id, type, prefix, count, offset);
                header.film.id = id;
                header.film.type = type;
                header.film.offset = offset;
                header.film.prefix = prefix;
                header.film.count = count;
            }
            i = info.tags.find("Film Format");
            if (i != info.tags.end())
            {
                fromString(i->second, header.film.format, 32, false);
            }
            i = info.tags.find("Film Frame");
            if (i != info.tags.end())
            {
                std::stringstream ss(i->second);
                ss >> header.film.frame;
            }
            i = info.tags.find("Film Frame Rate");
            if (i != info.tags.end())
            {
                std::stringstream ss(i->second);
                ss >> header.film.frameRate;
            }
            i = info.tags.find("Film Frame ID");
            if (i != info.tags.end())
            {
                fromString(i->second, header.film.frameId, 32, false);
            }
            i = info.tags.find("Film Slate");
            if (i != info.tags.end())
            {
                fromString(i->second, header.film.slate, 200, false);
            }

            // Write the header.
            const bool convertEndian =
                memory::getEndian() != memory::Endian::MSB;
            io->setEndianConversion(convertEndian);
            if (convertEndian)
            {
                cineon::convertEndian(header);
                header.file.magic = magic[1];
            }
            else
            {
                header.file.magic = magic[0];
            }
            io->write(&header.file, sizeof(Header::File));
            io->write(&header.image, sizeof(Header::Image));
            io->write(&header.source, sizeof(Header::Source));
            io->write(&header.film, sizeof(Header::Film));
        }

        void finishWrite(const std::shared_ptr<file::FileIO>& io)
        {
            const uint32_t size = static_cast<uint32_t>(io->getPos());
            io->setPos(20);
            io->writeU32(size);
        }

        void Plugin::_init(
            const std::shared_ptr<io::Cache>& cache,
            const std::weak_ptr<log::System>& logSystem)
        {
            IPlugin::_init(
                "Cineon", {{".cin", io::FileType::Sequence}}, cache, logSystem);
        }

        Plugin::Plugin() {}

        std::shared_ptr<Plugin> Plugin::create(
            const std::shared_ptr<io::Cache>& cache,
            const std::weak_ptr<log::System>& logSystem)
        {
            auto out = std::shared_ptr<Plugin>(new Plugin);
            out->_init(cache, logSystem);
            return out;
        }

        std::shared_ptr<io::IRead>
        Plugin::read(const file::Path& path, const io::Options& options)
        {
            return Read::create(path, options, _cache, _logSystem);
        }

        std::shared_ptr<io::IRead> Plugin::read(
            const file::Path& path, const std::vector<file::MemoryRead>& memory,
            const io::Options& options)
        {
            return Read::create(path, memory, options, _cache, _logSystem);
        }

        image::Info Plugin::getWriteInfo(
            const image::Info& info, const io::Options& options) const
        {
            image::Info out;
            out.size = info.size;
            switch (info.pixelType)
            {
            case image::PixelType::RGB_U10:
                out.pixelType = info.pixelType;
                break;
            default:
                break;
            }
            out.layout.mirror.y = true;
            out.layout.alignment = 4;
            out.layout.endian = memory::Endian::MSB;
            return out;
        }

        std::shared_ptr<io::IWrite> Plugin::write(
            const file::Path& path, const io::Info& info,
            const io::Options& options)
        {
            if (info.video.empty() ||
                (!info.video.empty() &&
                 !_isWriteCompatible(info.video[0], options)))
                throw std::runtime_error(string::Format("{0}: {1}")
                                             .arg(path.get())
                                             .arg("Unsupported video"));
            return Write::create(path, info, options, _logSystem);
        }
    } // namespace cineon
} // namespace tl
