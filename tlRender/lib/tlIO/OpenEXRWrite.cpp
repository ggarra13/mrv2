// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlIO/OpenEXRPrivate.h>

#include <tlCore/StringFormat.h>

#include <ImfHeader.h>
#include <ImfChannelList.h>
#include <ImfPartType.h>
#include <ImfFrameBuffer.h>
#include <ImfStandardAttributes.h>
#include <ImfMultiPartOutputFile.h>
#include <ImfOutputPart.h>

namespace tl
{
    namespace exr
    {
        namespace
        {
            void flipImageY(
                uint8_t* dst, const uint8_t* src, const size_t height,
                const size_t bytes)
            {
                for (size_t row = 0; row < height; ++row)
                {
                    const uint8_t* srcRow = src + row * bytes;
                    uint8_t* destRow = dst + (height - row - 1) * bytes;

                    // Copy the row from the source image to the destination
                    // image
                    std::memcpy(destRow, srcRow, bytes);
                }
            }
        } // namespace

        struct Write::Private
        {
            Imf::MultiPartOutputFile* outputFile = nullptr;
            image::PixelType pixelType = image::PixelType::RGBA_F16;
        };

        void Write::_init(
            const file::Path& path, const io::Info& info,
            const io::Options& options,
            const std::weak_ptr<log::System>& logSystem)
        {
            TLRENDER_P();

            ISequenceWrite::_init(path, info, options, logSystem);

            auto i = options.find("OpenEXR/Compression");
            if (i != options.end())
            {
                std::stringstream ss(i->second);
                int i;
                ss >> i;
                _compression = static_cast<Imf::Compression>(i);
            }
            i = options.find("OpenEXR/PixelType");
            if (i != options.end())
            {
                std::stringstream ss(i->second);
                ss >> p.pixelType;
            }
            i = options.find("OpenEXR/ZipCompressionLevel");
            if (i != options.end())
            {
                std::stringstream ss(i->second);
                ss >> _zipCompressionLevel;
            }
            i = options.find("OpenEXR/DWACompressionLevel");
            if (i != options.end())
            {
                std::stringstream ss(i->second);
                ss >> _dwaCompressionLevel;
            }
            i = options.find("OpenEXR/Speed");
            if (i != options.end())
            {
                std::stringstream ss(i->second);
                ss >> _speed;
            }
        }

        Write::Write() :
            _p(new Private)
        {
        }

        Write::~Write() {}

        std::shared_ptr<Write> Write::create(
            const file::Path& path, const io::Info& info,
            const io::Options& options,
            const std::weak_ptr<log::System>& logSystem)
        {
            auto out = std::shared_ptr<Write>(new Write);
            out->_init(path, info, options, logSystem);
            return out;
        }

        void Write::_writeLayer(
            const std::shared_ptr<image::Image>& image, int layerId)
        {
            TLRENDER_P();

            const uint8_t channelCount = getChannelCount(p.pixelType);
            const uint8_t bitDepth = getBitDepth(p.pixelType) / 8;
            Imf::OutputPart out(*p.outputFile, layerId);
            const Imf::Header& header = p.outputFile->header(layerId);
            const Imath::Box2i& dataWindow = header.dataWindow();
            const Imath::Box2i& displayWindow = header.displayWindow();

            const size_t width = dataWindow.max.x - dataWindow.min.x + 1;
            const size_t height = dataWindow.max.y - dataWindow.min.y + 1;
            const size_t xStride = bitDepth * channelCount;
            size_t yStride = xStride * width;

            const uint8_t* base =
                reinterpret_cast<const uint8_t*>(image->getData());
            uint8_t* flip = new uint8_t[height * yStride];
            flipImageY(flip, base, height, yStride);

            uint8_t* dest = flip;

            Imf::FrameBuffer fb;
            auto ci = header.channels().begin();
            auto ce = header.channels().end();
            int k = channelCount - 1;
            for (; ci != ce; ++ci)
            {
                const std::string& name = ci.name();
                uint8_t* buf = dest + k-- * bitDepth;
                fb.insert(
                    name, Imf::Slice::Make(
                              toImf(p.pixelType), (char*)buf, dataWindow,
                              xStride, yStride));
            }

            out.setFrameBuffer(fb);
            out.writePixels(height);
            delete[] flip;
        }

        void Write::_writeVideo(
            const std::string& fileName, const otime::RationalTime&,
            const std::shared_ptr<image::Image>& image, const io::Options&)
        {
            TLRENDER_P();

            const auto& info = image->getInfo();
            Imf::Header header(
                info.size.w, info.size.h, info.size.pixelAspectRatio,
                Imath::V2f(0.F, 0.F), 1.F, Imf::INCREASING_Y, _compression);
            header.zipCompressionLevel() = _zipCompressionLevel;
            header.dwaCompressionLevel() = _dwaCompressionLevel;
            const auto tags = image->getTags();
            writeTags(tags, _speed, header);

            const uint8_t channelCount = getChannelCount(p.pixelType);
            switch (channelCount)
            {
            case 2:
                header.channels().insert("A", Imf::Channel(toImf(p.pixelType)));
                // no break here
            case 1:
                header.channels().insert("L", Imf::Channel(toImf(p.pixelType)));
                break;
            case 4:
                header.channels().insert("A", Imf::Channel(toImf(p.pixelType)));
                // no break here
            case 3:
                header.channels().insert("B", Imf::Channel(toImf(p.pixelType)));
                header.channels().insert("G", Imf::Channel(toImf(p.pixelType)));
                header.channels().insert("R", Imf::Channel(toImf(p.pixelType)));
                break;
            default:
                throw std::runtime_error("Invalid channel count");
                break;
            }

            const std::string layerName = fileName;
            header.setName(layerName);
            header.setType(Imf::SCANLINEIMAGE);
            header.setVersion(1);

            auto i = tags.find("Display Window");
            if (i != tags.end())
            {
                std::stringstream s(i->second);
                math::Box2i box;
                s >> box;
                header.displayWindow() = Imath::Box2i(
                    Imath::V2i(box.min.x, box.min.y),
                    Imath::V2i(box.max.x, box.max.y));
            }
            i = tags.find("Data Window");
            if (i != tags.end())
            {
                std::stringstream s(i->second);
                math::Box2i box;
                s >> box;
                header.dataWindow() = Imath::Box2i(
                    Imath::V2i(box.min.x, box.min.y),
                    Imath::V2i(box.max.x, box.max.y));
            }

            std::vector<Imf::Header> headers;
            headers.push_back(header);

            const int numParts = static_cast<int>(headers.size());
            p.outputFile = new Imf::MultiPartOutputFile(
                fileName.c_str(), &headers[0], numParts);

            for (int part = 0; part < numParts; ++part)
            {
                _writeLayer(image, part);
            }

            delete p.outputFile;
        }
    } // namespace exr
} // namespace tl
