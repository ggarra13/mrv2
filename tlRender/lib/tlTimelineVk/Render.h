// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// Copyright (C) 2025-Present Gonzalo Garramuño.
// All rights reserved.

#pragma once

#include <tlTimeline/IRender.h>

#include <tlVk/Mesh.h>
#include <tlVk/OffscreenBuffer.h>
#include <tlVk/PipelineCreationState.h>
#include <tlVk/Shader.h>
#include <tlVk/Texture.h>

#include <tlCore/LRUCache.h>

#if defined(TLRENDER_OCIO)
#    include <OpenColorIO/OpenColorIO.h>
#endif // TLRENDER_OCIO

#if defined(TLRENDER_OCIO)
namespace OCIO = OCIO_NAMESPACE;
#endif // TLRENDER_OCIO

#include <FL/Fl_Vk_Window.H>

struct pl_shader_res;

namespace tl
{
    //! Timeline Vulkan support
    namespace timeline_vlk
    {
        //! Texture cache.
        typedef memory::LRUCache<
            std::shared_ptr<image::Image>,
            std::vector<std::shared_ptr<vlk::Texture> > >
            TextureCache;

        //! Stereo mode.
        enum class StereoType
        {
            kScanlines,
            kColumns,
            kCheckers,
        };
        
        //! Vulkan renderer.
        class Render : public timeline::IRender
        {
            TLRENDER_NON_COPYABLE(Render);

        protected:
            void _init(
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<TextureCache>&);

            Render(Fl_Vk_Context& ctx);

        public:
            virtual ~Render();

            //! Create a new renderer.
            static std::shared_ptr<Render> create(
                Fl_Vk_Context& ctx, const std::shared_ptr<system::Context>&,
                const std::shared_ptr<TextureCache>& = nullptr);

            //! Get the texture cache.
            const std::shared_ptr<TextureCache>& getTextureCache() const;

            //! Overriden begin function
            void begin(
                const math::Size2i&, const timeline::RenderOptions& =
                                         timeline::RenderOptions()) override;

            //! Proper Vulkan begin function
            void begin(
                VkCommandBuffer& cmd,
                const std::shared_ptr<vlk::OffscreenBuffer> fbo,
                const uint32_t frameIndex, const math::Size2i&,
                const timeline::RenderOptions& = timeline::RenderOptions());
            void end() override;

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
                const VkBlendOp alphaBlendOp = VK_BLEND_OP_ADD);
            void usePipeline(const std::string& pipelineName);
            math::Size2i getRenderSize() const override;
            void setRenderSize(const math::Size2i&) override;
            std::shared_ptr<vlk::OffscreenBuffer> getFBO() const;
            VkRenderPass getRenderPass() const;
            void setRenderPass(VkRenderPass);
            math::Box2i getViewport() const override;
            void setViewport(const math::Box2i&) override;
            void clearViewport(const image::Color4f&) override;
            bool getClipRectEnabled() const override;
            void setClipRectEnabled(bool) override;
            math::Box2i getClipRect() const override;
            void setClipRect(const math::Box2i&) override;
            math::Matrix4x4f getTransform() const override;
            void setTransform(const math::Matrix4x4f&) override;
            void applyTransforms();
            void setOCIOOptions(const timeline::OCIOOptions&) override;
            void setLUTOptions(const timeline::LUTOptions&) override;
            void setHDROptions(const timeline::HDROptions&) override;

            void setupViewportAndScissor() override;
            
            //! These functions draw to the internal FBO.
            void drawRect(const math::Box2i&, const image::Color4f&) override;
            void drawMesh(const std::string& pipelineName,
                          const std::string& pipelineLayoutName,
                          const std::string& shaderName,
                          const std::string& meshName,
                          const geom::TriangleMesh2&,
                          const math::Vector2i& position,
                          const image::Color4f&,
                          const bool enableBlending = false,
                          const VkBlendFactor srcColorBlendFactor =
                          VK_BLEND_FACTOR_SRC_ALPHA,
                          const VkBlendFactor dstColorBlendFactor =
                          VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
                          const VkBlendFactor srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
                          const VkBlendFactor dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
                          const VkBlendOp colorBlendOp = VK_BLEND_OP_ADD,
                          const VkBlendOp alphaBlendOp = VK_BLEND_OP_ADD);
            //! Create text meshes to speed up drawing
            void appendText(
                std::vector<timeline::TextInfo>& info,
                const std::vector<std::shared_ptr<image::Glyph> >& glyphs,
                const math::Vector2i& position) override;
            void drawText(
                const timeline::TextInfo&,
                const math::Vector2i& position,
                const image::Color4f&) override;
            //! These functions draw to the viewport
            void drawRect(const std::string& pipelineName,
                          const math::Box2i&, const image::Color4f&,
                          const bool enableBlending = true);
            void drawMesh(const std::string& pipelineName,
                          const std::string& shaderName,
                          const std::string& meshName,
                          const geom::TriangleMesh2&,
                          const math::Vector2i& position,
                          const image::Color4f&,
                          const bool enableBlending = false);
            
            void drawMesh(
                const geom::TriangleMesh2&, const math::Vector2i& position,
                const image::Color4f&, const std::string& meshName) override;
            void drawColorMesh(
                const geom::TriangleMesh2&, const math::Vector2i& position,
                const image::Color4f&) override {};
            void drawColorMesh(
                const std::string& pipelineLayoutName,
                const geom::TriangleMesh2&, const math::Vector2i& position,
                const image::Color4f&);
            void drawTexture(
                unsigned int, const math::Box2i&,
                const image::Color4f& =
                    image::Color4f(1.F, 1.F, 1.F)) override {};
            void drawTexture(
                const std::shared_ptr<vlk::Texture>&, const math::Box2i&,
                const image::Color4f& = image::Color4f(1.F, 1.F, 1.F));
            void drawImage(
                const std::shared_ptr<image::Image>&, const math::Box2i&,
                const image::Color4f& = image::Color4f(1.F, 1.F, 1.F),
                const timeline::ImageOptions& =
                    timeline::ImageOptions()) override;
            void drawImage(
                const std::shared_ptr<vlk::OffscreenBuffer>& fbo,
                const std::shared_ptr<image::Image>&, const math::Box2i&,
                const image::Color4f& = image::Color4f(1.F, 1.F, 1.F),
                const timeline::ImageOptions& = timeline::ImageOptions());
            void drawVideo(
                const std::vector<timeline::VideoData>&,
                const std::vector<math::Box2i>&,
                const std::vector<timeline::ImageOptions>& = {},
                const std::vector<timeline::DisplayOptions>& = {},
                const timeline::CompareOptions& = timeline::CompareOptions(),
                const timeline::BackgroundOptions& =
                    timeline::BackgroundOptions()) override;
            void drawStereo(
                const std::vector<timeline::VideoData>&,
                const std::vector<math::Box2i>&,
                const StereoType = StereoType::kScanlines,
                const float offset = 0.F,
                const std::vector<timeline::ImageOptions>& = {},
                const std::vector<timeline::DisplayOptions>& = {},
                const timeline::CompareOptions& = timeline::CompareOptions());
            void drawAnaglyph(
                const std::vector<timeline::VideoData>&,
                const std::vector<math::Box2i>&,
                const float offset = 0.F,
                const std::vector<timeline::ImageOptions>& = {},
                const std::vector<timeline::DisplayOptions>& = {},
                const timeline::CompareOptions& = timeline::CompareOptions());
            void drawMask(float pct = 0.F);


            // Vulkan overridden functions
            void createBindingSet(const std::string& shaderName) override;
            void beginLoadRenderPass() override;
            void beginRenderPass() override;
            void endRenderPass() override;
            
        private:
            void _displayShader();

            void _drawBackground(
                const std::vector<math::Box2i>&,
                const timeline::BackgroundOptions&);
            void _drawVideoA(
                const std::vector<timeline::VideoData>&,
                const std::vector<math::Box2i>&,
                const std::vector<timeline::ImageOptions>&,
                const std::vector<timeline::DisplayOptions>&,
                const timeline::CompareOptions&);
            void _drawVideoB(
                const std::vector<timeline::VideoData>&,
                const std::vector<math::Box2i>&,
                const std::vector<timeline::ImageOptions>&,
                const std::vector<timeline::DisplayOptions>&,
                const timeline::CompareOptions&);
            void _drawVideoWipe(
                const std::vector<timeline::VideoData>&,
                const std::vector<math::Box2i>&,
                const std::vector<timeline::ImageOptions>&,
                const std::vector<timeline::DisplayOptions>&,
                const timeline::CompareOptions&);
            void _drawVideoOverlay(
                const std::vector<timeline::VideoData>&,
                const std::vector<math::Box2i>&,
                const std::vector<timeline::ImageOptions>&,
                const std::vector<timeline::DisplayOptions>&,
                const timeline::CompareOptions&);
            void _drawVideoDifference(
                const std::vector<timeline::VideoData>&,
                const std::vector<math::Box2i>&,
                const std::vector<timeline::ImageOptions>&,
                const std::vector<timeline::DisplayOptions>&,
                const timeline::CompareOptions&);
            void _drawVideoTile(
                const std::vector<timeline::VideoData>&,
                const std::vector<math::Box2i>&,
                const std::vector<timeline::ImageOptions>&,
                const std::vector<timeline::DisplayOptions>&,
                const timeline::CompareOptions&);
            void _drawVideo(
                std::shared_ptr<vlk::OffscreenBuffer>& fbo,
                const std::string& pipelineName,
                const timeline::VideoData&, const math::Box2i&,
                const std::shared_ptr<timeline::ImageOptions>&,
                const timeline::DisplayOptions&);
            void _create2DMesh(
                const std::string& meshName, const geom::TriangleMesh2& mesh);
            void _createBindingSet(const std::shared_ptr<vlk::Shader>& shaderName);
            VkPipelineLayout _createPipelineLayout(
                const std::string& pipelineLayoutName,
                const std::shared_ptr<vlk::Shader> shader);
            void _bindDescriptorSets(
                const std::string& pipelineLayoutName,
                const std::string& shaderName);
            void _vkDraw(const std::string& meshName);

            void wait_device();
            void wait_queue();
            
#if defined(TLRENDER_LIBPLACEBO)
            void _addTextures(
                std::vector<std::shared_ptr<vlk::Texture> >& textures,
                const pl_shader_res* res);
            std::string _debugPLVar(const struct pl_shader_var& var);
#endif

#if defined(TLRENDER_OCIO)
            void _addTextures(
                std::vector<std::shared_ptr<vlk::Texture >>& textures,
                const OCIO::GpuShaderDescRcPtr& shaderDesc);
#endif
            Fl_Vk_Context& ctx;

            TLRENDER_PRIVATE();
        };
    } // namespace timeline_vlk
} // namespace tl
