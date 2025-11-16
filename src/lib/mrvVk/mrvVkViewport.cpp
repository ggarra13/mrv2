// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <cinttypes>

// mrViewer .fl includes
#include "mrViewer.h"
#include "mrvHotkeyUI.h"

#include "mrvNetwork/mrvDummyClient.h"

#include "mrvUI/mrvDesktop.h"
#include "mrvUI/mrvMonitor.h"

#include "mrvViewport/mrvTimelineViewport.h"

#include "mrvVk/mrvVkViewportPrivate.h"
#include "mrvVk/mrvVkDefines.h"
#include "mrvVk/mrvVkUtil.h"
#include "mrvVk/mrvVkShaders.h"
#include "mrvVk/mrvVkShadersBinary.h"
#include "mrvVk/mrvVkShape.h"
#include "mrvVk/mrvVkViewport.h"

#include "mrvFl/mrvIO.h"

#include "mrvWidgets/mrvMultilineInput.h"  // for annotationsPanel->notes

#include "mrvCore/mrvColor.h"
#include "mrvCore/mrvColorSpaces.h"
#include "mrvCore/mrvLocale.h"
#include "mrvCore/mrvSequence.h"
#include "mrvCore/mrvI8N.h"

#include <tlTimelineVk/RenderShadersBinary.h>

#include <tlCore/FontSystem.h>
#include <tlCore/StringFormat.h>

#include <tlDevice/IOutput.h>

#include <tlTimelineVk/RenderPrivate.h>

#include <tlVk/PipelineCreationState.h>
#include <tlVk/OffscreenBuffer.h>
#include <tlVk/Init.h>
#include <tlVk/Util.h>

#include <FL/vk_enum_string_helper.h>


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
            // m_debugSync = true;
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

        void Viewport::setSaveOverlay(bool value)
        {
            _vk->saveOverlay = true;
        }

        void Viewport::prepare_annotation_pipeline_layout()
        {
            MRV2_VK();

            VkResult result;

            //
            // Prepare annotation pipeline layout
            //            
            VkPipelineLayoutCreateInfo pPipelineLayoutCreateInfo = {};
            pPipelineLayoutCreateInfo.sType =
                VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
            pPipelineLayoutCreateInfo.pNext = NULL;
            pPipelineLayoutCreateInfo.setLayoutCount = 1;

            VkDescriptorSetLayout setLayout = vk.annotationShader->getDescriptorSetLayout();
            pPipelineLayoutCreateInfo.pSetLayouts = &setLayout;

            VkPushConstantRange pushConstantRange = {};
            std::size_t pushSize = vk.annotationShader->getPushSize();
            if (pushSize > 0)
            {
                pushConstantRange.stageFlags =
                    vk.annotationShader->getPushStageFlags();
                pushConstantRange.offset = 0;
                pushConstantRange.size = pushSize;

                pPipelineLayoutCreateInfo.pushConstantRangeCount = 1;
                pPipelineLayoutCreateInfo.pPushConstantRanges =
                    &pushConstantRange;
            }

            result = vkCreatePipelineLayout(
                device(), &pPipelineLayoutCreateInfo, NULL,
                &vk.annotation_pipeline_layout);
        }
        
        void Viewport::prepare_pipeline_layout()
        {
            MRV2_VK();
            
            VkResult result;

            //
            // Prepare main buffer comping layout 
            //

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

            if (pipeline() != VK_NULL_HANDLE)
            {
                vkDestroyPipeline(device(), m_pipeline, nullptr);
                pipeline() = VK_NULL_HANDLE;
            }
            
            // Elements of new Pipeline (fill with mesh info)
            vlk::VertexInputStateInfo vi;
            vi.bindingDescriptions = vk.vbo->getBindingDescription();
            vi.attributeDescriptions = vk.vbo->getAttributes();
            
            // Defaults are fine
            vlk::InputAssemblyStateInfo ia;

            // Defaults are fine
            vlk::RasterizationStateInfo rs;
            
            // Defaults are fine
            vlk::ViewportStateInfo vp;
            
            vlk::ColorBlendStateInfo cb;
            vlk::ColorBlendAttachmentStateInfo colorBlendAttachment;
            colorBlendAttachment.blendEnable = VK_FALSE;
            cb.attachments.push_back(colorBlendAttachment);

            const bool hasDepth = mode() & FL_DEPTH;
            const bool hasStencil = mode() & FL_STENCIL;
            
            vlk::DepthStencilStateInfo ds;
            ds.depthTestEnable = hasDepth ? VK_TRUE : VK_FALSE;
            ds.depthWriteEnable = hasDepth ? VK_TRUE : VK_FALSE;
            ds.stencilTestEnable = hasStencil ? VK_TRUE : VK_FALSE;
            
            vlk::DynamicStateInfo dynamicState;
            dynamicState.dynamicStates = {
                VK_DYNAMIC_STATE_VIEWPORT,
                VK_DYNAMIC_STATE_SCISSOR,
                VK_DYNAMIC_STATE_COLOR_WRITE_MASK_EXT,
            };
            
            // Defaults are fine
            vlk::MultisampleStateInfo ms;

            // Get the vertex and fragment shaders
            std::vector<vlk::PipelineCreationState::ShaderStageInfo>
                shaderStages(2);

            shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
            shaderStages[0].name = vk.shader->getName();
            shaderStages[0].module = vk.shader->getVertex();
            shaderStages[0].entryPoint = "main";

            shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
            shaderStages[1].name = vk.shader->getName();
            shaderStages[1].module = vk.shader->getFragment();
            shaderStages[1].entryPoint = "main";

            //
            // Pass pipeline creation parameters to pipelineState.
            //
            vlk::PipelineCreationState pipelineState;
            pipelineState.vertexInputState = vi;
            pipelineState.inputAssemblyState = ia;
            pipelineState.colorBlendState = cb;
            pipelineState.rasterizationState = rs;
            pipelineState.depthStencilState = ds;
            pipelineState.viewportState = vp;
            pipelineState.multisampleState = ms;
            pipelineState.dynamicState = dynamicState;
            pipelineState.stages = shaderStages;
            pipelineState.renderPass = renderPass();
            pipelineState.layout = vk.pipeline_layout;

            pipeline() = pipelineState.create(device());
            if (pipeline() == VK_NULL_HANDLE)
            {
                throw std::runtime_error("Composition pipeline failed");
            }
        }
        
        void Viewport::prepare_annotation_pipeline()
        {
            MRV2_VK();

            if (vk.annotation_pipeline != VK_NULL_HANDLE)
            {
                vkDestroyPipeline(device(), vk.annotation_pipeline, nullptr);
            }
            
            // Elements of new Pipeline (fill with mesh info)
            vlk::VertexInputStateInfo vi;
            vi.bindingDescriptions = vk.avbo->getBindingDescription();
            vi.attributeDescriptions = vk.avbo->getAttributes();

            // Defaults are fine
            vlk::InputAssemblyStateInfo ia;
            
            // Defaults are fine
            vlk::RasterizationStateInfo rs;
            
            // Defaults are fine
            vlk::ViewportStateInfo vp;

            vlk::ColorBlendStateInfo cb;
            vlk::ColorBlendAttachmentStateInfo colorBlendAttachment;
            colorBlendAttachment.blendEnable = VK_TRUE;
            colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
            colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
            colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
            colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
            cb.attachments.push_back(colorBlendAttachment);


            bool has_depth = mode() & FL_DEPTH;     // Check window depth
            bool has_stencil = mode() & FL_STENCIL; // Check window stencil
            
            vlk::DepthStencilStateInfo ds;
            ds.depthTestEnable = VK_FALSE;
            ds.depthWriteEnable = VK_FALSE;
            ds.stencilTestEnable = VK_FALSE;

            // Defaults are fine
            vlk::MultisampleStateInfo ms;
            
            // Defaults are fine
            vlk::DynamicStateInfo dynamicState;
            dynamicState.dynamicStates = {
                VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
            
            // Get the vertex and fragment shaders
            std::vector<vlk::PipelineCreationState::ShaderStageInfo>
                shaderStages(2);

            shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
            shaderStages[0].name = vk.annotationShader->getName();
            shaderStages[0].module = vk.annotationShader->getVertex();
            shaderStages[0].entryPoint = "main";

            shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
            shaderStages[1].name = vk.annotationShader->getName();
            shaderStages[1].module = vk.annotationShader->getFragment();
            shaderStages[1].entryPoint = "main";

            //
            // Pass pipeline creation parameters to pipelineState.
            //
            vlk::PipelineCreationState pipelineState;
            pipelineState.vertexInputState = vi;
            pipelineState.inputAssemblyState = ia;
            pipelineState.colorBlendState = cb;
            pipelineState.rasterizationState = rs;
            pipelineState.depthStencilState = ds;
            pipelineState.viewportState = vp;
            pipelineState.multisampleState = ms;
            pipelineState.dynamicState = dynamicState;
            pipelineState.stages = shaderStages;
            pipelineState.renderPass = renderPass();
            pipelineState.layout = vk.annotation_pipeline_layout;

            vk.annotation_pipeline = pipelineState.create(device());
            if (vk.annotation_pipeline == VK_NULL_HANDLE)
            {
                throw std::runtime_error("Annotation pipeline failed");
            }
        }

        std::vector<const char*> Viewport::get_device_extensions()
        {
            std::vector<const char*> out;
            out = Fl_Vk_Window::get_device_extensions();
            out.push_back(VK_EXT_HDR_METADATA_EXTENSION_NAME);
            return out;
        }
        
        void Viewport::init_colorspace()
        {
            TLRENDER_P();

            Fl_Vk_Window::init_colorspace();

            // Look for HDR10 or HLG if present
            p.hdrMonitorFound = false;

            bool valid_colorspace = false;
            switch (colorSpace())
            {
            case VK_COLOR_SPACE_DISPLAY_P3_NONLINEAR_EXT:
            case VK_COLOR_SPACE_HDR10_ST2084_EXT:
            case VK_COLOR_SPACE_HDR10_HLG_EXT:
            case VK_COLOR_SPACE_DOLBYVISION_EXT:
                valid_colorspace = true;
                break;
            default:
                break;
            }

            if (valid_colorspace && monitor::is_hdr_active())
            {
                p.hdrMonitorFound = true;
                LOG_STATUS(_("HDR monitor found."));
            }
            else
            {
                if (colorSpace() != VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
                {
                    colorSpace() = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
                    format() = VK_FORMAT_B8G8R8A8_UNORM;
                }
                LOG_STATUS(_("HDR monitor not found or not configured."));
            }
            LOG_STATUS("Vulkan color space is " << string_VkColorSpaceKHR(colorSpace()));
            LOG_STATUS("Vulkan format is " << string_VkFormat(format()));
        }

        
        void Viewport::prepare()
        {
            prepare_render_pass();       // Main swapchain render pass
            prepare_load_render_pass();  // swapchain render pass that loads contents
            prepare_shaders();
            prepare_pipeline_layout();
            prepare_annotation_pipeline_layout();
        }

        void Viewport::destroy()
        {
            MRV2_VK();

            //
            // Do *NOT* destroy buffers based on resolution here.
            //
            // vk.buffer.reset();
            // vk.stereoBuffer.reset();
            // vk.overlay.reset();

            // Destroy viewport size buffer here
            vk.annotation.reset();

            vk.vbo.reset();
            vk.vao.reset();
            
            vk.avbo.reset();
            vk.avao.reset();

            if (vk.loadRenderPass != VK_NULL_HANDLE)
            {
                vkDestroyRenderPass(device(), vk.loadRenderPass, nullptr);
                vk.loadRenderPass = VK_NULL_HANDLE;
            }
            
            if (vk.annotation_pipeline_layout != VK_NULL_HANDLE)
            {
                vkDestroyPipelineLayout(device(),
                                        vk.annotation_pipeline_layout, nullptr);
                vk.annotation_pipeline_layout = VK_NULL_HANDLE;
            }
            
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
            
            if (vk.annotation_pipeline != VK_NULL_HANDLE)
            {
                vkDestroyPipeline(device(), vk.annotation_pipeline, nullptr);
                vk.annotation_pipeline = VK_NULL_HANDLE;
            }

            VkWindow::destroy();
        }

        void Viewport::hide()
        {
            TLRENDER_P();
            MRV2_VK();

            wait_device();

            // Destroy main renders
            vk.render.reset();
            vk.annotationRender.reset();

            // Destroy auxiliary render classes
            vk.lines.reset();
            vk.viewport.reset();
            
            // Destroy Buffers
            vk.buffer.reset();
            vk.stereoBuffer.reset();
            vk.annotation.reset();
            vk.overlay.reset();

            // Destroy shaders
            vk.shader.reset();
            vk.annotationShader.reset();

            // Destroy meshes
            vk.vbo.reset();
            vk.vao.reset();

            vk.avbo.reset();
            vk.avao.reset();

            p.fontSystem.reset();

            TimelineViewport::hide();
        }


        std::shared_ptr<vlk::OffscreenBuffer> Viewport::getVideoFBO()
        {
            MRV2_VK();
            return vk.buffer;
        }

        std::shared_ptr<vlk::OffscreenBuffer> Viewport::getAnnotationFBO()
        {
            MRV2_VK();
            return vk.overlay;
        }

        
        void Viewport::prepare_shaders()
        {
            TLRENDER_P();
            MRV2_VK();

            if (auto context = vk.context.lock())
            {
                // Add the thumbnail system if not present.
                if (!context->getSystem<timelineui_vk::ThumbnailSystem>())
                {
                    context->addSystem(timelineui_vk::ThumbnailSystem::create(context, ctx));
                }
                
                vk.render = timeline_vlk::Render::create(ctx, context);

                vk.annotationRender = timeline_vlk::Render::create(ctx, context);
                
                p.fontSystem = image::FontSystem::create(context);

                vk.lines = std::make_shared<vulkan::Lines>(ctx, VK_NULL_HANDLE);
                vk.viewport = std::make_shared<vulkan::Lines>(ctx, renderPass());

                
                const image::Color4f color(1.F, 1.F, 1.F);
                math::Matrix4x4f mvp;
                if (!vk.shader)
                {
                    vk.shader = vlk::Shader::create(
                        ctx,
                        timeline_vlk::Vertex3_spv,
                        timeline_vlk::Vertex3_spv_len,
                        textureFragment_spv,
                        textureFragment_spv_len,
                        "vk.shader");

                    // Create parameters for shader.
                    vk.shader->createUniform(
                        "transform.mvp", mvp, vlk::kShaderVertex);
                    vk.shader->addFBO("textureSampler"); // default is fragment
                    float opacity = 1.F;
                    vk.shader->addPush("opacity", opacity, vlk::kShaderFragment);
                    vk.shader->createBindingSet();
                }

                if (!vk.annotationShader)
                {
                    vk.annotationShader = vlk::Shader::create(
                        ctx, 
                        timeline_vlk::Vertex3_spv,
                        timeline_vlk::Vertex3_spv_len,
                        annotationFragment_spv,
                        annotationFragment_spv_len,
                        "vk.annotationShader");
                    vk.annotationShader->createUniform(
                        "transform.mvp", mvp, vlk::kShaderVertex);
                    vk.annotationShader->addFBO("textureSampler");
                    int channels = 0; // Color Channel
                    vk.annotationShader->createUniform("channels", channels);
                    vk.annotationShader->createBindingSet();
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

            // Clear the frame
            begin_render_pass(cmd);
            end_render_pass(cmd);

            // Store command for easy access.
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
                        case image::PixelType::YUV_420P_U12:
                        case image::PixelType::YUV_422P_U12:
                        case image::PixelType::YUV_444P_U12:
                        case image::PixelType::YUV_420P_U16:
                        case image::PixelType::YUV_422P_U16:
                        case image::PixelType::YUV_444P_U16:
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
                    if (hasFrameView())
                        _frameView();
                }
            }
            else
            {
                wait_device();
                
                vk.buffer.reset();
                vk.stereoBuffer.reset();
            }

            const auto& player = getTimelinePlayer();
            if (!vk.buffer || !player)
            {
                return;
            }

            _updateDevices();

            const auto& annotations =
                player->getAnnotations(p.ghostPrevious, p.ghostNext);

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
                    if ((p.ui->uiPrefs && p.ui->uiPrefs->uiMain->visible()) ||
                        (p.ui->uiHotkey && p.ui->uiHotkey->uiMain->visible()) ||
                        (p.ui->uiAbout && p.ui->uiAbout->uiMain->visible()))
                    {
                        set_cursor(FL_CURSOR_DEFAULT);
                    }
                    else
                    {
                        set_cursor(FL_CURSOR_NONE);
                    }
                    p.presentationTime = time;
                }
            }

            locale::SetAndRestore saved;
            timeline::RenderOptions renderOptions;
            renderOptions.colorBuffer = vk.colorBufferType;

            _updateHDRMetadata();
            
            try
            {
                if (vk.buffer)
                {
                    vk.buffer->transitionToColorAttachment(cmd);
                }
                
                vk.render->begin(
                    cmd, vk.buffer, m_currentFrameIndex, renderSize,
                    renderOptions);
                vk.render->applyTransforms();

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
            catch (const std::exception& e)
            {
                LOG_ERROR(e.what());
            }

            _drawOverlays(renderSize);
            vk.render->end();

            
            m_clearColor = {r, g, b, a};

            math::Matrix4x4f mvp;

            if (p.environmentMapOptions.type != EnvironmentMapOptions::kNone)
            {
                mvp = _createEnvironmentMap();
            }
            else
            {
                mvp = _createTexturedRectangle();
            }


            float opacity = 1.F;
#ifdef __APPLE__
            set_window_transparency(alpha);
#else
            if (desktop::Wayland())
                opacity = alpha;
#endif

            if (panel::annotationsPanel)
            {
                panel::annotationsPanel->notes->value("");
            }
            
            const otime::RationalTime& currentTime = player->currentTime();
            
            const auto& voannotations = p.player->getVoiceAnnotations();
            
            if (p.showAnnotations &&
                (!annotations.empty() || !voannotations.empty()))
            {
                vlk::OffscreenBufferOptions offscreenBufferOptions;
                offscreenBufferOptions.colorType = image::PixelType::RGBA_U8;
                offscreenBufferOptions.clear = true;
                offscreenBufferOptions.clearColor = image::Color4f(0.F, 0.F, 0.F, 0.F);
                if (!p.displayOptions.empty())
                {
                    offscreenBufferOptions.colorFilters =
                        p.displayOptions[0].imageFilters;
                }
                offscreenBufferOptions.depth = vlk::OffscreenDepth::kNone;
                offscreenBufferOptions.stencil = vlk::OffscreenStencil::kNone;
                if (vlk::doCreate(
                    vk.annotation, viewportSize, offscreenBufferOptions))
                {
                    vk.annotation = vlk::OffscreenBuffer::create(
                        ctx, viewportSize, offscreenBufferOptions);
                    vk.avbo.reset();
                    
                    const auto& mesh = geom::box(math::Box2i(0, 0,
                                                             viewportSize.w,
                                                             viewportSize.h));
                    const size_t numTriangles = mesh.triangles.size();
                    vk.avbo = vlk::VBO::create(
                        numTriangles * 3, vlk::VBOType::Pos2_F32_UV_U16);
                
                    if (vk.avbo)
                    {
                        vk.avbo->copy(convert(mesh, vlk::VBOType::Pos2_F32_UV_U16));
                    }

                    if (!vk.avao && vk.avbo)
                    {
                        vk.avao = vlk::VAO::create(ctx);
                        prepare_annotation_pipeline();
                    }
                }

                _drawAnnotations(
                    vk.annotation, vk.annotationRender,
                    mvp, currentTime, annotations, voannotations, viewportSize);
                
            }
            
            //
            // Draw annotations for output device in an overlay buffer.
            //
            auto outputDevice = App::app->outputDevice();
            if ((outputDevice || vk.saveOverlay) && p.showAnnotations)
            {
                if (!annotations.empty())
                {
                    vlk::OffscreenBufferOptions offscreenBufferOptions;
                    offscreenBufferOptions.colorType = image::PixelType::RGBA_U8;
                    if (!p.displayOptions.empty())
                    {
                        offscreenBufferOptions.colorFilters =
                            p.displayOptions[0].imageFilters;
                    }
                    offscreenBufferOptions.depth = vlk::OffscreenDepth::kNone;
                    offscreenBufferOptions.stencil = vlk::OffscreenStencil::kNone;
                    offscreenBufferOptions.pbo = true;
                    if (vlk::doCreate(
                            vk.overlay, renderSize, offscreenBufferOptions))
                    {
                        vk.overlay = vlk::OffscreenBuffer::create(ctx,
                                                                  renderSize,
                                                                  offscreenBufferOptions);
                        
                        if (!vk.overlayRender)
                        {
                            if (auto context = vk.context.lock())
                            {
                                vk.overlayRender = timeline_vlk::Render::create(ctx, context);
                            }
                        }
                    }

                    const math::Matrix4x4f& renderMVP = _renderProjectionMatrix();
                    
                    _drawAnnotations(
                        vk.overlay, vk.overlayRender,
                        renderMVP, currentTime, annotations, voannotations,
                        renderSize);

                    _readOverlay(vk.overlay);

                    if (vk.overlay)
                        vk.overlay->transitionToColorAttachment(cmd);
                }
                else
                {
                    const image::PixelType pixelType = image::PixelType::RGBA_U8;
                    vk.annotationImage = image::Image::create(1, 1, pixelType);
                    
                }

                if (outputDevice)
                    outputDevice->setOverlay(vk.annotationImage);
            }
            
            uint32_t W = pixel_w();
            uint32_t H = pixel_h();
            
            VkViewport viewport = {};
            viewport.width = static_cast<float>(W);
            viewport.height = static_cast<float>(H);
            viewport.minDepth = 0.0f;
            viewport.maxDepth = 1.0f;
            vkCmdSetViewport(cmd, 0, 1, &viewport);

            VkRect2D scissor = {};
            scissor.extent.width = W;
            scissor.extent.height = H;
            vkCmdSetScissor(cmd, 0, 1, &scissor);
                
            const float rotation = _getRotation();
            bool blitViewport = false;
            if (this == p.ui->uiView)
                blitViewport = p.ui->uiPrefs->uiPrefsBlitMainViewport->value();
            else
                blitViewport = p.ui->uiPrefs->uiPrefsBlitSecondaryViewport->value();
            if (p.presentation ||
                blitViewport == kNoBlit ||
                p.environmentMapOptions.type != EnvironmentMapOptions::kNone ||
                rotation != 0.F || (transparent && hasAlpha))
            {
                // --- Final Render Pass: Render to Swapchain (Composition) ---
                vk.buffer->transitionToShaderRead(cmd);
                if (vk.annotation)
                {
                    vk.annotation->transitionToShaderRead(cmd);
                }
                
                begin_render_pass(cmd);

                // Bind the shaders to the current frame index.
                vk.shader->bind(m_currentFrameIndex);

                // Bind the main composition pipeline (created/managed outside this
                // draw loop)
                vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline());
            
                // --- Update Descriptor Set for the SECOND pass (Composition) ---
                // This updates the descriptor set for the CURRENT frame index on
                // the CPU.
                vk.shader->setUniform("transform.mvp", mvp, vlk::kShaderVertex);
                vk.shader->setFBO("textureSampler", vk.buffer);
            
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

                if (vk.vao && vk.vbo)
                {
                    const VkColorComponentFlags allMask[] =
                        { VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                          VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT };
                    ctx.vkCmdSetColorWriteMaskEXT(cmd, 0, 1, allMask);
            
                    vk.vao->bind(m_currentFrameIndex);
                    vk.vao->draw(cmd, vk.vbo);
                }
            }
            else
            {
                VkImage srcImage = vk.buffer->getImage();
                VkImage dstImage = get_back_buffer_image();
                
                // srcImage barrier
                transitionImageLayout(cmd, srcImage,
                                      VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                      VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
                // srcImage barrier
                vk.buffer->setImageLayout(VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL); 
                
                // dstImage barrier (swapchain image)
                transitionImageLayout(cmd, dstImage,
                                      VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
                                      VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

                // Blit the offscreen buffer contents to the viewport
                VkFilter filter = VK_FILTER_NEAREST;
                if (!p.displayOptions.empty())
                {
                    const auto& filters = p.displayOptions[0].imageFilters;
                    if (p.viewZoom < 1.0f &&
                        filters.minify == timeline::ImageFilter::Linear)
                        filter = VK_FILTER_LINEAR;
                    else if (
                        p.viewZoom > 1.0f &&
                        filters.magnify == timeline::ImageFilter::Linear)
                        filter = VK_FILTER_LINEAR;
                }
                
                const int32_t viewportX = p.viewPos.x;
                const int32_t viewportY = p.viewPos.y;
                const int32_t sizeW = renderSize.w * p.viewZoom;
                const int32_t sizeH = renderSize.h * p.viewZoom;
                
                int dstRight  = std::min(viewportX + sizeW, viewportSize.w);
                int dstBottom = std::min(viewportY + sizeH, viewportSize.h);
                int dstLeft   = std::max(viewportX, 0);
                int dstTop    = std::max(viewportY, 0);

                const int dstWidth = dstRight - dstLeft;
                const int dstHeight = dstBottom - dstTop;

                if (dstWidth <= 0 || dstHeight <= 0)
                {
                    m_in_render_pass = false;
                    return; // Nothing to blit
                }
                
                // Adjust source region.
                const int srcWidth = renderSize.w;
                const int srcHeight = renderSize.h;
                
                const float scaleX = static_cast<float>(srcWidth) / sizeW;
                const float scaleY = static_cast<float>(srcHeight) / sizeH;
                
                const int offsetX = dstLeft - viewportX;
                const int offsetY = dstTop - viewportY;

                const int visibleW = dstWidth;
                const int visibleH = dstHeight;

                int srcLeft   = static_cast<int>(offsetX * scaleX);
                int srcTop    = static_cast<int>(offsetY * scaleY);
                int srcRight  = static_cast<int>((offsetX + visibleW) * scaleX);
                int srcBottom = static_cast<int>((offsetY + visibleH) * scaleY);
                
                VkImageBlit region = {};
                region.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                region.srcSubresource.mipLevel = 0;
                region.srcSubresource.baseArrayLayer = 0;
                region.srcSubresource.layerCount = 1;
                region.srcOffsets[0] = {srcLeft, srcTop, 0};
                region.srcOffsets[1] = {srcRight, srcBottom, 1};
                region.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                region.dstSubresource.mipLevel = 0;
                region.dstSubresource.baseArrayLayer = 0;
                region.dstSubresource.layerCount = 1;
                region.dstOffsets[0] = {dstLeft, dstTop, 0};
                region.dstOffsets[1] = {dstRight, dstBottom, 1};
                vkCmdBlitImage(
                    cmd,
                    srcImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                    dstImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                    1, &region,
                    filter
                    );
                
                // srcImage barrier
                transitionImageLayout(cmd, srcImage,
                                      VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                      VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
                vk.buffer->setImageLayout(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
                                
                // dstImage barrier (swapchain image)
                transitionImageLayout(cmd, dstImage,
                                      VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                      VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
            }
            


            if (!m_in_render_pass)
            {
                VkClearValue clear_values[2];
                clear_values[0].color = m_clearColor;
                clear_values[1].depthStencil = {m_depthStencil, 0};

                VkRenderPassBeginInfo rp_begin = {};
                rp_begin.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
                rp_begin.renderPass = vk.loadRenderPass;
                rp_begin.framebuffer = m_buffers[m_current_buffer].framebuffer;
                rp_begin.renderArea.offset.x = 0;
                rp_begin.renderArea.offset.y = 0;
                rp_begin.renderArea.extent.width = W;
                rp_begin.renderArea.extent.height = H;
                rp_begin.clearValueCount = 2;
                rp_begin.pClearValues = clear_values;
                
                vkCmdBeginRenderPass(cmd, &rp_begin, VK_SUBPASS_CONTENTS_INLINE);
                m_in_render_pass = true;
            }

            
            if (p.showAnnotations &&
                vk.annotation_pipeline != VK_NULL_HANDLE &&
                (!annotations.empty() || !voannotations.empty()))
            {             
                vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
                                  vk.annotation_pipeline);
            
                // We already flipped the coords, so we use a normal ortho
                // matrix here
                const math::Matrix4x4f orthoMatrix = math::ortho(
                    0.F, static_cast<float>(viewportSize.w),
                    static_cast<float>(viewportSize.h), 0.0F, -1.F, 1.F);
                _compositeAnnotations(cmd, vk.annotation, orthoMatrix,
                                      vk.annotationShader, vk.avbo,
                                      viewportSize);
            }
    
            if (p.dataWindow)
                _drawDataWindow();
            if (p.displayWindow)
                _drawDisplayWindow();

            if (p.safeAreas)
                _drawSafeAreas();
            
            if (p.actionMode != ActionMode::kScrub &&
                p.actionMode != ActionMode::kText &&
                p.actionMode != ActionMode::kVoice &&
                p.actionMode != ActionMode::kSelection &&
                p.actionMode != ActionMode::kRotate &&
                p.actionMode != ActionMode::kLink &&
                Fl::belowmouse() == this)
            {
                _drawCursor(mvp);
            }
                
            if (p.hudActive && p.hud != HudDisplay::kNone)
                _drawHUD(alpha);
            
            if (!p.helpText.empty())
                _drawHelpText();
            
            math::Box2i selection = p.colorAreaInfo.box = p.selection;
            if (selection.max.x >= 0)
            {
                // Check min < max
                if (selection.min.x > selection.max.x)
                {
                    int tmp = selection.max.x;
                    selection.max.x = selection.min.x;
                    selection.min.x = tmp;
                }
                if (selection.min.y > selection.max.y)
                {
                    int tmp = selection.max.y;
                    selection.max.y = selection.min.y;
                    selection.min.y = tmp;
                }
                // Copy it again in case it changed
                p.colorAreaInfo.box = selection;

                // Copy the pixel type (needed for Vulkan)
                const vlk::OffscreenBufferOptions& options = vk.buffer->getOptions();
                p.colorAreaInfo.pixelType = options.colorType;
            }
            
                    
            if (selection.max.x >= 0)
                _drawAreaSelection();
            
            end_render_pass();

            if (selection.max.x >= 0)
            {
                if (panel::colorAreaPanel || panel::histogramPanel ||
                    panel::vectorscopePanel)
                {
                    _mapBuffer();

                    if (p.image)
                    {
                        if (panel::colorAreaPanel)
                        {
                            _calculateColorArea(p.colorAreaInfo);
                            panel::colorAreaPanel->update(p.colorAreaInfo);
                        }
                        if (panel::histogramPanel)
                        {
                            panel::histogramPanel->update(p.colorAreaInfo);
                        }
                        if (panel::vectorscopePanel)
                        {
                            panel::vectorscopePanel->update(p.colorAreaInfo);
                        }
                    }
                }
            }
            
            // Update the pixel bar from here only if we are playing a movie
            // and one that is not 1 frames long.                
            bool update = !_shouldUpdatePixelBar();
            if (update)
                updatePixelBar();

            if (vk.buffer)
            {
                vk.buffer->transitionToColorAttachment(cmd);
            }
            if (vk.annotation)
            {
                vk.annotation->transitionToColorAttachment(cmd);
            }
        }

        
        void Viewport::_calculateColorAreaFullValues(area::Info& info) noexcept
        {
            TLRENDER_P();
            MRV2_VK();
            if (!p.image)
                return;

            PixelToolBarClass* c = p.ui->uiPixelWindow;
            BrightnessType brightness_type =
                (BrightnessType)c->uiLType->value();
            int hsv_colorspace = c->uiBColorType->value() + 1;

            const uint32_t W = info.box.w();
            const uint32_t H = info.box.h();
            
            const uint8_t* ptr = reinterpret_cast<const uint8_t*>(p.image);            
            const int channelCount = image::getChannelCount(info.pixelType);
            const int byteCount = image::getBitDepth(info.pixelType) / 8;

            const size_t dataSize = W * H;
            
            image::Color4f rgba, hsv;
            
            for (size_t i = 0; i < dataSize; ++i)
            {
                rgba = color::fromVoidPtr(ptr, info.pixelType);

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

                ptr += channelCount * byteCount;
            }

            info.rgba.mean.r /= dataSize;
            info.rgba.mean.g /= dataSize;
            info.rgba.mean.b /= dataSize;
            info.rgba.mean.a /= dataSize;

            info.rgba.diff.r = info.rgba.max.r - info.rgba.min.r;
            info.rgba.diff.g = info.rgba.max.g - info.rgba.min.g;
            info.rgba.diff.b = info.rgba.max.b - info.rgba.min.b;
            info.rgba.diff.a = info.rgba.max.a - info.rgba.min.a;

            info.hsv.mean.r /= dataSize;
            info.hsv.mean.g /= dataSize;
            info.hsv.mean.b /= dataSize;
            info.hsv.mean.a /= dataSize;

            info.hsv.diff.r = info.hsv.max.r - info.hsv.min.r;
            info.hsv.diff.g = info.hsv.max.g - info.hsv.min.g;
            info.hsv.diff.b = info.hsv.max.b - info.hsv.min.b;
            info.hsv.diff.a = info.hsv.max.a - info.hsv.min.a;
        }

        void Viewport::_calculateColorArea(area::Info& info)
        {
            TLRENDER_P();
            MRV2_VK();

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
                        image::Color4f pixel, pixelB;

                        
                        const auto& image = layer.image;
                        if (image && image->isValid())
                        {
                            _getPixelValue(pixel, image, pos);
                        }

                        const auto& imageB = layer.image;
                        if (imageB && imageB->isValid())
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
                    VkCommandBuffer cmd =
                        beginSingleTimeCommands(device(), commandPool());

                    vk.buffer->readPixels(cmd, pos.x, pos.y, 1, 1);

                    vkEndCommandBuffer(cmd);
                    
                    vk.buffer->submitReadback(cmd);

                    wait_queue();
                        
                    vkFreeCommandBuffers(device(), commandPool(), 1, &cmd);

                    const void* data = vk.buffer->getLatestReadPixels();
                    if (!data)
                        return;

                    const auto options = vk.buffer->getOptions();
                    rgba = color::fromVoidPtr(data, options.colorType);
                    return;
                }
            }
        }
        
        void Viewport::_mapBuffer()
        {
            TLRENDER_P();
            MRV2_VK();
            
            if (p.ui->uiPixelWindow->uiPixelValue->value() == PixelValue::kFull)
            {
                    
                const math::Box2i box = p.colorAreaInfo.box;

                const uint32_t W = box.w();
                const uint32_t H = box.h();

                VkCommandBuffer cmd = beginSingleTimeCommands(device(), commandPool());

                vk.buffer->readPixels(cmd, box.min.x, box.min.y, W, H);

                vkEndCommandBuffer(cmd);

                vk.buffer->submitReadback(cmd);

                wait_queue();

                vkFreeCommandBuffers(device(), commandPool(), 1, &cmd);

                if (p.rawImage)
                {
                    free(p.image);
                    p.image = nullptr;
                }
                
                p.image = vk.buffer->getLatestReadPixels();
                if (!p.image)
                {
                    LOG_ERROR("Could not get pixel under mouse");
                    return;
                }
                
                p.rawImage = false;
            }
            else
            {
                _unmapBuffer();
                TimelineViewport::_mapBuffer();
            }
        }

        void Viewport::_unmapBuffer()
        {
            TLRENDER_P();
            if (!p.rawImage)
            {
                p.image = nullptr;
                p.rawImage = true;
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
            else if (dynamic_cast< VKTextShape* >(shape))
            {
                auto s = dynamic_cast< VKTextShape* >(shape);
                value = *s;
            }
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
        
        // m_depth (optionally) -> creates m_renderPass
        void Viewport::prepare_load_render_pass() 
        {
            MRV2_VK();

            if (vk.loadRenderPass != VK_NULL_HANDLE &&
                format() == _oldRenderPassFormat)
            {
                // Format hasn't changed and render pass exists: DO NOTHING.
                return;
            }
            
            bool has_depth = mode() & FL_DEPTH;
            bool has_stencil = mode() & FL_STENCIL;

            VkAttachmentDescription attachments[2];
            attachments[0] = VkAttachmentDescription();
            attachments[0].format = format();
            attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
            attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
            attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            attachments[0].initialLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
            attachments[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR; // Final layout for presentation

            attachments[1] = VkAttachmentDescription();


            VkAttachmentReference color_reference = {};
            color_reference.attachment = 0;
            color_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    
            VkAttachmentReference depth_reference = {};
            depth_reference.attachment = 1;
            depth_reference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    
            VkSubpassDescription subpass = {};
            subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
            subpass.flags = 0;
            subpass.inputAttachmentCount = 0;
            subpass.pInputAttachments = NULL;
            subpass.colorAttachmentCount = 1;
            subpass.pColorAttachments = &color_reference;
            subpass.pResolveAttachments = NULL;

            if (has_depth || has_stencil)
            {
                attachments[1].format = m_depth.format;
                attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
                attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
                attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
                if (has_stencil)
                {
                    attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
                }
                else
                {
                    attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
                }
                attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
                attachments[1].initialLayout =
                    VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
                attachments[1].finalLayout =
                    VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    
                subpass.pDepthStencilAttachment = &depth_reference;
                subpass.preserveAttachmentCount = 0;
                subpass.pPreserveAttachments = NULL;
            }

            VkRenderPassCreateInfo rp_info = {};
            rp_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
            rp_info.pNext = NULL;
            rp_info.attachmentCount = (has_depth || has_stencil) ? 2: 1;
            rp_info.pAttachments = attachments;
            rp_info.subpassCount = 1;
            rp_info.pSubpasses = &subpass;
            rp_info.dependencyCount = 0;
            rp_info.pDependencies = NULL;
                    
            VkResult result;
            result = vkCreateRenderPass(device(), &rp_info, NULL, &vk.loadRenderPass);
            VK_CHECK(result);
        }
    } // namespace vulkan

} // namespace mrv
