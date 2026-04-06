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
#include <tlCore/Monitor.h>

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

            //! Copy textures.
            void copyTextures(
                const std::shared_ptr<image::Image>&,
                const std::vector<std::shared_ptr<vlk::Texture> >&,
                size_t offset = 0);

            //! Proper Vulkan begin function
            void begin(
                VkCommandBuffer& cmd,
                const std::shared_ptr<vlk::OffscreenBuffer> fbo,
                const uint32_t frameIndex, const math::Size2i&,
                const timeline::RenderOptions& = timeline::RenderOptions());
            void end() override;

            Fl_Vk_Context& getContext() const;
            VkCommandBuffer getCommandBuffer() const;
            uint32_t getFrameIndex() const;

            // HDR functions
            void setMonitorCapabilities(const monitor::Capabilities&);
            
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

            //! Overriden begin function
            void begin(
                const math::Size2i&, const timeline::RenderOptions& =
                timeline::RenderOptions()) override;

            math::Size2i getRenderSize() const override;
            void setRenderSize(const math::Size2i&) override;

            //! Gets the internal FBO
            std::shared_ptr<vlk::OffscreenBuffer> getFBO() const;

            //! Gets the current Vulkan render pass.
            VkRenderPass getRenderPass() const;

            //! Changes the current Vulkan render pass.
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
            void setShaderOptions(const timeline::ShaderOptions&);
            void setOCIOOptions(const timeline::OCIOOptions&) override;
            void setLUTOptions(const timeline::LUTOptions&) override;
            void setHDROptions(const timeline::HDROptions&) override;
            
            //! These functions draw to the internal FBO.
            void drawRect(const math::Box2i&, const image::Color4f&,
                          const std::string& shaderName = "") override;
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
            void draw3DMesh(const std::string& pipelineName,
                            const std::string& pipelineLayoutName,
                            const std::string& shaderName,
                            const std::string& meshName,
                            const geom::TriangleMesh3&,
                            const math::Matrix4x4f&,
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

            //! Creates text meshes to speed up drawing
            void appendText(
                std::vector<timeline::TextInfo>& info,
                const std::vector<std::shared_ptr<image::Glyph> >& glyphs,
                const math::Vector2i& position) override;
            
            //! Draws the text meshes.
            void drawText(
                const timeline::TextInfo&,
                const math::Vector2i& position,
                const image::Color4f&) override;

            //! Draws a mesh to the internal FBO.
            void drawMesh(
                const geom::TriangleMesh2&, const math::Vector2i& position,
                const image::Color4f&, const std::string& meshName) override;

            //! NO-OP for Vulkan
            void drawColorMesh(
                const geom::TriangleMesh2&, const math::Vector2i& position,
                const image::Color4f&) override {};

            //! Draws a color mesh to the internal FBO.
            void drawColorMesh(
                const std::string& pipelineLayoutName,
                const geom::TriangleMesh2&, const math::Vector2i& position,
                const image::Color4f&);

            //! NO-OP for Vulkan
            void drawTexture(
                unsigned int, const math::Box2i&,
                const image::Color4f& =
                    image::Color4f(1.F, 1.F, 1.F)) override {};

            //! Draws a texture to the internal FBO.
            void drawTexture(
                const std::shared_ptr<vlk::Texture>&, const math::Box2i&,
                const image::Color4f& = image::Color4f(1.F, 1.F, 1.F));

            //! Draws an image to the internal FBO.
            void drawImage(
                const std::shared_ptr<image::Image>&, const math::Box2i&,
                const image::Color4f& = image::Color4f(1.F, 1.F, 1.F),
                const timeline::ImageOptions& =
                    timeline::ImageOptions()) override;

            //! Draws an image to a provided FBO.
            void drawImage(
                const std::shared_ptr<vlk::OffscreenBuffer>& fbo,
                const std::shared_ptr<image::Image>&, const math::Box2i&,
                const image::Color4f& = image::Color4f(1.F, 1.F, 1.F),
                const timeline::ImageOptions& = timeline::ImageOptions(),
                const bool clearRenderPass = true);

            //! Draws the video data to the internal FBO.
            void drawVideo(
                const std::vector<timeline::VideoData>&,
                const std::vector<math::Box2i>&,
                const std::vector<timeline::ImageOptions>& = {},
                const std::vector<timeline::DisplayOptions>& = {},
                const timeline::CompareOptions& = timeline::CompareOptions(),
                const timeline::BackgroundOptions& =
                    timeline::BackgroundOptions()) override;

            //! Draws the stereo video data to the internal FBO with a
            //! particular method.
            void drawStereo(
                const std::vector<timeline::VideoData>&,
                const std::vector<math::Box2i>&,
                const StereoType = StereoType::kScanlines,
                const float offset = 0.F,
                const std::vector<timeline::ImageOptions>& = {},
                const std::vector<timeline::DisplayOptions>& = {},
                const timeline::CompareOptions& = timeline::CompareOptions());

            //! Draws the stereo video data as an anaglyph to the internal FBO.
            void drawAnaglyph(
                const std::vector<timeline::VideoData>&,
                const std::vector<math::Box2i>&,
                const float offset = 0.F,
                const std::vector<timeline::ImageOptions>& = {},
                const std::vector<timeline::DisplayOptions>& = {},
                const timeline::CompareOptions& = timeline::CompareOptions());

            //! Draws a black mask to the internal FBO.
            void drawMask(float pct = 0.F);

            void drawRectViewport(const std::string& pipelineName,
                                  const math::Box2i&, const image::Color4f&,
                                  const bool enableBlending = true,
                                  const std::string& shaderName = "");
            void drawMeshViewport(const std::string& pipelineName,
                                  const std::string& shaderName,
                                  const std::string& meshName,
                                  const geom::TriangleMesh2&,
                                  const math::Vector2i& position,
                                  const image::Color4f&,
                                  const bool enableBlending = false);

            //! Vulkan render pass functions
            void beginLoadRenderPass() override;
            void beginRenderPass() override;
            void endRenderPass() override;
            void setupViewportAndScissor() override;
            
        private:
            void _displayShader();

            void _emitMeshDraw(const std::string& pipelineLayoutName,
                               const std::string& shaderName,
                               const std::string& meshName,
                               const math::Matrix4x4f& transform,
                               const image::Color4f& color);

            void _setupRectCommon(const math::Box2i& box);
            
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
            void _bindComputeDescriptorSets(
                const std::string& pipelineLayoutName,
                const std::string& shaderName);
            void _vkDraw(const std::string& meshName);

            
#if defined(TLRENDER_LIBPLACEBO)
            void _addTextures(
                std::vector<std::shared_ptr<vlk::Texture> >& textures,
                const pl_shader_res* res);
            std::string _debugPLVar(const struct pl_shader_var& var);

            void _parseVariables(std::stringstream& s,
                                 std::size_t& currentOffset,
                                 const struct pl_shader_res* res,
                                 const size_t pushConstantsMaxSize);

#endif

#if defined(TLRENDER_OCIO)
            std::string
            _getOCIOUniforms(const OCIO::GpuShaderDescRcPtr&);
            void _createOCIOUniforms(const OCIO::GpuShaderDescRcPtr&);
            void _updateOCIOUniforms(const OCIO::GpuShaderDescRcPtr&);
            void _addTextures(
                std::vector<std::shared_ptr<vlk::Texture >>& textures,
                const OCIO::GpuShaderDescRcPtr& shaderDesc);
#endif
            Fl_Vk_Context& ctx;

            TLRENDER_PRIVATE();
        };
    } // namespace timeline_vlk
} // namespace tl
