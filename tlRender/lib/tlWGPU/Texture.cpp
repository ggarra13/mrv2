// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// Copyright (c) 2025-Present Gonzalo Garramuño
// All rights reserved.

#include <tlWGPU/Texture.h>

#include <tlCore/Assert.h>
#include <tlCore/Error.h>
#include <tlCore/String.h>
#include <tlCore/Util.h>

#include <algorithm>
#include <array>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <map>
#include <mutex>
#include <stdexcept>
#include <string>
#include <tuple>
#include <vector>

// The 16-bit normalized texture formats (R16Unorm, RG16Unorm, RGBA16Unorm)
// are gated behind a device feature whose name differs between
// implementations / header versions.  Override this macro if needed, e.g.
// for wgpu-native: (WGPUFeatureName)WGPUNativeFeature_TextureFormat16bitNorm
#ifndef TLRENDER_WGPU_FEATURE_16BIT_NORM
#    define TLRENDER_WGPU_FEATURE_16BIT_NORM \
        WGPUFeatureName_Unorm16TextureFormats
#endif

namespace tl
{
    namespace wgpu
    {
        TLRENDER_ENUM_IMPL(
            TextureBorder,
            "ClampToEdge",
            "Repeat",
            "MirroredRepeat",
            "ClampToBorder",
            "MirrorClampToEdge");
        TLRENDER_ENUM_SERIALIZE_IMPL(TextureBorder);

        WGPUAddressMode getTextureBorder(TextureBorder value)
        {
            WGPUAddressMode out = WGPUAddressMode_ClampToEdge;
            switch (value)
            {
            case TextureBorder::ClampToEdge:
                out = WGPUAddressMode_ClampToEdge;
                break;
            case TextureBorder::Repeat:
                out = WGPUAddressMode_Repeat;
                break;
            case TextureBorder::MirroredRepeat:
                out = WGPUAddressMode_MirrorRepeat;
                break;
            case TextureBorder::ClampToBorder:
            case TextureBorder::MirrorClampToEdge:
            {
                // WebGPU has neither border colors nor mirror-clamp.
                static std::atomic<bool> warned{false};
                if (!warned.exchange(true))
                {
                    std::cerr << "tl::webgpu::Texture: border mode '"
                              << getLabel(value)
                              << "' is not supported by WebGPU, using "
                                 "ClampToEdge"
                              << std::endl;
                }
                out = WGPUAddressMode_ClampToEdge;
                break;
            }
            default:
                std::cerr << "Unhandled texture sampler border "
                          << (int)value << std::endl;
                break;
            };
            return out;
        }

        namespace
        {
            bool isUnorm16Format(WGPUTextureFormat f)
            {
                return f == WGPUTextureFormat_R16Unorm ||
                       f == WGPUTextureFormat_RG16Unorm ||
                       f == WGPUTextureFormat_RGBA16Unorm;
            }

            bool isFloat32Format(WGPUTextureFormat f)
            {
                return f == WGPUTextureFormat_R32Float ||
                       f == WGPUTextureFormat_RG32Float ||
                       f == WGPUTextureFormat_RGBA32Float;
            }

            bool isUintFormat(WGPUTextureFormat f)
            {
                return f == WGPUTextureFormat_R32Uint ||
                       f == WGPUTextureFormat_RG32Uint ||
                       f == WGPUTextureFormat_RGBA32Uint;
            }

            //! Bytes per texel of the (uncompressed color) formats we use.
            std::size_t getBytesPerTexel(WGPUTextureFormat f)
            {
                switch (f)
                {
                case WGPUTextureFormat_R8Unorm:
                    return 1;
                case WGPUTextureFormat_RG8Unorm:
                case WGPUTextureFormat_R16Unorm:
                case WGPUTextureFormat_R16Float:
                    return 2;
                case WGPUTextureFormat_RGBA8Unorm:
                case WGPUTextureFormat_RG16Unorm:
                case WGPUTextureFormat_RG16Float:
                case WGPUTextureFormat_R32Float:
                case WGPUTextureFormat_R32Uint:
                case WGPUTextureFormat_RGB10A2Unorm:
                    return 4;
                case WGPUTextureFormat_RGBA16Unorm:
                case WGPUTextureFormat_RGBA16Float:
                case WGPUTextureFormat_RG32Float:
                case WGPUTextureFormat_RG32Uint:
                    return 8;
                case WGPUTextureFormat_RGBA32Float:
                case WGPUTextureFormat_RGBA32Uint:
                    return 16;
                default:
                    break;
                }
                return 0;
            }

            //! Pixel type to describe a raw WebGPU format (used by the
            //! format-based constructor).
            image::PixelType pixelTypeFromFormat(WGPUTextureFormat f)
            {
                switch (f)
                {
                case WGPUTextureFormat_R8Unorm:
                    return image::PixelType::L_U8;
                case WGPUTextureFormat_R16Float:
                    return image::PixelType::L_F16;
                case WGPUTextureFormat_R32Float:
                    return image::PixelType::L_F32;
                case WGPUTextureFormat_RGBA8Unorm:
                    return image::PixelType::RGBA_U8;
                case WGPUTextureFormat_RGBA16Float:
                    return image::PixelType::RGBA_F16;
                case WGPUTextureFormat_RGBA32Float:
                default:
                    return image::PixelType::RGBA_F32;
                }
            }

            //! Alpha value (as raw bits) used when padding RGB to RGBA.
            uint32_t opaqueAlphaBits(WGPUTextureFormat f)
            {
                switch (f)
                {
                case WGPUTextureFormat_RGBA8Unorm:
                    return 0xFFu;
                case WGPUTextureFormat_RGBA16Unorm:
                    return 0xFFFFu;
                case WGPUTextureFormat_RGBA16Float:
                    return 0x3C00u; // 1.0 as half
                case WGPUTextureFormat_RGBA32Float:
                    return 0x3F800000u; // 1.0f
                case WGPUTextureFormat_RGBA32Uint:
                    return 0xFFFFFFFFu;
                default:
                    break;
                }
                throw std::runtime_error(
                    "tl::webgpu::Texture: unsupported format for RGB to "
                    "RGBA conversion");
            }

            constexpr uint64_t alignUp(uint64_t value, uint64_t alignment)
            {
                return alignment ? (value + alignment - 1) / alignment *
                                       alignment
                                 : value;
            }

            //! Bytes per pixel of an image::PixelType as stored in an
            //! image::Image.
            std::size_t getPixelByteSize(image::PixelType type)
            {
                if (image::PixelType::RGB_U10 == type)
                    return sizeof(uint32_t);
                return image::getBitDepth(type) / 8 *
                       image::getChannelCount(type);
            }

            void checkFormatSupported(WGPUDevice device, WGPUTextureFormat f)
            {
                if (WGPUTextureFormat_Undefined == f ||
                    0 == getBytesPerTexel(f))
                {
                    throw std::runtime_error(
                        "tl::webgpu::Texture: unsupported texture format");
                }
                if (isUnorm16Format(f) &&
                    !wgpuDeviceHasFeature(
                        device, TLRENDER_WGPU_FEATURE_16BIT_NORM))
                {
                    throw std::runtime_error(
                        "tl::webgpu::Texture: 16-bit normalized texture "
                        "formats need the 16-bit-norm device feature; "
                        "request it when creating the device");
                }
            }

            //! Float32 textures are only filterable with an optional
            //! feature, and integer textures never are.
            bool isFilterable(WGPUDevice device, WGPUTextureFormat f)
            {
                if (isUintFormat(f))
                    return false;
                if (isFloat32Format(f))
                    return wgpuDeviceHasFeature(
                        device, WGPUFeatureName_Float32Filterable);
                return true;
            }

            // ----------------------------------------------------------------
            //  Sampler cache (replaces SamplersCache): one sampler per unique
            //  filter/address-mode combination per device.
            // ----------------------------------------------------------------
            class SamplerCache
            {
            public:
                WGPUSampler get(
                    WGPUDevice device, WGPUFilterMode mag, WGPUFilterMode min,
                    WGPUAddressMode u, WGPUAddressMode v, WGPUAddressMode w)
                {
                    const Key key(device, mag, min, u, v, w);

                    std::lock_guard<std::mutex> lock(_mutex);
                    auto it = _samplers.find(key);
                    if (it != _samplers.end())
                        return it->second;

                    WGPUSamplerDescriptor desc = {};
                    desc.label = {"tl::webgpu::Texture sampler", WGPU_STRLEN};
                    desc.addressModeU = u;
                    desc.addressModeV = v;
                    desc.addressModeW = w;
                    desc.magFilter = mag;
                    desc.minFilter = min;
                    desc.mipmapFilter = WGPUMipmapFilterMode_Nearest;
                    desc.lodMinClamp = 0.F;
                    desc.lodMaxClamp = 32.F;
                    desc.compare = WGPUCompareFunction_Undefined;
                    desc.maxAnisotropy = 1;

                    WGPUSampler sampler = wgpuDeviceCreateSampler(device, &desc);
                    if (!sampler)
                    {
                        throw std::runtime_error(
                            "tl::webgpu::Texture: failed to create sampler");
                    }
                    _samplers[key] = sampler;
                    return sampler;
                }

                void clear()
                {
                    std::lock_guard<std::mutex> lock(_mutex);
                    for (auto& i : _samplers)
                        wgpuSamplerRelease(i.second);
                    _samplers.clear();
                }

            private:
                using Key = std::tuple<
                    WGPUDevice, WGPUFilterMode, WGPUFilterMode, WGPUAddressMode,
                    WGPUAddressMode, WGPUAddressMode>;

                std::mutex _mutex;
                std::map<Key, WGPUSampler> _samplers;
            };

            SamplerCache& samplerCache()
            {
                static SamplerCache cache;
                return cache;
            }
        } // namespace

        std::size_t getDataByteCount(
            const WGPUTextureDimension dimension, uint32_t w, uint32_t h,
            uint32_t d, WGPUTextureFormat format)
        {
            switch (dimension)
            {
            case WGPUTextureDimension_1D:
                h = d = 1;
                break;
            case WGPUTextureDimension_2D:
                d = 1;
                break;
            default:
                break;
            }
            return static_cast<std::size_t>(w) * h * d *
                   getBytesPerTexel(format);
        }

        WGPUTextureFormat getTextureFormat(image::PixelType type)
        {
            const std::array<
                WGPUTextureFormat,
                static_cast<std::size_t>(image::PixelType::Count)>
                data = {
                    WGPUTextureFormat_Undefined,

                    WGPUTextureFormat_R8Unorm,  // L_U8
                    WGPUTextureFormat_R16Unorm, // L_U16 (feature)
                    WGPUTextureFormat_R32Uint,  // L_U32
                    WGPUTextureFormat_R16Float, // L_F16
                    WGPUTextureFormat_R32Float, // L_F32

                    WGPUTextureFormat_RG8Unorm,  // LA_U8
                    WGPUTextureFormat_RG16Unorm, // LA_U16 (feature)
                    WGPUTextureFormat_RG32Uint,  // LA_U32
                    WGPUTextureFormat_RG16Float, // LA_F16
                    WGPUTextureFormat_RG32Float, // LA_F32

                    // No 3-channel formats in WebGPU: RGB is stored as RGBA.
                    WGPUTextureFormat_RGBA8Unorm,     // RGB_U8
                    WGPUTextureFormat_RGB10A2Unorm,   // RGB_U10
                    WGPUTextureFormat_RGBA16Unorm,    // RGB_U16 (feature)
                    WGPUTextureFormat_RGBA32Uint,     // RGB_U32
                    WGPUTextureFormat_RGBA16Float,    // RGB_F16
                    WGPUTextureFormat_RGBA32Float,    // RGB_F32

                    WGPUTextureFormat_RGBA8Unorm,  // RGBA_U8
                    WGPUTextureFormat_RGBA16Unorm, // RGBA_U16 (feature)
                    WGPUTextureFormat_RGBA32Uint,  // RGBA_U32
                    WGPUTextureFormat_RGBA16Float, // RGBA_F16
                    WGPUTextureFormat_RGBA32Float, // RGBA_F32

                    WGPUTextureFormat_Undefined, // YUV_420P_U8
                    WGPUTextureFormat_Undefined, // YUV_422P_U8
                    WGPUTextureFormat_Undefined, // YUV_444P_U8

                    WGPUTextureFormat_Undefined, // YUV_420P_U10
                    WGPUTextureFormat_Undefined, // YUV_422P_U10
                    WGPUTextureFormat_Undefined, // YUV_444P_U10

                    WGPUTextureFormat_Undefined, // YUV_420P_U12
                    WGPUTextureFormat_Undefined, // YUV_422P_U12
                    WGPUTextureFormat_Undefined, // YUV_444P_U12

                    WGPUTextureFormat_Undefined, // YUV_420P_U16
                    WGPUTextureFormat_Undefined, // YUV_422P_U16
                    WGPUTextureFormat_Undefined, // YUV_444P_U16

                    WGPUTextureFormat_Undefined, // YUV_420SP_U8
                    WGPUTextureFormat_Undefined, // YUV_420SP_U16

                    WGPUTextureFormat_Undefined, // YUV_422SP_U8
                    WGPUTextureFormat_Undefined, // YUV_422SP_U16

                    WGPUTextureFormat_Undefined, // YUV_444SP_U8
                    WGPUTextureFormat_Undefined, // YUV_444SP_U16

                    WGPUTextureFormat_RGBA8Unorm, // ARGB_4444_Premult
                };
            return data[static_cast<std::size_t>(type)];
        }

        WGPUFilterMode getTextureFilter(timeline::ImageFilter value)
        {
            const std::array<
                WGPUFilterMode,
                static_cast<std::size_t>(timeline::ImageFilter::Count)>
                data = {WGPUFilterMode_Nearest, WGPUFilterMode_Linear};
            return data[static_cast<std::size_t>(value)];
        }

        bool TextureBorders::operator==(const TextureBorders& other) const
        {
            return U == other.U && V == other.V && W == other.W;
        }

        bool TextureBorders::operator!=(const TextureBorders& other) const
        {
            return !(*this == other);
        }

        bool TextureOptions::operator==(const TextureOptions& other) const
        {
            return filters == other.filters && borders == other.borders &&
                   usage == other.usage && samples == other.samples;
        }

        bool TextureOptions::operator!=(const TextureOptions& other) const
        {
            return !(*this == other);
        }

        namespace
        {
            std::atomic<size_t> objectCount = 0;
            std::atomic<size_t> totalByteCount = 0;
        } // namespace

        size_t Texture::getTotalByteCount()
        {
            return totalByteCount;
        }

        size_t Texture::getObjectCount()
        {
            return objectCount;
        }

        struct Texture::Private
        {
            WGPUDevice device = nullptr;
            WGPUQueue queue = nullptr;

            image::Info info;
            TextureOptions options;
            std::string name;

            WGPUTextureDimension dimension = WGPUTextureDimension_2D;
            uint32_t depth = 1;

            WGPUTexture texture = nullptr;
            WGPUTextureView view = nullptr;
            WGPUSampler sampler = nullptr; // owned by the sampler cache

            WGPUTextureFormat internalFormat = WGPUTextureFormat_Undefined;

            // Source data has 3 channels, texture has 4.
            bool needPadRgbToRgba = false;
            // Source is A2R10G10B10 (Vulkan layout); WebGPU's RGB10A2Unorm
            // is A2B10G10R10, so red and blue must be swapped.
            bool swizzleRgb10 = false;

            std::size_t dstBytesPerPixel() const
            {
                return getBytesPerTexel(internalFormat);
            }

            //! Bytes per pixel of the data the caller passes in.
            std::size_t srcBytesPerPixel() const
            {
                const std::size_t dst = dstBytesPerPixel();
                return needPadRgbToRgba ? (dst / 4) * 3 : dst;
            }

            //! Upload a region.  \p srcRowPitch is in bytes (0 = tight).
            void write(
                const uint8_t* src, std::size_t srcSize,
                std::size_t srcRowPitch, uint32_t x, uint32_t y, uint32_t w,
                uint32_t h, uint32_t d)
            {
                if (!src || 0 == w || 0 == h || 0 == d)
                    return;

                const std::size_t srcPixel = srcBytesPerPixel();
                const std::size_t dstPixel = dstBytesPerPixel();
                const std::size_t srcRowBytes = w * srcPixel;
                if (0 == srcRowPitch)
                    srcRowPitch = srcRowBytes;
                else if (srcRowPitch < srcRowBytes)
                    throw std::runtime_error(
                        "tl::webgpu::Texture::copy: row pitch is smaller "
                        "than a row");

                const std::size_t needed =
                    srcRowPitch * h * (d - 1) + srcRowPitch * (h - 1) +
                    srcRowBytes;
                if (srcSize < needed)
                    throw std::runtime_error(
                        "tl::webgpu::Texture::copy: source data is smaller "
                        "than the region being copied");

                const uint8_t* upload = src;
                std::size_t uploadPitch = srcRowPitch;
                std::size_t uploadSize = srcSize;

                std::vector<uint8_t> converted;
                if (needPadRgbToRgba || swizzleRgb10)
                {
                    const std::size_t dstRowBytes = w * dstPixel;
                    converted.resize(dstRowBytes * h * d);

                    if (needPadRgbToRgba)
                    {
                        const std::size_t comp = dstPixel / 4;
                        const uint32_t alpha = opaqueAlphaBits(internalFormat);
                        for (uint32_t z = 0; z < d; ++z)
                        {
                            for (uint32_t row = 0; row < h; ++row)
                            {
                                const uint8_t* s =
                                    src + (z * h + row) * srcRowPitch;
                                uint8_t* dst =
                                    converted.data() +
                                    (z * h + row) * dstRowBytes;
                                for (uint32_t col = 0; col < w; ++col)
                                {
                                    std::memcpy(dst, s, 3 * comp);
                                    // Little-endian: the low `comp` bytes of
                                    // the 32-bit pattern are the value.
                                    std::memcpy(dst + 3 * comp, &alpha, comp);
                                    s += 3 * comp;
                                    dst += 4 * comp;
                                }
                            }
                        }
                    }
                    else
                    {
                        for (uint32_t z = 0; z < d; ++z)
                        {
                            for (uint32_t row = 0; row < h; ++row)
                            {
                                const uint8_t* s =
                                    src + (z * h + row) * srcRowPitch;
                                uint8_t* dst =
                                    converted.data() +
                                    (z * h + row) * dstRowBytes;
                                for (uint32_t col = 0; col < w; ++col)
                                {
                                    uint32_t v;
                                    std::memcpy(&v, s, sizeof(v));
                                    const uint32_t r = (v >> 20) & 0x3FFu;
                                    const uint32_t g = (v >> 10) & 0x3FFu;
                                    const uint32_t b = v & 0x3FFu;
                                    // RGB has no alpha: force opaque.
                                    const uint32_t out =
                                        r | (g << 10) | (b << 20) | (3u << 30);
                                    std::memcpy(dst, &out, sizeof(out));
                                    s += sizeof(v);
                                    dst += sizeof(out);
                                }
                            }
                        }
                    }

                    upload = converted.data();
                    uploadPitch = dstRowBytes;
                    uploadSize = converted.size();
                }

                WGPUTexelCopyTextureInfo dstInfo = {};
                dstInfo.texture = texture;
                dstInfo.mipLevel = 0;
                dstInfo.origin = {x, y, 0};
                dstInfo.aspect = WGPUTextureAspect_All;

                WGPUTexelCopyBufferLayout layout = {};
                layout.offset = 0;
                layout.bytesPerRow = static_cast<uint32_t>(uploadPitch);
                layout.rowsPerImage = h;

                const WGPUExtent3D extent = {w, h, d};
                wgpuQueueWriteTexture(
                    queue, &dstInfo, upload, uploadSize, &layout, &extent);
            }
        };

        void
        Texture::_init(const image::Info& info, const TextureOptions& options)
        {
            TLRENDER_P();
            p.device = _device;
            p.queue = _queue;

            p.info = info;
            if (!p.info.isValid())
            {
                throw std::runtime_error("Invalid texture");
            }
            p.internalFormat = getTextureFormat(info.pixelType);
            p.options = options;
            p.dimension = WGPUTextureDimension_2D;
            p.depth = 1;

            // WebGPU has no 3-channel formats: store RGB as RGBA and pad the
            // alpha channel on upload.
            switch (info.pixelType)
            {
            case image::PixelType::RGB_U8:
                p.info.pixelType = image::PixelType::RGBA_U8;
                p.needPadRgbToRgba = true;
                break;
            case image::PixelType::RGB_U16:
                p.info.pixelType = image::PixelType::RGBA_U16;
                p.needPadRgbToRgba = true;
                break;
            case image::PixelType::RGB_U32:
                p.info.pixelType = image::PixelType::RGBA_U32;
                p.needPadRgbToRgba = true;
                break;
            case image::PixelType::RGB_F16:
                p.info.pixelType = image::PixelType::RGBA_F16;
                p.needPadRgbToRgba = true;
                break;
            case image::PixelType::RGB_F32:
                p.info.pixelType = image::PixelType::RGBA_F32;
                p.needPadRgbToRgba = true;
                break;
            case image::PixelType::RGB_U10:
                p.swizzleRgb10 = true;
                break;
            default:
                break;
            }

            createTexture();
            createTextureView();
            createSampler();

            totalByteCount += getDataByteCount(
                p.dimension, p.info.size.w, p.info.size.h, p.depth,
                p.internalFormat);
            ++objectCount;
        }

        void Texture::_init(
            const WGPUTextureDimension dimension, const uint32_t width,
            const uint32_t height, const uint32_t depth,
            const WGPUTextureFormat format, const std::string& name,
            const TextureOptions& options)
        {
            TLRENDER_P();
            p.device = _device;
            p.queue = _queue;

            p.dimension = dimension;
            p.info.size.w = width;
            p.info.size.h =
                (WGPUTextureDimension_1D == dimension) ? 1 : height;
            p.depth = (WGPUTextureDimension_3D == dimension)
                          ? std::max(depth, 1u)
                          : 1;
            p.internalFormat = format;
            p.needPadRgbToRgba = false;
            p.options = options;
            p.name = name;

            // This is CRITICAL for Texture::copy logic to work.
            p.info.pixelType = pixelTypeFromFormat(format);

            createTexture();
            createTextureView();
            createSampler();

            totalByteCount += getDataByteCount(
                p.dimension, p.info.size.w, p.info.size.h, p.depth,
                p.internalFormat);
            ++objectCount;
        }

        Texture::Texture(Fl_WebGPU_Context& ctx) :
            _device(ctx.device),
            _queue(ctx.queue()),
            _p(new Private)
        {
        }

        Texture::~Texture()
        {
            TLRENDER_P();

            // Release only drops our reference; the implementation keeps the
            // texture alive until submitted work using it has finished.
            if (p.view)
                wgpuTextureViewRelease(p.view);
            if (p.texture)
                wgpuTextureRelease(p.texture);

            if (p.internalFormat != WGPUTextureFormat_Undefined)
            {
                totalByteCount -= getDataByteCount(
                    p.dimension, p.info.size.w, p.info.size.h, p.depth,
                    p.internalFormat);
                if (0 == --objectCount)
                {
                    samplerCache().clear();
                }
            }
        }

        std::shared_ptr<Texture> Texture::create(
            Fl_WebGPU_Context& ctx,
            const image::Info& info,
            const TextureOptions& options)
        {
            auto out = std::shared_ptr<Texture>(new Texture(ctx));
            out->_init(info, options);
            return out;
        }

        std::shared_ptr<Texture> Texture::create(
            Fl_WebGPU_Context& ctx,
            const WGPUTextureDimension dimension, const uint32_t width,
            const uint32_t height, const uint32_t depth,
            const WGPUTextureFormat format, const std::string& name,
            const TextureOptions& options)
        {
            auto out = std::shared_ptr<Texture>(new Texture(ctx));
            out->_init(dimension, width, height, depth, format, name, options);
            return out;
        }

        const image::Info& Texture::getInfo() const
        {
            return _p->info;
        }

        const image::Size& Texture::getSize() const
        {
            return _p->info.size;
        }

        int Texture::getWidth() const
        {
            return _p->info.size.w;
        }

        int Texture::getHeight() const
        {
            return _p->info.size.h;
        }

        int Texture::getDepth() const
        {
            return _p->depth;
        }

        image::PixelType Texture::getPixelType() const
        {
            return _p->info.pixelType;
        }

        const std::string& Texture::getName() const
        {
            return _p->name;
        }

        WGPUTextureFormat Texture::getInternalFormat() const
        {
            return _p->internalFormat;
        }

        WGPUTextureView Texture::getTextureView() const
        {
            return _p->view;
        }

        WGPUSampler Texture::getSampler() const
        {
            return _p->sampler;
        }

        WGPUTexture Texture::getTexture() const
        {
            return _p->texture;
        }

        WGPUBindGroupEntry
        Texture::getTextureBindGroupEntry(uint32_t binding) const
        {
            WGPUBindGroupEntry out = {};
            out.binding = binding;
            out.textureView = _p->view;
            return out;
        }

        WGPUBindGroupEntry
        Texture::getSamplerBindGroupEntry(uint32_t binding) const
        {
            WGPUBindGroupEntry out = {};
            out.binding = binding;
            out.sampler = _p->sampler;
            return out;
        }

        void Texture::setRGBToRGBA(bool value)
        {
            TLRENDER_P();
            p.needPadRgbToRgba = value;
        }

        // ====================================================================
        //  Copy
        //
        //  wgpuQueueWriteTexture() stages the data and orders the copy before
        //  any later submit, so none of the Vulkan staging buffer / command
        //  pool / fence / layout transition code is needed.
        // ====================================================================

        void
        Texture::copy(const std::shared_ptr<image::Image>& data, int x, int y)
        {
            TLRENDER_P();

            const auto& info = data->getInfo();
            const int w = info.size.w;
            const int h = info.size.h;
            if (x < 0 || y < 0 || x + w > p.info.size.w ||
                y + h > p.info.size.h)
            {
                throw std::runtime_error(
                    "tl::webgpu::Texture::copy: region is outside the "
                    "texture");
            }

            // Rows of an image::Image are aligned to layout.alignment.
            const std::size_t rowBytes = static_cast<std::size_t>(w) *
                                         p.srcBytesPerPixel();
            const std::size_t alignment =
                std::max<std::size_t>(info.layout.alignment, 1);
            const std::size_t rowPitch = alignUp(rowBytes, alignment);

            p.write(
                data->getData(), data->getDataByteCount(), rowPitch,
                static_cast<uint32_t>(x), static_cast<uint32_t>(y), w, h, 1);
        }

        void Texture::copy(
            const uint8_t* data, const std::size_t size, const int rowPitch)
        {
            TLRENDER_P();
            p.write(
                data, size, rowPitch > 0 ? static_cast<std::size_t>(rowPitch) : 0,
                0, 0, p.info.size.w, p.info.size.h, p.depth);
        }

        void Texture::copy(
            const uint8_t* data, const image::Info& info, const int rowPitch)
        {
            const size_t size = image::getDataByteCount(info);
            copy(data, size, rowPitch);
        }

        void Texture::copy(const std::shared_ptr<image::Image>& data)
        {
            copy(data, 0, 0);
        }

        // ====================================================================
        //  Resource creation
        // ====================================================================

        void Texture::createTexture()
        {
            TLRENDER_P();

            checkFormatSupported(_device, p.internalFormat);

            // Check against the device limits (replaces
            // vkGetPhysicalDeviceImageFormatProperties2).
            WGPULimits limits = {};
            if (wgpuDeviceGetLimits(_device, &limits) == WGPUStatus_Success)
            {
                uint32_t maxDim = limits.maxTextureDimension2D;
                if (WGPUTextureDimension_1D == p.dimension)
                    maxDim = limits.maxTextureDimension1D;
                else if (WGPUTextureDimension_3D == p.dimension)
                    maxDim = limits.maxTextureDimension3D;

                const uint32_t w = p.info.size.w;
                const uint32_t h = p.info.size.h;
                if (w > maxDim || h > maxDim ||
                    (WGPUTextureDimension_3D == p.dimension && p.depth > maxDim))
                {
                    throw std::runtime_error(
                        "tl::webgpu::Texture: size " + std::to_string(w) +
                        "x" + std::to_string(h) + " exceeds the device limit " +
                        std::to_string(maxDim));
                }
            }

            WGPUTextureDescriptor desc = {};
            if (p.name.empty())
                desc.label = {nullptr, WGPU_STRLEN};
            else
                desc.label = {p.name.c_str(), p.name.size()};
            desc.usage = p.options.usage;
            desc.dimension = p.dimension;
            desc.size = {
                static_cast<uint32_t>(p.info.size.w),
                static_cast<uint32_t>(p.info.size.h), p.depth};
            desc.format = p.internalFormat;
            desc.mipLevelCount = 1;
            desc.sampleCount = p.options.samples;

            // On failure WebGPU reports through the device's error callback
            // (and hands back an error object) rather than returning null.
            p.texture = wgpuDeviceCreateTexture(_device, &desc);
            if (!p.texture)
            {
                throw std::runtime_error(
                    "tl::webgpu::Texture: failed to create texture");
            }
        }

        void Texture::createTextureView()
        {
            TLRENDER_P();

            WGPUTextureViewDescriptor desc = {};
            desc.format = p.internalFormat;
            switch (p.dimension)
            {
            case WGPUTextureDimension_1D:
                desc.dimension = WGPUTextureViewDimension_1D;
                break;
            case WGPUTextureDimension_3D:
                desc.dimension = WGPUTextureViewDimension_3D;
                break;
            case WGPUTextureDimension_2D:
            default:
                desc.dimension = WGPUTextureViewDimension_2D;
                break;
            }
            desc.baseMipLevel = 0;
            desc.mipLevelCount = 1;
            desc.baseArrayLayer = 0;
            desc.arrayLayerCount = 1;
            desc.aspect = WGPUTextureAspect_All;

            p.view = wgpuTextureCreateView(p.texture, &desc);
            if (!p.view)
            {
                throw std::runtime_error(
                    "tl::webgpu::Texture: failed to create texture view");
            }
        }

        void Texture::createSampler()
        {
            TLRENDER_P();

            WGPUFilterMode magFilter = getTextureFilter(p.options.filters.magnify);
            WGPUFilterMode minFilter = getTextureFilter(p.options.filters.minify);

            // A filtering sampler can't be used with these formats.
            if (!isFilterable(_device, p.internalFormat))
            {
                magFilter = WGPUFilterMode_Nearest;
                minFilter = WGPUFilterMode_Nearest;
            }

            p.sampler = samplerCache().get(
                _device, magFilter, minFilter,
                getTextureBorder(p.options.borders.U),
                getTextureBorder(p.options.borders.V),
                getTextureBorder(p.options.borders.W));
        }

        const TextureOptions& Texture::getOptions() const
        {
            return _p->options;
        }

        bool doCreate(
            const std::shared_ptr<Texture>& texture, const math::Size2i& size,
            const TextureOptions& options)
        {
            bool out = false;
            out |= size.isValid() && !texture;
            out |= size.isValid() && texture &&
                   (texture->getWidth() != size.w ||
                    texture->getHeight() != size.h);
            out |= texture && texture->getOptions() != options;
            return out;
        }
    } // namespace webgpu
} // namespace tl
