// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// Copyright (c) 2025-Present Gonzalo Garramuño
// All rights reserved.

#include "USDTextureSlots.h"
#include "USDRenderPrivate.h"
#include "USDRenderStructs.h"

#include <tlVk/Vk.h>

#include <cstdint>

namespace tl
{
    namespace usd
    {

        // ------------------------------------------------------------------
        //  _create3DMesh
        //
        //  Responsible only for the VBO (CPU-side vertex data).  The VAO
        //  pool lives in Private and is managed by the pool itself; we no
        //  longer create or cache per-mesh VAOs here.
        // ------------------------------------------------------------------
        void Render::_create3DMesh(const std::string& meshName,
                                   const geom::TriangleMesh3& mesh,
                                   const usd::MeshOptimization& opt)
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
                if (opt.floatUVs || opt.floatColors || opt.floatNormals)
                    type = vlk::VBOType::Pos3_F32_UV_F32_Normal_F32_Color_F32;
            }
            else if (!mesh.t.empty() && !mesh.n.empty())
            {
                type = vlk::VBOType::Pos3_F32_UV_U16_Normal_U10;
                if (opt.floatUVs || opt.floatNormals)
                    type = vlk::VBOType::Pos3_F32_UV_F32_Normal_F32;
            }
            else if (!mesh.t.empty())
            {
                type = vlk::VBOType::Pos3_F32_UV_U16;
                if (opt.floatUVs)
                    type = vlk::VBOType::Pos3_F32_UV_F32;
            }
            else if (!mesh.c.empty())
            {
                type = vlk::VBOType::Pos3_F32_Color_U8;
            }

            // Rebuild the VBO whenever the triangle count or type changes.
            if (!p.vbos[meshName] ||
                p.vbos[meshName]->getSize() != triangleCount * 3 ||
                p.vbos[meshName]->getType() != type)
            {
                p.vbos[meshName] = vlk::VBO::create(triangleCount * 3, type);
            }
            if (p.vbos[meshName])
                p.vbos[meshName]->copy(convert(mesh, type));

            // ------------------------------------------------------------------
            //  Pool initialisation – create the pool on first use.
            //
            //  The pool is a member of Private:
            //    std::shared_ptr<vlk::VAOPool> vaoPool;
            //
            //  Call  p.vaoPool->bind(p.frameIndex)  once per frame, e.g. in
            //  Render::begin() - NOT here
            // ------------------------------------------------------------------
            if (!p.vaoPool)
            {
                VkDeviceSize slotSize =
                    static_cast<VkDeviceSize>(memory::gigabyte);
                p.vaoPool = vlk::VAOPool::create(ctx, slotSize);
            }
        }

        // ------------------------------------------------------------------
        //  _emitMeshDraw
        //
        //  Uploads the VBO into whichever pool slot has room and issues the
        //  draw command.  The VAOAllocation token is transient – it is only
        //  valid for the current command buffer recording.
        // ------------------------------------------------------------------
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
            transforms.mvp   = mvp;
            transforms.model = model;
            transforms.view  = p.viewMatrix;
            shader->setUniform("transforms", transforms);
            _bindDescriptorSets(pipelineLayoutName, shaderName);

            // Upload the vertex data into the pool and draw immediately.
            // The pool selects a slot with enough room, overflowing to a new
            // 1 GB buffer when the current one is full.
            const vlk::VAOAllocation alloc =
                p.vaoPool->upload(p.vbos[meshName]);
            p.vaoPool->draw(p.cmd, alloc);
        }


        void Render::draw3DMesh(const geom::TriangleMesh3& mesh,
                                const usd::MeshOptimization& meshOptimization,
                                const math::Matrix4x4f& model,
                                const image::Color4f& color,
                                const std::string& shaderId,
                                const std::unordered_map<int, std::shared_ptr<vlk::Texture> >& textures,
                                const usd::Material& material,
                                const bool enableBlending,
                                const VkBool32 depthTest,
                                const VkBool32 depthWrite,
                                const VkBlendFactor srcColorBlendFactor,
                                const VkBlendFactor dstColorBlendFactor,
                                const VkBlendFactor srcAlphaBlendFactor,
                                const VkBlendFactor dstAlphaBlendFactor,
                                const VkBlendOp colorBlendOp,
                                const VkBlendOp alphaBlendOp)
        {
            TLRENDER_P();

            std::string pipelineName;
            std::string pipelineLayoutName;
            std::string shaderName;
            const std::string meshName = "3DMeshes";
            if (enableBlending)
            {
                pipelineName = "blending";
            }
            else
            {
                pipelineName = "no_blending";
            }
            if (depthTest)
            {
                pipelineName += "_depth_test";
            }
            else
            {
                pipelineName += "_no_depth_test";
            }
            if (depthWrite)
            {
                pipelineName += "_depth_write";
            }
            else
            {
                pipelineName += "_no_depth_write";
            }
            
            _create3DMesh(meshName, mesh, meshOptimization);


            const auto mvp = p.transform * model;
            
            if (shaderId.empty() || shaderId == "dummy" || textures.empty())
            {
                shaderName = "dummy";
                pipelineLayoutName = shaderName;

                _createBindingSet(p.shaders[shaderName]);
                
                p.shaders[shaderName]->bind(p.frameIndex);
            }
            else if (shaderId == "UsdPreviewSurface")
            {
                shaderName = "usd";
                pipelineLayoutName = shaderName;

                _createBindingSet(p.shaders[shaderName]);
                
                p.shaders[shaderName]->bind(p.frameIndex);
                
                auto i = textures.find(USD_DiffuseMap);
                p.shaders[shaderName]->setTexture("u_DiffuseMap", i->second);
                
                i = textures.find(USD_EmissiveMap);
                p.shaders[shaderName]->setTexture("u_EmissiveMap", i->second);
                
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

                USDShaderParameters params;
                params.opacityThreshold = material.opacityThreshold;
                p.shaders[shaderName]->setUniform("params", params);

                USDSceneParameters scene;
                scene.camPos = p.cameraPosition;
                p.shaders[shaderName]->setUniform("scene", scene);
            }
            else if (shaderId == "st")
            {
                shaderName = "st";
                pipelineLayoutName = shaderName;
                
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
                           colorBlendOp, alphaBlendOp, depthTest,
                           depthWrite);

            _emitMeshDraw(pipelineLayoutName, shaderName, meshName, mvp,
                          model, color);
        }

        
    } // namespace usd
} // namespace tl
