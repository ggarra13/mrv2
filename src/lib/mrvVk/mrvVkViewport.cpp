// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <cinttypes>

#include <tlCore/FontSystem.h>
#include <tlCore/StringFormat.h>

#include <tlDevice/IOutput.h>

#include <tlVk/OffscreenBuffer.h>
#include <tlTimelineVk/RenderPrivate.h>
#include <tlVk/Init.h>
#include <tlVk/Util.h>

// mrViewer .fl includes
#include "mrViewer.h"
#include "mrvHotkeyUI.h"

#include "mrvCore/mrvColorSpaces.h"
#include "mrvCore/mrvLocale.h"
#include "mrvCore/mrvSequence.h"
#include "mrvCore/mrvI8N.h"

#include "mrvWidgets/mrvMultilineInput.h"

#include "mrvFl/mrvOCIO.h"
#include "mrvFl/mrvIO.h"
#include "mrvFl/mrvTimelinePlayer.h"

#include "mrvUI/mrvDesktop.h"

#include "mrvVk/mrvVkViewportPrivate.h"
#include "mrvVk/mrvVkDefines.h"
// #include "mrvVk/mrvVkErrors.h"
#include "mrvVk/mrvVkUtil.h"
#include "mrvVk/mrvVkShaders.h"
#include "mrvVk/mrvVkShape.h"
#include "mrvVk/mrvTimelineViewport.h"
#include "mrvVk/mrvTimelineViewportPrivate.h"
#include "mrvVk/mrvVkViewport.h"

#include "mrvPanels/mrvPanelsCallbacks.h"

#include "mrvNetwork/mrvDummyClient.h"

#include "mrvApp/mrvSettingsObject.h"

namespace
{
    const char* kModule = "view";
}

namespace mrv
{
    using namespace tl;

    namespace vulkan
    {

        Viewport::Viewport(int X, int Y, int W, int H, const char* L) :
            TimelineViewport(X, Y, W, H, L),
            _vk(new VKPrivate)
        {
            int stereo = 0;
            int fl_double = FL_DOUBLE; // needed on Linux and _WIN32
                                       // #if defined(__APPLE__)
            //             // \bug: Do not use FL_DOUBLE on APPLE as it makes
            //             // playback slow
            //             fl_double = 0;
            // #endif
            mode(FL_RGB | fl_double | FL_ALPHA | FL_STENCIL | stereo);
        }

        Viewport::~Viewport() {}

        void Viewport::setContext(const std::weak_ptr<system::Context>& context)
        {
            _vk->context = context;
        }

        int Viewport::log_level() const
        {
            return 4;
        }

        void Viewport::prepare_descriptor_layout()
        {
            MRV2_VK();

            VkResult result;

            VkPipelineLayoutCreateInfo pPipelineLayoutCreateInfo = {};
            pPipelineLayoutCreateInfo.sType =
                VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
            pPipelineLayoutCreateInfo.pNext = NULL;
            pPipelineLayoutCreateInfo.setLayoutCount = 1;
            pPipelineLayoutCreateInfo.pSetLayouts =
                &vk.shader->getDescriptorSetLayout();

            result = vkCreatePipelineLayout(
                device(), &pPipelineLayoutCreateInfo, NULL,
                &vk.pipeline_layout);
        }

        void Viewport::prepare_pipeline()
        {
            MRV2_VK();

            if (m_pipeline != VK_NULL_HANDLE)
            {
                vkDestroyPipeline(device(), m_pipeline, nullptr);
            }

            VkGraphicsPipelineCreateInfo pipeline;

            VkPipelineVertexInputStateCreateInfo vi = {};
            VkPipelineInputAssemblyStateCreateInfo ia = {};
            VkPipelineRasterizationStateCreateInfo rs = {};
            VkPipelineColorBlendStateCreateInfo cb = {};
            VkPipelineDepthStencilStateCreateInfo ds = {};
            VkPipelineViewportStateCreateInfo vp = {};
            VkPipelineMultisampleStateCreateInfo ms = {};
            VkDynamicState dynamicStateEnables[(
                VK_DYNAMIC_STATE_STENCIL_REFERENCE - VK_DYNAMIC_STATE_VIEWPORT +
                1)];
            VkPipelineDynamicStateCreateInfo dynamicState = {};

            VkResult result;

            memset(dynamicStateEnables, 0, sizeof dynamicStateEnables);
            memset(&dynamicState, 0, sizeof dynamicState);
            dynamicState.sType =
                VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
            dynamicState.pDynamicStates = dynamicStateEnables;

            memset(&pipeline, 0, sizeof(pipeline));
            pipeline.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
            pipeline.layout =
                vk.pipeline_layout; // Use the main pipeline layout

            vi.sType =
                VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
            vi.pNext = NULL;
            vi.vertexBindingDescriptionCount =
                vk.vbo->getBindingDescription().size();
            vi.pVertexBindingDescriptions =
                vk.vbo->getBindingDescription().data();
            vi.vertexAttributeDescriptionCount = vk.vbo->getAttributes().size();
            vi.pVertexAttributeDescriptions = vk.vbo->getAttributes().data();

            memset(&ia, 0, sizeof(ia));
            ia.sType =
                VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
            ia.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

            memset(&rs, 0, sizeof(rs));
            rs.sType =
                VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
            rs.polygonMode = VK_POLYGON_MODE_FILL;
            rs.cullMode = VK_CULL_MODE_BACK_BIT;
            rs.frontFace = VK_FRONT_FACE_CLOCKWISE;
            rs.depthClampEnable = VK_FALSE;
            rs.rasterizerDiscardEnable = VK_FALSE;
            rs.depthBiasEnable = VK_FALSE;
            rs.lineWidth = 1.0f;

            memset(&cb, 0, sizeof(cb));
            cb.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
            VkPipelineColorBlendAttachmentState att_state[1];
            memset(att_state, 0, sizeof(att_state));
            att_state[0].colorWriteMask = 0xf;
            att_state[0].blendEnable = VK_FALSE;
            cb.attachmentCount = 1;
            cb.pAttachments = att_state;

            memset(&vp, 0, sizeof(vp));
            vp.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
            vp.viewportCount = 1;
            dynamicStateEnables[dynamicState.dynamicStateCount++] =
                VK_DYNAMIC_STATE_VIEWPORT;
            vp.scissorCount = 1;
            dynamicStateEnables[dynamicState.dynamicStateCount++] =
                VK_DYNAMIC_STATE_SCISSOR;

            bool has_depth = mode() & FL_DEPTH;     // Check window depth
            bool has_stencil = mode() & FL_STENCIL; // Check window stencil

            memset(&ds, 0, sizeof(ds));
            ds.sType =
                VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
            ds.depthTestEnable = has_depth ? VK_TRUE : VK_FALSE;
            ds.depthWriteEnable = has_depth ? VK_TRUE : VK_FALSE;
            ds.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
            ds.depthBoundsTestEnable = VK_FALSE;
            ds.stencilTestEnable = has_stencil ? VK_TRUE : VK_FALSE;
            ds.back.failOp = VK_STENCIL_OP_KEEP;
            ds.back.passOp = VK_STENCIL_OP_KEEP;
            ds.back.compareOp = VK_COMPARE_OP_ALWAYS;
            ds.front = ds.back;

            memset(&ms, 0, sizeof(ms));
            ms.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
            ms.pSampleMask = NULL;
            ms.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

            // Two stages: vs and fs
            pipeline.stageCount = 2;
            VkPipelineShaderStageCreateInfo shaderStages[2];
            memset(
                &shaderStages, 0, 2 * sizeof(VkPipelineShaderStageCreateInfo));

            shaderStages[0].sType =
                VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
            shaderStages[0].module =
                vk.shader->getVertex(); // Use main vertex shader
            shaderStages[0].pName = "main";

            shaderStages[1].sType =
                VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
            shaderStages[1].module =
                vk.shader->getFragment(); // Use main fragment shader
            shaderStages[1].pName = "main";

            pipeline.pVertexInputState = &vi;
            pipeline.pInputAssemblyState = &ia;
            pipeline.pRasterizationState = &rs;
            pipeline.pColorBlendState = &cb;
            pipeline.pMultisampleState = &ms;
            pipeline.pViewportState = &vp;
            pipeline.pDepthStencilState = &ds;
            pipeline.pStages = shaderStages;
            pipeline.renderPass =
                m_renderPass; // Use main render pass (swapchain)
            pipeline.pDynamicState = &dynamicState;

            // Create a temporary pipeline cache
            VkPipelineCacheCreateInfo pipelineCacheCreateInfo{};
            pipelineCacheCreateInfo.sType =
                VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
            VkPipelineCache pipelineCache;
            result = vkCreatePipelineCache(
                device(), &pipelineCacheCreateInfo, NULL, &pipelineCache);
            VK_CHECK(result);

            result = vkCreateGraphicsPipelines(
                device(), pipelineCache, 1, &pipeline, NULL, &m_pipeline);
            VK_CHECK(result);

            // Destroy the temporary pipeline cache
            vkDestroyPipelineCache(device(), pipelineCache, NULL);
        }

        void Viewport::prepare()
        {
            _initializeVK();

            prepare_render_pass();       // Main swapchain render pass
            prepare_descriptor_layout(); // Main shader layout
        }

        void Viewport::destroy_resources()
        {
            MRV2_VK();

            refresh();

            if (vk.pipeline_layout != VK_NULL_HANDLE)
            {
                vkDestroyPipelineLayout(device(), vk.pipeline_layout, nullptr);
                vk.pipeline_layout = VK_NULL_HANDLE;
            }

            if (m_pipeline != VK_NULL_HANDLE)
            {
                vkDestroyPipeline(device(), m_pipeline, nullptr);
                m_pipeline = VK_NULL_HANDLE;
            }

            VkWindow::destroy_resources();
        }

        //! Refresh window by clearing the associated resources.
        void Viewport::refresh()
        {
            TLRENDER_P();
            MRV2_VK();

            wait_device();

            vk.render.reset();
            vk.lines.reset();
#ifdef USE_ONE_PIXEL_LINES
            vk.outline.reset();
#endif
            vk.buffer.reset();
            vk.annotation.reset();
            vk.overlay.reset();
            vk.shader.reset();
            vk.annotationShader.reset();
            vk.vbo.reset();
            vk.vao.reset();
            p.fontSystem.reset();
            vk.currentPBOIndex = 0;
            vk.nextPBOIndex = 1;
        }

        void Viewport::_initializeVKResources()
        {
            TLRENDER_P();
            MRV2_VK();

            if (auto context = vk.context.lock())
            {

                vk.render = timeline_vlk::Render::create(ctx, context);
                p.fontSystem = image::FontSystem::create(context);

#ifdef USE_ONE_PIXEL_LINES
                vk.outline = std::make_shared<vulkan::Outline>();
#endif

                vk.lines = std::make_shared<vulkan::Lines>();

                try
                {
                    const std::string& vertexSource =
                        timeline_vlk::vertexSource();
                    vk.shader = vlk::Shader::create(
                        ctx, vertexSource, textureFragmentSource(),
                        "composite");

                    // Create parameters for shader.
                    math::Matrix4x4f mvp;
                    vk.shader->createUniform(
                        "transform.mvp", mvp, vlk::kShaderVertex);
                    vk.shader->addFBO("textureSampler"); // default is fragment
                    float opacity = 1.0;
                    vk.shader->createUniform("opacity", opacity);
                    vk.shader->createDescriptorSets();

                    vk.annotationShader = vlk::Shader::create(
                        ctx, vertexSource, annotationFragmentSource(),
                        "annotation");
                    vk.annotationShader->createUniform(
                        "transform.mvp", mvp, vlk::kShaderVertex);
                    int channels = 0; // Color
                    vk.annotationShader->createUniform("channels", channels);
                    vk.annotationShader->createDescriptorSets();
                }
                catch (const std::exception& e)
                {
                    LOG_ERROR(e.what());
                }
            }
        }

        void Viewport::_initializeVK()
        {
            MRV2_VK();

            refresh();

            _initializeVKResources();
        }

        int Viewport::handle(int event)
        {
            return TimelineViewport::handle(event);
        }

        void Viewport::vk_draw_begin()
        {
            m_clearColor = {1.F, 0.F, 0.F, 0.F};
            m_depthStencil = 1.0;

            VkWindow::vk_draw_begin();
        }

        void Viewport::draw()
        {
            TLRENDER_P();
            MRV2_VK();

            // Get the command buffer started for the current frame.
            VkCommandBuffer cmd = getCurrentCommandBuffer();
            vkCmdEndRenderPass(cmd);

            vk.cmd = cmd;

            const auto& viewportSize = getViewportSize();
            const auto& renderSize = getRenderSize();

            bool hasAlpha = false;
            const float alpha = p.ui->uiMain->get_alpha() / 255.F;
            if (alpha < 1.0F)
            {
                hasAlpha = true;
            }

            const bool transparent =
                hasAlpha || getBackgroundOptions().type ==
                                timeline::Background::Transparent;

            if (renderSize.isValid())
            {

                vk.colorBufferType = image::PixelType::RGBA_U8;

                int accuracy = p.ui->uiPrefs->uiPrefsColorAccuracy->value();
                switch (accuracy)
                {
                case kAccuracyFloat32:
                    vk.colorBufferType = image::PixelType::RGBA_F32;
                    hasAlpha = true;
                    break;
                case kAccuracyFloat16:
                    vk.colorBufferType = image::PixelType::RGBA_F16;
                    hasAlpha = true;
                    break;
                case kAccuracyAuto:
                    image::PixelType pixelType = image::PixelType::RGBA_U8;
                    auto& video = p.videoData[0];
                    if (p.missingFrame &&
                        p.missingFrameType != MissingFrameType::kBlackFrame)
                    {
                        video = p.lastVideoData;
                    }

                    if (!video.layers.empty() && video.layers[0].image &&
                        video.layers[0].image->isValid())
                    {
                        pixelType = video.layers[0].image->getPixelType();
                        switch (pixelType)
                        {
                        case image::PixelType::RGBA_F32:
                        case image::PixelType::LA_F32:
                            hasAlpha = true;
                        case image::PixelType::RGB_F32:
                        case image::PixelType::L_F32:
                            vk.colorBufferType = image::PixelType::RGBA_F32;
                            break;
                        case image::PixelType::RGBA_F16:
                        case image::PixelType::LA_F16:
                            hasAlpha = true;
                        case image::PixelType::RGB_F16:
                        case image::PixelType::L_F16:
                            vk.colorBufferType = image::PixelType::RGBA_F16;
                            break;
                        case image::PixelType::RGBA_U16:
                        case image::PixelType::LA_U16:
                            hasAlpha = true;
                        case image::PixelType::RGB_U16:
                        case image::PixelType::L_U16:
                            vk.colorBufferType = image::PixelType::RGBA_U16;
                            break;
                        case image::PixelType::RGBA_U8:
                        case image::PixelType::LA_U8:
                            hasAlpha = true;
                            break;
                        default:
                            break;
                        }
                    }
                    break;
                }

                vlk::OffscreenBufferOptions offscreenBufferOptions;
                offscreenBufferOptions.colorType = vk.colorBufferType;

                if (!p.displayOptions.empty())
                {
                    offscreenBufferOptions.colorFilters =
                        p.displayOptions[0].imageFilters;
                }
                offscreenBufferOptions.depth = vlk::OffscreenDepth::_24;
                offscreenBufferOptions.stencil = vlk::OffscreenStencil::_8;
                offscreenBufferOptions.allowCompositing = true;

                if (vlk::doCreate(
                        vk.buffer, renderSize, offscreenBufferOptions))
                {
                    vk.buffer = vlk::OffscreenBuffer::create(
                        ctx, renderSize, offscreenBufferOptions);
                    // As render resolution might have changed,
                    // we need to reset the quad size.
                    vk.vbo.reset();
                }
            }
            else
            {
                vk.buffer.reset();
                vk.stereoBuffer.reset();
            }

            if (!vk.buffer)
            {
                // Must begin a dummy render pass, as vk_end_draw()
                // will call vkCmdEndRenderPass.
                begin_render_pass();
                return;
            }

            if (p.pixelAspectRatio > 0.F && !p.videoData.empty() &&
                !p.videoData[0].layers.empty())
            {
                auto image = p.videoData[0].layers[0].image;
                p.videoData[0].size.pixelAspectRatio = p.pixelAspectRatio;
                image->setPixelAspectRatio(p.pixelAspectRatio);
            }

            locale::SetAndRestore saved;
            timeline::RenderOptions renderOptions;
            renderOptions.colorBuffer = vk.colorBufferType;
            renderOptions.clear = true;
            renderOptions.clearColor = image::Color4f(0, 0, 1, 0);

            vk.render->begin(
                cmd, vk.buffer, m_currentFrameIndex, renderSize, renderOptions);
            vk.render->drawVideo(
                p.videoData,
                timeline::getBoxes(p.compareOptions.mode, p.videoData),
                p.imageOptions, p.displayOptions, p.compareOptions,
                getBackgroundOptions());
            vk.render->end();

            vk.buffer->transitionToShaderRead(cmd);

            math::Matrix4x4f mvp;

            const float rotation = _getRotation();

            mvp = _createTexturedRectangle();

            // --- Final Render Pass: Render to Swapchain (Composition) ---
            begin_render_pass();

            // Bind the shaders to the current frame index.
            vk.shader->bind(m_currentFrameIndex);
            vk.annotationShader->bind(m_currentFrameIndex);

            // Bind the main composition pipeline (created/managed outside this
            // draw loop)
            vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline);

            // --- Update Descriptor Set for the SECOND pass (Composition) ---
            // This updates the descriptor set for the CURRENT frame index on
            // the CPU.
            vk.shader->setUniform("transform.mvp", mvp, vlk::kShaderVertex);
            vk.shader->setFBO("textureSampler", vk.buffer);
#ifdef __APPLE__
            vk.shader->setUniform("opacity", 1.0F);
            set_window_transparency(alpha);
#else
            if (desktop::Wayland())
                vk.shader->setUniform("opacity", alpha);
            else if (desktop::X11() || desktop::Windows())
                vk.shader->setUniform("opacity", 1.0F);
#endif

            // --- Bind Descriptor Set for the SECOND pass ---
            // Record the command to bind the descriptor set for the CURRENT
            // frame index
            vkCmdBindDescriptorSets(
                cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, vk.pipeline_layout, 0, 1,
                &vk.shader->getDescriptorSet(),
                // Bind the set for THIS frame
                0, nullptr);

            VkViewport viewport = {};
            viewport.width = static_cast<float>(w());
            viewport.height = static_cast<float>(h());
            viewport.minDepth = 0.0f;
            viewport.maxDepth = 1.0f;
            vkCmdSetViewport(cmd, 0, 1, &viewport);

            VkRect2D scissor = {};
            scissor.extent.width = w();
            scissor.extent.height = h();
            vkCmdSetScissor(cmd, 0, 1, &scissor);

            if (vk.vao && vk.vbo)
            {
                // Draw calls for the composition geometry (e.g., a
                // screen-filling quad)
                vk.vao->bind(m_currentFrameIndex);
                vk.vao->draw(cmd, vk.vbo);
            }

            end_render_pass();

            // Correctly transition from SHADER_READ to COLOR_ATTACHMENT
            vk.buffer->transitionToColorAttachment(cmd);

            // Draw FLTK children
            // Fl_Window::draw();
        }

        void Viewport::_calculateColorAreaFullValues(area::Info& info) noexcept
        {
            TLRENDER_P();
            MRV2_VK();

            PixelToolBarClass* c = p.ui->uiPixelWindow;
            BrightnessType brightness_type =
                (BrightnessType)c->uiLType->value();
            int hsv_colorspace = c->uiBColorType->value() + 1;

            const int maxX = info.box.max.x;
            const int maxY = info.box.max.y;
            const auto& renderSize = vk.buffer->getSize();

            for (int Y = info.box.y(); Y <= maxY; ++Y)
            {
                for (int X = info.box.x(); X <= maxX; ++X)
                {
                    image::Color4f rgba, hsv;
                    rgba.b = p.image[(X + Y * renderSize.w) * 4];
                    rgba.g = p.image[(X + Y * renderSize.w) * 4 + 1];
                    rgba.r = p.image[(X + Y * renderSize.w) * 4 + 2];
                    rgba.a = p.image[(X + Y * renderSize.w) * 4 + 3];

                    info.rgba.mean.r += rgba.r;
                    info.rgba.mean.g += rgba.g;
                    info.rgba.mean.b += rgba.b;
                    info.rgba.mean.a += rgba.a;

                    if (rgba.r < info.rgba.min.r)
                        info.rgba.min.r = rgba.r;
                    if (rgba.g < info.rgba.min.g)
                        info.rgba.min.g = rgba.g;
                    if (rgba.b < info.rgba.min.b)
                        info.rgba.min.b = rgba.b;
                    if (rgba.a < info.rgba.min.a)
                        info.rgba.min.a = rgba.a;

                    if (rgba.r > info.rgba.max.r)
                        info.rgba.max.r = rgba.r;
                    if (rgba.g > info.rgba.max.g)
                        info.rgba.max.g = rgba.g;
                    if (rgba.b > info.rgba.max.b)
                        info.rgba.max.b = rgba.b;
                    if (rgba.a > info.rgba.max.a)
                        info.rgba.max.a = rgba.a;

                    hsv = rgba_to_hsv(hsv_colorspace, rgba);
                    hsv.a = calculate_brightness(rgba, brightness_type);
                    hsv_to_info(hsv, info);
                }
            }

            int num = info.box.w() * info.box.h();
            info.rgba.mean.r /= num;
            info.rgba.mean.g /= num;
            info.rgba.mean.b /= num;
            info.rgba.mean.a /= num;

            info.rgba.diff.r = info.rgba.max.r - info.rgba.min.r;
            info.rgba.diff.g = info.rgba.max.g - info.rgba.min.g;
            info.rgba.diff.b = info.rgba.max.b - info.rgba.min.b;
            info.rgba.diff.a = info.rgba.max.a - info.rgba.min.a;

            info.hsv.mean.r /= num;
            info.hsv.mean.g /= num;
            info.hsv.mean.b /= num;
            info.hsv.mean.a /= num;

            info.hsv.diff.r = info.hsv.max.r - info.hsv.min.r;
            info.hsv.diff.g = info.hsv.max.g - info.hsv.min.g;
            info.hsv.diff.b = info.hsv.max.b - info.hsv.min.b;
            info.hsv.diff.a = info.hsv.max.a - info.hsv.min.a;
        }

        void Viewport::_calculateColorArea(area::Info& info)
        {
            TLRENDER_P();
            MRV2_VK();

            if (!p.image || !vk.buffer)
                return;

            info.rgba.max.r = std::numeric_limits<float>::min();
            info.rgba.max.g = std::numeric_limits<float>::min();
            info.rgba.max.b = std::numeric_limits<float>::min();
            info.rgba.max.a = std::numeric_limits<float>::min();

            info.rgba.min.r = std::numeric_limits<float>::max();
            info.rgba.min.g = std::numeric_limits<float>::max();
            info.rgba.min.b = std::numeric_limits<float>::max();
            info.rgba.min.a = std::numeric_limits<float>::max();

            info.rgba.mean.r = info.rgba.mean.g = info.rgba.mean.b =
                info.rgba.mean.a = 0.F;

            info.hsv.max.r = std::numeric_limits<float>::min();
            info.hsv.max.g = std::numeric_limits<float>::min();
            info.hsv.max.b = std::numeric_limits<float>::min();
            info.hsv.max.a = std::numeric_limits<float>::min();

            info.hsv.min.r = std::numeric_limits<float>::max();
            info.hsv.min.g = std::numeric_limits<float>::max();
            info.hsv.min.b = std::numeric_limits<float>::max();
            info.hsv.min.a = std::numeric_limits<float>::max();

            info.hsv.mean.r = info.hsv.mean.g = info.hsv.mean.b =
                info.hsv.mean.a = 0.F;

            if (p.ui->uiPixelWindow->uiPixelValue->value() == PixelValue::kFull)
                _calculateColorAreaFullValues(info);
            else
                _calculateColorAreaRawValues(info);
        }

        void Viewport::_mapBuffer() noexcept
        {
            MRV2_VK();
            TLRENDER_P();

            if (p.ui->uiPixelWindow->uiPixelValue->value() == PixelValue::kFull)
            {

                // For faster access, we muse use BGRA.
                // constexpr VKenum format = GL_BGRA;
                // constexpr VKenum type = GL_FLOAT;

                // vlk::OffscreenBufferBinding binding(vk.buffer);
                const auto& renderSize = vk.buffer->getSize();

                // bool update = _shouldUpdatePixelBar();
                bool stopped = _isPlaybackStopped();
                bool single_frame = _isSingleFrame();

                // set the target framebuffer to read
                // "index" is used to read pixels from framebuffer to a PBO
                // "nextIndex" is used to update pixels in the other PBO
                vk.currentPBOIndex = (vk.currentPBOIndex + 1) % 2;
                vk.nextPBOIndex = (vk.currentPBOIndex + 1) % 2;
            }
            else
            {
                TimelineViewport::_mapBuffer();
            }
        }

        void Viewport::_unmapBuffer() noexcept
        {
            MRV2_VK();
            TLRENDER_P();

            if (p.image)
            {
                if (!p.rawImage)
                {
                    // glUnmapBuffer(GL_PIXEL_PACK_BUFFER);

                    // // Create a new fence
                    // vk.pboFences[vk.nextPBOIndex] =
                    //     glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
                    p.image = nullptr;
                    p.rawImage = true;
                }
            }
        }

        void Viewport::_readPixel(image::Color4f& rgba)
        {

            TLRENDER_P();
            MRV2_VK();

            math::Vector2i pos = _getRaster();

            if (p.ui->uiPixelWindow->uiPixelValue->value() != PixelValue::kFull)
            {
                if (_isEnvironmentMap())
                    return;

                rgba.r = rgba.g = rgba.b = rgba.a = 0.f;

                for (const auto& video : p.videoData)
                {
                    for (const auto& layer : video.layers)
                    {
                        const auto& image = layer.image;
                        if (!image->isValid())
                            continue;

                        image::Color4f pixel, pixelB;

                        _getPixelValue(pixel, image, pos);

                        const auto& imageB = layer.image;
                        if (imageB->isValid())
                        {
                            _getPixelValue(pixelB, imageB, pos);

                            if (layer.transition ==
                                timeline::Transition::Dissolve)
                            {
                                float f2 = layer.transitionValue;
                                float f = 1.0 - f2;
                                pixel.r = pixel.r * f + pixelB.r * f2;
                                pixel.g = pixel.g * f + pixelB.g * f2;
                                pixel.b = pixel.b * f + pixelB.b * f2;
                                pixel.a = pixel.a * f + pixelB.a * f2;
                            }
                        }
                        rgba.r += pixel.r;
                        rgba.g += pixel.g;
                        rgba.b += pixel.b;
                        rgba.a += pixel.a;
                    }
                }
            }
            else
            {
                // This is needed as the FL_MOVE of fltk wouuld get called
                // before the draw routine
                if (!vk.buffer || !valid())
                {
                    return;
                }

                // glPixelStorei(GL_PACK_ALIGNMENT, 1);
                // glPixelStorei(GL_PACK_SWAP_BYTES, GL_FALSE);

                // We use ReadPixels when the movie is stopped or has only a
                // a single frame.
                bool update = _shouldUpdatePixelBar();

                if (_isEnvironmentMap())
                {
                    update = true;
                }

                // const VKenum type = GL_FLOAT;

                if (update)
                {
                    _unmapBuffer();
                    if (_isEnvironmentMap())
                    {
                        pos = _getFocus();
                        // glBindFramebuffer(GL_FRAMEBUFFER, 0);
                        // glReadBuffer(GL_FRONT);
                        // glReadPixels(pos.x, pos.y, 1, 1, GL_RGBA, type,
                        // &rgba);
                        return;
                    }
                    else
                    {
                        // MyViewport* self = const_cast<Viewport*>(this);
                        // self->make_current();
                        // vlk::OffscreenBufferBinding binding(vk.buffer);
                        // glReadPixels(pos.x, pos.y, 1, 1, GL_RGBA, type,
                        // &rgba);
                        return;
                    }
                }

                if (!p.image)
                    _mapBuffer();

                if (p.image)
                {
                    const auto& renderSize = vk.buffer->getSize();
                    rgba.b = p.image[(pos.x + pos.y * renderSize.w) * 4];
                    rgba.g = p.image[(pos.x + pos.y * renderSize.w) * 4 + 1];
                    rgba.r = p.image[(pos.x + pos.y * renderSize.w) * 4 + 2];
                    rgba.a = p.image[(pos.x + pos.y * renderSize.w) * 4 + 3];
                }
            }

            _unmapBuffer();
        }

        void Viewport::_pushAnnotationShape(const std::string& command) const
        {
            // We should not update tcp client when not needed
            if (dynamic_cast< DummyClient* >(tcp))
                return;

            bool send = _p->ui->uiPrefs->SendAnnotations->value();
            if (!send)
                return;

            const auto player = getTimelinePlayer();
            if (!player)
                return;

            auto annotation = player->getAnnotation();
            if (!annotation)
                return;
            auto shape = annotation->lastShape().get();
            if (!shape)
                return;

            Message value;
            if (dynamic_cast< VKArrowShape* >(shape))
            {
                auto s = dynamic_cast< VKArrowShape* >(shape);
                value = *s;
            }
            else if (dynamic_cast< VKFilledRectangleShape* >(shape))
            {
                auto s = dynamic_cast< VKFilledRectangleShape* >(shape);
                value = *s;
            }
            else if (dynamic_cast< VKRectangleShape* >(shape))
            {
                auto s = dynamic_cast< VKRectangleShape* >(shape);
                value = *s;
            }
            else if (dynamic_cast< VKFilledCircleShape* >(shape))
            {
                auto s = dynamic_cast< VKFilledCircleShape* >(shape);
                value = *s;
            }
            else if (dynamic_cast< VKCircleShape* >(shape))
            {
                auto s = dynamic_cast< VKCircleShape* >(shape);
                value = *s;
            }
#ifdef USE_OPENVK2
            else if (dynamic_cast< VK2TextShape* >(shape))
            {
                auto s = dynamic_cast< VK2TextShape* >(shape);
                value = *s;
            }
#else
            else if (dynamic_cast< VKTextShape* >(shape))
            {
                auto s = dynamic_cast< VKTextShape* >(shape);
                value = *s;
            }
#endif
            else if (dynamic_cast< draw::NoteShape* >(shape))
            {
                auto s = dynamic_cast< draw::NoteShape* >(shape);
                value = *s;
            }
            else if (dynamic_cast< VKFilledPolygonShape* >(shape))
            {
                auto s = dynamic_cast< VKFilledPolygonShape* >(shape);
                value = *s;
            }
            else if (dynamic_cast< VKPolygonShape* >(shape))
            {
                auto s = dynamic_cast< VKPolygonShape* >(shape);
                value = *s;
            }
            else if (dynamic_cast< VKErasePathShape* >(shape))
            {
                auto s = dynamic_cast< VKErasePathShape* >(shape);
                value = *s;
            }
            else if (dynamic_cast< VKPathShape* >(shape))
            {
                auto s = dynamic_cast< VKPathShape* >(shape);
                value = *s;
            }
            else
            {
                throw std::runtime_error("Unknown shape");
            }
            Message msg;
            msg["command"] = command;
            msg["value"] = value;
            tcp->pushMessage(msg);
        }

    } // namespace vulkan

} // namespace mrv
