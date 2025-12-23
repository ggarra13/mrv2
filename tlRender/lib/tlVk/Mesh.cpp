// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// Copyright (c) 2025-Present Gonzalo Garramuño
// All rights reserved.

#include <tlVk/Mesh.h>
#include <tlVk/Vk.h>

#include <FL/Fl_Vk_Utils.H>

#include <tlCore/Error.h>
#include <tlCore/Math.h>
#include <tlCore/Mesh.h>
#include <tlCore/String.h>

#include <array>
#include <set>
#include <stdexcept>
#include <vector>

namespace tl
{
    namespace vlk
    {

        namespace
        {
            // 16 MB
            constexpr size_t DYNAMIC_VERTEX_BUFFER_SIZE = 16 * 1024 * 1024;
        }
        
        TLRENDER_ENUM_IMPL(
            VBOType,
            "Pos2_F32",
            "Pos2_F32_UV_U16",
            "Pos2_F32_Color_F32",
            "Pos3_F32",
            "Pos3_F32_UV_U16",
            "Pos3_F32_UV_U16_Normal_U10",
            "Pos3_F32_UV_U16_Normal_U10_Color_U8",
            "Pos3_F32_UV_F32_Normal_F32",
            "Pos3_F32_UV_F32_Normal_F32_Color_F32",
            "Pos3_F32_Color_U8");
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
        convert(const geom::TriangleMesh2& mesh, vlk::VBOType type)
        {
            return convert(
                mesh, type,
                math::SizeTRange(
                    0, mesh.triangles.size() > 0 ? (mesh.triangles.size() - 1)
                                                 : 0));
        }

        std::vector<uint8_t> convert(
            const geom::TriangleMesh2& mesh, vlk::VBOType type,
            const math::SizeTRange& range)
        {
            const size_t vertexByteCount = vlk::getByteCount(type);
            std::vector<uint8_t> out(
                (range.getMax() - range.getMin() + 1) * 3 * vertexByteCount);
            uint8_t* p = out.data();
            switch (type)
            {
            case vlk::VBOType::Pos2_F32:
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
            case vlk::VBOType::Pos2_F32_UV_U16:
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
            case vlk::VBOType::Pos2_F32_Color_F32:
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
        convert(const geom::TriangleMesh3& mesh, vlk::VBOType type)
        {
            return convert(
                mesh, type,
                math::SizeTRange(
                    0, mesh.triangles.size() > 0 ? (mesh.triangles.size() - 1)
                                                 : 0));
        }

        std::vector<uint8_t> convert(
            const geom::TriangleMesh3& mesh, vlk::VBOType type,
            const math::SizeTRange& range)
        {
            const size_t vertexByteCount = vlk::getByteCount(type);
            std::vector<uint8_t> out(
                (range.getMax() - range.getMin() + 1) * 3 * vertexByteCount);
            uint8_t* p = out.data();
            switch (type)
            {
            case vlk::VBOType::Pos3_F32:
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
            case vlk::VBOType::Pos3_F32_UV_U16:
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
            case vlk::VBOType::Pos3_F32_UV_U16_Normal_U10:
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
            case vlk::VBOType::Pos3_F32_UV_U16_Normal_U10_Color_U8:
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
            case vlk::VBOType::Pos3_F32_UV_F32_Normal_F32:
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
            case vlk::VBOType::Pos3_F32_UV_F32_Normal_F32_Color_F32:
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

            // Actual converted vertex data in Vulkan layout
            std::vector<uint8_t> data;
            std::vector<VkVertexInputBindingDescription> bindingDesc;
            std::vector<VkVertexInputAttributeDescription> attributes;
        };

        void VBO::_init(std::size_t size, VBOType type)
        {
            TLRENDER_P();

            p.size = size;
            p.type = type;

            p.bindingDesc.resize(1);
            p.bindingDesc[0].binding = 0;
            p.bindingDesc[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

            switch (type)
            {
            case VBOType::Pos2_F32:
                p.bindingDesc[0].stride = 2 * sizeof(float);

                p.attributes.push_back({
                    0, // location
                    0, // binding
                    VK_FORMAT_R32G32_SFLOAT,
                    0 // offset
                });
                break;
            case VBOType::Pos2_F32_UV_U16:
                p.bindingDesc[0].stride =
                    2 * sizeof(float) + 2 * sizeof(uint16_t);

                p.attributes.push_back({
                    0, // location
                    0, // binding
                    VK_FORMAT_R32G32_SFLOAT,
                    0 // offset
                });

                p.attributes.push_back(
                    {1, 0, VK_FORMAT_R16G16_UNORM,
                     static_cast<uint32_t>(2 * sizeof(float))});
                break;
            case VBOType::Pos2_F32_Color_F32:
                p.bindingDesc[0].stride = 2 * sizeof(float) + 4 * sizeof(float);

                p.attributes.push_back({
                    0, // location
                    0, // binding
                    VK_FORMAT_R32G32_SFLOAT,
                    0 // offset
                });

                p.attributes.push_back(
                    {1, 0, VK_FORMAT_R32G32B32A32_SFLOAT,
                     static_cast<uint32_t>(2 * sizeof(float))});
                break;
            case VBOType::Pos3_F32:
                p.bindingDesc[0].stride = 3 * sizeof(float);

                p.attributes.push_back({
                    0, // location
                    0, // binding
                    VK_FORMAT_R32G32B32_SFLOAT,
                    0 // offset
                });
                break;
            case VBOType::Pos3_F32_UV_U16:
                p.bindingDesc[0].stride =
                    3 * sizeof(float) + 2 * sizeof(uint16_t);

                p.attributes.push_back({
                    0, // location
                    0, // binding
                    VK_FORMAT_R32G32B32_SFLOAT,
                    0 // offset
                });

                p.attributes.push_back(
                    {1, 0, VK_FORMAT_R16G16_UNORM,
                     static_cast<uint32_t>(3 * sizeof(float))});
                break;
            case VBOType::Pos3_F32_UV_F32_Normal_F32:
                p.bindingDesc[0].stride =
                    3 * sizeof(float) + 2 * sizeof(float) + 3 * sizeof(float);

                p.attributes.push_back({
                    0, // location
                    0, // binding
                    VK_FORMAT_R32G32B32_SFLOAT,
                    0 // offset
                });

                p.attributes.push_back(
                    {1, 0, VK_FORMAT_R32G32_SFLOAT,
                     static_cast<uint32_t>(3 * sizeof(float))});

                p.attributes.push_back(
                    {2, // location
                     0, // binding
                     VK_FORMAT_R32G32B32_SFLOAT,
                     static_cast<uint32_t>(6 * sizeof(float))});
                break;
            case VBOType::Pos3_F32_UV_F32_Normal_F32_Color_F32:
                p.bindingDesc[0].stride = 3 * sizeof(float) +
                                          2 * sizeof(float) +
                                          3 * sizeof(float) + 4 * sizeof(float);

                p.attributes.push_back({
                    0, // location
                    0, // binding
                    VK_FORMAT_R32G32B32_SFLOAT,
                    0 // offset
                });

                p.attributes.push_back(
                    {1, 0, VK_FORMAT_R32G32_SFLOAT,
                     static_cast<uint32_t>(3 * sizeof(float))});

                p.attributes.push_back(
                    {2, // location
                     0, // binding
                     VK_FORMAT_R32G32B32_SFLOAT,
                     static_cast<uint32_t>(5 * sizeof(float))});

                p.attributes.push_back(
                    {3, // location
                     0, // binding
                     VK_FORMAT_R32G32B32A32_SFLOAT,
                     static_cast<uint32_t>(8 * sizeof(float))});
                break;
            case VBOType::Pos3_F32_Color_U8:
                p.bindingDesc[0].stride =
                    3 * sizeof(float) + 4 * sizeof(uint8_t);

                p.attributes.push_back({
                    0, // location
                    0, // binding
                    VK_FORMAT_R32G32B32_SFLOAT,
                    0 // offset
                });

                p.attributes.push_back(
                    {1, 0, VK_FORMAT_R8G8B8A8_UNORM,
                     static_cast<uint32_t>(3 * sizeof(float))});
                break;
            default:
                break;
            }
        }

        const std::vector<VkVertexInputBindingDescription>&
        VBO::getBindingDescription() const
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

        VBO::~VBO() {}

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

        struct VAO::Private
        {
            uint32_t frameIndex = 0;

            VkBuffer vertexBuffer = VK_NULL_HANDLE;
#ifdef MRV2_NO_VMA
            VkDeviceMemory vertexMemory = VK_NULL_HANDLE;
#else
            VmaAllocation allocation = VK_NULL_HANDLE;
#endif
            
            void* mappedPtr = nullptr;

            // relative offset within this frame's partition
            size_t relativeOffset = 0;

            // Aligned region size
            size_t regionSize = 0;

            size_t alignment = 1;
            
        };

        void VAO::_init()
        {
            TLRENDER_P();
            
            VkDevice device = ctx.device;
            VkPhysicalDevice gpu = ctx.gpu;

            VkBufferCreateInfo bufferInfo{};
            bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
            bufferInfo.size = DYNAMIC_VERTEX_BUFFER_SIZE;
            bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
                               VK_BUFFER_USAGE_TRANSFER_DST_BIT;
            bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

#ifdef MRV2_NO_VMA
            if (vkCreateBuffer(device, &bufferInfo, nullptr, &p.vertexBuffer) != VK_SUCCESS)
                throw std::runtime_error("Failed to create persistent vertex buffer");

#else
            VmaAllocationCreateInfo allocCreateInfo = {};
            allocCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
            allocCreateInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
                                    VMA_ALLOCATION_CREATE_MAPPED_BIT; // Persistent mapping

            VmaAllocationInfo allocInfo = {};
            if (vmaCreateBuffer(ctx.allocator, &bufferInfo, &allocCreateInfo, &p.vertexBuffer,
                                &p.allocation, &allocInfo) != VK_SUCCESS)
            {
                throw std::runtime_error("Failed to create vertex buffer with VMA");
            }
            
            p.mappedPtr = allocInfo.pMappedData;
            if (!p.mappedPtr)
            {
                throw std::runtime_error("Failed to get mapped pointer from VMA");
            }
#endif
            
            VkMemoryRequirements memRequirements;
            vkGetBufferMemoryRequirements(device, p.vertexBuffer, &memRequirements);

            VkPhysicalDeviceProperties props;
            vkGetPhysicalDeviceProperties(gpu, &props);
            VkDeviceSize memAlign   = memRequirements.alignment;
            VkDeviceSize vertAlign  = props.limits.minUniformBufferOffsetAlignment;
            VkDeviceSize atomSize   = props.limits.nonCoherentAtomSize;
            p.alignment             = std::max(std::max(memAlign, vertAlign),
                                               atomSize);

            // Now carve equal regions, rounded down to p.alignment:
            const VkDeviceSize totalSize  = memRequirements.size;
            const VkDeviceSize frameCount = MAX_FRAMES_IN_FLIGHT;
            const VkDeviceSize regionSize = totalSize / frameCount;
            p.regionSize = (totalSize / frameCount / p.alignment) * p.alignment;
            if (!p.regionSize)
                throw std::runtime_error("Per-frame region too small for required alignment");
            if (p.regionSize > regionSize)
                throw std::runtime_error("Per-frame region too big with required alignment");
            if (p.regionSize * frameCount > totalSize)
                throw std::runtime_error("Per-frame region * MAX_FRAMES_IN_FLIGHT too big with required alignment");

#ifdef MRV2_NO_VMA
            VkMemoryAllocateInfo allocInfo{};
            allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            allocInfo.allocationSize = memRequirements.size;
            allocInfo.memoryTypeIndex = findMemoryType(
                gpu, memRequirements.memoryTypeBits,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

            if (vkAllocateMemory(device, &allocInfo, nullptr, &p.vertexMemory) != VK_SUCCESS)
                throw std::runtime_error("Failed to allocate persistent vertex memory");

            vkBindBufferMemory(device, p.vertexBuffer, p.vertexMemory, 0);

            // Persistently map
            if (vkMapMemory(device, p.vertexMemory, 0, memRequirements.size, 0, &p.mappedPtr) != VK_SUCCESS)
                throw std::runtime_error("Failed to map persistent vertex buffer");
#endif
        }

        VAO::VAO(Fl_Vk_Context& context) :
            _p(new Private),
            ctx(context)
        {
        }

        VAO::~VAO()
        {
            TLRENDER_P();

            VkDevice device = ctx.device;

#ifdef MRV2_NO_VMA
            if (p.vertexBuffer != VK_NULL_HANDLE)
                vkDestroyBuffer(device, p.vertexBuffer, nullptr);
            if (p.vertexMemory != VK_NULL_HANDLE)
            {
                vkUnmapMemory(device, p.vertexMemory);
                vkFreeMemory(device, p.vertexMemory, nullptr);
            }
#else            
            if (p.vertexBuffer != VK_NULL_HANDLE && p.allocation != VK_NULL_HANDLE)
            {
                vmaDestroyBuffer(ctx.allocator, p.vertexBuffer, p.allocation);
            }
#endif
        }

        std::shared_ptr<VAO> VAO::create(Fl_Vk_Context& context)
        {
            auto out = std::shared_ptr<VAO>(new VAO(context));
            out->_init();
            return out;
        }

        void VAO::bind(uint32_t value)
        {
            TLRENDER_P();
            if (p.frameIndex == value)
                return;
            
            p.frameIndex = value;
            p.relativeOffset = 0;
        }
        
        std::size_t VAO::upload(const std::vector<uint8_t>& vertexData)
        {
            TLRENDER_P();
            const size_t dataSize = vertexData.size();
            const std::size_t startRegion = p.frameIndex * p.regionSize;

            // Compute absolute offset in the big buffer:
            // per-frame region + relative
            size_t absOffset = startRegion + p.relativeOffset;
            void* dst = reinterpret_cast<uint8_t*>(p.mappedPtr) + absOffset;
            memcpy(dst, vertexData.data(), dataSize);

            // Increase the relative offset
            p.relativeOffset += dataSize;
            
            // Align relativeOffset for the next upload
            p.relativeOffset = (p.relativeOffset + p.alignment - 1) & ~(p.alignment - 1);

#ifdef MRV2_NO_VMA
            VkDevice device = ctx.device;
            VkMappedMemoryRange range = {};
            range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
            range.memory = p.vertexMemory;
            range.offset = absOffset;
            range.size = VK_WHOLE_SIZE;

            VkResult result = vkFlushMappedMemoryRanges(device, 1, &range);
            if (result != VK_SUCCESS)
                throw std::runtime_error("vkFlushMappedMemoryRanges failed on macOS");
#else
            VmaAllocationInfo allocInfo;
            vmaGetAllocationInfo(ctx.allocator, p.allocation, &allocInfo);
            if (vmaFlushAllocation(ctx.allocator, p.allocation, absOffset, dataSize) != VK_SUCCESS)
            {
                throw std::runtime_error("vmaFlushAllocation failed");
            }
#endif

            return absOffset;
        }

        void VAO::draw(VkCommandBuffer& cmd, const std::size_t vertexCount,
                       const VkDeviceSize offset)
        {
            TLRENDER_P();

            vkCmdBindVertexBuffers(cmd, 0, 1, &p.vertexBuffer, &offset);
            vkCmdDraw(cmd, static_cast<uint32_t>(vertexCount), 1, 0, 0);
        }

        void VAO::draw(VkCommandBuffer& cmd, const std::shared_ptr<VBO>& vbo)
        {
            const std::size_t offset = upload(vbo->getData());
            draw(cmd, vbo->getSize(), offset);
        }

        VkBuffer VAO::getBuffer() const { return _p->vertexBuffer; }
        VkDeviceMemory VAO::getDeviceMemory() const
        {
#ifdef MRV2_NO_VMA
            return _p->vertexMemory;
#else
            VmaAllocationInfo allocInfo;
            vmaGetAllocationInfo(ctx.allocator, _p->allocation, &allocInfo);
            return allocInfo.deviceMemory;
#endif
        }
        
    } // namespace vlk
} // namespace tl
