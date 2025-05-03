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
#include <set>
#include <stdexcept>
#include <vector>

namespace tl
{
    namespace vlk
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

        unsigned int VBO::getID() const
        {
            return 0; // unused in Vulkan
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

        //! Structure used to hold the resource of each upload.
        struct UploadBuffer
        {
            VkBuffer buffer;
            VkDeviceMemory memory;
        };

        //! Structure used to hold per frames in flight resources.
        //! For example, multiple meshes.
        struct FrameResources
        {
            std::vector<UploadBuffer> buffersThisFrame;
        };
        
        struct VAO::Private
        {
            uint32_t frameIndex = 0;

            std::vector<VkBuffer> buffers;
            std::vector<VkDeviceMemory> memories;
            
            std::array<FrameResources, MAX_FRAMES_IN_FLIGHT> frames;
        };

        void VAO::_init()
        {
            TLRENDER_P();

            p.buffers.resize(MAX_FRAMES_IN_FLIGHT, VK_NULL_HANDLE);
            p.memories.resize(MAX_FRAMES_IN_FLIGHT, VK_NULL_HANDLE);
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

            std::set<VkBuffer> buffers;
            for (auto& buffer : p.buffers)
            {
                if (buffer != VK_NULL_HANDLE)
                {
                    buffers.insert(buffer);
                    
                    vkDestroyBuffer(device, buffer, nullptr);
                    buffer = VK_NULL_HANDLE;
                }
            }

            std::set<VkDeviceMemory> memories;
            for (auto& memory : p.memories)
            {
                if (memory != VK_NULL_HANDLE)
                {
                    memories.insert(memory);

                    vkFreeMemory(device, memory, nullptr);
                    memory = VK_NULL_HANDLE;
                }
            }

            for (auto& frame : p.frames)
            {
                for (auto& upload : frame.buffersThisFrame)
                {
                    if (buffers.find(upload.buffer) == buffers.end())
                    {
                        vkDestroyBuffer(device, upload.buffer, nullptr);
                    }

                    if (memories.find(upload.memory) == memories.end())
                    {
                        vkFreeMemory(device, upload.memory, nullptr);
                    }
                }
            }
        }

        std::shared_ptr<VAO> VAO::create(Fl_Vk_Context& context)
        {
            auto out = std::shared_ptr<VAO>(new VAO(context));
            out->_init();
            return out;
        }

        void VAO::upload(const std::vector<uint8_t>& vertexData)
        {
            TLRENDER_P();

            VkDevice device = ctx.device;
            VkPhysicalDevice gpu = ctx.gpu;
            VkBuffer& buffer = p.buffers[p.frameIndex];
            VkDeviceMemory& memory = p.memories[p.frameIndex];

            // 1. Create Buffer
            VkBufferCreateInfo bufferInfo = {};
            bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
            bufferInfo.size = vertexData.size();
            bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
            bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

            if (vkCreateBuffer(device, &bufferInfo, nullptr, &buffer) !=
                VK_SUCCESS)
            {
                throw std::runtime_error("Failed to create vertex buffer!");
            }

            // 2. Allocate memory
            VkMemoryRequirements memRequirements;
            vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

            VkMemoryAllocateInfo allocInfo = {};
            allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            allocInfo.allocationSize = memRequirements.size;
            allocInfo.memoryTypeIndex = findMemoryType(
                gpu, memRequirements.memoryTypeBits,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                    VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

            if (vkAllocateMemory(device, &allocInfo, nullptr, &memory) !=
                VK_SUCCESS)
            {
                throw std::runtime_error(
                    "Failed to allocate vertex buffer memory!");
            }

            // 3. Bind buffer + memory
            vkBindBufferMemory(device, buffer, memory, 0);

            // 4. Copy data to memory
            void* mappedData;
            vkMapMemory(device, memory, 0, vertexData.size(), 0, &mappedData);
            memcpy(mappedData, vertexData.data(), vertexData.size());
            vkUnmapMemory(device, memory);

            // 5. Add it to the queue
            FrameResources& frame = p.frames[p.frameIndex];
            frame.buffersThisFrame.push_back({buffer, memory});
        }

        void VAO::bind(uint32_t value)
        {
            TLRENDER_P();

            if (p.frameIndex == value)
                return;

            p.frameIndex = value;

            FrameResources& frame = p.frames[p.frameIndex];
            VkDevice device = ctx.device;
                
            for (auto& upload : frame.buffersThisFrame)
            {
                vkDestroyBuffer(device, upload.buffer, nullptr);
                vkFreeMemory(device, upload.memory, nullptr);
            }

            frame.buffersThisFrame.clear();
        }

        void VAO::draw(VkCommandBuffer& cmd, std::size_t size)
        {
            TLRENDER_P();

            if (size == 0)
            {
                throw std::runtime_error("VAO::draw tried to draw with a size of 0");
            }

            VkDevice device = ctx.device;
            VkBuffer buffer = p.buffers[p.frameIndex];
            
            VkDeviceSize offsets[1] = {0};
            vkCmdBindVertexBuffers(cmd, 0, 1, &buffer, offsets);
            vkCmdDraw(cmd, size, 1, 0, 0);
        }

        void VAO::draw(VkCommandBuffer& cmd, const std::shared_ptr<VBO>& vbo)
        {
            upload(vbo->getData());
            draw(cmd, vbo->getSize() * 3);
        }
    } // namespace vlk
} // namespace tl
