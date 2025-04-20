// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlVk/Mesh.h>
#include <tlVk/Vk.h>

#include <FL/Fl_Vk_Utils.H>

#include <tlCore/Error.h>
#include <tlCore/Math.h>
#include <tlCore/Mesh.h>
#include <tlCore/String.h>

#include <array>

namespace tl
{
    namespace vk
    {
        TLRENDER_ENUM_IMPL(
            VBOType, "Pos2_F32", "Pos2_F32_UV_U16", "Pos2_F32_Color_F32",
            "Pos3_F32", "Pos3_F32_UV_U16", "Pos3_F32_UV_U16_Normal_U10",
            "Pos3_F32_UV_U16_Normal_U10_Color_U8", "Pos3_F32_UV_F32_Normal_F32",
            "Pos3_F32_UV_F32_Normal_F32_Color_F32", "Pos3_F32_Color_U8");
        TLRENDER_ENUM_SERIALIZE_IMPL(VBOType);

        namespace
        {
            struct PackedNormal
            {
                unsigned int x : 10;
                unsigned int y : 10;
                unsigned int z : 10;
                unsigned int unused : 2;
            };

            struct PackedColor
            {
                unsigned int r : 8;
                unsigned int g : 8;
                unsigned int b : 8;
                unsigned int a : 8;
            };
        } // namespace

        std::size_t getByteCount(VBOType value)
        {
            const std::array<size_t, static_cast<size_t>(VBOType::Count)> data =
                {2 * sizeof(float),
                 2 * sizeof(float) + 2 * sizeof(uint16_t),
                 2 * sizeof(float) + 4 * sizeof(float),
                 3 * sizeof(float),
                 3 * sizeof(float) + 2 * sizeof(uint16_t),
                 3 * sizeof(float) + 2 * sizeof(uint16_t) +
                     sizeof(PackedNormal),
                 3 * sizeof(float) + 2 * sizeof(uint16_t) +
                     sizeof(PackedNormal) + sizeof(PackedColor),
                 3 * sizeof(float) + 2 * sizeof(float) + 3 * sizeof(float),
                 3 * sizeof(float) + 2 * sizeof(float) + 3 * sizeof(float) +
                     4 * sizeof(float),
                 3 * sizeof(float) + sizeof(PackedColor)};
            return data[static_cast<size_t>(value)];
        }

        std::vector<uint8_t>
        convert(const geom::TriangleMesh2& mesh, vk::VBOType type)
        {
            return convert(
                mesh, type,
                math::SizeTRange(
                    0, mesh.triangles.size() > 0 ? (mesh.triangles.size() - 1)
                                                 : 0));
        }

        std::vector<uint8_t> convert(
            const geom::TriangleMesh2& mesh, vk::VBOType type,
            const math::SizeTRange& range)
        {
            const size_t vertexByteCount = vk::getByteCount(type);
            std::vector<uint8_t> out(
                (range.getMax() - range.getMin() + 1) * 3 * vertexByteCount);
            uint8_t* p = out.data();
            switch (type)
            {
            case vk::VBOType::Pos2_F32:
                for (size_t i = range.getMin(); i <= range.getMax(); ++i)
                {
                    const geom::Vertex2* vertices[] = {
                        &mesh.triangles[i].v[0], &mesh.triangles[i].v[1],
                        &mesh.triangles[i].v[2]};
                    for (size_t k = 0; k < 3; ++k)
                    {
                        const size_t v = vertices[k]->v;
                        float* pf = reinterpret_cast<float*>(p);
                        pf[0] = v ? mesh.v[v - 1].x : 0.F;
                        pf[1] = v ? mesh.v[v - 1].y : 0.F;
                        p += 2 * sizeof(float);
                    }
                }
                break;
            case vk::VBOType::Pos2_F32_UV_U16:
                for (size_t i = range.getMin(); i <= range.getMax(); ++i)
                {
                    const geom::Vertex2* vertices[] = {
                        &mesh.triangles[i].v[0], &mesh.triangles[i].v[1],
                        &mesh.triangles[i].v[2]};
                    for (size_t k = 0; k < 3; ++k)
                    {
                        const size_t v = vertices[k]->v;
                        float* pf = reinterpret_cast<float*>(p);
                        pf[0] = v ? mesh.v[v - 1].x : 0.F;
                        pf[1] = v ? mesh.v[v - 1].y : 0.F;
                        p += 2 * sizeof(float);

                        const size_t t = vertices[k]->t;
                        uint16_t* pu16 = reinterpret_cast<uint16_t*>(p);
                        pu16[0] =
                            t ? math::clamp(
                                    static_cast<int>(mesh.t[t - 1].x * 65535.F),
                                    0, 65535)
                              : 0;
                        pu16[1] =
                            t ? math::clamp(
                                    static_cast<int>(mesh.t[t - 1].y * 65535.F),
                                    0, 65535)
                              : 0;
                        p += 2 * sizeof(uint16_t);
                    }
                }
                break;
            case vk::VBOType::Pos2_F32_Color_F32:
                for (size_t i = range.getMin(); i <= range.getMax(); ++i)
                {
                    const geom::Vertex2* vertices[] = {
                        &mesh.triangles[i].v[0], &mesh.triangles[i].v[1],
                        &mesh.triangles[i].v[2]};
                    for (size_t k = 0; k < 3; ++k)
                    {
                        const size_t v = vertices[k]->v;
                        float* pf = reinterpret_cast<float*>(p);
                        pf[0] = v ? mesh.v[v - 1].x : 0.F;
                        pf[1] = v ? mesh.v[v - 1].y : 0.F;
                        p += 2 * sizeof(float);

                        const size_t c = vertices[k]->c;
                        pf = reinterpret_cast<float*>(p);
                        pf[0] = c ? mesh.c[c - 1].x : 1.F;
                        pf[1] = c ? mesh.c[c - 1].y : 1.F;
                        pf[2] = c ? mesh.c[c - 1].z : 1.F;
                        pf[3] = c ? mesh.c[c - 1].w : 1.F;
                        p += 4 * sizeof(float);
                    }
                }
                break;
            default:
                break;
            }
            return out;
        }

        std::vector<uint8_t>
        convert(const geom::TriangleMesh3& mesh, vk::VBOType type)
        {
            return convert(
                mesh, type,
                math::SizeTRange(
                    0, mesh.triangles.size() > 0 ? (mesh.triangles.size() - 1)
                                                 : 0));
        }

        std::vector<uint8_t> convert(
            const geom::TriangleMesh3& mesh, vk::VBOType type,
            const math::SizeTRange& range)
        {
            const size_t vertexByteCount = vk::getByteCount(type);
            std::vector<uint8_t> out(
                (range.getMax() - range.getMin() + 1) * 3 * vertexByteCount);
            uint8_t* p = out.data();
            switch (type)
            {
            case vk::VBOType::Pos3_F32:
                for (size_t i = range.getMin(); i <= range.getMax(); ++i)
                {
                    const geom::Vertex3* vertices[] = {
                        &mesh.triangles[i].v[0], &mesh.triangles[i].v[1],
                        &mesh.triangles[i].v[2]};
                    for (size_t k = 0; k < 3; ++k)
                    {
                        const size_t v = vertices[k]->v;
                        float* pf = reinterpret_cast<float*>(p);
                        pf[0] = v ? mesh.v[v - 1].x : 0.F;
                        pf[1] = v ? mesh.v[v - 1].y : 0.F;
                        pf[2] = v ? mesh.v[v - 1].z : 0.F;
                        p += 3 * sizeof(float);
                    }
                }
                break;
            case vk::VBOType::Pos3_F32_UV_U16:
                for (size_t i = range.getMin(); i <= range.getMax(); ++i)
                {
                    const geom::Vertex3* vertices[] = {
                        &mesh.triangles[i].v[0], &mesh.triangles[i].v[1],
                        &mesh.triangles[i].v[2]};
                    for (size_t k = 0; k < 3; ++k)
                    {
                        const size_t v = vertices[k]->v;
                        float* pf = reinterpret_cast<float*>(p);
                        pf[0] = v ? mesh.v[v - 1].x : 0.F;
                        pf[1] = v ? mesh.v[v - 1].y : 0.F;
                        pf[2] = v ? mesh.v[v - 1].z : 0.F;
                        p += 3 * sizeof(float);

                        const size_t t = vertices[k]->t;
                        uint16_t* pu16 = reinterpret_cast<uint16_t*>(p);
                        pu16[0] =
                            t ? math::clamp(
                                    static_cast<int>(mesh.t[t - 1].x * 65535.F),
                                    0, 65535)
                              : 0;
                        pu16[1] =
                            t ? math::clamp(
                                    static_cast<int>(mesh.t[t - 1].y * 65535.F),
                                    0, 65535)
                              : 0;
                        p += 2 * sizeof(uint16_t);
                    }
                }
                break;
            case vk::VBOType::Pos3_F32_UV_U16_Normal_U10:
                for (size_t i = range.getMin(); i <= range.getMax(); ++i)
                {
                    const geom::Vertex3* vertices[] = {
                        &mesh.triangles[i].v[0], &mesh.triangles[i].v[1],
                        &mesh.triangles[i].v[2]};
                    for (size_t k = 0; k < 3; ++k)
                    {
                        const size_t v = vertices[k]->v;
                        float* pf = reinterpret_cast<float*>(p);
                        pf[0] = v ? mesh.v[v - 1].x : 0.F;
                        pf[1] = v ? mesh.v[v - 1].y : 0.F;
                        pf[2] = v ? mesh.v[v - 1].z : 0.F;
                        p += 3 * sizeof(float);

                        const size_t t = vertices[k]->t;
                        uint16_t* pu16 = reinterpret_cast<uint16_t*>(p);
                        pu16[0] =
                            t ? math::clamp(
                                    static_cast<int>(mesh.t[t - 1].x * 65535.F),
                                    0, 65535)
                              : 0;
                        pu16[1] =
                            t ? math::clamp(
                                    static_cast<int>(mesh.t[t - 1].y * 65535.F),
                                    0, 65535)
                              : 0;
                        p += 2 * sizeof(uint16_t);

                        const size_t n = vertices[k]->n;
                        auto packedNormal = reinterpret_cast<PackedNormal*>(p);
                        packedNormal->x =
                            n ? math::clamp(
                                    static_cast<int>(mesh.n[n - 1].x * 511.F),
                                    -512, 511)
                              : 0;
                        packedNormal->y =
                            n ? math::clamp(
                                    static_cast<int>(mesh.n[n - 1].y * 511.F),
                                    -512, 511)
                              : 0;
                        packedNormal->z =
                            n ? math::clamp(
                                    static_cast<int>(mesh.n[n - 1].z * 511.F),
                                    -512, 511)
                              : 0;
                        p += sizeof(PackedNormal);
                    }
                }
                break;
            case vk::VBOType::Pos3_F32_UV_U16_Normal_U10_Color_U8:
                for (size_t i = range.getMin(); i <= range.getMax(); ++i)
                {
                    const geom::Vertex3* vertices[] = {
                        &mesh.triangles[i].v[0], &mesh.triangles[i].v[1],
                        &mesh.triangles[i].v[2]};
                    for (size_t k = 0; k < 3; ++k)
                    {
                        const size_t v = vertices[k]->v;
                        float* pf = reinterpret_cast<float*>(p);
                        pf[0] = v ? mesh.v[v - 1].x : 0.F;
                        pf[1] = v ? mesh.v[v - 1].y : 0.F;
                        pf[2] = v ? mesh.v[v - 1].z : 0.F;
                        p += 3 * sizeof(float);

                        const size_t t = vertices[k]->t;
                        uint16_t* pu16 = reinterpret_cast<uint16_t*>(p);
                        pu16[0] =
                            t ? math::clamp(
                                    static_cast<int>(mesh.t[t - 1].x * 65535.F),
                                    0, 65535)
                              : 0;
                        pu16[1] =
                            t ? math::clamp(
                                    static_cast<int>(mesh.t[t - 1].y * 65535.F),
                                    0, 65535)
                              : 0;
                        p += 2 * sizeof(uint16_t);

                        const size_t n = vertices[k]->n;
                        auto packedNormal = reinterpret_cast<PackedNormal*>(p);
                        packedNormal->x =
                            n ? math::clamp(
                                    static_cast<int>(mesh.n[n - 1].x * 511.F),
                                    -512, 511)
                              : 0;
                        packedNormal->y =
                            n ? math::clamp(
                                    static_cast<int>(mesh.n[n - 1].y * 511.F),
                                    -512, 511)
                              : 0;
                        packedNormal->z =
                            n ? math::clamp(
                                    static_cast<int>(mesh.n[n - 1].z * 511.F),
                                    -512, 511)
                              : 0;
                        p += sizeof(PackedNormal);

                        const size_t c = vertices[k]->c;
                        auto packedColor = reinterpret_cast<PackedColor*>(p);
                        packedColor->r =
                            c ? math::clamp(
                                    static_cast<int>(mesh.c[c - 1].x * 255.F),
                                    0, 255)
                              : 255;
                        packedColor->g =
                            c ? math::clamp(
                                    static_cast<int>(mesh.c[c - 1].y * 255.F),
                                    0, 255)
                              : 255;
                        packedColor->b =
                            c ? math::clamp(
                                    static_cast<int>(mesh.c[c - 1].z * 255.F),
                                    0, 255)
                              : 255;
                        packedColor->a =
                            c ? math::clamp(
                                    static_cast<int>(mesh.c[c - 1].w * 255.F),
                                    0, 255)
                              : 255;
                        p += sizeof(PackedColor);
                    }
                }
                break;
            case vk::VBOType::Pos3_F32_UV_F32_Normal_F32:
                for (size_t i = range.getMin(); i <= range.getMax(); ++i)
                {
                    const geom::Vertex3* vertices[] = {
                        &mesh.triangles[i].v[0], &mesh.triangles[i].v[1],
                        &mesh.triangles[i].v[2]};
                    for (size_t k = 0; k < 3; ++k)
                    {
                        const size_t v = vertices[k]->v;
                        float* pf = reinterpret_cast<float*>(p);
                        pf[0] = v ? mesh.v[v - 1].x : 0.F;
                        pf[1] = v ? mesh.v[v - 1].y : 0.F;
                        pf[2] = v ? mesh.v[v - 1].z : 0.F;
                        p += 3 * sizeof(float);

                        const size_t t = vertices[k]->t;
                        pf = reinterpret_cast<float*>(p);
                        pf[0] = t ? mesh.t[t - 1].x : 0.F;
                        pf[1] = t ? mesh.t[t - 1].y : 0.F;
                        p += 2 * sizeof(float);

                        const size_t n = vertices[k]->n;
                        pf = reinterpret_cast<float*>(p);
                        pf[0] = n ? mesh.n[n - 1].x : 0.F;
                        pf[1] = n ? mesh.n[n - 1].y : 0.F;
                        pf[2] = n ? mesh.n[n - 1].z : 0.F;
                        p += 3 * sizeof(float);
                    }
                }
                break;
            case vk::VBOType::Pos3_F32_UV_F32_Normal_F32_Color_F32:
                for (size_t i = range.getMin(); i <= range.getMax(); ++i)
                {
                    const geom::Vertex3* vertices[] = {
                        &mesh.triangles[i].v[0], &mesh.triangles[i].v[1],
                        &mesh.triangles[i].v[2]};
                    for (size_t k = 0; k < 3; ++k)
                    {
                        const size_t v = vertices[k]->v;
                        float* pf = reinterpret_cast<float*>(p);
                        pf[0] = v ? mesh.v[v - 1].x : 0.F;
                        pf[1] = v ? mesh.v[v - 1].y : 0.F;
                        pf[2] = v ? mesh.v[v - 1].z : 0.F;
                        p += 3 * sizeof(float);

                        const size_t t = vertices[k]->t;
                        pf = reinterpret_cast<float*>(p);
                        pf[0] = t ? mesh.t[t - 1].x : 0.F;
                        pf[1] = t ? mesh.t[t - 1].y : 0.F;
                        p += 2 * sizeof(float);

                        const size_t n = vertices[k]->n;
                        pf = reinterpret_cast<float*>(p);
                        pf[0] = n ? mesh.n[n - 1].x : 0.F;
                        pf[1] = n ? mesh.n[n - 1].y : 0.F;
                        pf[2] = n ? mesh.n[n - 1].z : 0.F;
                        p += 3 * sizeof(float);

                        const size_t c = vertices[k]->c;
                        pf = reinterpret_cast<float*>(p);
                        pf[0] = c ? mesh.c[c - 1].x : 1.F;
                        pf[1] = c ? mesh.c[c - 1].y : 1.F;
                        pf[2] = c ? mesh.c[c - 1].z : 1.F;
                        pf[3] = c ? mesh.c[c - 1].w : 1.F;
                        p += 4 * sizeof(float);
                    }
                }
                break;
            default:
                break;
            }
            return out;
        }
        
        struct VBO::Private
        {
            std::size_t size = 0;
            VBOType type = VBOType::First;
            std::vector<uint8_t> data; // Actual converted vertex data in Vulkan layout
            VkVertexInputBindingDescription bindingDesc;
            std::vector<VkVertexInputAttributeDescription> attributes;
        };

        void VBO::_init(std::size_t size, VBOType type)
        {
            TLRENDER_P();
            p.size = size;
            p.type = type;
            
            p.bindingDesc = {};
            p.bindingDesc.binding = 0;
            p.bindingDesc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
            
            switch (type)
            {
            case VBOType::Pos2_F32:
                p.bindingDesc.stride = 2 * sizeof(float);
                
                p.attributes.push_back({
                        0,  // location
                        0,  // binding
                        VK_FORMAT_R32G32_SFLOAT,
                        0   // offset
                    });
                break;
            case VBOType::Pos2_F32_UV_U16:
                p.bindingDesc.stride = 2 * sizeof(float) + 2 * sizeof(uint16_t);
                
                p.attributes.push_back({
                        0,  // location
                        0,  // binding
                        VK_FORMAT_R32G32B32_SFLOAT,
                        0   // offset
                    });
                
                p.attributes.push_back({
                        1,
                        0,
                        VK_FORMAT_R16G16_UNORM,
                        static_cast<uint32_t>(2 * sizeof(float))
                    });
                break;
            case VBOType::Pos2_F32_Color_F32:
                p.bindingDesc.stride = 2 * sizeof(float) + 4 * sizeof(float);
                
                p.attributes.push_back({
                        0,  // location
                        0,  // binding
                        VK_FORMAT_R32G32B32_SFLOAT,
                        0   // offset
                    });
                
                p.attributes.push_back({
                        1,
                        0,
                        VK_FORMAT_R32G32B32A32_SFLOAT,
                        static_cast<uint32_t>(3 * sizeof(float))
                    });
                break;
            case VBOType::Pos3_F32:
                p.bindingDesc.stride = 3 * sizeof(float);
                
                p.attributes.push_back({
                        0,  // location
                        0,  // binding
                        VK_FORMAT_R32G32B32_SFLOAT,
                        0   // offset
                    });
                break;
            case VBOType::Pos3_F32_UV_U16:
                p.bindingDesc.stride = 3 * sizeof(float) + 2 * sizeof(uint16_t);
                
                p.attributes.push_back({
                        0,  // location
                        0,  // binding
                        VK_FORMAT_R32G32B32_SFLOAT,
                        0   // offset
                    });
                
                p.attributes.push_back({
                        1,
                        0,
                        VK_FORMAT_R16G16_UNORM,
                        static_cast<uint32_t>(3 * sizeof(float))
                    });
                break;
            case VBOType::Pos3_F32_UV_F32_Normal_F32:
                p.bindingDesc.stride = 3 * sizeof(float) + 2 * sizeof(float) +
                                       3 * sizeof(float);
                
                p.attributes.push_back({
                        0,  // location
                        0,  // binding
                        VK_FORMAT_R32G32B32_SFLOAT,
                        0   // offset
                    });
                
                p.attributes.push_back({
                        1,
                        0,
                        VK_FORMAT_R32G32_SFLOAT,
                        static_cast<uint32_t>(3 * sizeof(float))
                    });
                
                p.attributes.push_back({
                        2,  // location
                        0,  // binding
                        VK_FORMAT_R32G32B32_SFLOAT,
                        static_cast<uint32_t>(6 * sizeof(float))
                    });
                break;
            case VBOType::Pos3_F32_UV_F32_Normal_F32_Color_F32:
                p.bindingDesc.stride = 3 * sizeof(float) + 2 * sizeof(float) +
                                       3 * sizeof(float) + 4 * sizeof(float);
                
                p.attributes.push_back({
                        0,  // location
                        0,  // binding
                        VK_FORMAT_R32G32B32_SFLOAT,
                        0   // offset
                    });
                
                p.attributes.push_back({
                        1,
                        0,
                        VK_FORMAT_R32G32_SFLOAT,
                        static_cast<uint32_t>(3 * sizeof(float))
                    });
                
                p.attributes.push_back({
                        2,  // location
                        0,  // binding
                        VK_FORMAT_R32G32B32_SFLOAT,
                        static_cast<uint32_t>(5 * sizeof(float))
                    });
                
                p.attributes.push_back({
                        3,  // location
                        0,  // binding
                        VK_FORMAT_R32G32B32A32_SFLOAT,
                        static_cast<uint32_t>(8 * sizeof(float))
                    });
                break;
            case VBOType::Pos3_F32_Color_U8:
                p.bindingDesc.stride = 3 * sizeof(float) + 2 * sizeof(uint8_t);
                
                p.attributes.push_back({
                        0,  // location
                        0,  // binding
                        VK_FORMAT_R32G32B32_SFLOAT,
                        0   // offset
                    });
                
                p.attributes.push_back({
                        1,
                        0,
                        VK_FORMAT_R8G8B8A8_UNORM,
                        static_cast<uint32_t>(3 * sizeof(float))
                    });
                break;
            default:
                break;
            }
        }

        const VkVertexInputBindingDescription VBO::getBindingDescription() const
        {
            return _p->bindingDesc;
        }
        
        const std::vector<VkVertexInputAttributeDescription>&
        VBO::getAttributes() const
        {
            return _p->attributes;
        }
        
        VBO::VBO() :
            _p(new Private)
        {
        }

        VBO::~VBO()
        {
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

        unsigned int VBO::getID() const
        {
            return 0;  // unused in Vulkan
        }

        void VBO::copy(const std::vector<uint8_t>& data)
        {
            _p->data = data;
        }

        void VBO::copy(
            const std::vector<uint8_t>& data, std::size_t offset,
            std::size_t size)
        {
            _p->data = data;
        }

        struct VAO::Private
        {
            VkBuffer buffer;
            VkDeviceMemory memory;
            
            uint32_t vao = 0;  // unused in Vulkan
        };

        void VAO::_init(VBOType type, unsigned int vbo)
        {
            TLRENDER_P();
        }

        VAO::VAO(Fl_Vk_Context& context) :
            _p(new Private),
            ctx(context)
        {
        }

        VAO::~VAO()
        {
            TLRENDER_P();
        }

        std::shared_ptr<VAO> VAO::create(Fl_Vk_Context& context,
                                         VBOType type, unsigned int vbo)
        {
            auto out = std::shared_ptr<VAO>(new VAO(context));
            out->_init(type, vbo);
            return out;
        }

        unsigned int VAO::getID() const
        {
            return _p->vao;
        }
        
        void VAO::upload(const std::vector<uint8_t>& vertexData)
        {
            TLRENDER_P();
            
            // 1. Create Buffer
            VkBufferCreateInfo bufferInfo = {};
            bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
            bufferInfo.size = vertexData.size();
            bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
            bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

            VkDevice device = ctx.device;
            VkPhysicalDevice gpu = ctx.gpu;
            
            if (vkCreateBuffer(device, &bufferInfo, nullptr, &p.buffer) !=
                VK_SUCCESS)
            {
                throw std::runtime_error("failed to create vertex buffer!");
            }

            // 2. Allocate memory
            VkMemoryRequirements memRequirements;
            vkGetBufferMemoryRequirements(device, p.buffer, &memRequirements);

            VkMemoryAllocateInfo allocInfo = {};
            allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            allocInfo.allocationSize = memRequirements.size;
            allocInfo.memoryTypeIndex = findMemoryType(
                gpu,
                memRequirements.memoryTypeBits,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
                );

            if (vkAllocateMemory(device, &allocInfo, nullptr,
                                 &p.memory) != VK_SUCCESS)
            {
                throw std::runtime_error("failed to allocate vertex buffer memory!");
            }

            // 3. Bind buffer + memory
            vkBindBufferMemory(device, p.buffer, p.memory, 0);

            // 4. Copy data to memory
            void* mappedData;
            vkMapMemory(device, p.memory, 0, vertexData.size(), 0, &mappedData);
            memcpy(mappedData, vertexData.data(), vertexData.size());
            vkUnmapMemory(device, p.memory);
        }
        
        void VAO::bind() {}

        void VAO::draw(unsigned int mode, std::size_t offset, std::size_t size)
        {
            // old opengl function
        }

        void VAO::draw(VkCommandBuffer& cmd, VkDeviceSize* offsets,
                       std::size_t size)
        {
            TLRENDER_P();
            
            vkCmdBindVertexBuffers(cmd, 0, 1, &p.buffer, offsets);
            vkCmdDraw(cmd, size, 1, 0,  0);
        }
    } // namespace vk
} // namespace tl
