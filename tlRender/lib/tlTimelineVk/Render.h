// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTimeline/IRender.h>

#include <tlVk/OffscreenBuffer.h>
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

            math::Size2i getRenderSize() const override;
            void setRenderSize(const math::Size2i&) override;
            math::Box2i getViewport() const override;
            void setViewport(const math::Box2i&) override;
            void clearViewport(const image::Color4f&) override;
            bool getClipRectEnabled() const override;
            void setClipRectEnabled(bool) override;
            math::Box2i getClipRect() const override;
            void setClipRect(const math::Box2i&) override;
            math::Matrix4x4f getTransform() const override;
            void setTransform(const math::Matrix4x4f&) override;
            void setOCIOOptions(const timeline::OCIOOptions&) override;
            void setLUTOptions(const timeline::LUTOptions&) override;
            void setHDROptions(const timeline::HDROptions&) override;

            void drawRect(const math::Box2i&, const image::Color4f&) override;
            void drawMesh(
                const geom::TriangleMesh2&, const math::Vector2i& position,
                const image::Color4f&) override;
            void drawColorMesh(
                const geom::TriangleMesh2&, const math::Vector2i& position,
                const image::Color4f&) override {};
            void drawColorMesh(
                const std::string& pipelineLayoutName,
                const geom::TriangleMesh2&, const math::Vector2i& position,
                const image::Color4f&);
            void drawText(
                const std::vector<std::shared_ptr<image::Glyph> >& glyphs,
                const math::Vector2i& position, const image::Color4f&) override;
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
                    timeline::ImageOptions()) override {};
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
                const timeline::VideoData&, const math::Box2i&,
                const std::shared_ptr<timeline::ImageOptions>&,
                const timeline::DisplayOptions&);

            void _createMesh(
                const std::string& meshName, const geom::TriangleMesh2& mesh);
            void _createPipeline(
                const std::shared_ptr<vlk::OffscreenBuffer>& fbo,
                const std::string& pipelineName, const std::string& shaderName,
                const std::string& meshName, const bool enableBlending = false,
                const VkBlendFactor srcColorBlendFactor =
                    VK_BLEND_FACTOR_SRC_ALPHA,
                const VkBlendFactor dstColorBlendFactor =
                    VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
                const VkBlendOp colorBlendOp = VK_BLEND_OP_ADD,
                const VkBlendFactor srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
                const VkBlendFactor dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
                const VkBlendOp alphaBlendOp = VK_BLEND_OP_ADD);
            void _bindDescriptorSets(
                const std::string& pipelineName, const std::string& shaderName);

#if defined(TLRENDER_LIBPLACEBO)
            void _addTextures(
                std::vector<std::shared_ptr<vlk::Texture> >& textures,
                const pl_shader_res* res);
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
