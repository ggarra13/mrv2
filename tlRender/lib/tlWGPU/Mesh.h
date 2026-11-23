// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// Copyright (c) 2025-Present Gonzalo Garramuño
// All rights reserved.

#pragma once

#include <tlWGPU/WGPU.h>

#include <tlCore/Mesh.h>
#include <tlCore/Range.h>
#include <tlCore/Util.h>

#include <cstdint>
#include <memory>
#include <vector>

namespace tl
{
    namespace geom
    {
        struct TriangleMesh2;
        struct TriangleMesh3;
    } // namespace geom

    namespace wgpu
    {
        //! Vertex buffer object types.
        enum class VBOType {
            Pos2_F32,
            Pos2_F32_UV_U16,
            Pos2_F32_Color_F32,
            Pos3_F32,
            Pos3_F32_UV_U16,
            Pos3_F32_UV_U16_Normal_U10,
            Pos3_F32_UV_U16_Color_U8,
            Pos3_F32_UV_U16_Normal_U10_Color_U8,
            Pos3_F32_UV_F32,
            Pos3_F32_UV_F32_Normal_F32,
            Pos3_F32_UV_F32_Normal_F32_Color_F32,
            Pos3_F32_Color_U8,

            Count,
            First = Pos2_F32
        };
        TLRENDER_ENUM(VBOType);
        TLRENDER_ENUM_SERIALIZE(VBOType);

        //! Get the number of bytes used to store vertex buffer object types.
        std::size_t getByteCount(VBOType);

        //! Convert a triangle mesh to vertex buffer data.
        std::vector<uint8_t>
        convert(const geom::TriangleMesh2& mesh, VBOType type);

        //! Convert a triangle mesh to vertex buffer data.
        std::vector<uint8_t> convert(
            const geom::TriangleMesh2& mesh, VBOType type,
            const math::SizeTRange& range);

        //! Convert a triangle mesh to vertex buffer data.
        std::vector<uint8_t>
        convert(const geom::TriangleMesh3& mesh, VBOType type);

        //! Convert a triangle mesh to vertex buffer data.
        std::vector<uint8_t> convert(
            const geom::TriangleMesh3& mesh, VBOType type,
            const math::SizeTRange& range);

        //! Vertex buffer object.
        //!
        //! Holds CPU-side converted vertex data plus the WebGPU vertex
        //! layout needed to build a render pipeline.
        //!
        //! \note The packed 10-bit normal types are exposed to the shader
        //! as a single `u32` attribute (WebGPU has no snorm10 vertex
        //! format).  Use `unpackNormal10()` (see Mesh.cpp) in WGSL.
        class VBO : public std::enable_shared_from_this<VBO>
        {
            TLRENDER_NON_COPYABLE(VBO);

        protected:
            void _init(std::size_t size, VBOType);

            VBO();

        public:
            ~VBO();

            //! Create a new object.
            //! \param size Number of vertices.
            static std::shared_ptr<VBO> create(std::size_t size, VBOType);

            //! Get the number of vertices.
            std::size_t getSize() const;

            //! Get the type.
            VBOType getType() const;

            const std::vector<uint8_t>& getData() const;

            //! Get the vertex buffer layout for WGPUVertexState::buffers.
            //! The attribute pointer inside refers to storage owned by this
            //! object and is valid for the lifetime of the VBO.
            const WGPUVertexBufferLayout& getVertexLayout() const;

            //! Get the vertex attributes (shaderLocation == attribute index).
            const std::vector<WGPUVertexAttribute>& getAttributes() const;

            //! \name Copy
            //! Copy data to the vertex buffer object.
            ///@{

            void copy(const std::vector<uint8_t>&);

            ///@}

            //! Get the number of objects currenty instantiated.
            static size_t getObjectCount();

            //! Get the total number of bytes currently used.
            static size_t getTotalByteCount();

        private:
            TLRENDER_PRIVATE();
        };

        //! Dynamic vertex buffer with a bump allocator.
        //!
        //! Unlike the Vulkan VAO there are no per-frame partitions, no
        //! persistent mapping and no manual flush: uploads go through
        //! wgpuQueueWriteBuffer(), which WebGPU orders against queue
        //! submissions for us.
        //!
        //! Frame usage:
        //!   vao->beginFrame();                // after the previous frame's
        //!                                     // commands were submitted
        //!   offset = vao->upload(data);       // any number of times
        //!   vao->draw(pass, count, offset);
        //!   ... wgpuQueueSubmit(...)
        class VAO : public std::enable_shared_from_this<VAO>
        {
            TLRENDER_NON_COPYABLE(VAO);

        protected:
            void _init(std::size_t memorySize);

            VAO(Fl_WebGPU_Context&);

        public:
            ~VAO();

            //! Create a new object.
            //! \p device and \p queue are not owned and must outlive the VAO.
            static std::shared_ptr<VAO> create(
                Fl_WebGPU_Context&,
                std::size_t memorySize = 16 * 1024 * 1024);

            //! Largest buffer this device can create
            //! (WGPULimits::maxBufferSize; 256 MiB unless a higher limit was
            //! requested when the device was created).
            static uint64_t getMaxMemorySize(WGPUDevice);

            //! Rewind the allocator.  Call once per frame, after the previous
            //! frame's draws have been submitted and before new uploads.
            void beginFrame();

            //! Set memory for VAO assets in bytes.  Only grows; recreates the
            //! buffer (and rewinds the allocator) when it does.
            void setMemorySize(const std::size_t);

            //! Get the buffer size in bytes.
            std::size_t getMemorySize() const;

            //! Get the vertex buffer.
            WGPUBuffer getBuffer() const;

            //! Return true if @p dataSize bytes fit in the remaining space.
            bool canFit(std::size_t dataSize) const;

            //! Upload vertex data and return its byte offset in the buffer.
            std::size_t upload(const std::vector<uint8_t>& data);

            //! Upload the vertex buffer object's data and draw it.
            void draw(WGPURenderPassEncoder pass, const std::shared_ptr<VBO>& vbo);

            //! Draw previously uploaded data.
            void draw(
                WGPURenderPassEncoder pass, const std::size_t vertexCount,
                const uint64_t offset);

        private:
            WGPUDevice _device = nullptr;
            WGPUQueue _queue = nullptr;

            TLRENDER_PRIVATE();
        };
    } // namespace webgpu
} // namespace tl
