// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlIO/Normalize.h>
#include <tlIO/ZFile.h>

#include <tlCore/StringFormat.h>

namespace tl
{
    namespace
    {
        template<class T>
        inline T
        byteswap(T n)
        {
            unsigned char* c = reinterpret_cast<unsigned char*>(&n);
            if (sizeof(T) == 2) {
                std::swap(c[0], c[1]);
            } else if (sizeof(T) == 4) {
                std::swap(c[0], c[3]);
                std::swap(c[1], c[2]);
            } else if (sizeof(T) == 8) {
                std::swap(c[0], c[7]);
                std::swap(c[1], c[6]);
                std::swap(c[2], c[5]);
                std::swap(c[3], c[4]);
            }
            return n;
        }
        
        template<class T>
        inline void
        swap_endian(T* vals, int len = 1)
        {
            for (int i = 0; i < len; ++i)
                vals[i] = byteswap(vals[i]);
        }

    }

    
    namespace zfile
    {
        namespace
        {
        
            static const int kMagic       = 0x2f0867ab;
            static const int kMagicEndian = 0xab67082f;

            //! Struct used for header of a Pixar shadow map file
            struct Header
            {
                unsigned int magic;
                short width;
                short height;
                math::Matrix4x4f worldToScreen;
                math::Matrix4x4f worldToCamera;
            };

            class File
            {
            public:
                File(const std::string& fileName)
                {
                    image::Info info;
                    int res = 0, w = 0, h = 0, n = 0, bits = 8;

                    _gz = gzopen(fileName.c_str(), "rb");
                    if (!_gz)
                    {
                        throw std::runtime_error(string::Format("{0}: {1}")
                                                 .arg(fileName)
                                                 .arg("Coult not open fileName"));
                    }
                        
                    Header header;
                    gzread(_gz, &header, sizeof(header));
                        
                    if (header.magic != kMagic && header.magic != kMagicEndian)
                    {
                        throw std::runtime_error(string::Format("{0}: {1}")
                                                 .arg(fileName)
                                                 .arg("Not a valid Pixar's Zfile"));
                    }
                        
                     _swapEndian = (header.magic == kMagicEndian);
    
                    info.size.w = header.width;
                    info.size.h = header.height;
                    info.pixelType = image::PixelType::L_F32;
                    
                    if (_swapEndian)
                    {
                        swap_endian(&info.size.w);
                        swap_endian(&info.size.h);
                        swap_endian((float*)header.worldToScreen.e, sizeof(math::Matrix4x4f));
                        swap_endian((float*)header.worldToCamera.e, sizeof(math::Matrix4x4f));
                    }

                    _worldToScreen = header.worldToScreen;
                    _worldToCamera = header.worldToCamera;
                    
                    _info.video.push_back(info);
                }

                ~File()
                    {
                        if (_gz)
                        {
                            gzclose(_gz);
                            _gz = 0;
                        }
                    }
                
                const io::Info& getInfo() const { return _info; }

                io::VideoData read(
                    const std::string& fileName,
                    const otime::RationalTime& time)
                {
                    io::VideoData out;
                    out.time = time;

                    const image::Info& info = _info.video[0];
                    out.image = image::Image::create(info);

                    gzread(_gz, out.image->getData(), info.size.w * info.size.h * sizeof(float));

                    if (_swapEndian)
                    {
                        swap_endian(reinterpret_cast<float*>(out.image->getData()),
                                    info.size.w * info.size.h);
                    }

                    {
                        std::stringstream o;
                        o << _worldToScreen;
                        _info.tags["worldToScreen"] = o.str();
                    }
                    
                    {
                        std::stringstream o;
                        o << _worldToCamera;
                        _info.tags["worldToCamera"] = o.str();
                    }
                    
                    out.image->setTags(_info.tags);

                    return out;
                }

            private:
                io::Info _info;
                bool _swapEndian = false;
                gzFile _gz;             ///< Handle for compressed files
                math::Matrix4x4f _worldToScreen;
                math::Matrix4x4f _worldToCamera;
            };
        } // namespace

        void Read::_init(
            const file::Path& path, const std::vector<file::MemoryRead>& memory,
            const io::Options& options, const std::shared_ptr<io::Cache>& cache,
            const std::weak_ptr<log::System>& logSystem)
        {
            ISequenceRead::_init(path, memory, options, cache, logSystem);
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
            io::Info out = File(fileName).getInfo();
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
            return File(fileName).read(fileName, time);
        }
    } // namespace stb
} // namespace tl
