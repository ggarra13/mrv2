// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// Copyright (C) 2025-Present Gonzalo Garramuño.
// All rights reserved.

#include "FL/Fl_Vk_Utils.H"
#include "FL/vk_enum_string_helper.h"

#include "USDRenderStructs.h"
#include "USDRenderPrivate.h"

#include <tlVk/Buffer.h>
#include <tlVk/Vk.h>
#include <tlVk/Mesh.h>
#include <tlVk/Util.h>

#include <tlCore/Assert.h>
#include <tlCore/Context.h>
#include <tlCore/Error.h>
#include <tlCore/Monitor.h>
#include <tlCore/String.h>
#include <tlCore/StringFormat.h>


#include <array>
#include <cstdint>
#include <list>
#include <regex>

#define _USE_MATH_DEFINES
#include <math.h>


namespace tl
{
    namespace usd
    {
        namespace
        {
            const int pboSizeMin = 1024;
        }

        void Render::_init(const std::shared_ptr<system::Context>& context)
        {
            TLRENDER_P();

            p.context = context;
            p.logTimer = std::chrono::steady_clock::now();
        }

        Render::Render(Fl_Vk_Context& context) :
            ctx(context),
            _p(new Private)
        {
            TLRENDER_P();

            for (int i = 0; i < vlk::MAX_FRAMES_IN_FLIGHT; ++i)
            {
                p.garbage[i].pipelines.reserve(20);
                p.garbage[i].pipelineLayouts.reserve(20);
                p.garbage[i].bindingSets.reserve(20);
                p.garbage[i].buffers.reserve(20);
            }
        }

        Render::~Render()
        {
            TLRENDER_P();

            VkDevice device = ctx.device;
            
            for (auto& [_, pipeline] : p.pipelines)
            {
                vkDestroyPipeline(device, pipeline.second, nullptr);
            }
            
            for (auto& [_, pipelineLayout] : p.pipelineLayouts)
            {
                vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
            }
            for (auto& g : p.garbage)
            {
                for (auto& pipeline : g.pipelines)
                {
                    vkDestroyPipeline(device, pipeline, nullptr);
                }
                // Destroy old pipelineLayouts that are no longer used.
                for (auto& pipelineLayout : g.pipelineLayouts)
                {
                    vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
                }
            }
        }

        std::shared_ptr<Render> Render::create(
            Fl_Vk_Context& vulkanContext,
            const std::shared_ptr<system::Context>& context)
        {
            auto out = std::shared_ptr<Render>(new Render(vulkanContext));
            out->_init(context);
            return out;
        }

        void Render::begin(
            VkCommandBuffer& cmd, std::shared_ptr<vlk::OffscreenBuffer> fbo,
            const uint32_t frameIndex, const math::Size2i& renderSize,
            const timeline::RenderOptions& renderOptions)
        {
            TLRENDER_P();

            p.cmd = cmd;
            p.fbo = fbo;
            p.renderPass = fbo->getClearRenderPass();
            p.frameIndex = frameIndex;

#if USE_DYNAMIC_RGBA_WRITE_MASKS
            const VkColorComponentFlags allMask[] =
                { VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                  VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT };
            ctx.vkCmdSetColorWriteMaskEXT(cmd, 0, 1, allMask);
#endif

#if USE_DYNAMIC_STENCILS
            ctx.vkCmdSetStencilTestEnableEXT(cmd, VK_FALSE);
            ctx.vkCmdSetStencilOpEXT(cmd, VK_STENCIL_FACE_FRONT_AND_BACK,
                                     VK_STENCIL_OP_KEEP,
                                     VK_STENCIL_OP_KEEP,
                                     VK_STENCIL_OP_KEEP,
                                     VK_COMPARE_OP_ALWAYS);
            
            vkCmdSetStencilCompareMask(cmd, VK_STENCIL_FACE_FRONT_AND_BACK,
                                       0xFFFFFFFF);
            vkCmdSetStencilWriteMask(cmd, VK_STENCIL_FACE_FRONT_AND_BACK,
                                     0xFFFFFFFF);
#endif
            
            begin(renderSize, renderOptions);
        }

        void Render::begin(
            const math::Size2i& renderSize,
            const timeline::RenderOptions& renderOptions)
        {
            TLRENDER_P();

            p.timer = std::chrono::steady_clock::now();

            p.renderSize = renderSize;
            p.renderOptions = renderOptions;

            VkDevice device = ctx.device;

            // Destroy old pipelines that are no longer used.
            auto& g = p.garbage[p.frameIndex];
            for (auto& pipeline : g.pipelines)
            {
                vkDestroyPipeline(device, pipeline, nullptr);
            }
            // Destroy old pipelineLayouts that are no longer used.
            for (auto& pipelineLayout : g.pipelineLayouts)
            {
                vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
            }
            g.pipelines.clear();
            g.pipelineLayouts.clear();
            g.bindingSets.clear();
            g.buffers.clear();
            
            const image::Color4f color(1.F, 1.F, 1.F);
            USDTransforms transforms;

            if (!p.shaders["dummy"])
            {
                p.shaders["dummy"] = vlk::Shader::create(
                    ctx, vertexDummy(), fragmentDummy(), "dummy");
                p.shaders["dummy"]->createUniform(
                    "transforms", transforms, vlk::kShaderVertex);
                p.shaders["dummy"]->addPush("color", color, vlk::kShaderFragment);
                _createBindingSet(p.shaders["dummy"]);
            }
#if USE_ST_SHADER
            if (!p.shaders["st"])
            {
                p.shaders["st"] = vlk::Shader::create(
                    ctx, vertexSTs(), fragmentSTs(), "st");
                p.shaders["st"]->createUniform(
                    "transforms", transforms, vlk::kShaderVertex);
                p.shaders["st"]->addPush("color", color, vlk::kShaderFragment);
                _createBindingSet(p.shaders["st"]);
            }
#endif
            if (!p.shaders["usd"])
            {
                p.shaders["usd"] = vlk::Shader::create(
                    ctx, vertexUSD(), fragmentUSD(), "usd");
                p.shaders["usd"]->createUniform(
                    "transforms", transforms, vlk::kShaderVertex);
                p.shaders["usd"]->addPush("color", color, vlk::kShaderFragment);
                p.shaders["usd"]->addTexture("u_DiffuseMap");
                p.shaders["usd"]->addTexture("u_EmissiveMap");
                p.shaders["usd"]->addTexture("u_MetallicMap");
                p.shaders["usd"]->addTexture("u_RoughnessMap");
                p.shaders["usd"]->addTexture("u_NormalMap");
                p.shaders["usd"]->addTexture("u_AOMap");
                p.shaders["usd"]->addTexture("u_OpacityMap");
                USDShaderParameters params;
                p.shaders["usd"]->createUniform("params", params);
                USDSceneParameters scene;
                p.shaders["usd"]->createUniform("scene", scene);
                _createBindingSet(p.shaders["usd"]);
            }
            
            //
            // Meshes
            //
            if (renderOptions.clear)
            {
                clearViewport(renderOptions.clearColor);
            }

            setTransform(
                math::ortho(
                    0.F, static_cast<float>(renderSize.w), 0.F,
                    static_cast<float>(renderSize.h), -1.F, 1.F));
        }

        void Render::end()
        {
            TLRENDER_P();

            p.fbo->transitionToShaderRead(p.cmd);

            const auto now = std::chrono::steady_clock::now();
            const auto diff =
                std::chrono::duration_cast<std::chrono::milliseconds>(
                    now - p.timer);
            p.currentStats.time = diff.count();
            p.stats.push_back(p.currentStats);
            p.currentStats = Private::Stats();
            while (p.stats.size() > 60)
            {
                p.stats.pop_front();
            }
            
            const std::chrono::duration<float> logDiff = now - p.logTimer;
            if (logDiff.count() > 10.F)
            {
                p.logTimer = now;
                if (auto context = p.context.lock())
                {
                    Private::Stats average;
                    const size_t size = p.stats.size();
                    if (size > 0)
                    {
                        for (const auto& i : p.stats)
                        {
                            average.time += i.time;
                            average.rects += i.rects;
                            average.meshes += i.meshes;
                            average.meshTriangles += i.meshTriangles;
                            average.text += i.text;
                            average.textTriangles += i.textTriangles;
                            average.textures += i.textures;
                            average.images += i.images;
                            average.pipelineChanges += i.pipelineChanges;
                        }
                        average.time /= p.stats.size();
                        average.rects /= p.stats.size();
                        average.meshes /= p.stats.size();
                        average.meshTriangles /= p.stats.size();
                        average.text /= p.stats.size();
                        average.textTriangles /= p.stats.size();
                        average.textures /= p.stats.size();
                        average.images /= p.stats.size();
                        average.pipelineChanges /= p.stats.size();
                    }

                    context->log(
                        string::Format("tl::usd::Render {0}").arg(this),
                        string::Format(
                            "\n"
                            "    Average render time: {0}ms\n"
                            "    Average rectangle count: {1}\n"
                            "    Average mesh count: {2}\n"
                            "    Average mesh triangles: {3}\n"
                            "    Average text count: {4}\n"
                            "    Average text triangles: {5}\n"
                            "    Average texture count: {6}\n"
                            "    Average image count: {7}\n"
                            "    Average pipeline changes: {8}\n"
                            "    Glyph texture atlas: {9}%\n"
                            "    Glyph IDs: {10}")
                            .arg(average.time)
                            .arg(average.rects)
                            .arg(average.meshes)
                            .arg(average.meshTriangles)
                            .arg(average.text)
                            .arg(average.textTriangles)
                            .arg(average.textures)
                            .arg(average.images)
                            .arg(average.pipelineChanges));
                }
            }
        }

        VkCommandBuffer Render::getCommandBuffer() const
        {
            return _p->cmd;
        }

        uint32_t Render::getFrameIndex() const
        {
            return _p->frameIndex;
        }
        
        Fl_Vk_Context& Render::getContext() const
        {
            return ctx;
        }
        
        math::Size2i Render::getRenderSize() const
        {
            return _p->renderSize;
        }

        void Render::setRenderSize(const math::Size2i& value)
        {
            _p->renderSize = value;
        }

        math::Box2i Render::getViewport() const
        {
            return _p->viewport;
        }

        void Render::setViewport(const math::Box2i& value)
        {
            _p->viewport = value;
        }
        
        void Render::beginLoadRenderPass()
        {
            TLRENDER_P();

            p.fbo->transitionToColorAttachment(p.cmd);
            p.fbo->beginLoadRenderPass(p.cmd);
        }
        
        void Render::beginRenderPass()
        {
            TLRENDER_P();

            p.fbo->transitionToColorAttachment(p.cmd);
            p.fbo->beginClearRenderPass(p.cmd);
        }

        void Render::endRenderPass()
        {
            TLRENDER_P();
            
            p.fbo->endRenderPass(p.cmd);
        }
        
        void Render::setupViewportAndScissor()
        {
            TLRENDER_P();
            
            p.fbo->setupViewportAndScissor();
        }
        
        void Render::clearViewport(const image::Color4f& value)
        {
            TLRENDER_P();

            std::vector<VkClearValue> clearValues;
            
            // Color clear (always for attachment 0)
            VkClearValue colorClear = {};
            colorClear.color = {{value.r, value.g, value.b, value.a}};
            clearValues.push_back(colorClear);
            
            bool multisampled = (p.fbo->getSampleCount() != VK_SAMPLE_COUNT_1_BIT);
            
            if (multisampled)
            {
                // Resolve attachment (index 1) uses DONT_CARE → value ignored, but slot required
                clearValues.push_back({});  // dummy
            }

            if (p.fbo->hasDepth() || p.fbo->hasStencil())
            {
                // Depth/stencil clear (attachment index 2 when MS, or 1 when not)
                VkClearValue depthClear = {};
                depthClear.depthStencil = {1.0f, 0};   // depth=1.0, stencil=0
                clearValues.push_back(depthClear);
            }

            VkRenderPassBeginInfo rpBegin{};
            rpBegin.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            rpBegin.renderPass = p.fbo->getClearRenderPass();
            rpBegin.framebuffer = p.fbo->getFramebuffer();
            rpBegin.renderArea.offset = {0, 0};
            rpBegin.renderArea.extent = p.fbo->getExtent(); // Use FBO extent
            rpBegin.clearValueCount = clearValues.size();
            rpBegin.pClearValues = clearValues.data();

            // Begin the first render pass instance within the single command
            // buffer
            vkCmdBeginRenderPass(p.cmd, &rpBegin, VK_SUBPASS_CONTENTS_INLINE);
            vkCmdEndRenderPass(p.cmd);
            
            // Update C++ layout tracking to match render pass
            //finalLayout
            p.fbo->setImageLayout(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
        }

        bool Render::getClipRectEnabled() const
        {
            return _p->clipRectEnabled;
        }

        void Render::setClipRectEnabled(bool value)
        {
            _p->clipRectEnabled = value;
        }

        math::Box2i Render::getClipRect() const
        {
            return _p->clipRect;
        }

        std::shared_ptr<vlk::OffscreenBuffer> Render::getFBO() const
        {
            return _p->fbo;
        }

        void Render::setClipRect(const math::Box2i& value)
        {
            TLRENDER_P();
            p.clipRect = value;
            if (p.clipRectEnabled && p.clipRect.w() > 0 && p.clipRect.h() > 0)
            {
                VkRect2D scissorRect = {
                    p.clipRect.x(),
                    p.clipRect.y(),
                    static_cast<uint32_t>(p.clipRect.w()),
                    static_cast<uint32_t>(p.clipRect.h())
                };
                vkCmdSetScissor(p.cmd, 0, 1, &scissorRect);
            }
        }

        math::Matrix4x4f Render::getTransform() const
        {
            return _p->transform;
        }

        void Render::setCameraWorldPosition(const math::Vector3f& value)
        {
            _p->cameraPosition = value;
        }
        
        void Render::setTransform(const math::Matrix4x4f& value)
        {
            _p->transform = value;
        }
        
        void Render::applyTransforms()
        {
            TLRENDER_P();
            
            for (auto& i : p.shaders)
            {
                if (i.second)
                {
                    i.second->bind(p.frameIndex);
                    USDTransforms transforms;
                    transforms.mvp = transforms.model = p.transform;
                    i.second->setUniform("transforms", transforms,
                                         vlk::kShaderVertex);
                }
            }
        }        

        
    } // namespace usd
} // namespace tl
