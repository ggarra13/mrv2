// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// Copyright (c) 2024-Present Gonzalo Garramuño
// All rights reserved.

#include <tlIO/Normalize.h>
#include <tlIO/OpenEXRPrivate.h>

#include <tlCore/FileIO.h>
#include <tlCore/HDR.h>
#include <tlCore/Locale.h>
#include <tlCore/LogSystem.h>
#include <tlCore/StringFormat.h>

#include <ImfInputPart.h>
#include <ImfChannelList.h>
#include <ImfStandardAttributes.h>
#include <ImfRgbaFile.h>
#include <ImfTiledInputPart.h>

#include <ImathMatrix.h>
#include <ImathVec.h>
using namespace Imath;

#include <array>
#include <cstring>
#include <sstream>

namespace tl
{
    namespace exr
    {
        namespace
        {
            template <typename T>
            std::string serialize(const Imath::Box<Imath::Vec2<T> >& value)
            {
                std::stringstream ss;
                ss << value.min.x << "," << value.min.y << "*" << value.max.x
                   << "," << value.max.y;
                return ss.str();
            }
        } // namespace

        struct IStream::Private
        {
            std::shared_ptr<file::FileIO> f;
            const uint8_t* p = nullptr;
            uint64_t size = 0;
            uint64_t pos = 0;
        };

        IStream::IStream(const std::string& fileName) :
            Imf::IStream(fileName.c_str()),
            _p(new Private)
        {
            TLRENDER_P();
            p.f = file::FileIO::create(fileName, file::Mode::Read);
            p.p = p.f->getMemoryP();
            p.size = p.f->getSize();
        }

        IStream::IStream(
            const std::string& fileName, const uint8_t* memoryP,
            size_t memorySize) :
            Imf::IStream(fileName.c_str()),
            _p(new Private)
        {
            TLRENDER_P();
            p.p = memoryP;
            p.size = memorySize;
        }

        IStream::~IStream() {}

        bool IStream::isMemoryMapped() const
        {
            return _p->p;
        }

        char* IStream::readMemoryMapped(int n)
        {
            TLRENDER_P();
            if (p.pos >= p.size || (p.pos + n) > p.size)
            {
                throw std::runtime_error(
                    string::Format("{0}: Error reading file").arg(fileName()));
            }
            char* out = nullptr;
            if (p.p)
            {
                out = const_cast<char*>(reinterpret_cast<const char*>(p.p)) +
                      p.pos;
                p.pos += n;
            }
            return out;
        }

        bool IStream::read(char c[], int n)
        {
            TLRENDER_P();
            if (p.pos >= p.size || (p.pos + n) > p.size)
            {
                throw std::runtime_error(
                    string::Format("{0}: Error reading file").arg(fileName()));
            }
            if (p.p)
            {
                std::memcpy(c, p.p + p.pos, n);
            }
            else
            {
                p.f->read(c, n);
            }
            p.pos += n;
            return p.pos < p.size;
        }

        uint64_t IStream::tellg()
        {
            return _p->pos;
        }

        void IStream::seekg(uint64_t pos)
        {
            TLRENDER_P();
            if (p.f)
            {
                p.f->setPos(pos);
            }
            p.pos = pos;
        }

        namespace
        {
            std::string getLabel(Imf::PixelType value)
            {
                const std::array<std::string, 3> data = {
                    "UInt", "Half", "Float"};
                return data[value];
            }

            class File
            {
            protected:
                // Compute the luminance coefficients dynamically from
                // the chromaticities
                Imath::V3f computeLuminanceCoefficients()
                {
                    // RGB -> XYZ matrix
                    const Imath::M44f& M = RGBtoXYZ(_chromaticities, 1);

                    // Extract the Y coefficients (second row of the matrix)
                    Imath::V3f Yw(M[1][0], M[1][1], M[1][2]);

                    // Normalize the Y coefficients so they sum to 1
                    float sum = Yw[0] + Yw[1] + Yw[2];
                    if (sum > 0.0f)
                    {
                        Yw /= sum;
                    }

                    return Yw;
                }

                // Function to convert an interleaved Y, RY, BY (YC) image to
                // RGB.
                template <typename T>
                void ycToRgb(T* image, int numChannels, int width, int height)
                {
                    // Compute luminance coefficients
                    Imath::V3f yw = computeLuminanceCoefficients();

                    for (int i = 0; i < width * height; ++i)
                    {
                        T Y = image[i * numChannels + 0];
                        T RY = image[i * numChannels + 1];
                        T BY = image[i * numChannels + 2];

                        T R = (RY + 1.0) * Y;
                        T B = (BY + 1.0) * Y;
                        T G = (Y - R * yw[0] - B * yw[2]) / yw[1];

                        // Store in the RGB image buffer
                        image[i * numChannels + 0] = R;
                        image[i * numChannels + 1] = G;
                        image[i * numChannels + 2] = B;
                    }
                }

                //--------------------------------------------
                // Chromaticities
                // according to Rec. ITU-R BT.709-3
                //--------------------------------------------
                bool useChromaticities()
                {
                    return (
                        _chromaticities.red.x != 0.6400f ||
                        _chromaticities.red.y != 0.3300f ||
                        _chromaticities.green.x != 0.3000f ||
                        _chromaticities.green.y != 0.6000f ||
                        _chromaticities.blue.x != 0.1500f ||
                        _chromaticities.blue.y != 0.0600f ||
                        _chromaticities.white.x != 0.3127f ||
                        _chromaticities.white.y != 0.3290f);
                }

                void parseHeader(const Imf::Header& header, int partNumber = 0)
                {
                    if (hasChromaticities(header))
                        _chromaticities = chromaticities(header);

                    // Get the display and data windows.
                    _displayWindow = fromImath(header.displayWindow());
                    _dataWindow = fromImath(header.dataWindow());
                    _intersectedWindow = _displayWindow.intersect(_dataWindow);
                    _fast = _displayWindow == _dataWindow;

                    std::string compressionName;
                    const Imf::Compression compression = header.compression();
                    Imf::getCompressionNameFromId(compression, compressionName);

                    int compressionNumScanlines =
                        Imf::getCompressionNumScanlines(compression);

                    bool isLossyCompression =
                        Imf::isLossyCompression(compression);

                    bool isValidDeepCompression =
                        Imf::isValidDeepCompression(compression);

                    if (auto logSystem = logSystemWeak.lock())
                    {
                        const std::string id =
                            string::Format("tl::io::exr::Read {0}").arg(this);
                        std::vector<std::string> s;
                        s.push_back(string::Format("\n"
                                                   "    file name: {0}\n"
                                                   "    display window {1}\n"
                                                   "    data window: {2}\n"
                                                   "    compression: {3}")
                                        .arg(_fileName)
                                        .arg(_displayWindow)
                                        .arg(_dataWindow)
                                        .arg(compressionName));
                        const auto& channels = header.channels();
                        for (auto i = channels.begin(); i != channels.end();
                             ++i)
                        {
                            std::stringstream ss2;
                            ss2 << "    channel " << i.name() << ": "
                                << getLabel(i.channel().type) << ", "
                                << i.channel().xSampling << "x"
                                << i.channel().ySampling;
                            s.push_back(ss2.str());
                        }
                        logSystem->print(id, string::join(s, '\n'));
                    }

                    // Get the tags.
                    readTags(header, _info.tags);

                    if (hasChromaticities(header))
                    {
                        using V2f = math::Vector2f;
                        const std::string& name =
                            image::primariesName(V2f(_chromaticities.red.x,
                                                     _chromaticities.red.y),
                                                 V2f(_chromaticities.green.x,
                                                     _chromaticities.green.y),
                                                 V2f(_chromaticities.blue.x,
                                                     _chromaticities.blue.y),
                                                 V2f(_chromaticities.white.x,
                                                     _chromaticities.white.y));
                        _info.tags["Video Color Primaries"] = name;
                    }

                    // Get the layers.
                    std::string view;
                    if (header.hasView())
                        view = header.view() + " ";
                    std::vector<Layer> layers =
                        getLayers(header.channels(), _channelGrouping);
                    size_t offset = _info.video.size();
                    _info.video.resize(offset + layers.size());
                    _info.video[partNumber].compression = compressionName;
                    _info.video[partNumber].compressionNumScanlines =
                        compressionNumScanlines;
                    _info.video[partNumber].isLossyCompression =
                        isLossyCompression;
                    _info.video[partNumber].isValidDeepCompression =
                        isValidDeepCompression;
                    for (size_t i = 0; i < layers.size(); ++i)
                    {
                        layers[i].partNumber = partNumber;
                        const auto& layer = layers[i];
                        _layers.push_back(layer);
                        const math::Vector2i sampling(
                            layer.channels[0].sampling.x,
                            layer.channels[0].sampling.y);
                        if (sampling.x != 1 || sampling.y != 1)
                            _fast = false;
                        auto& info = _info.video[offset + i];
                        info.name = view + layer.name;
                        int w = std::max(_displayWindow.w(), _dataWindow.w());
                        int h = std::max(_displayWindow.h(), _dataWindow.h());
                        if (_ignoreDisplayWindow)
                        {
                            info.size.w = w;
                            info.size.h = h;
                        }
                        else
                        {
                            info.size.w = _displayWindow.w();
                            info.size.h = _displayWindow.h();
                        }
                        info.size.pixelAspectRatio = header.pixelAspectRatio();
                        switch (layer.channels[0].pixelType)
                        {
                        case Imf::PixelType::HALF:
                            info.pixelType =
                                image::getFloatType(layer.channels.size(), 16);
                            break;
                        case Imf::PixelType::FLOAT:
                            info.pixelType =
                                image::getFloatType(layer.channels.size(), 32);
                            break;
                        case Imf::PixelType::UINT:
                            info.pixelType =
                                image::getIntType(layer.channels.size(), 32);
                            break;
                        default:
                            break;
                        }
                        if (image::PixelType::kNone == info.pixelType)
                        {
                            throw std::runtime_error(
                                string::Format("{0}: Unsupported image type")
                                    .arg(_fileName));
                        }
                        info.layout.mirror.y = true;
                    }
                }

                void readMipmap()
                {
                    int numXLevels = _t_part->numXLevels();
                    int numYLevels = _t_part->numYLevels();

                    if (_xLevel > numXLevels - 1)
                        _xLevel = numXLevels - 1;
                    if (_yLevel > numYLevels - 1)
                        _yLevel = numYLevels - 1;

                    Imf::Header header = _t_part->header();
                    header.dataWindow() =
                        _t_part->dataWindowForLevel(_xLevel, _yLevel);
                    header.displayWindow() = header.dataWindow();

                    parseHeader(header);
                }

            public:
                File(
                    const std::string& fileName, const file::MemoryRead* memory,
                    ChannelGrouping channelGrouping,
                    const bool ignoreDisplayWindow,
                    const bool ignoreChromaticities, const bool autoNormalize,
                    const int xLevel, const int yLevel,
                    const std::weak_ptr<log::System>& logSystem) :
                    _fileName(fileName),
                    _channelGrouping(channelGrouping),
                    _ignoreDisplayWindow(ignoreDisplayWindow),
                    _ignoreChromaticities(ignoreChromaticities),
                    _autoNormalize(autoNormalize),
                    _xLevel(xLevel),
                    _yLevel(yLevel),
                    logSystemWeak(logSystem)
                {
                    // Open the file.
                    if (memory)
                    {
                        _s.reset(new IStream(
                            fileName.c_str(), memory->p, memory->size));
                    }
                    else
                    {
                        _s.reset(new IStream(fileName.c_str()));
                    }

                    // 2. ALWAYS open as MultiPartInputFile. 
                    try
                    {
                        _f.reset(new Imf::MultiPartInputFile(*_s));
                    }
                    catch (const std::exception& e)
                    {
                        if (auto log = logSystemWeak.lock())
                        {
                            std::string msg = "Failed to open EXR file: " + std::string(e.what());
                            log->print(_fileName, msg, log::Type::Error);
                        }
                        return; // Failed to open, exit constructor
                    }

                    int numberOfParts = _f->parts();
                    if (numberOfParts == 0)
                    {
                        if (auto log = logSystemWeak.lock())
                        {
                            log->print(_fileName, "EXR file has no parts.",
                                       log::Type::Error);
                        }
                        return;
                    }

                    // 3. Now check logic based on mipmap request
                    if (_xLevel > 0 || _yLevel > 0)
                    {
                        // Mipmaps requested. The part MUST be tiled.
                        // (This example assumes you only read mipmaps from part 0)
                        const Imf::Header& header = _f->header(0);
                        if (!header.hasTileDescription())
                        {
                            if (auto log = logSystemWeak.lock())
                            {
                                log->print(_fileName,
                                           "Cannot read mipmaps: File is not tiled.", log::Type::Error);
                            }
                            // Do NOT proceed
                        }
                        else
                        {
                            // Safe to read mipmaps.
                            // Create a TiledInputPart for reading.
                            _t_part.reset(new Imf::TiledInputPart(*_f, 0));

                            int numXLevels = _t_part->numXLevels();
                            int numYLevels = _t_part->numYLevels();
            
                            {
                                std::stringstream ss;
                                ss << numXLevels;
                                _info.tags["numXLevels"] = ss.str();
                            }
                            {
                                std::stringstream ss;
                                ss << numYLevels;
                                _info.tags["numYLevels"] = ss.str();
                            }
                            readMipmap();
                        }
                    }
                    else
                    {
                        // Base level requested. Just parse headers.
                        // The actual read() function (called later) will need to check
                        // header.hasTileDescription() to decide whether to create
                        // an InputPart or a TiledInputPart.
                        for (int partNumber = 0; partNumber < numberOfParts; ++partNumber)
                        {
                            const Imf::Header& header = _f->header(partNumber);
                            parseHeader(header, partNumber);
                            
                            // Check for tiling and get counts for base level reads ---
                            if (header.hasTileDescription())
                            {
                                // Temporarily create TiledInputPart just for querying metadata
                                Imf::TiledInputPart tempPart(*_f, partNumber);
                                int numXLevels = tempPart.numXLevels();
                                int numYLevels = tempPart.numYLevels();
                                int numXTiles = tempPart.numXTiles(0); // Level 0
                                int numYTiles = tempPart.numYTiles(0); // Level 0

                                // Assuming single-part/part 0 for simplicity if tags are file-wide
                                if (partNumber == 0)
                                {
                                    {
                                        std::stringstream ss;
                                        ss << numXLevels;
                                        _info.tags["numXLevels"] = ss.str();
                                    }
                                    {
                                        std::stringstream ss;
                                        ss << numYLevels;
                                        _info.tags["numYLevels"] = ss.str();
                                    }
                                    {
                                        std::stringstream ss;
                                        ss << numXTiles;
                                        _info.tags["numXTiles_BaseLevel"] = ss.str();
                                    }
                                    {
                                        std::stringstream ss;
                                        ss << numYTiles;
                                        _info.tags["numYTiles_BaseLevel"] = ss.str();
                                    }
                                }
                            }
                        }
                    }
                }

                const io::Info& getInfo() const { return _info; }

                // Function to upscale RY and BY channels in an interleaved Y,
                // RY, BY image
                template < typename T >
                void upscaleRYBY(
                    T* image, int channel, int numChannels, int width,
                    int height)
                {
                    int srcWidth = width / 2;
                    int srcHeight = height / 2;

                    // Allocate memory for temporary upscaled RY and BY channels
                    T* upscaled = new T[width * height];

                    for (int y = 0; y < height; ++y)
                    {
                        for (int x = 0; x < width; ++x)
                        {
                            // Compute source coordinates
                            float srcX = (x + 0.5f) * srcWidth /
                                             static_cast<float>(width) -
                                         0.5f;
                            float srcY = (y + 0.5f) * srcHeight /
                                             static_cast<float>(height) -
                                         0.5f;

                            // Integer pixel indices
                            int x0 = static_cast<int>(srcX);
                            int y0 = static_cast<int>(srcY);
                            int x1 = std::min(x0 + 1, srcWidth - 1);
                            int y1 = std::min(y0 + 1, srcHeight - 1);

                            // Compute bilinear interpolation weights
                            float wx1 = srcX - x0;
                            float wx0 = 1.0f - wx1;
                            float wy1 = srcY - y0;
                            float wy0 = 1.0f - wy1;

                            int srcIdx00 =
                                (y0 * width + x0) * numChannels + channel;
                            int srcIdx10 =
                                (y0 * width + x1) * numChannels + channel;
                            int srcIdx01 =
                                (y1 * width + x0) * numChannels + channel;
                            int srcIdx11 =
                                (y1 * width + x1) * numChannels + channel;

                            // Read original values
                            float val00 = image[srcIdx00];
                            float val10 = image[srcIdx10];
                            float val01 = image[srcIdx01];
                            float val11 = image[srcIdx11];

                            // Perform bilinear interpolation
                            float interpolated =
                                wx0 * wy0 * val00 + wx1 * wy0 * val10 +
                                wx0 * wy1 * val01 + wx1 * wy1 * val11;

                            // Store in the temporary buffer
                            upscaled[(y * width + x)] = interpolated;
                        }
                    }

                    // Copy back the upscaled RY and BY values into the
                    // interleaved image
                    for (int y = 0; y < height; ++y)
                    {
                        for (int x = 0; x < width; ++x)
                        {
                            image[(y * width + x) * numChannels + channel] =
                                upscaled[(y * width + x)];
                        }
                    }

                    delete[] upscaled;
                }

                void applyChromaticities(
                    std::shared_ptr<image::Image> inout,
                    const image::Info& info, const int minX, const int maxX,
                    const int minY, const int maxY)
                {
                    switch (info.pixelType)
                    {
                    case image::PixelType::L_F16:
                    case image::PixelType::L_F32:
                    case image::PixelType::LA_F16:
                    case image::PixelType::LA_F32:
                        return;
                    default:
                        break;
                    }

                    const size_t channels =
                        image::getChannelCount(info.pixelType);
                    const size_t channelByteCount =
                        image::getBitDepth(info.pixelType) / 8;
                    const size_t cb = channels * channelByteCount;

                    uint8_t* p = inout->getData();

                    Imf::Chromaticities rec709;
                    Imath::M44f M =
                        RGBtoXYZ(_chromaticities, 1) * XYZtoRGB(rec709, 1);
                    Imath::V3f in;
                    Imath::V3f out;
                    for (int y = minY; y <= maxY; ++y)
                    {
                        for (int x = minX; x <= maxX; ++x)
                        {
                            switch (info.pixelType)
                            {
                            case image::PixelType::RGB_F16:
                            case image::PixelType::RGBA_F16:
                            {
                                half* values = reinterpret_cast<half*>(p);
                                in =
                                    Imath::V3f(values[0], values[1], values[2]);
                                out = in * M;
                                values[0] = out[0];
                                values[1] = out[1];
                                values[2] = out[2];
                                break;
                            }
                            case image::PixelType::RGB_F32:
                            case image::PixelType::RGBA_F32:
                            {
                                Imath::V3f* values =
                                    reinterpret_cast<Imath::V3f*>(p);
                                in = *values;
                                out = in * M;
                                in[0] = out[0];
                                in[1] = out[1];
                                in[2] = out[2];
                                break;
                            }
                            default:
                                break;
                            }
                            p += cb;
                        }
                    }
                }

                bool readTiled(
                    io::VideoData& out, const int layer, const int minX,
                    const int maxX, const int minY, const int maxY,
                    Imf::TiledInputPart& tiledInputPart)
                {
                    Imf::Header header = tiledInputPart.header();
                    header.dataWindow() =
                        tiledInputPart.dataWindowForLevel(_xLevel, _yLevel);
                    header.displayWindow() = header.dataWindow();
                    Imf::LineOrder lineOrder = header.lineOrder();

                    image::Info imageInfo = _info.video[layer];
                    out.image = image::Image::create(imageInfo);
                    const size_t channels =
                        image::getChannelCount(imageInfo.pixelType);
                    const size_t channelByteCount =
                        image::getBitDepth(imageInfo.pixelType) / 8;
                    const size_t cb = channels * channelByteCount;
                    const size_t scb =
                        imageInfo.size.w * channels * channelByteCount;

                    Imf::FrameBuffer frameBuffer;
                    bool YBYRY = false;
                    for (size_t c = 0; c < channels; ++c)
                    {
                        const std::string& name =
                            _layers[layer].channels[c].name;
                        const math::Vector2i& sampling =
                            _layers[layer].channels[c].sampling;
                        
                        if (name == "RY" || name == "BY") // <-- Check for YBYRY
                            YBYRY = true;
                        
                        frameBuffer.insert(
                            name.c_str(),
                            Imf::Slice(
                                _layers[layer].channels[c].pixelType,
                                reinterpret_cast<char*>(out.image->getData()) +
                                     (c * channelByteCount),
                                cb, scb, sampling.x, sampling.y, 0.F));
                    }
                    tiledInputPart.setFrameBuffer(frameBuffer);

                    int tx = tiledInputPart.numXTiles(_xLevel);
                    int ty = tiledInputPart.numYTiles(_yLevel);

                    // Read tiles in order for most efficiency.
                    if (lineOrder == Imf::INCREASING_Y)
                    {
                        for (int y = 0; y < ty; ++y)
                            for (int x = 0; x < tx; ++x)
                                tiledInputPart.readTile(x, y, _xLevel, _yLevel);
                    }
                    else
                    {
                        for (int y = ty - 1; y >= 0; --y)
                            for (int x = 0; x < tx; ++x)
                                tiledInputPart.readTile(x, y, _xLevel, _yLevel);
                    }

                    return YBYRY;
                }

                io::VideoData read(
                    const std::string& fileName,
                    const otime::RationalTime& time, const io::Options& options)
                {
                    io::VideoData out;
                    int layer = 0;
                    auto i = options.find("Layer");
                    if (i != options.end())
                    {
                        layer = std::min(
                            std::atoi(i->second.c_str()),
                            static_cast<int>(_info.video.size()) - 1);
                    }

                    // 1. Get header for the current part.
                    const Imf::Header& header =
                        _f->header(_layers[layer].partNumber);

                    // 2. Update window info if not a mipmap read (it was set for mipmap in constructor).
                    if (!_t_part)
                    {
                        // Get the display and data windows which can change
                        // from frame to frame.
                        const auto& displayWindow = header.displayWindow();
                        const auto& dataWindow = header.dataWindow();

                        _displayWindow = fromImath(displayWindow);
                        _dataWindow = fromImath(dataWindow);
                        _intersectedWindow =
                            _displayWindow.intersect(_dataWindow);

                        _info.tags["Display Window"] = serialize(displayWindow);
                        _info.tags["Data Window"] = serialize(dataWindow);
                    }

                    _info.tags["otioClipName"] = fileName;
                    {
                        std::stringstream ss;
                        ss << time;
                        _info.tags["otioClipTime"] = ss.str();
                    }

                    int minY =
                        std::min(_dataWindow.min.y, _displayWindow.min.y);
                    int minX =
                        std::min(_dataWindow.min.x, _displayWindow.min.x);
                    int maxY =
                        std::max(_dataWindow.max.y, _displayWindow.max.y);
                    int maxX =
                        std::max(_dataWindow.max.x, _displayWindow.max.x);
                    image::Info imageInfo = _info.video[layer];
                    
                    // 3. Determine Tiled status and prepare TiledInputPart if necessary
                    bool isTiled = false;
                    std::unique_ptr<Imf::TiledInputPart> tiledPartForRead;
                    bool YBYRY = false;
                    if (_t_part)
                    {
                        // Case 1: Mipmap read (tiled data, _t_part is set from constructor)
                        isTiled = true;
                        readTiled(out, layer, minX, maxX, minY, maxY, *_t_part);
                    }
                    else if (header.hasTileDescription())
                    {
                        // Case 2: Base level tiled read (tiledPartForRead was created above)
                        isTiled = true;
                        tiledPartForRead.reset(new Imf::TiledInputPart(*_f, _layers[layer].partNumber));
                       YBYRY = readTiled(out, layer, minX, maxX, minY, maxY, *tiledPartForRead);
                    }
                    else
                    {
                        // Case 3: Scanline read
                        out.image = image::Image::create(imageInfo);
                        const size_t channels =
                            image::getChannelCount(imageInfo.pixelType);
                        const size_t channelByteCount =
                            image::getBitDepth(imageInfo.pixelType) / 8;
                        const size_t cb = channels * channelByteCount;
                        const size_t scb =
                            imageInfo.size.w * channels * channelByteCount;
                        if (_fast)
                        {
                            Imf::FrameBuffer frameBuffer;
                            for (size_t c = 0; c < channels; ++c)
                            {
                                const std::string& name =
                                    _layers[layer].channels[c].name;
                                const math::Vector2i& sampling =
                                    _layers[layer].channels[c].sampling;
                                if (name == "RY" || name == "BY")
                                    YBYRY = true;

                                frameBuffer.insert(
                                    name.c_str(),
                                    Imf::Slice(
                                        _layers[layer].channels[c].pixelType,
                                        reinterpret_cast<char*>(
                                            out.image->getData()) +
                                            (c * channelByteCount),
                                        cb, scb, sampling.x, sampling.y, 0.F));
                            }
                            Imf::InputPart in(
                                *_f.get(), _layers[layer].partNumber);
                            in.setFrameBuffer(frameBuffer);
                            in.readPixels(
                                _displayWindow.min.y, _displayWindow.max.y);
                        }
                        else
                        {
                            Imf::FrameBuffer frameBuffer;
                            std::vector<char> buf(_dataWindow.w() * cb);
                            for (int c = 0; c < channels; ++c)
                            {
                                const std::string& name =
                                    _layers[layer].channels[c].name;
                                const math::Vector2i& sampling =
                                    _layers[layer].channels[c].sampling;
                                if (name == "RY" || name == "BY")
                                    YBYRY = true;

                                frameBuffer.insert(
                                    name.c_str(),
                                    Imf::Slice(
                                        _layers[layer].channels[c].pixelType,
                                        buf.data() - (_dataWindow.min.x * cb) +
                                            (c * channelByteCount),
                                        cb, 0, sampling.x, sampling.y, 0.F));
                            }
                            Imf::InputPart in(
                                *_f.get(), _layers[layer].partNumber);
                            in.setFrameBuffer(frameBuffer);

                            if (!_ignoreDisplayWindow ||
                                _dataWindow.min.x >= _displayWindow.min.x ||
                                _dataWindow.max.x <= _displayWindow.max.x ||
                                _dataWindow.min.y >= _displayWindow.min.y ||
                                _dataWindow.max.y <= _displayWindow.max.y)
                            {
                                for (int y = _displayWindow.min.y;
                                     y <= _displayWindow.max.y; ++y)
                                {
                                    uint8_t* p =
                                        out.image->getData() +
                                        ((y - _displayWindow.min.y) * scb);
                                    uint8_t* end = p + scb;
                                    if (y >= _intersectedWindow.min.y &&
                                        y <= _intersectedWindow.max.y)
                                    {
                                        size_t size =
                                            (_intersectedWindow.min.x -
                                             _displayWindow.min.x) *
                                            cb;
                                        std::memset(p, 0, size);
                                        p += size;
                                        size = _intersectedWindow.w() * cb;
                                        in.readPixels(y, y);
                                        std::memcpy(
                                            p,
                                            buf.data() +
                                                std::max(
                                                    _displayWindow.min.x -
                                                        _dataWindow.min.x,
                                                    0) *
                                                    cb,
                                            size);
                                        p += size;
                                    }
                                    std::memset(p, 0, end - p);
                                }

                                minY = _displayWindow.min.y;
                                maxY = _displayWindow.max.y;
                                minX = _intersectedWindow.min.x;
                                maxX = _intersectedWindow.max.x;
                            }
                            else
                            {
                                // Display the full data window
                                for (int y = minY; y <= maxY; ++y)
                                {
                                    uint8_t* p =
                                        out.image->getData() + (y - minY) * scb;
                                    uint8_t* end = p + scb;
                                    size_t size = _dataWindow.w() * cb;
                                    in.readPixels(y, y);
                                    std::memcpy(
                                        p,
                                        buf.data() +
                                            std::max(_dataWindow.min.x, 0) * cb,
                                        size);
                                    p += size;
                                    std::memset(p, 0, end - p);
                                }

                                const Imf::Header& header =
                                    _f->header(_layers[layer].partNumber);

                                // Get the display and data windows which can
                                // change from frame to frame.
                                auto display = header.displayWindow();
                                auto data = header.dataWindow();

                                display.min.y += -data.min.y;
                                display.max.y += -data.min.y;
                                display.min.x += -data.min.x;
                                display.max.x += -data.min.x;
                                data.max.y += -data.min.y;
                                data.min.y = 0;
                                data.max.x += -data.min.x;
                                data.min.x = 0;

                                _info.tags["Display Window"] =
                                    serialize(display);
                                _info.tags["Data Window"] = serialize(data);
                            }
                        }

                        if (YBYRY)
                        {
                            uint8_t* origPtr = out.image->getData();
                            int dstWidth = maxX - minX + 1;
                            int dstHeight = maxY - minY + 1;

                            for (int c = 0; c < channels; ++c)
                            {
                                const std::string& name =
                                    _layers[layer].channels[c].name;
                                const math::Vector2i& sampling =
                                    _layers[layer].channels[c].sampling;

                                if ((name == "RY" || name == "BY") &&
                                    sampling.x == 2 && sampling.y == 2)
                                {

                                    switch (imageInfo.pixelType)
                                    {
                                    case image::PixelType::RGB_F16:
                                    case image::PixelType::RGBA_F16:
                                    {
                                        half* src =
                                            reinterpret_cast<half*>(origPtr);
                                        upscaleRYBY(
                                            src, c, channels, dstWidth,
                                            dstHeight);
                                        break;
                                    }
                                    case image::PixelType::RGB_F32:
                                    case image::PixelType::RGBA_F32:
                                    {
                                        float* src =
                                            reinterpret_cast<float*>(origPtr);
                                        upscaleRYBY(
                                            src, c, channels, dstWidth,
                                            dstHeight);
                                        break;
                                    }
                                    default:
                                        break;
                                    }
                                }
                            }

                            switch (imageInfo.pixelType)
                            {
                            case image::PixelType::RGB_F16:
                            case image::PixelType::RGBA_F16:
                            {
                                half* src = reinterpret_cast<half*>(origPtr);
                                ycToRgb(src, channels, dstWidth, dstHeight);
                                break;
                            }
                            case image::PixelType::RGB_F32:
                            case image::PixelType::RGBA_F32:
                            {
                                float* src = reinterpret_cast<float*>(origPtr);
                                ycToRgb(src, channels, dstWidth, dstHeight);
                                break;
                            }
                            default:
                                break;
                            }
                        }

                        if (!_ignoreChromaticities && useChromaticities())
                        {
                            applyChromaticities(
                                out.image, imageInfo, minX, maxX, minY, maxY);
                        }
                    }

                    if (_autoNormalize)
                    {
                        math::Vector4f minimum, maximum;
                        io::normalizeImage(
                            minimum, maximum, out.image, imageInfo, minX, maxX,
                            minY, maxY);

                        _info.tags["Autonormalize Minimum"] =
                            io::serialize(minimum);
                        _info.tags["Autonormalize Maximum"] =
                            io::serialize(maximum);
                    }

                    out.image->setTags(_info.tags);
                    return out;
                }

            private:
                std::string _fileName;
                ChannelGrouping _channelGrouping = ChannelGrouping::Known;
                bool _autoNormalize = false;
                bool _ignoreDisplayWindow = false;
                bool _ignoreChromaticities = false;
                int _xLevel;
                int _yLevel;
                std::weak_ptr<log::System> logSystemWeak;
                Imf::Chromaticities _chromaticities;
                std::unique_ptr<Imf::IStream> _s;
                std::unique_ptr<Imf::TiledInputPart> _t_part;
                std::unique_ptr<Imf::MultiPartInputFile> _f;
                math::Box2i _displayWindow;
                math::Box2i _dataWindow;
                math::Box2i _intersectedWindow;
                std::vector<Layer> _layers;
                bool _fast = false;
                io::Info _info;
            };
        } // namespace

        void Read::_init(
            const file::Path& path, const std::vector<file::MemoryRead>& memory,
            const io::Options& options, const std::shared_ptr<io::Cache>& cache,
            const std::weak_ptr<log::System>& logSystem)
        {
            ISequenceRead::_init(path, memory, options, cache, logSystem);

            auto option = options.find("OpenEXR/ChannelGrouping");
            if (option != options.end())
            {
                std::stringstream ss(option->second);
                ss >> _channelGrouping;
            }

            option = options.find("OpenEXR/IgnoreDisplayWindow");
            if (option != options.end())
            {
                _ignoreDisplayWindow =
                    static_cast<bool>(std::atoi(option->second.c_str()));
            }

            option = options.find("AutoNormalize");
            if (option != options.end())
            {
                _autoNormalize =
                    static_cast<bool>(std::atoi(option->second.c_str()));
            }

            option = options.find("IgnoreChromaticities");
            if (option != options.end())
            {
                _ignoreChromaticities =
                    static_cast<bool>(std::atoi(option->second.c_str()));
            }

            _xLevel = 0;
            option = options.find("X Level");
            if (option != options.end())
            {
                std::stringstream ss(option->second);
                ss >> _xLevel;
            }

            _yLevel = 0;
            option = options.find("Y Level");
            if (option != options.end())
            {
                std::stringstream ss(option->second);
                ss >> _yLevel;
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
            io::Info out =
                File(
                    fileName, memory, _channelGrouping, _ignoreDisplayWindow,
                    false, false, 0, 0, _logSystem.lock())
                    .getInfo();
            float speed = _defaultSpeed;
            auto i = out.tags.find("Frame Per Second");
            if (i != out.tags.end())
            {
                locale::SetAndRestore saved;
                speed = std::stof(i->second);
            }
            i = out.tags.find("FramesPerSecond");
            if (i != out.tags.end())
            {
                int num = 1;
                int den = 24;
                std::stringstream s(i->second);
                s >> num >> den;
                speed = static_cast<double>(num) / static_cast<double>(den);
            }
            out.videoTime =
                otime::TimeRange::range_from_start_end_time_inclusive(
                    otime::RationalTime(_startFrame, speed),
                    otime::RationalTime(_endFrame, speed));
            return out;
        }

        io::VideoData Read::_readVideo(
            const std::string& fileName, const file::MemoryRead* memory,
            const otime::RationalTime& time, const io::Options& options)
        {
            return File(
                       fileName, memory, _channelGrouping, _ignoreDisplayWindow,
                       _ignoreChromaticities, _autoNormalize, _xLevel, _yLevel,
                       _logSystem)
                .read(fileName, time, options);
        }
    } // namespace exr
} // namespace tl
