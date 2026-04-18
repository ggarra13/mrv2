// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// Copyright (C) 2025-Present Gonzalo Garramuño.
// All rights reserved.

#pragma once

#include "USDMaterial.h"
#include "USDMeshOptimization.h"

#include <tlTimeline/RenderOptions.h>

#include <tlVk/Mesh.h>
#include <tlVk/OffscreenBuffer.h>
#include <tlVk/PipelineCreationState.h>
#include <tlVk/Shader.h>
#include <tlVk/Texture.h>

#include <tlIO/System.h>

#include <FL/Fl_Vk_Window.H>

#include <unordered_map>

namespace tl
{
    //! USD support
    namespace usd
    {   
        //! Vulkan renderer.
        class Render
        {
            TLRENDER_NON_COPYABLE(Render);

        protected:
            void _init(const std::shared_ptr<system::Context>&);

            Render(Fl_Vk_Context& ctx);

        public:
            virtual ~Render();

            //! Create a new renderer.
            static std::shared_ptr<Render> create(
                Fl_Vk_Context& ctx, const std::shared_ptr<system::Context>&);

            //! Proper Vulkan begin function
            void begin(
                VkCommandBuffer& cmd,
                const std::shared_ptr<vlk::OffscreenBuffer> fbo,
                const uint32_t frameIndex, const math::Size2i&,
                const timeline::RenderOptions& = timeline::RenderOptions());
            void end();

            Fl_Vk_Context& getContext() const;
            VkCommandBuffer getCommandBuffer() const;
            uint32_t getFrameIndex() const;

            // Main entry pipeline creation function
            void createPipeline(const std::string& pipelineName,
                                const std::string& pipelineLayoutName,
                                const VkRenderPass renderPass,
                                const std::shared_ptr<vlk::Shader>& shader,
                                const std::shared_ptr<vlk::VBO>& mesh,
                                const vlk::ColorBlendStateInfo& cb = vlk::ColorBlendStateInfo(),
                                const vlk::DepthStencilStateInfo& ds = vlk::DepthStencilStateInfo(),
                                const vlk::MultisampleStateInfo& ms = vlk::MultisampleStateInfo());         
            void createPipeline(
                const std::shared_ptr<vlk::OffscreenBuffer>& fbo,
                const std::string& pipelineName,
                const std::string& pipelineLayoutName,
                const std::string& shaderName,
                const std::string& meshName,
                const bool enableBlending = false,
                const VkBlendFactor srcColorBlendFactor =
                    VK_BLEND_FACTOR_SRC_ALPHA,
                const VkBlendFactor dstColorBlendFactor =
                    VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
                const VkBlendFactor srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
                const VkBlendFactor dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
                const VkBlendOp colorBlendOp = VK_BLEND_OP_ADD,
                const VkBlendOp alphaBlendOp = VK_BLEND_OP_ADD,
                const VkBool32 depthTest = VK_TRUE,
                const VkBool32 depthWrite = VK_TRUE);
            void usePipeline(const std::string& pipelineName);

            //! Overriden begin function
            void begin(
                const math::Size2i&, const timeline::RenderOptions& =
                timeline::RenderOptions());

            math::Size2i getRenderSize() const;
            void setRenderSize(const math::Size2i&);

            //! Gets the internal FBO
            std::shared_ptr<vlk::OffscreenBuffer> getFBO() const;

            //! Gets the current Vulkan render pass.
            VkRenderPass getRenderPass() const;

            //! Changes the current Vulkan render pass.
            void setRenderPass(VkRenderPass);
            
            math::Box2i getViewport() const;
            void setViewport(const math::Box2i&);
            void clearViewport(const image::Color4f&);
            bool getClipRectEnabled() const;
            void setClipRectEnabled(bool);
            math::Box2i getClipRect() const;
            void setClipRect(const math::Box2i&);


            void setCameraWorldPosition(const math::Vector3f&);
            
            math::Matrix4x4f getTransform() const;
            void setTransform(const math::Matrix4x4f&);
            void setViewMatrix(const math::Matrix4x4f&);
            
            void applyTransforms();
            
            //! @{
            //!     These functions draw to the internal FBO.
            void drawMesh(const geom::TriangleMesh3&,
                          const usd::MeshOptimization&,
                          const math::Matrix4x4f&,
                          const image::Color4f&,
                          const std::string& shaderName,
                          const std::unordered_map<int, std::shared_ptr<vlk::Texture> >& textures,
                          const Material& material = Material(), 
                          const bool enableBlending = false,
                          const VkBool32 depthTest = VK_TRUE,
                          const VkBool32 depthWrite = VK_TRUE,
                          const VkBlendFactor srcColorBlendFactor =
                          VK_BLEND_FACTOR_SRC_ALPHA,
                          const VkBlendFactor dstColorBlendFactor =
                          VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
                          const VkBlendFactor srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
                          const VkBlendFactor dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
                          const VkBlendOp colorBlendOp = VK_BLEND_OP_ADD,
                          const VkBlendOp alphaBlendOp = VK_BLEND_OP_ADD);
            
            //! Vulkan render pass functions
            void beginLoadRenderPass();
            void beginRenderPass();
            void endRenderPass();
            void setupViewportAndScissor();
            
        private:
            void _uploadMesh(const std::string& meshName,
                             const geom::TriangleMesh2& mesh,
                             size_t triangleCount);

            void _emitMeshDraw(const std::string& pipelineLayoutName,
                               const std::string& shaderName,
                               const std::string& meshName,
                               const math::Matrix4x4f& mvp,
                               const math::Matrix4x4f& model,
                               const image::Color4f& color);

            void _setupRectCommon(const math::Box2i& box);
            
            void _create3DMesh(
                const std::string& meshName, const geom::TriangleMesh3& mesh,
                const usd::MeshOptimization&);
            void _createBindingSet(const std::shared_ptr<vlk::Shader>& shaderName);
            VkPipelineLayout _createPipelineLayout(
                const std::string& pipelineLayoutName,
                const std::shared_ptr<vlk::Shader> shader);
            void _bindDescriptorSets(
                const std::string& pipelineLayoutName,
                const std::string& shaderName);
            void _vkDraw(const std::string& meshName);

            Fl_Vk_Context& ctx;

            TLRENDER_PRIVATE();
        };

    } // namespace usd
} // namespace tl
