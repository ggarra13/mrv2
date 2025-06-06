// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// Copyright (c) 2025-Present Gonzalo Garramuño
// All rights reserved.

#pragma once

#include <tlVk/Vk.h>

#include <tlCore/Mesh.h>
#include <tlCore/Range.h>
#include <tlCore/Util.h>

#include <memory>
#include <vector>

namespace tl
{
    namespace geom
    {
        struct TriangleMesh2;
        struct TriangleMesh3;
    } // namespace geom

    namespace vlk
    {
        //! Vertex buffer object types.
        enum class VBOType {
            Pos2_F32,
            Pos2_F32_UV_U16,
            Pos2_F32_Color_F32,
            Pos3_F32,
            Pos3_F32_UV_U16,
            Pos3_F32_UV_U16_Normal_U10,
            Pos3_F32_UV_U16_Normal_U10_Color_U8,
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
        convert(const geom::TriangleMesh2& mesh, vlk::VBOType type);

        //! Convert a triangle mesh to vertex buffer data.
        std::vector<uint8_t> convert(
            const geom::TriangleMesh2& mesh, vlk::VBOType type,
            const math::SizeTRange& range);

        //! Convert a triangle mesh to vertex buffer data.
        std::vector<uint8_t>
        convert(const geom::TriangleMesh3& mesh, vlk::VBOType type);

        //! Convert a triangle mesh to vertex buffer data.
        std::vector<uint8_t> convert(
            const geom::TriangleMesh3& mesh, vlk::VBOType type,
            const math::SizeTRange& range);

        //! OpenGL vertex buffer object.
        class VBO : public std::enable_shared_from_this<VBO>
        {
            TLRENDER_NON_COPYABLE(VBO);

        protected:
            void _init(std::size_t size, VBOType);

            VBO();

        public:
            ~VBO();

            //! Create a new object.
            static std::shared_ptr<VBO> create(std::size_t size, VBOType);

            //! Get the vertex size.
            std::size_t getSize() const;

            //! Get the type.
            VBOType getType() const;

            const std::vector<uint8_t>& getData() const;

            const std::vector<VkVertexInputBindingDescription>&
            getBindingDescription() const;
            const std::vector<VkVertexInputAttributeDescription>&
            getAttributes() const;

            //! \name Copy
            //! Copy data to the vertex buffer object.
            ///@{

            void copy(const std::vector<uint8_t>&);

            ///@}

        private:
            TLRENDER_PRIVATE();
        };

        //! Vulkan vertex array object.
        class VAO : public std::enable_shared_from_this<VAO>
        {
            TLRENDER_NON_COPYABLE(VAO);

        protected:
            void _init();

            VAO(Fl_Vk_Context& ctx);

        public:
            ~VAO();

            //! Create a new object.
            static std::shared_ptr<VAO> create(Fl_Vk_Context& ctx);

            //! Bind the vertex array object.
            void bind(uint32_t frameIndex);

            //! Get Mesh buffer.
            VkBuffer getBuffer() const;

            //! Get device memory.
            VkDeviceMemory getDeviceMemory() const;

            //! Upload mesh data and return offset.
            std::size_t upload(const std::vector<uint8_t>& data);

            //! Draw the vertex array object, by uploading the vertex buffer
            //! object.
            void draw(VkCommandBuffer& cmd, const std::shared_ptr<VBO>& vbo);

            //! Draw the vertex array object.
            void draw(VkCommandBuffer& cmd, const std::size_t size,
                      const VkDeviceSize offset);

        private:
            Fl_Vk_Context& ctx;

            TLRENDER_PRIVATE();
        };
    } // namespace vlk
} // namespace tl
