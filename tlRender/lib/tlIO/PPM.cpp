// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlIO/PPM.h>

#include <tlCore/Error.h>
#include <tlCore/String.h>
#include <tlCore/StringFormat.h>

#include <array>
#include <sstream>

namespace tl
{
    namespace ppm
    {
        TLRENDER_ENUM_IMPL(Data, "ASCII", "Binary");
        TLRENDER_ENUM_SERIALIZE_IMPL(Data);

        size_t getFileScanlineByteCount(
            int width, size_t channelCount, size_t bitDepth)
        {
            size_t chars = 0;
            switch (bitDepth)
            {
            case 8:
                chars = 3;
                break;
            case 16:
                chars = 5;
                break;
            default:
                break;
            }
            return (chars + 1) * width * channelCount + 1;
        }

        namespace
        {
            template <typename T>
            void _readASCII(
                const std::shared_ptr<file::FileIO>& io, uint8_t* out,
                size_t size)
            {
                char tmp[string::cBufferSize] = "";
                T* outP = reinterpret_cast<T*>(out);
                for (int i = 0; i < size; ++i)
                {
                    file::readWord(io, tmp, string::cBufferSize);
                    int value = 0;
                    string::fromString(tmp, string::cBufferSize, value);
                    outP[i] = value;
                }
            }

        } // namespace

        void readASCII(
            const std::shared_ptr<file::FileIO>& io, uint8_t* out, size_t size,
            size_t bitDepth)
        {
            switch (bitDepth)
            {
            case 8:
                _readASCII<uint8_t>(io, out, size);
                break;
            case 16:
                _readASCII<uint16_t>(io, out, size);
                break;
            default:
                break;
            }
        }

        namespace
        {
            template <typename T>
            size_t _writeASCII(const uint8_t* in, char* out, size_t size)
            {
                char* outP = out;
                const T* inP = reinterpret_cast<const T*>(in);
                for (size_t i = 0; i < size; ++i)
                {
                    const std::string s =
                        std::to_string(static_cast<unsigned int>(inP[i]));
                    const char* c = s.c_str();
                    for (size_t j = 0; j < s.size(); ++j)
                    {
                        *outP++ = c[j];
                    }
                    *outP++ = ' ';
                }
                *outP++ = '\n';
                return outP - out;
            }

        } // namespace

        size_t
        writeASCII(const uint8_t* in, char* out, size_t size, size_t bitDepth)
        {
            switch (bitDepth)
            {
            case 8:
                return _writeASCII<uint8_t>(in, out, size);
            case 16:
                return _writeASCII<uint16_t>(in, out, size);
            default:
                break;
            }
            return 0;
        }

        Plugin::Plugin() {}

        std::shared_ptr<Plugin> Plugin::create(
            const std::shared_ptr<io::Cache>& cache,
            const std::weak_ptr<log::System>& logSystem)
        {
            auto out = std::shared_ptr<Plugin>(new Plugin);
            out->_init(
                "PPM", {{".ppm", io::FileType::Sequence}}, cache, logSystem);
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
            case image::PixelType::L_U8:
            case image::PixelType::L_U16:
            case image::PixelType::RGB_U8:
            case image::PixelType::RGB_U16:
                out.pixelType = info.pixelType;
                break;
            default:
                break;
            }
            Data data = Data::Binary;
            auto option = options.find("PPM/Data");
            if (option != options.end())
            {
                std::stringstream ss(option->second);
                ss >> data;
            }
            out.layout.endian = Data::Binary == data ? memory::Endian::MSB
                                                     : memory::getEndian();
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
    } // namespace ppm
} // namespace tl
