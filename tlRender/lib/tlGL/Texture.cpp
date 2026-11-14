// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlGL/Texture.h>

#include <tlGL/GL.h>
#include <tlGL/Init.h>

#include <tlCore/Assert.h>

#include <array>
#include <atomic>
#include <cstring>
#include <stdexcept>
#include <tuple>

namespace tl
{
    namespace gl
    {

        namespace
        {
            constexpr std::size_t pixelTypeCount =
                static_cast<std::size_t>(image::PixelType::Count);

            //! Build a lookup table indexed by image::PixelType. The size is
            //! checked against image::PixelType::Count at compile time so a
            //! table can never silently fall out of sync with the enum.
            template <typename... Args>
            constexpr std::array<GLenum, sizeof...(Args)> makeTable(Args... args)
            {
                return {{static_cast<GLenum>(args)...}};
            }
        } // namespace

        unsigned int getTextureFormat(image::PixelType value)
        {
            static const auto gl = makeTable(
                // kNone
                GL_NONE,
                // L_*
                GL_RED, GL_RED, GL_RED, GL_RED, GL_RED,
                // LA_*
                GL_RG, GL_RG, GL_RG, GL_RG, GL_RG,
                // RGB_U8, RGB_U10 (packed, needs RGBA), RGB_U16, RGB_U32, RGB_F16, RGB_F32
                GL_RGB, GL_RGBA, GL_RGB, GL_RGB, GL_RGB, GL_RGB,
                // RGBA_*
                GL_RGBA, GL_RGBA, GL_RGBA, GL_RGBA, GL_RGBA,
                // YUV planar (uploaded as one L_* texture per plane)
                GL_NONE, GL_NONE, GL_NONE,
                GL_NONE, GL_NONE, GL_NONE,
                GL_NONE, GL_NONE, GL_NONE,
                GL_NONE, GL_NONE, GL_NONE,
                // YUV semi-planar (uploaded as L_* + LA_* textures)
                GL_NONE, GL_NONE, GL_NONE, GL_NONE, GL_NONE, GL_NONE,
                // ARGB_4444_Premult
                GL_BGRA,
                // RGB_F16_P (one single channel texture per plane)
                GL_RED);
            static const auto gles = makeTable(
                // kNone
                GL_NONE,
                // L_U8, L_U16, L_U32, L_F16, L_F32
                GL_RED, GL_NONE, GL_NONE, GL_RED, GL_RED,
                // LA_*
                GL_RG, GL_NONE, GL_NONE, GL_RG, GL_RG,
                // RGB_U8, RGB_U10, RGB_U16, RGB_U32, RGB_F16, RGB_F32
                GL_RGB, GL_RGBA, GL_NONE, GL_NONE, GL_RGB, GL_RGB,
                // RGBA_*
                GL_RGBA, GL_NONE, GL_NONE, GL_RGBA, GL_RGBA,
                // YUV planar
                GL_NONE, GL_NONE, GL_NONE,
                GL_NONE, GL_NONE, GL_NONE,
                GL_NONE, GL_NONE, GL_NONE,
                GL_NONE, GL_NONE, GL_NONE,
                // YUV semi-planar
                GL_NONE, GL_NONE, GL_NONE, GL_NONE, GL_NONE, GL_NONE,
                // ARGB_4444_Premult (no BGRA in GLES)
                GL_NONE,
                // RGB_F16_P
                GL_RED);
            static_assert(std::tuple_size<decltype(gl)>::value == pixelTypeCount);
            static_assert(std::tuple_size<decltype(gles)>::value == pixelTypeCount);
            return (isGLES() ? gles : gl)[static_cast<std::size_t>(value)];
        }

        unsigned int getTextureInternalFormat(image::PixelType type)
        {
            static const auto gl = makeTable(
                // kNone
                GL_NONE,
                // L_*
                GL_R8, GL_R16, GL_R32I, GL_R16F, GL_R32F,
                // LA_*
                GL_RG8, GL_RG16, GL_RG32I, GL_RG16F, GL_RG32F,
                // RGB_*
                GL_RGB8, GL_RGB10_A2, GL_RGB16, GL_RGB32I, GL_RGBA16F, GL_RGBA32F,
                // RGBA_*
                GL_RGBA8, GL_RGBA16, GL_RGBA32I, GL_RGBA16F, GL_RGBA32F,
                // YUV planar
                GL_NONE, GL_NONE, GL_NONE,
                GL_NONE, GL_NONE, GL_NONE,
                GL_NONE, GL_NONE, GL_NONE,
                GL_NONE, GL_NONE, GL_NONE,
                // YUV semi-planar
                GL_NONE, GL_NONE, GL_NONE, GL_NONE, GL_NONE, GL_NONE,
                // ARGB_4444_Premult
                GL_RGBA,
                // RGB_F16_P
                GL_R16F);
            static const auto gles = makeTable(
                // kNone
                GL_NONE,
                // L_*
                GL_R8, GL_NONE, GL_NONE, GL_R16F, GL_R32F,
                // LA_*
                GL_RG8, GL_NONE, GL_NONE, GL_RG16F, GL_RG32F,
                // RGB_*
                GL_RGB8, GL_RGB10_A2, GL_NONE, GL_NONE, GL_RGB16F, GL_RGB32F,
                // RGBA_*
                GL_RGBA8, GL_NONE, GL_NONE, GL_RGBA16F, GL_RGBA32F,
                // YUV planar
                GL_NONE, GL_NONE, GL_NONE,
                GL_NONE, GL_NONE, GL_NONE,
                GL_NONE, GL_NONE, GL_NONE,
                GL_NONE, GL_NONE, GL_NONE,
                // YUV semi-planar
                GL_NONE, GL_NONE, GL_NONE, GL_NONE, GL_NONE, GL_NONE,
                // ARGB_4444_Premult
                GL_NONE,
                // RGB_F16_P
                GL_R16F);
            static_assert(std::tuple_size<decltype(gl)>::value == pixelTypeCount);
            static_assert(std::tuple_size<decltype(gles)>::value == pixelTypeCount);
            return (isGLES() ? gles : gl)[static_cast<std::size_t>(type)];
        }

        unsigned int getTextureType(image::PixelType value)
        {
            static const auto gl = makeTable(
                // kNone
                GL_NONE,
                // L_*
                GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT, GL_UNSIGNED_INT, GL_HALF_FLOAT, GL_FLOAT,
                // LA_*
                GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT, GL_UNSIGNED_INT, GL_HALF_FLOAT, GL_FLOAT,
                // RGB_*
                GL_UNSIGNED_BYTE, GL_UNSIGNED_INT_10_10_10_2, GL_UNSIGNED_SHORT, GL_UNSIGNED_INT, GL_HALF_FLOAT, GL_FLOAT,
                // RGBA_*
                GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT, GL_UNSIGNED_INT, GL_HALF_FLOAT, GL_FLOAT,
                // YUV planar
                GL_NONE, GL_NONE, GL_NONE,
                GL_NONE, GL_NONE, GL_NONE,
                GL_NONE, GL_NONE, GL_NONE,
                GL_NONE, GL_NONE, GL_NONE,
                // YUV semi-planar
                GL_NONE, GL_NONE, GL_NONE, GL_NONE, GL_NONE, GL_NONE,
                // ARGB_4444_Premult
                GL_UNSIGNED_SHORT_4_4_4_4_REV,
                // RGB_F16_P
                GL_HALF_FLOAT);
            static const auto gles = makeTable(
                // kNone
                GL_NONE,
                // L_*
                GL_UNSIGNED_BYTE, GL_NONE, GL_NONE, GL_HALF_FLOAT, GL_FLOAT,
                // LA_*
                GL_UNSIGNED_BYTE, GL_NONE, GL_NONE, GL_HALF_FLOAT, GL_FLOAT,
                // RGB_*
                GL_UNSIGNED_BYTE, GL_UNSIGNED_INT_2_10_10_10_REV, GL_NONE, GL_NONE, GL_HALF_FLOAT, GL_FLOAT,
                // RGBA_*
                GL_UNSIGNED_BYTE, GL_NONE, GL_NONE, GL_HALF_FLOAT, GL_FLOAT,
                // YUV planar
                GL_NONE, GL_NONE, GL_NONE,
                GL_NONE, GL_NONE, GL_NONE,
                GL_NONE, GL_NONE, GL_NONE,
                GL_NONE, GL_NONE, GL_NONE,
                // YUV semi-planar
                GL_NONE, GL_NONE,
                GL_NONE, GL_NONE,
                GL_NONE, GL_NONE,
                // ARGB_4444_Premult
                GL_NONE,
                // RGB_F16_P
                GL_HALF_FLOAT);
            static_assert(std::tuple_size<decltype(gl)>::value == pixelTypeCount);
            static_assert(std::tuple_size<decltype(gles)>::value == pixelTypeCount);
            return (isGLES() ? gles : gl)[static_cast<std::size_t>(value)];
        }

        bool TextureOptions::operator==(const TextureOptions& other) const
        {
            return filters == other.filters && pbo == other.pbo;
        }

        bool TextureOptions::operator!=(const TextureOptions& other) const
        {
            return !(*this == other);
        }

        unsigned int getTextureFilter(timeline::ImageFilter value)
        {
            const std::array<
                GLenum, static_cast<std::size_t>(timeline::ImageFilter::Count)>
                data = {GL_NEAREST, GL_LINEAR};
            return data[static_cast<std::size_t>(value)];
        }

        
        UploadBuffer::~UploadBuffer()
        {
            for (auto& id : _ids)
                if (id) glDeleteBuffers(1, &id);
        }

        void* UploadBuffer::map(size_t byteCount)
        {
            const int i = _index;
            _index = (_index + 1) % count;
            if (!_ids[i]) glGenBuffers(1, &_ids[i]);
            glBindBuffer(GL_PIXEL_UNPACK_BUFFER, _ids[i]);
            if (_sizes[i] < byteCount) // Grow only; never reallocate per frame.
            {
                glBufferData(GL_PIXEL_UNPACK_BUFFER, byteCount, nullptr,
                             GL_STREAM_DRAW);
                _sizes[i] = byteCount;
            }
            // Invalidate so we never wait on the previous upload still reading this buffer.
            return glMapBufferRange(
                GL_PIXEL_UNPACK_BUFFER, 0, byteCount,
                GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
        }

        void UploadBuffer::unmap() { glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER); }
        
        namespace
        {
            std::atomic<size_t> objectCount = 0;
            std::atomic<size_t> totalByteCount = 0;

            //! Creating or uploading to a texture is not a drawing operation,
            //! so it should neither depend on which texture unit the last draw
            //! left selected, nor change it. bind() is the opposite: callers
            //! choose the unit and it binds there.
            class UploadUnit
            {
            public:
                UploadUnit()
                {
                    glGetIntegerv(GL_ACTIVE_TEXTURE, &_previous);
                    glActiveTexture(GL_TEXTURE0);
                }

                ~UploadUnit()
                {
                    glActiveTexture(static_cast<GLenum>(_previous));
                }

            private:
                GLint _previous = GL_TEXTURE0;
            };

            //! Issue glTexSubImage2D, sourcing from the currently bound
            //! GL_PIXEL_UNPACK_BUFFER (pass nullptr) or from client memory.
            //! Every piece of unpack state the upload depends on is set here
            //! rather than assumed, and the ones that are easy to forget
            //! (row length, byte swapping) are put back afterwards so they
            //! can't corrupt an unrelated upload elsewhere.
            void texSubImage(
                GLuint id, int x, int y, const image::Info& info,
                int alignment, int rowLength, const void* data)
            {
                glBindTexture(GL_TEXTURE_2D, id);
                glPixelStorei(GL_UNPACK_ALIGNMENT, alignment);
                glPixelStorei(GL_UNPACK_ROW_LENGTH, rowLength);
#if defined(TLRENDER_API_GL_4_1)
                // Not available in GLES.
                glPixelStorei(
                    GL_UNPACK_SWAP_BYTES,
                    info.layout.endian != memory::getEndian());
#endif // TLRENDER_API_GL_4_1
                glTexSubImage2D(
                    GL_TEXTURE_2D, 0, x, y, info.size.w, info.size.h,
                    getTextureFormat(info.pixelType),
                    getTextureType(info.pixelType), data);
#if defined(TLRENDER_API_GL_4_1)
                glPixelStorei(GL_UNPACK_SWAP_BYTES, GL_FALSE);
#endif // TLRENDER_API_GL_4_1
                glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
            }

#if defined(TLRENDER_API_GL_4_1)
            //! Bind the PBO and map it for writing.
            //!
            //! The buffer handed to the previous upload may still be read by
            //! the driver (uploads are asynchronous), and mapping that same
            //! storage would block until the GPU is done with it. Re-specifying
            //! the data store ("orphaning") gives us fresh storage with no
            //! reader to wait for; the driver frees the old storage once the
            //! pending upload finishes. INVALIDATE_BUFFER_BIT repeats the
            //! request for drivers that honor the map flag but not the
            //! orphan, and costs nothing where both are honored.
            //!
            //! The PBO is left bound, even on failure; the caller unbinds it.
            void* mapUnpackBuffer(GLuint pbo, size_t capacity, size_t byteCount)
            {
                glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo);
                glBufferData(
                    GL_PIXEL_UNPACK_BUFFER, capacity, nullptr, GL_STREAM_DRAW);
                return glMapBufferRange(
                    GL_PIXEL_UNPACK_BUFFER, 0, byteCount,
                    GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
            }
#endif // TLRENDER_API_GL_4_1

            //! Upload pixels to a texture, through the PBO when there is one
            //! and the data fits in it, otherwise from client memory.
            //! rowStride is in bytes; zero means the rows follow the image's
            //! own layout (including its alignment padding).
            void uploadPixels(
                GLuint id, GLuint pbo, size_t pboCapacity,
                int x, int y, const image::Info& info,
                const uint8_t* data, int rowStride)
            {
                const UploadUnit uploadUnit;

                // A caller supplied stride is exact, so alignment must not
                // round it. With no stride the image's own layout rules.
                const bool strided = rowStride != 0;
                const int alignment = strided ? 1 : info.layout.alignment;

                size_t rowBytes = 0;
                size_t tightByteCount = 0;
                int rowLength = 0;
                if (strided)
                {
                    // Derive bytes per pixel from the library rather than
                    // channels * bitDepth / 8, which is wrong for packed
                    // types such as RGB_U10 (4 bytes per pixel, not 3).
                    image::Info tight = info;
                    tight.layout.alignment = 1;
                    tightByteCount = image::getDataByteCount(tight);
                    const size_t pixelCount =
                        static_cast<size_t>(info.size.w) * info.size.h;
                    const size_t bytesPerPixel =
                        pixelCount > 0 ? tightByteCount / pixelCount : 0;
                    if (0 == bytesPerPixel)
                        return;
                    rowBytes = bytesPerPixel * info.size.w;
                    rowLength = static_cast<int>(rowStride / bytesPerPixel);
                }
                const size_t byteCount =
                    strided ? tightByteCount : image::getDataByteCount(info);

#if defined(TLRENDER_API_GL_4_1)
                if (pbo && byteCount <= pboCapacity)
                {
                    bool done = false;
                    if (void* dst = mapUnpackBuffer(pbo, pboCapacity, byteCount))
                    {
                        if (strided)
                        {
                            // Repack into tightly packed rows.
                            auto out = static_cast<uint8_t*>(dst);
                            for (int row = 0; row < info.size.h; ++row)
                            {
                                memcpy(
                                    out + row * rowBytes,
                                    data + static_cast<size_t>(row) * rowStride,
                                    rowBytes);
                            }
                        }
                        else
                        {
                            memcpy(dst, data, byteCount);
                        }
                        // The data store can be lost while mapped (rare,
                        // e.g. a display mode change); then fall back.
                        if (glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER))
                        {
                            texSubImage(id, x, y, info, alignment, 0, nullptr);
                            done = true;
                        }
                    }
                    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
                    if (done)
                        return;
                }
#endif // TLRENDER_API_GL_4_1
                texSubImage(id, x, y, info, alignment, rowLength, data);
            }
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
            image::Info info;
            size_t byteCount = 0;
            GLuint pbo = 0;
            size_t pboSize = 0;
            GLuint id = 0;
        };

        void
        Texture::_init(const image::Info& info, const TextureOptions& options)
        {
            TLRENDER_P();
            p.info = info;
            if (!p.info.isValid())
            {
                throw std::runtime_error("Invalid texture");
            }
            p.byteCount = image::getDataByteCount(p.info);

            const UploadUnit uploadUnit;
#if defined(TLRENDER_API_GL_4_1)
            if (options.pbo && 1 == p.info.layout.alignment &&
                memory::getEndian() == p.info.layout.endian)
            {
                p.pboSize = p.byteCount;
                glGenBuffers(1, &p.pbo);
                glBindBuffer(GL_PIXEL_UNPACK_BUFFER, p.pbo);
                glBufferData(
                    GL_PIXEL_UNPACK_BUFFER, p.pboSize, NULL, GL_STREAM_DRAW);
                glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
            }
#endif // TLRENDER_API_GL_4_1
            glGenTextures(1, &p.id);
            glBindTexture(GL_TEXTURE_2D, p.id);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(
                GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                getTextureFilter(options.filters.minify));
            glTexParameteri(
                GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
                getTextureFilter(options.filters.magnify));
            glTexImage2D(
                GL_TEXTURE_2D, 0, getTextureInternalFormat(p.info.pixelType),
                p.info.size.w, p.info.size.h, 0,
                getTextureFormat(p.info.pixelType),
                getTextureType(p.info.pixelType), NULL);

            totalByteCount += p.byteCount;
        }

        Texture::Texture() :
            _p(new Private)
        {
            // Counted here, and uncounted in the destructor, so the two
            // always match even when _init() throws on an invalid texture.
            ++objectCount;
        }

        Texture::~Texture()
        {
            TLRENDER_P();
            if (p.pbo)
            {
                glDeleteBuffers(1, &p.pbo);
                p.pbo = 0;
            }
            if (p.id)
            {
                glDeleteTextures(1, &p.id);
                p.id = 0;
            }

            --objectCount;
            totalByteCount -= p.byteCount;
        }

        std::shared_ptr<Texture>
        Texture::create(const image::Info& info, const TextureOptions& options)
        {
            auto out = std::shared_ptr<Texture>(new Texture);
            out->_init(info, options);
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

        image::PixelType Texture::getPixelType() const
        {
            return _p->info.pixelType;
        }

        unsigned int Texture::getID() const
        {
            return _p->id;
        }

        void
        Texture::copy(const std::shared_ptr<image::Image>& data, int x, int y)
        {
            TLRENDER_P();
            uploadPixels(
                p.id, p.pbo, p.pboSize, x, y, data->getInfo(), data->getData(),
                0);
        }

        void Texture::copy(const std::shared_ptr<image::Image>& data)
        {
            copy(data, 0, 0);
        }

        void Texture::copy(const uint8_t* data, const image::Info& info,
                           const int rowStride)
        {
            TLRENDER_P();
            uploadPixels(p.id, p.pbo, p.pboSize, 0, 0, info, data, rowStride);
        }

        void Texture::copy(const std::shared_ptr<image::Image>& data,
                           UploadBuffer& upload)
        {
            TLRENDER_P();
            const auto& info = data->getInfo();
            if (1 == info.layout.alignment && info.layout.endian == memory::getEndian())
            {
                const UploadUnit uploadUnit;
                const size_t byteCount = data->getDataByteCount();
                if (void* dst = upload.map(byteCount))
                {
                    memcpy(dst, data->getData(), byteCount);
                    upload.unmap();
                    texSubImage(p.id, 0, 0, info, 1, 0, nullptr);
                    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
                    return;
                }
                glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
            }
            copy(data, 0, 0); // Fallback: the texture's own PBO or client memory.
        }

        void Texture::bind()
        {
            glBindTexture(GL_TEXTURE_2D, _p->id);
        }

    } // namespace gl
} // namespace tl
