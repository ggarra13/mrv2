// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// Copyright (c) 2025-Present Gonzalo Garramuño
// All rights reserved.

#include "USDRenderPrivate.h"
#include "USDRenderStructs.h"
#include "USDTextureSlots.h"

#include <tlVk/Vk.h>

#include <cstdint>

namespace tl
{
    namespace usd
    {

        void Render::_create3DMesh(const std::string& meshName,
                                   const geom::TriangleMesh3& mesh)
        {
            TLRENDER_P();

            size_t triangleCount = mesh.triangles.size();
            if (triangleCount == 0) return;

            ++(p.currentStats.meshes);
            p.currentStats.meshTriangles += triangleCount;

            vlk::VBOType type = vlk::VBOType::Pos3_F32;
            if (!mesh.t.empty() && !mesh.n.empty() && !mesh.c.empty())
            {
                type = vlk::VBOType::Pos3_F32_UV_U16_Normal_U10_Color_U8;
            }
            else if (!mesh.t.empty() && !mesh.n.empty() && !mesh.c.empty())
            {
                // \todo: How do we this distinguish this type?
                type = vlk::VBOType::Pos3_F32_UV_F32_Normal_F32_Color_F32;
            }
            else if (!mesh.t.empty() && !mesh.n.empty())
            {
                type = vlk::VBOType::Pos3_F32_UV_U16_Normal_U10;
            }
            else if (!mesh.t.empty() && !mesh.n.empty())
            {
                // \todo: How do we this distinguish this type?
                type = vlk::VBOType::Pos3_F32_UV_F32_Normal_F32;
            }
            else if (!mesh.t.empty())
            {
                type = vlk::VBOType::Pos3_F32_UV_U16;
            }
            else if (!mesh.c.empty())
            {
                type = vlk::VBOType::Pos3_F32_Color_U8;
            }
            
            if (!p.vbos[meshName] ||
                (p.vbos[meshName] &&
                 p.vbos[meshName]->getSize() != triangleCount * 3))
            {
                p.vbos[meshName] = vlk::VBO::create(triangleCount * 3, type);
            }
            if (p.vbos[meshName])
                p.vbos[meshName]->copy(convert(mesh, type));

            if (!p.vaos[meshName] && p.vbos[meshName])
            {
                p.vaos[meshName] = vlk::VAO::create(ctx);
                
                // Use a 4 Gb cache \@todo: make it optional
                // value must be less than (4292870144 on RTX 3080)
                VkPhysicalDeviceVulkan11Properties vulkan11Props = {};
                vulkan11Props.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_PROPERTIES;
                
                VkPhysicalDeviceProperties2 props2 = {};
                props2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
                props2.pNext = &vulkan11Props;

                vkGetPhysicalDeviceProperties2(ctx.gpu, &props2);

                // Now you have the limit:
                VkDeviceSize size = vulkan11Props.maxMemoryAllocationSize - 1024;
                p.vaos[meshName]->setMemorySize(size);
                p.vaos[meshName]->bind(p.frameIndex);
            }
        }
        
        void Render::_emitMeshDraw(const std::string& pipelineLayoutName,
                                   const std::string& shaderName,
                                   const std::string& meshName,
                                   const math::Matrix4x4f& mvp,
                                   const math::Matrix4x4f& model,
                                   const image::Color4f& color)
        {
            TLRENDER_P();
            auto shader = p.shaders[shaderName];
            VkPipelineLayout pipelineLayout = p.pipelineLayouts[pipelineLayoutName];
            shader->bind(p.frameIndex);
            vkCmdPushConstants(p.cmd, pipelineLayout,
                               shader->getPushStageFlags(), 0,
                               sizeof(color), &color);
            USDTransforms transforms;
            transforms.mvp = mvp;
            transforms.model = model;
            shader->setUniform("transforms", transforms);
            _bindDescriptorSets(pipelineLayoutName, shaderName);
            _vkDraw(meshName);
        }


        void Render::draw3DMesh(const geom::TriangleMesh3& mesh,
                                const math::Matrix4x4f& model,
                                const image::Color4f& color,
                                const std::string& shaderId,
                                const std::unordered_map<int, std::shared_ptr<vlk::Texture> >& textures,
                                const bool enableBlending,
                                const VkBlendFactor srcColorBlendFactor,
                                const VkBlendFactor dstColorBlendFactor,
                                const VkBlendFactor srcAlphaBlendFactor,
                                const VkBlendFactor dstAlphaBlendFactor,
                                const VkBlendOp colorBlendOp,
                                const VkBlendOp alphaBlendOp)
        {
            TLRENDER_P();

            const std::string meshName = "3DMeshes";
            
            _create3DMesh(meshName, mesh);

            std::string pipelineName;
            std::string pipelineLayoutName;
            std::string shaderName;

            const auto mvp = p.transform * model;
            
            if (shaderId.empty() || shaderId == "dummy" || textures.empty())
            {
                shaderName = "dummy";
                pipelineName = pipelineLayoutName = shaderName;

                _createBindingSet(p.shaders[shaderName]);
                
                p.shaders[shaderName]->bind(p.frameIndex);        
            }
            else if (shaderId == "UsdPreviewSurface")
            {
                shaderName = "usd";
                pipelineName = pipelineLayoutName = shaderName;

                _createBindingSet(p.shaders[shaderName]);
                
                p.shaders[shaderName]->bind(p.frameIndex);
                
                auto i = textures.find(USD_DiffuseMap);
                p.shaders[shaderName]->setTexture("u_DiffuseMap", i->second);
                
                i = textures.find(USD_MetallicMap);
                p.shaders[shaderName]->setTexture("u_MetallicMap", i->second);
                
                i = textures.find(USD_RoughnessMap);
                p.shaders[shaderName]->setTexture("u_RoughnessMap", i->second);
                
                i = textures.find(USD_NormalMap);
                p.shaders[shaderName]->setTexture("u_NormalMap", i->second);
                
                i = textures.find(USD_OcclusionMap);
                p.shaders[shaderName]->setTexture("u_AOMap", i->second);
                
                i = textures.find(USD_OpacityMap);
                p.shaders[shaderName]->setTexture("u_OpacityMap", i->second);
            }
            else if (shaderId == "st")
            {
                shaderName = "st";
                pipelineName = pipelineLayoutName = shaderName;

                _createBindingSet(p.shaders[shaderName]);
                
                p.shaders[shaderName]->bind(p.frameIndex);  
            }
            else
            {
                throw std::runtime_error("Unknown shader type " + shaderId);
            }
                
            createPipeline(p.fbo, pipelineName, pipelineLayoutName,
                           shaderName, meshName, enableBlending,
                           srcColorBlendFactor, dstColorBlendFactor,
                           srcAlphaBlendFactor, dstAlphaBlendFactor,
                           colorBlendOp, alphaBlendOp);

            _emitMeshDraw(pipelineLayoutName, shaderName, meshName, mvp,
                          model, color);
        }

        
    } // namespace usd
} // namespace tl
