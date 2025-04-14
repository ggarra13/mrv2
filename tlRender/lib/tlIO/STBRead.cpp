// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#define STBI_NO_JPEG
#define STBI_NO_PNG
#define STBI_NO_PNM
#define STBI_WINDOWS_UTF8
#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#include <tlIO/Normalize.h>
#include <tlIO/STB.h>

#include <tlCore/StringFormat.h>

namespace tl
{
    namespace stb
    {
        namespace
        {

            class File
            {
            public:
                File(
                    const std::string& fileName, const file::MemoryRead* memory,
                    const bool autoNormalize)
                {
                    image::Info info;
                    int res = 0, w = 0, h = 0, n = 0, bits = 8;

                    _memory = memory;
                    _autoNormalize = autoNormalize;

                    if (memory)
                    {
                        res = stbi_info_from_memory(
                            memory->p, memory->size, &w, &h, &n);
                        if (res == 0)
                            throw std::runtime_error(
                                string::Format("{0}: {1}")
                                    .arg(fileName)
                                    .arg("Corrupted image type"));

                        info.size.w = w;
                        info.size.h = h;

                        res =
                            stbi_is_16_bit_from_memory(memory->p, memory->size);
                        if (res)
                            bits = 16;

                        info.pixelType = image::getIntType(n, bits);

                        if (image::PixelType::kNone == info.pixelType)
                        {
                            throw std::runtime_error(
                                string::Format("{0}: {1}")
                                    .arg(fileName)
                                    .arg("Unsupported image type"));
                        }
                        info.layout.endian = memory::Endian::MSB;
                    }
                    else
                    {
                        res = stbi_info(fileName.c_str(), &w, &h, &n);
                        if (res == 0)
                            throw std::runtime_error(
                                string::Format("{0}: {1}")
                                    .arg(fileName)
                                    .arg("Corrupted image type"));

                        info.size.w = w;
                        info.size.h = h;

                        if (stbi_is_hdr(fileName.c_str()))
                        {
                            info.pixelType = image::PixelType::RGB_F32;
                        }
                        else
                        {
                            res = stbi_is_16_bit(fileName.c_str());
                            if (res)
                                bits = 16;

                            info.pixelType = image::getIntType(n, bits);
                            if (image::PixelType::kNone == info.pixelType)
                            {
                                throw std::runtime_error(
                                    string::Format("{0}: {1}")
                                        .arg(fileName)
                                        .arg("Unsupported image type"));
                            }
                            info.layout.endian = memory::Endian::MSB;
                        }
                    }

                    _info.video.push_back(info);
                }

                const io::Info& getInfo() const { return _info; }

                io::VideoData read(
                    const std::string& fileName,
                    const otime::RationalTime& time)
                {
                    io::VideoData out;
                    out.time = time;

                    image::Info imageInfo = _info.video[0];
                    out.image = image::Image::create(imageInfo);

                    const int channels =
                        image::getChannelCount(imageInfo.pixelType);
                    const size_t bytes =
                        image::getBitDepth(imageInfo.pixelType) / 8;

                    stbi_set_flip_vertically_on_load(1);

                    int x = 0, y = 0, n = 1;
                    stbi_uc* data = nullptr;

                    if (_memory)
                    {
                        if (bytes == 1)
                            data = stbi_load_from_memory(
                                _memory->p, _memory->size, &x, &y, &n, 0);
                        else if (bytes == 2)
                            data = reinterpret_cast<stbi_uc*>(
                                stbi_load_16_from_memory(
                                    _memory->p, _memory->size, &x, &y, &n, 0));
                        else
                            data = reinterpret_cast<stbi_uc*>(
                                stbi_loadf_from_memory(
                                    _memory->p, _memory->size, &x, &y, &n, 0));
                    }
                    else
                    {
                        if (bytes == 1)
                            data = stbi_load(fileName.c_str(), &x, &y, &n, 0);
                        else if (bytes == 2)
                            data = reinterpret_cast<stbi_uc*>(
                                stbi_load_16(fileName.c_str(), &x, &y, &n, 0));
                        else
                            data = reinterpret_cast<stbi_uc*>(
                                stbi_loadf(fileName.c_str(), &x, &y, &n, 0));
                    }

                    memcpy(
                        out.image->getData(), data,
                        imageInfo.size.w * imageInfo.size.h * channels * bytes);

                    stbi_image_free(data);

                    if (imageInfo.pixelType == image::PixelType::RGB_F32 &&
                        imageInfo.size.w > 0 && imageInfo.size.h > 0)
                    {
                        if (_autoNormalize)
                        {
#ifdef TLRENDER_EXR
                            math::Vector4f minimum, maximum;
                            io::normalizeImage(
                                minimum, maximum, out.image, imageInfo, 0,
                                imageInfo.size.w - 1, 0, imageInfo.size.h - 1);

                            _info.tags["Autonormalize Minimum"] =
                                io::serialize(minimum);
                            _info.tags["Autonormalize Maximum"] =
                                io::serialize(maximum);
#endif
                        }

                        _info.tags["otioClipName"] = fileName;
                        {
                            std::stringstream ss;
                            ss << time;
                            _info.tags["otioClipTime"] = ss.str();
                        }
                        out.image->setTags(_info.tags);
                    }

                    return out;
                }

            private:
                io::Info _info;
                bool _autoNormalize = false;
                const file::MemoryRead* _memory;
            };
        } // namespace

        void Read::_init(
            const file::Path& path, const std::vector<file::MemoryRead>& memory,
            const io::Options& options, const std::shared_ptr<io::Cache>& cache,
            const std::weak_ptr<log::System>& logSystem)
        {
            ISequenceRead::_init(path, memory, options, cache, logSystem);

            auto option = options.find("AutoNormalize");
            if (option != options.end())
            {
                _autoNormalize =
                    static_cast<bool>(std::atoi(option->second.c_str()));
            }
        }

        Read::Read() {}

        Read::~Read()
        {
            _finish();
        }

        std::shared_ptr<Read> Read::create(
            const file::Path& path, const io::Options& options,
            const std::shared_ptr<io::Cache>& cache,
            const std::weak_ptr<log::System>& logSystem)
        {
            auto out = std::shared_ptr<Read>(new Read);
            out->_init(path, {}, options, cache, logSystem);
            return out;
        }

        std::shared_ptr<Read> Read::create(
            const file::Path& path, const std::vector<file::MemoryRead>& memory,
            const io::Options& options, const std::shared_ptr<io::Cache>& cache,
            const std::weak_ptr<log::System>& logSystem)
        {
            auto out = std::shared_ptr<Read>(new Read);
            out->_init(path, memory, options, cache, logSystem);
            return out;
        }

        io::Info Read::_getInfo(
            const std::string& fileName, const file::MemoryRead* memory)
        {
            io::Info out = File(fileName, memory, false).getInfo();
            out.videoTime =
                otime::TimeRange::range_from_start_end_time_inclusive(
                    otime::RationalTime(_startFrame, _defaultSpeed),
                    otime::RationalTime(_endFrame, _defaultSpeed));
            return out;
        }

        io::VideoData Read::_readVideo(
            const std::string& fileName, const file::MemoryRead* memory,
            const otime::RationalTime& time, const io::Options&)
        {
            return File(fileName, memory, _autoNormalize).read(fileName, time);
        }
    } // namespace stb
} // namespace tl
