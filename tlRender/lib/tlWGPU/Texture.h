// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// Copyright (c) 2024-Present Gonzalo Garramuño
// All rights reserved.

#pragma once

#include <tlWGPU/WGPU.h> // must pull in <webgpu/webgpu.h>

#include <tlTimeline/ImageOptions.h>

#include <tlCore/Image.h>
#include <tlCore/Size.h>

#include <cstdint>
#include <memory>
#include <string>

namespace tl
{
    namespace wgpu
    {
        //! Get the WebGPU texture format used to store a pixel type.
        //!
        //! WebGPU has no 3-channel formats, so RGB_* types map to the matching
        //! RGBA format (Texture pads the alpha channel on upload).  YUV types
        //! return WGPUTextureFormat_Undefined.  The 16-bit normalized formats
        //! require the "16-bit norm texture formats" device feature.
        WGPUTextureFormat getTextureFormat(image::PixelType);

        //! Get the number of bytes needed to store a texture.
        std::size_t getDataByteCount(
            const WGPUTextureDimension dimension, uint32_t w, uint32_t h,
            uint32_t d, WGPUTextureFormat format);

        enum class TextureBorder
        {
            ClampToEdge,
            Repeat,
            MirroredRepeat,
            ClampToBorder,
            MirrorClampToEdge,

            Count,
            First = ClampToEdge
        };
        TLRENDER_ENUM(TextureBorder);
        TLRENDER_ENUM_SERIALIZE(TextureBorder);

        struct TextureBorders
        {
            TextureBorder U = TextureBorder::ClampToEdge;
            TextureBorder V = TextureBorder::ClampToEdge;
            TextureBorder W = TextureBorder::ClampToEdge;

            bool operator==(const TextureBorders&) const;
            bool operator!=(const TextureBorders&) const;
        };

        //! WebGPU texture options.
        //!
        //! There is no tiling option (WebGPU hides memory layout).
        struct TextureOptions
        {
            timeline::ImageFilters filters;
            TextureBorders borders;
            WGPUTextureUsage usage =
                WGPUTextureUsage_CopyDst | WGPUTextureUsage_TextureBinding;
            uint32_t samples = 1;

            bool operator==(const TextureOptions&) const;
            bool operator!=(const TextureOptions&) const;
        };

        //! Get the WebGPU texture filter.
        WGPUFilterMode getTextureFilter(timeline::ImageFilter);

        //! Get the WebGPU address mode.  WebGPU has no border color, so
        //! ClampToBorder and MirrorClampToEdge fall back to ClampToEdge.
        WGPUAddressMode getTextureBorder(TextureBorder);

        //! WebGPU texture.
        //!
        //! Uploads go through wgpuQueueWriteTexture(): no staging buffers,
        //! command pools, fences or layout transitions are needed, and the
        //! source row pitch can be passed straight through (no 256-byte
        //! alignment restriction, unlike buffer-to-texture copies).
        class Texture : public std::enable_shared_from_this<Texture>
        {
            TLRENDER_NON_COPYABLE(Texture);

        protected:
            void
            _init(const image::Info&, const TextureOptions& = TextureOptions());

            void _init(
                const WGPUTextureDimension dimension, const uint32_t width,
                const uint32_t height, const uint32_t depth,
                const WGPUTextureFormat format, const std::string& name,
                const TextureOptions& = TextureOptions());

            Texture(Fl_WebGPU_Context&);

        public:
            ~Texture();

            //! Create a new texture.  \p device and \p queue are not owned
            //! and must outlive the texture.
            static std::shared_ptr<Texture> create(
                Fl_WebGPU_Context&, const image::Info&,
                const TextureOptions& = TextureOptions());

            static std::shared_ptr<Texture> create(
                Fl_WebGPU_Context&,
                const WGPUTextureDimension dimension, const uint32_t width,
                const uint32_t height, const uint32_t depth,
                const WGPUTextureFormat format,
                const std::string& name = "sampler1",
                const TextureOptions& = TextureOptions());

            //! Get the image information.  For RGB sources this describes the
            //! RGBA pixel type actually stored on the GPU.
            const image::Info& getInfo() const;

            //! Get the size.
            const image::Size& getSize() const;

            //! Get the options.
            const TextureOptions& getOptions() const;

            //! Get the width.
            int getWidth() const;

            //! Get the height.
            int getHeight() const;

            //! Get the depth.
            int getDepth() const;

            //! Get the pixel type.
            image::PixelType getPixelType() const;

            //! Get the name.
            const std::string& getName() const;

            //! \name Copy
            //! Copy image data to the texture.  Row pitches are in bytes; 0
            //! means tightly packed.
            ///@{

            //! Source data has 3 channels but the texture has 4: the alpha
            //! channel is filled in with "opaque" during upload.
            void setRGBToRGBA(bool t);

            void copy(const std::shared_ptr<image::Image>&);
            void copy(
                const uint8_t*, const image::Info&, const int rowPitch = 0);
            void copy(const uint8_t*, const std::size_t, const int rowPitch = 0);

            //! Copy an image into a sub-region starting at (x, y).
            void copy(const std::shared_ptr<image::Image>&, int x, int y);

            ///@}

            //! WebGPU's internal format representation.
            WGPUTextureFormat getInternalFormat() const;

            WGPUTexture getTexture() const;

            WGPUTextureView getTextureView() const;

            WGPUSampler getSampler() const;

            //! Convenience: bind group entries for this texture's view and
            //! sampler (replaces VkDescriptorImageInfo).
            WGPUBindGroupEntry getTextureBindGroupEntry(uint32_t binding) const;
            WGPUBindGroupEntry getSamplerBindGroupEntry(uint32_t binding) const;

            //! Get the number of objects currenty instantiated.
            static size_t getObjectCount();

            //! Get the total number of bytes currently used.
            static size_t getTotalByteCount();

        private:
            WGPUDevice _device = nullptr;
            WGPUQueue _queue = nullptr;

            void createTexture();
            void createTextureView();
            void createSampler();

            TLRENDER_PRIVATE();
        };

        //! Check whether the texture should be created or re-created.
        bool doCreate(
            const std::shared_ptr<Texture>&, const math::Size2i&,
            const TextureOptions&);
    } // namespace webgpu
} // namespace tl
