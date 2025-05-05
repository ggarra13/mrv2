// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <cinttypes>

#include <tlCore/FontSystem.h>
#include <tlCore/StringFormat.h>

#include <tlDevice/IOutput.h>

#include <tlTimelineVk/RenderPrivate.h>

#include <tlVk/PipelineCreationState.h>
#include <tlVk/OffscreenBuffer.h>
#include <tlVk/Init.h>
#include <tlVk/Util.h>

#include <FL/vk_enum_string_helper.h>

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
            mode(FL_RGB | FL_DOUBLE | FL_ALPHA | FL_STENCIL | stereo);
        }

        Viewport::~Viewport() {}

        void Viewport::setContext(const std::weak_ptr<system::Context>& context)
        {
            _vk->context = context;
        }

        int Viewport::log_level() const
        {
            return 0;
        }

        void Viewport::prepare_pipeline_layout()
        {
            MRV2_VK();

            VkResult result;

            VkPipelineLayoutCreateInfo pPipelineLayoutCreateInfo = {};
            pPipelineLayoutCreateInfo.sType =
                VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
            pPipelineLayoutCreateInfo.pNext = NULL;
            pPipelineLayoutCreateInfo.setLayoutCount = 1;

            VkDescriptorSetLayout setLayout = vk.shader->getDescriptorSetLayout();
            pPipelineLayoutCreateInfo.pSetLayouts = &setLayout;

            VkPushConstantRange pushConstantRange = {};
            std::size_t pushSize = vk.shader->getPushSize();
            if (pushSize > 0)
            {
                pushConstantRange.stageFlags = vk.shader->getPushStageFlags();
                pushConstantRange.offset = 0;
                pushConstantRange.size = pushSize;

                pPipelineLayoutCreateInfo.pushConstantRangeCount = 1;
                pPipelineLayoutCreateInfo.pPushConstantRanges =
                    &pushConstantRange;
            }

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

            const auto& bindingDescs = vk.vbo->getBindingDescription();
            const auto& attrDescs = vk.vbo->getAttributes();
            vi.sType =
                VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
            vi.pNext = NULL;
            vi.vertexBindingDescriptionCount = bindingDescs.size();
            vi.pVertexBindingDescriptions = bindingDescs.data();
            vi.vertexAttributeDescriptionCount = attrDescs.size();
            vi.pVertexAttributeDescriptions = attrDescs.data();

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
            prepare_shaders();
            prepare_render_pass();       // Main swapchain render pass
            prepare_pipeline_layout(); 
        }

        void Viewport::destroy_resources()
        {
            MRV2_VK();

            vk.vbo.reset();

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

        void Viewport::hide()
        {
            TLRENDER_P();
            MRV2_VK();

            wait_device();

            // Destroy main render
            vk.render.reset();

            // Destroy auxiliary render classes
            vk.lines.reset();
#ifdef USE_ONE_PIXEL_LINES
            vk.outline.reset();
#endif
            // Destroy Buffers
            vk.buffer.reset();
            vk.annotation.reset();
            vk.overlay.reset();

            // Destroy shaders
            vk.shader.reset();
            vk.annotationShader.reset();

            // Destroy meshes
            vk.vbo.reset();
            vk.vao.reset();

            p.fontSystem.reset();

            TimelineViewport::hide();
        }

        void Viewport::prepare_shaders()
        {
            TLRENDER_P();
            MRV2_VK();

            if (auto context = vk.context.lock())
            {

                if (!vk.render)
                    vk.render = timeline_vlk::Render::create(ctx, context);

                if (!p.fontSystem)
                    p.fontSystem = image::FontSystem::create(context);

#ifdef USE_ONE_PIXEL_LINES
                if (!vk.outline)
                    vk.outline = std::make_shared<vulkan::Outline>();
#endif
                if (!vk.lines)
                    vk.lines = std::make_shared<vulkan::Lines>();

                const std::string& vertexSource = timeline_vlk::vertexSource();
                const image::Color4f color(1.F, 1.F, 1.F);
                math::Matrix4x4f mvp;
                if (!vk.shader)
                {
                    vk.shader = vlk::Shader::create(
                        ctx, vertexSource, textureFragmentSource(),
                        "vk.shader");

                    // Create parameters for shader.
                    vk.shader->createUniform(
                        "transform.mvp", mvp, vlk::kShaderVertex);
                    vk.shader->addFBO("textureSampler"); // default is fragment
                    float opacity = 1.F;
                    vk.shader->addPush("opacity", opacity, vlk::kShaderFragment);
                    auto bindingSet = vk.shader->createBindingSet();
                    vk.shader->useBindingSet(bindingSet);
                }

                if (!vk.annotationShader)
                {
                    vk.annotationShader = vlk::Shader::create(
                        ctx, vertexSource, annotationFragmentSource(),
                        "vk.annotationShader");
                    vk.annotationShader->createUniform(
                        "transform.mvp", mvp, vlk::kShaderVertex);
                    int channels = 0; // Color
                    vk.annotationShader->createUniform("channels", channels);
                    auto bindingSet = vk.annotationShader->createBindingSet();
                    vk.annotationShader->useBindingSet(bindingSet);
                }
            }
        }

        int Viewport::handle(int event)
        {
            return TimelineViewport::handle(event);
        }

        void Viewport::draw()
        {
            TLRENDER_P();
            MRV2_VK();

            // Get the command buffer started for the current frame.
            VkCommandBuffer cmd = getCurrentCommandBuffer();
            end_render_pass(cmd);

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
                offscreenBufferOptions.pbo = true;
            
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
                return;
            }

            if (p.pixelAspectRatio > 0.F && !p.videoData.empty() &&
                !p.videoData[0].layers.empty())
            {
                auto image = p.videoData[0].layers[0].image;
                p.videoData[0].size.pixelAspectRatio = p.pixelAspectRatio;
                image->setPixelAspectRatio(p.pixelAspectRatio);
            }

            // Get the background colors
            float r = 0.F, g = 0.F, b = 0.F, a = 0.F;

            if (!p.presentation)
            {
                Fl_Color c = p.ui->uiPrefs->uiPrefsViewBG->color();

                uint8_t ur = 0, ug = 0, ub = 0, ua = 0;
                Fl::get_color(c, ur, ug, ub, ua);

                r = ur / 255.0f;
                g = ug / 255.0f;
                b = ub / 255.0f;
                a = alpha;

                if (desktop::Wayland())
                {
                    p.ui->uiViewGroup->color(fl_rgb_color(ur, ug, ub));
                    p.ui->uiViewGroup->redraw();
                }
            }
            else
            {
                if (desktop::Wayland())
                {
                    p.ui->uiViewGroup->color(fl_rgb_color(0, 0, 0));
                    p.ui->uiViewGroup->redraw();
                }

                // Hide the cursor if in presentation time after 3 seconds of
                // inactivity.
                const auto& time = std::chrono::high_resolution_clock::now();
                const auto elapsedTime =
                    std::chrono::duration_cast<std::chrono::milliseconds>(
                        time - p.presentationTime)
                        .count();
                if (elapsedTime >= 3000)
                {
                    // If user is changing preferences or hotkeys, keep
                    // the default cursor.
                    if (p.ui->uiPrefs->uiMain->visible() ||
                        p.ui->uiHotkey->uiMain->visible() ||
                        p.ui->uiAbout->uiMain->visible())
                    {
                        window()->cursor(FL_CURSOR_DEFAULT);
                    }
                    else
                    {
                        window()->cursor(FL_CURSOR_NONE);
                    }
                    p.presentationTime = time;
                }
            }

            locale::SetAndRestore saved;
            timeline::RenderOptions renderOptions;
            renderOptions.colorBuffer = vk.colorBufferType;

            _checkHDR();
            
#ifndef NDEBUG
            try
#endif
            {
                vk.buffer->transitionToColorAttachment(cmd);
            
                vk.render->begin(
                    cmd, vk.buffer, m_currentFrameIndex, renderSize,
                    renderOptions);

                if (p.showVideo)
                {
                    int screen = this->screen_num();
                    if (screen >= 0 && !p.monitorOCIOOptions.empty() &&
                        screen < p.monitorOCIOOptions.size())
                    {
                        timeline::OCIOOptions o = p.ocioOptions;
                        o.display = p.monitorOCIOOptions[screen].display;
                        o.view = p.monitorOCIOOptions[screen].view;
                        vk.render->setOCIOOptions(o);

                        _updateMonitorDisplayView(screen, o);
                    }
                    else
                    {
                        vk.render->setOCIOOptions(p.ocioOptions);
                        _updateMonitorDisplayView(screen, p.ocioOptions);
                    }

                    timeline::BackgroundOptions backgroundOptions = getBackgroundOptions();        
                    if (transparent)
                    {
                        backgroundOptions.type = timeline::Background::Solid;
                        backgroundOptions.color0 = image::Color4f(r, g, b, a);
                    }
            
                    vk.render->setLUTOptions(p.lutOptions);
                    vk.render->setHDROptions(p.hdrOptions);
                    if (p.missingFrame &&
                        p.missingFrameType != MissingFrameType::kBlackFrame)
                    {
                        _drawMissingFrame(renderSize);
                    }
                    else
                    {
                        if (p.stereo3DOptions.input == Stereo3DInput::Image &&
                            p.videoData.size() > 1)
                        {
                            _drawStereo3D();
                        }
                        else
                        {
                            vk.render->drawVideo(
                                p.videoData,
                                timeline::getBoxes(
                                    p.compareOptions.mode, p.videoData),
                                p.imageOptions, p.displayOptions,
                                p.compareOptions, backgroundOptions);
                        }
                    }
                }

            }
#ifndef NDEBUG
            catch (const std::exception& e)
            {
                LOG_ERROR(e.what());
            }
#endif

            vk.render->end();

            m_clearColor = {r, g, b, a};

            math::Matrix4x4f mvp;

            const float rotation = _getRotation();

            if (p.environmentMapOptions.type != EnvironmentMapOptions::kNone)
            {
                mvp = _createEnvironmentMap();
            }
            else
            {
                mvp = _createTexturedRectangle();
            }
                
            
            // --- Final Render Pass: Render to Swapchain (Composition) ---
            vk.buffer->transitionToShaderRead(cmd);

            
            begin_render_pass(cmd);

            
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

            float opacity = 1.F;
#ifdef __APPLE__
            set_window_transparency(alpha);
#else
            if (desktop::Wayland())
                opacity = alpha;
#endif

            // --- Bind Descriptor Set for the SECOND pass ---
            // Record the command to bind the descriptor set for the CURRENT
            // frame index
            VkDescriptorSet descriptorSet = vk.shader->getDescriptorSet();
            vkCmdBindDescriptorSets(
                cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, vk.pipeline_layout, 0, 1,
                &descriptorSet, 0, nullptr);
            
            vkCmdPushConstants(
                cmd, vk.pipeline_layout,
                vk.shader->getPushStageFlags(), 0, sizeof(float), &opacity);

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
            
            if (p.hudActive && p.hud != HudDisplay::kNone)
                _drawHUD(cmd, alpha);
            
            end_render_pass(cmd);
            
            // Update the pixel bar from here only if we are playing a movie
            // and one that is not 1 frames long.
            bool update = !_shouldUpdatePixelBar();
            if (update)
                updatePixelBar();

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
                // This is needed as the FL_MOVE of fltk would get called
                // before the draw routine
                if (!vk.buffer)
                {
                    return;
                }

                // const VKenum type = GL_FLOAT;

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
                    VkCommandBuffer cmd =
                        beginSingleTimeCommands(device(), commandPool());

                    vk.buffer->readPixels(cmd, pos.x, pos.y, 1, 1);

                    vkEndCommandBuffer(cmd);

                    vk.buffer->submitReadback(cmd);

                    wait_queue();

                    vkFreeCommandBuffers(device(), commandPool(), 1, &cmd);

                    void* data = vk.buffer->getLatestReadPixels();
                    if (!data)
                    {
                        LOG_ERROR("Could not get pixel under mouse");
                        return;
                    }

                    auto options = vk.buffer->getOptions();

                    switch (options.colorType)
                    {
                    case image::PixelType::RGB_F32:
                    {
                        float* pixels = reinterpret_cast<float*>(data);
                        rgba.r = pixels[0];
                        rgba.g = pixels[1];
                        rgba.b = pixels[2];
                        rgba.a = 1.F;
                        break;
                    }
                    case image::PixelType::RGBA_F32:
                    {
                        float* pixels = reinterpret_cast<float*>(data);
                        rgba.r = pixels[0];
                        rgba.g = pixels[1];
                        rgba.b = pixels[2];
                        rgba.a = pixels[3];
                        break;
                    }
                    case image::PixelType::RGB_F16:
                    {
                        half* pixels = reinterpret_cast<half*>(data);
                        rgba.r = pixels[0];
                        rgba.g = pixels[1];
                        rgba.b = pixels[2];
                        rgba.a = 1.F;
                        break;
                    }
                    case image::PixelType::RGBA_F16:
                    {
                        half* pixels = reinterpret_cast<half*>(data);
                        rgba.r = pixels[0];
                        rgba.g = pixels[1];
                        rgba.b = pixels[2];
                        rgba.a = pixels[3];
                        break;
                    }
                    case image::PixelType::RGB_U8:
                    {
                        uint8_t* pixels = reinterpret_cast<uint8_t*>(data);
                        rgba.r = static_cast<float>(pixels[0]) / 255.F;
                        rgba.g = static_cast<float>(pixels[1]) / 255.F;
                        rgba.b = static_cast<float>(pixels[2]) / 255.F;
                        rgba.a = 1.F;
                        break;
                    }
                    case image::PixelType::RGBA_U8:
                    {
                        uint8_t* pixels = reinterpret_cast<uint8_t*>(data);
                        rgba.r = static_cast<float>(pixels[0]) / 255.F;
                        rgba.g = static_cast<float>(pixels[1]) / 255.F;
                        rgba.b = static_cast<float>(pixels[2]) / 255.F;
                        rgba.a = static_cast<float>(pixels[3]) / 255.F;
                        break;
                    }
                    case image::PixelType::RGBA_U16:
                    {
                        uint16_t* pixels = reinterpret_cast<uint16_t*>(data);
                        rgba.r = static_cast<float>(pixels[0]) / 65535.F;
                        rgba.g = static_cast<float>(pixels[1]) / 65535.F;
                        rgba.b = static_cast<float>(pixels[2]) / 65535.F;
                        rgba.a = static_cast<float>(pixels[3]) / 65535.F;
                        break;
                    }
                    case image::PixelType::L_F32:
                    {
                        float* pixels = reinterpret_cast<float*>(data);
                        rgba.r = pixels[0];
                        rgba.g = pixels[0];
                        rgba.b = pixels[0];
                        rgba.a = 1.0F;
                        break;
                    }
                    case image::PixelType::LA_F32:
                    {
                        float* pixels = reinterpret_cast<float*>(data);
                        rgba.r = pixels[0];
                        rgba.g = pixels[0];
                        rgba.b = pixels[0];
                        rgba.a = pixels[1];
                        break;
                    }
                    case image::PixelType::L_F16:
                    {
                        half* pixels = reinterpret_cast<half*>(data);
                        rgba.r = pixels[0];
                        rgba.g = pixels[0];
                        rgba.b = pixels[0];
                        rgba.a = 1.0F;
                        break;
                    }
                    case image::PixelType::LA_F16:
                    {
                        half* pixels = reinterpret_cast<half*>(data);
                        rgba.r = pixels[0];
                        rgba.g = pixels[0];
                        rgba.b = pixels[0];
                        rgba.a = pixels[1];
                        break;
                    }
                    case image::PixelType::L_U8:
                    {
                        uint16_t* pixels = reinterpret_cast<uint16_t*>(data);
                        rgba.r = static_cast<float>(pixels[0]) / 65535.F;
                        rgba.g = static_cast<float>(pixels[0]) / 65535.F;
                        rgba.b = static_cast<float>(pixels[0]) / 65535.F;
                        rgba.a = 1.0F;
                        break;
                    }
                    case image::PixelType::LA_U8:
                    {
                        uint8_t* pixels = reinterpret_cast<uint8_t*>(data);
                        rgba.r = static_cast<float>(pixels[0]) / 255.F;
                        rgba.g = static_cast<float>(pixels[0]) / 255.F;
                        rgba.b = static_cast<float>(pixels[0]) / 255.F;
                        rgba.a = static_cast<float>(pixels[1]) / 255.F;
                        break;
                    }
                    case image::PixelType::L_U16:
                    {
                        uint16_t* pixels = reinterpret_cast<uint16_t*>(data);
                        rgba.r = static_cast<float>(pixels[0]) / 65535.F;
                        rgba.g = static_cast<float>(pixels[0]) / 65535.F;
                        rgba.b = static_cast<float>(pixels[0]) / 65535.F;
                        rgba.a = 1.0F;
                        break;
                    }
                    case image::PixelType::LA_U16:
                    {
                        uint16_t* pixels = reinterpret_cast<uint16_t*>(data);
                        rgba.r = static_cast<float>(pixels[0]) / 65535.F;
                        rgba.g = static_cast<float>(pixels[0]) / 65535.F;
                        rgba.b = static_cast<float>(pixels[0]) / 65535.F;
                        rgba.a = static_cast<float>(pixels[1]) / 65535.F;
                        break;
                    }
                    default:
                        std::cerr << "Unhandled format " << options.colorType
                                  << std::endl;
                        break;
                    }
                    return;
                }

                // if (p.image)
                // {
                //     const auto& renderSize = vk.buffer->getSize();
                //     rgba.b = p.image[(pos.x + pos.y * renderSize.w) * 4];
                //     rgba.g = p.image[(pos.x + pos.y * renderSize.w) * 4 + 1];
                //     rgba.r = p.image[(pos.x + pos.y * renderSize.w) * 4 + 2];
                //     rgba.a = p.image[(pos.x + pos.y * renderSize.w) * 4 + 3];
                // }
            }
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
