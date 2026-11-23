// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// Copyright (c) 2025-Present Gonzalo Garramuño
// All rights reserved.

#include <tlWGPU/Mesh.h>

#include <tlCore/Error.h>
#include <tlCore/Math.h>
#include <tlCore/Mesh.h>
#include <tlCore/String.h>

#include <algorithm>
#include <atomic>
#include <cstring>
#include <stdexcept>
#include <string>
#include <vector>

namespace tl
{
    namespace wgpu
    {
        namespace
        {
            // 16 MB
            constexpr size_t DYNAMIC_VERTEX_BUFFER_SIZE = 16 * 1024 * 1024;

            // wgpuQueueWriteBuffer() requires offset and size to be multiples
            // of 4 (COPY_BUFFER_ALIGNMENT).  We use 16 to keep every upload
            // nicely aligned; setVertexBuffer() only requires 4.
            constexpr uint64_t VERTEX_ALIGNMENT = 16;

            constexpr uint64_t alignUp(uint64_t value, uint64_t alignment)
            {
                return (value + alignment - 1) & ~(alignment - 1);
            }
        } // namespace

        TLRENDER_ENUM_IMPL(
            VBOType,
            "Pos2_F32",
            "Pos2_F32_UV_U16",
            "Pos2_F32_Color_F32",
            "Pos3_F32",
            "Pos3_F32_UV_U16",
            "Pos3_F32_UV_U16_Normal_U10",
            "Pos3_F32_UV_U16_Color_U8",
            "Pos3_F32_UV_U16_Normal_U10_Color_U8",
            "Pos3_F32_UV_F32",
            "Pos3_F32_UV_F32_Normal_F32",
            "Pos3_F32_UV_F32_Normal_F32_Color_F32",
            "Pos3_F32_Color_U8");
        TLRENDER_ENUM_SERIALIZE_IMPL(VBOType);

        namespace
        {
            //
            // Every VBOType is: position, then optional UV, normal, color
            // (always in that order).  Describing the types this way gives
            // us ONE source of truth for byte counts, the vertex layout and
            // the CPU-side conversion, instead of three parallel switches.
            //
            enum class UVKind { None, U16, F32 };
            enum class NormalKind { None, U10, F32 };
            enum class ColorKind { None, U8, F32 };

            struct VertexFormat
            {
                int posComponents;
                UVKind uv;
                NormalKind normal;
                ColorKind color;
            };

            VertexFormat getVertexFormat(VBOType type)
            {
                using U = UVKind;
                using N = NormalKind;
                using C = ColorKind;
                switch (type)
                {
                case VBOType::Pos2_F32:
                    return {2, U::None, N::None, C::None};
                case VBOType::Pos2_F32_UV_U16:
                    return {2, U::U16, N::None, C::None};
                case VBOType::Pos2_F32_Color_F32:
                    return {2, U::None, N::None, C::F32};
                case VBOType::Pos3_F32:
                    return {3, U::None, N::None, C::None};
                case VBOType::Pos3_F32_UV_U16:
                    return {3, U::U16, N::None, C::None};
                case VBOType::Pos3_F32_UV_U16_Normal_U10:
                    return {3, U::U16, N::U10, C::None};
                case VBOType::Pos3_F32_UV_U16_Color_U8:
                    return {3, U::U16, N::None, C::U8};
                case VBOType::Pos3_F32_UV_U16_Normal_U10_Color_U8:
                    return {3, U::U16, N::U10, C::U8};
                case VBOType::Pos3_F32_UV_F32:
                    return {3, U::F32, N::None, C::None};
                case VBOType::Pos3_F32_UV_F32_Normal_F32:
                    return {3, U::F32, N::F32, C::None};
                case VBOType::Pos3_F32_UV_F32_Normal_F32_Color_F32:
                    return {3, U::F32, N::F32, C::F32};
                case VBOType::Pos3_F32_Color_U8:
                    return {3, U::None, N::None, C::U8};
                default:
                    break;
                }
                throw std::invalid_argument("Unknown VBOType");
            }

            //! Tiny sequential writer; memcpy avoids alignment/aliasing UB.
            class Writer
            {
            public:
                explicit Writer(uint8_t* p) : _p(p) {}

                template <typename T> void put(T value)
                {
                    std::memcpy(_p, &value, sizeof(T));
                    _p += sizeof(T);
                }

            private:
                uint8_t* _p;
            };

            inline uint16_t toUnorm16(float v)
            {
                return static_cast<uint16_t>(
                    math::clamp(static_cast<int>(v * 65535.F), 0, 65535));
            }

            inline uint8_t toUnorm8(float v)
            {
                return static_cast<uint8_t>(
                    math::clamp(static_cast<int>(v * 255.F), 0, 255));
            }

            //! 10-bit two's complement, in the low 10 bits.
            inline uint32_t toSnorm10(float v)
            {
                const int i =
                    math::clamp(static_cast<int>(v * 511.F), -512, 511);
                return static_cast<uint32_t>(i) & 0x3FFu;
            }

            //! Explicit shifts instead of C++ bitfields, whose layout is
            //! implementation-defined.  x = bits 0-9, y = 10-19, z = 20-29.
            template <typename V> uint32_t packNormal10(const V& n)
            {
                return toSnorm10(n.x) | (toSnorm10(n.y) << 10) |
                       (toSnorm10(n.z) << 20);
            }

            template <int Dim, typename TriangleMeshT>
            std::vector<uint8_t> convertImpl(
                const TriangleMeshT& mesh, VBOType type,
                const math::SizeTRange& range)
            {
                const VertexFormat fmt = getVertexFormat(type);
                if (fmt.posComponents != Dim)
                {
                    throw std::invalid_argument(
                        "convert: VBOType does not match the mesh dimension");
                }
                if (mesh.triangles.empty())
                {
                    return {};
                }
                if (range.max() >= mesh.triangles.size() ||
                    range.min() > range.max())
                {
                    throw std::out_of_range("convert: invalid triangle range");
                }

                const size_t triangleCount = range.max() - range.min() + 1;
                std::vector<uint8_t> out(
                    triangleCount * 3 * getByteCount(type));
                Writer w(out.data());

                for (size_t i = range.min(); i <= range.max(); ++i)
                {
                    for (size_t k = 0; k < 3; ++k)
                    {
                        const auto& vert = mesh.triangles[i].v[k];

                        // Position.
                        const size_t v = vert.v;
                        w.put(v ? mesh.v[v - 1].x : 0.F);
                        w.put(v ? mesh.v[v - 1].y : 0.F);
                        if constexpr (Dim == 3)
                        {
                            w.put(v ? mesh.v[v - 1].z : 0.F);
                        }

                        // UV.
                        const size_t t = vert.t;
                        switch (fmt.uv)
                        {
                        case UVKind::U16:
                            w.put(t ? toUnorm16(mesh.t[t - 1].x) : uint16_t(0));
                            w.put(t ? toUnorm16(mesh.t[t - 1].y) : uint16_t(0));
                            break;
                        case UVKind::F32:
                            w.put(t ? mesh.t[t - 1].x : 0.F);
                            w.put(t ? mesh.t[t - 1].y : 0.F);
                            break;
                        default:
                            break;
                        }

                        // Normal (3D meshes only).
                        if constexpr (Dim == 3)
                        {
                            const size_t n = vert.n;
                            switch (fmt.normal)
                            {
                            case NormalKind::U10:
                                w.put(
                                    n ? packNormal10(mesh.n[n - 1])
                                      : uint32_t(0));
                                break;
                            case NormalKind::F32:
                                w.put(n ? mesh.n[n - 1].x : 0.F);
                                w.put(n ? mesh.n[n - 1].y : 0.F);
                                w.put(n ? mesh.n[n - 1].z : 0.F);
                                break;
                            default:
                                break;
                            }
                        }

                        // Color.
                        const size_t c = vert.c;
                        switch (fmt.color)
                        {
                        case ColorKind::U8:
                            // Byte order r,g,b,a matches Unorm8x4.
                            w.put(c ? toUnorm8(mesh.c[c - 1].x) : uint8_t(255));
                            w.put(c ? toUnorm8(mesh.c[c - 1].y) : uint8_t(255));
                            w.put(c ? toUnorm8(mesh.c[c - 1].z) : uint8_t(255));
                            w.put(c ? toUnorm8(mesh.c[c - 1].w) : uint8_t(255));
                            break;
                        case ColorKind::F32:
                            w.put(c ? mesh.c[c - 1].x : 1.F);
                            w.put(c ? mesh.c[c - 1].y : 1.F);
                            w.put(c ? mesh.c[c - 1].z : 1.F);
                            w.put(c ? mesh.c[c - 1].w : 1.F);
                            break;
                        default:
                            break;
                        }
                    }
                }
                return out;
            }
        } // namespace

        std::size_t getByteCount(VBOType value)
        {
            const VertexFormat fmt = getVertexFormat(value);
            size_t out = fmt.posComponents * sizeof(float);
            switch (fmt.uv)
            {
            case UVKind::U16: out += 2 * sizeof(uint16_t); break;
            case UVKind::F32: out += 2 * sizeof(float); break;
            default: break;
            }
            switch (fmt.normal)
            {
            case NormalKind::U10: out += sizeof(uint32_t); break;
            case NormalKind::F32: out += 3 * sizeof(float); break;
            default: break;
            }
            switch (fmt.color)
            {
            case ColorKind::U8: out += 4 * sizeof(uint8_t); break;
            case ColorKind::F32: out += 4 * sizeof(float); break;
            default: break;
            }
            return out;
        }

        std::vector<uint8_t> convert(const geom::TriangleMesh2& mesh, VBOType type)
        {
            return convert(
                mesh, type,
                math::SizeTRange(
                    0, mesh.triangles.size() > 0 ? (mesh.triangles.size() - 1)
                                                 : 0));
        }

        std::vector<uint8_t> convert(
            const geom::TriangleMesh2& mesh, VBOType type,
            const math::SizeTRange& range)
        {
            return convertImpl<2>(mesh, type, range);
        }

        std::vector<uint8_t> convert(const geom::TriangleMesh3& mesh, VBOType type)
        {
            return convert(
                mesh, type,
                math::SizeTRange(
                    0, mesh.triangles.size() > 0 ? (mesh.triangles.size() - 1)
                                                 : 0));
        }

        std::vector<uint8_t> convert(
            const geom::TriangleMesh3& mesh, VBOType type,
            const math::SizeTRange& range)
        {
            return convertImpl<3>(mesh, type, range);
        }

        // ------------------------------------------------------------------
        //  VBO
        // ------------------------------------------------------------------

        namespace
        {
            std::atomic<size_t> objectCount = 0;
            std::atomic<size_t> totalByteCount = 0;
        } // namespace

        size_t VBO::getObjectCount()
        {
            return objectCount;
        }

        size_t VBO::getTotalByteCount()
        {
            return totalByteCount;
        }

        struct VBO::Private
        {
            std::size_t size = 0;
            VBOType type = VBOType::First;

            // Converted vertex data, laid out to match the attributes below.
            std::vector<uint8_t> data;

            WGPUVertexBufferLayout layout = {};
            std::vector<WGPUVertexAttribute> attributes;
        };

        void VBO::_init(std::size_t size, VBOType type)
        {
            TLRENDER_P();

            ++objectCount;
            totalByteCount += size * getByteCount(type);

            p.size = size;
            p.type = type;

            const VertexFormat fmt = getVertexFormat(type);

            // At most 4 attributes; reserve so the pointer we hand to the
            // layout below can never be invalidated by a reallocation.
            p.attributes.reserve(4);

            uint64_t offset = 0;
            uint32_t location = 0;
            auto add = [&](WGPUVertexFormat format, uint64_t bytes)
            {
                WGPUVertexAttribute a = {};
                a.format = format;
                a.offset = offset;
                a.shaderLocation = location++;
                p.attributes.push_back(a);
                offset += bytes;
            };

            // Position.
            if (fmt.posComponents == 2)
                add(WGPUVertexFormat_Float32x2, 2 * sizeof(float));
            else
                add(WGPUVertexFormat_Float32x3, 3 * sizeof(float));

            // UV.
            switch (fmt.uv)
            {
            case UVKind::U16:
                add(WGPUVertexFormat_Unorm16x2, 2 * sizeof(uint16_t));
                break;
            case UVKind::F32:
                add(WGPUVertexFormat_Float32x2, 2 * sizeof(float));
                break;
            default:
                break;
            }

            // Normal.
            switch (fmt.normal)
            {
            case NormalKind::U10:
                // WebGPU has no snorm 10-10-10-2 vertex format (the Vulkan
                // code used A2B10G10R10_SNORM_PACK32).  Expose the packed
                // word as a u32 and decode it in the shader; see
                // unpackNormal10() below.
                add(WGPUVertexFormat_Uint32, sizeof(uint32_t));
                break;
            case NormalKind::F32:
                add(WGPUVertexFormat_Float32x3, 3 * sizeof(float));
                break;
            default:
                break;
            }

            // Color.
            switch (fmt.color)
            {
            case ColorKind::U8:
                add(WGPUVertexFormat_Unorm8x4, 4 * sizeof(uint8_t));
                break;
            case ColorKind::F32:
                add(WGPUVertexFormat_Float32x4, 4 * sizeof(float));
                break;
            default:
                break;
            }

            p.layout.arrayStride = getByteCount(type);
            p.layout.stepMode = WGPUVertexStepMode_Vertex;
            p.layout.attributeCount = p.attributes.size();
            p.layout.attributes = p.attributes.data();
        }

        const WGPUVertexBufferLayout& VBO::getVertexLayout() const
        {
            return _p->layout;
        }

        const std::vector<WGPUVertexAttribute>& VBO::getAttributes() const
        {
            return _p->attributes;
        }

        VBO::VBO() :
            _p(new Private)
        {
        }

        VBO::~VBO()
        {
            TLRENDER_P();

            --objectCount;
            totalByteCount -= p.size * getByteCount(p.type);
        }

        std::shared_ptr<VBO> VBO::create(std::size_t size, VBOType type)
        {
            auto out = std::shared_ptr<VBO>(new VBO);
            out->_init(size, type);
            return out;
        }

        size_t VBO::getSize() const
        {
            return _p->size;
        }

        const std::vector<uint8_t>& VBO::getData() const
        {
            return _p->data;
        }

        VBOType VBO::getType() const
        {
            return _p->type;
        }

        void VBO::copy(const std::vector<uint8_t>& data)
        {
            _p->data = data;
        }

        /*  WGSL helper for VBOType::*_Normal_U10*
         *
         *  Declare the attribute as `@location(N) normalPacked : u32` and
         *  decode with:
         *
         *  fn unpackNormal10(p : u32) -> vec3<f32> {
         *      // Sign-extend each 10-bit field (x: 0-9, y: 10-19, z: 20-29).
         *      let x = i32(p << 22u) >> 22u;
         *      let y = i32(p << 12u) >> 22u;
         *      let z = i32(p <<  2u) >> 22u;
         *      // -512 and -511 both map to -1, like Vulkan SNORM.
         *      return max(vec3<f32>(f32(x), f32(y), f32(z)) / 511.0,
         *                 vec3<f32>(-1.0));
         *  }
         */

        // ------------------------------------------------------------------
        //  VAO
        // ------------------------------------------------------------------

        struct VAO::Private
        {
            WGPUBuffer buffer = nullptr;

            // Buffer size in bytes.
            uint64_t totalSize = 0;

            // Bump-allocator cursor.
            uint64_t offset = 0;
        };

        uint64_t VAO::getMaxMemorySize(WGPUDevice device)
        {
            WGPULimits limits = {};
            if (wgpuDeviceGetLimits(device, &limits) != WGPUStatus_Success)
            {
                // WebGPU default maxBufferSize.
                return 256ull * 1024 * 1024;
            }
            return limits.maxBufferSize;
        }

        void VAO::setMemorySize(const std::size_t value)
        {
            TLRENDER_P();

            // Buffer sizes must be a multiple of 4 for our writes.
            const uint64_t size = value & ~uint64_t(3);
            if (size <= p.totalSize)
                return;

            const uint64_t maxSize = getMaxMemorySize(_device);
            if (size > maxSize)
            {
                throw std::runtime_error(
                    "VAO: requested " + std::to_string(size) +
                    " bytes exceeds device maxBufferSize " +
                    std::to_string(maxSize) +
                    " (request a higher limit when creating the device, or "
                    "use VAOPool with smaller slots)");
            }

            WGPUBufferDescriptor desc = {};
            desc.label = {"Mesh Vertex Buffer", WGPU_STRLEN};
            desc.usage = WGPUBufferUsage_Vertex | WGPUBufferUsage_CopyDst;
            desc.size = size;
            desc.mappedAtCreation = false;

            // NOTE: on out-of-memory WebGPU does not return an error code; it
            // hands back an error buffer and reports through the device's
            // uncaptured-error callback.  Wrap this call in
            // wgpuDevicePushErrorScope(OutOfMemory) / PopErrorScope if you
            // want to recover from it.
            WGPUBuffer buffer = wgpuDeviceCreateBuffer(_device, &desc);
            if (!buffer)
            {
                throw std::runtime_error("Failed to create vertex buffer");
            }

            // Release, never Destroy: commands already submitted may still
            // reference the old buffer.  The implementation frees it once the
            // GPU is done.
            if (p.buffer)
            {
                wgpuBufferRelease(p.buffer);
            }

            p.buffer = buffer;
            p.totalSize = size;
            p.offset = 0;
        }

        std::size_t VAO::getMemorySize() const
        {
            return static_cast<std::size_t>(_p->totalSize);
        }

        void VAO::_init(std::size_t memorySize)
        {
            setMemorySize(memorySize);
        }

        VAO::VAO(Fl_WebGPU_Context& ctx) :
            _device(ctx.device),
            _queue(ctx.queue()),
            _p(new Private)
        {
        }

        VAO::~VAO()
        {
            TLRENDER_P();
            if (p.buffer)
            {
                wgpuBufferRelease(p.buffer);
                p.buffer = nullptr;
            }
        }

        std::shared_ptr<VAO>
        VAO::create(Fl_WebGPU_Context& ctx, std::size_t memorySize)
        {
            auto out = std::shared_ptr<VAO>(new VAO(ctx));
            out->_init(memorySize > 0 ? memorySize : DYNAMIC_VERTEX_BUFFER_SIZE);
            return out;
        }

        void VAO::beginFrame()
        {
            // wgpuQueueWriteBuffer() is ordered on the queue timeline: a
            // write issued after wgpuQueueSubmit(N) lands after N's commands
            // have run.  So the whole buffer can simply be reused each frame
            // - no per-frame partitions needed.  Only call this once the
            // previous frame's command buffers have been *submitted*.
            _p->offset = 0;
        }

        bool VAO::canFit(std::size_t dataSize) const
        {
            return _p->offset + alignUp(dataSize, VERTEX_ALIGNMENT) <=
                   _p->totalSize;
        }

        std::size_t VAO::upload(const std::vector<uint8_t>& vertexData)
        {
            TLRENDER_P();

            const uint64_t dataSize = vertexData.size();
            const uint64_t advance = alignUp(dataSize, VERTEX_ALIGNMENT);

            if (p.offset + advance > p.totalSize)
            {
                throw std::runtime_error(
                    "VAO: buffer overflow! dataSize=" +
                    std::to_string(dataSize) +
                    " used=" + std::to_string(p.offset) +
                    " total=" + std::to_string(p.totalSize) +
                    ". Call setMemorySize() with a larger size or use "
                    "VAOPool.");
            }

            const uint64_t dstOffset = p.offset;

            if (dataSize > 0)
            {
                // writeBuffer needs a size that is a multiple of 4.
                const uint64_t writeSize = alignUp(dataSize, 4);
                if (writeSize == dataSize)
                {
                    wgpuQueueWriteBuffer(
                        _queue, p.buffer, dstOffset, vertexData.data(),
                        dataSize);
                }
                else
                {
                    std::vector<uint8_t> padded(vertexData);
                    padded.resize(writeSize, 0);
                    wgpuQueueWriteBuffer(
                        _queue, p.buffer, dstOffset, padded.data(), writeSize);
                }
            }

            p.offset += advance;
            return static_cast<std::size_t>(dstOffset);
        }

        void VAO::draw(
            WGPURenderPassEncoder pass, const std::size_t vertexCount,
            const uint64_t offset)
        {
            wgpuRenderPassEncoderSetVertexBuffer(
                pass, 0, _p->buffer, offset, WGPU_WHOLE_SIZE);
            wgpuRenderPassEncoderDraw(
                pass, static_cast<uint32_t>(vertexCount), 1, 0, 0);
        }

        void VAO::draw(
            WGPURenderPassEncoder pass, const std::shared_ptr<VBO>& vbo)
        {
            const std::size_t offset = upload(vbo->getData());
            draw(pass, vbo->getSize(), offset);
        }

        WGPUBuffer VAO::getBuffer() const
        {
            return _p->buffer;
        }

    } // namespace webgpu
} // namespace tl
