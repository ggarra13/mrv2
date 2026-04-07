// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include "mrViewer.h"

#include "mrvApp/mrvSettingsObject.h"

#include "mrvNetwork/mrvTCP.h"

#include "mrvEdit/mrvEditCallbacks.h"
#include "mrvEdit/mrvEditUtil.h"

#include "mrvUI/mrvDesktop.h"

#include "mrvFl/mrvIO.h"

#include "mrvVk/mrvVkShadersBinary.h"
#include "mrvVk/mrvusd_window.h"

#include "mrvCore/mrvFile.h"
#include "mrvCore/mrvHotkey.h"
#include "mrvCore/mrvTimeObject.h"

#include <tlTimelineUI/usd_window.h>

#include <tlTimelineVk/Render.h>
#include <tlTimelineVk/RenderShadersBinary.h>

#include <tlUI/IWindow.h>

#include <tlVk/Init.h>
#include <tlVk/Mesh.h>
#include <tlVk/OffscreenBuffer.h>
#include <tlVk/Shader.h>
#include <tlVk/ShaderBindingSet.h>

#include <FL/Fl_Box.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl.H>
#include <FL/names.h>


namespace mrv
{

    namespace vulkan
    {

        struct usd_window::Private
        {
            std::weak_ptr<system::Context> context;

            ViewerUI* ui = nullptr;
            Fl_Window* topWindow = nullptr;

            
            std::weak_ptr<timelineui_vk::ThumbnailSystem> thumbnailSystem;

            struct ThumbnailData
            {
                timelineui_vk::ThumbnailRequest request;
                std::shared_ptr<image::Image> image;
            };
            ThumbnailData thumbnail;

            timeline::OCIOOptions ocioOptions;
            timeline::LUTOptions lutOptions;

            mrv::TimeUnits units = mrv::TimeUnits::Timecode;

            // Render data
            std::shared_ptr<ui::Style> style;
            std::shared_ptr<image::FontSystem> fontSystem;
            std::shared_ptr<timeline_vlk::Render> render;
            timelineui_vk::DisplayOptions displayOptions;
            std::shared_ptr<timelineui_vk::usd_window> timelineWidget;
            std::shared_ptr<TimelineWindow> timelineWindow;
            std::shared_ptr<tl::vlk::Shader> shader;
            std::shared_ptr<tl::vlk::OffscreenBuffer> buffer;
            std::shared_ptr<vlk::VBO> vbo;
            std::shared_ptr<vlk::VAO> vao;
            std::chrono::steady_clock::time_point mouseWheelTimer;
            VkPipelineLayout pipeline_layout = VK_NULL_HANDLE;

        };

        usd_window::usd_window(
            int X, int Y, int W, int H, const char* L) :
            VkWindow(X, Y, W, H, L),
            _p(new Private)
        {
            int fl_double = FL_DOUBLE;
            mode(FL_RGB | FL_ALPHA | fl_double);
            // m_debugSync = true;
        }

        void usd_window::init_colorspace()
        {
            Fl_Vk_Window::init_colorspace();

            colorSpace() = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
            format() = VK_FORMAT_B8G8R8A8_UNORM;
        }
        
        void usd_window::setContext(
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<timeline::TimeUnitsModel>& timeUnitsModel,
            ViewerUI* ui)
        {
            TLRENDER_P();

            p.context = context;
        }

        void usd_window::setStyle(const std::shared_ptr<ui::Style>& style)
        {
        }

        usd_window::~usd_window()
        {
        }
        


        void usd_window::prepare_shaders()
        {
            TLRENDER_P();

            if (auto context = p.context.lock())
            {
                try
                {
                    if (!p.render)
                        p.render = timeline_vlk::Render::create(ctx, context);

                    if (!p.shader)
                    {
                        p.shader = vlk::Shader::create(
                            ctx,
                            timeline_vlk::Vertex2_spv,
                            timeline_vlk::Vertex2_spv_len,
                            textureFragment_spv,
                            textureFragment_spv_len,
                            "timeline p.shader");
                        math::Matrix4x4f pm;
                        p.shader->createUniform(
                            "transform.mvp", pm, vlk::kShaderVertex);
                        p.shader->addFBO("textureSampler");
                        p.shader->addPush("opacity", 1.0, vlk::kShaderFragment);
                        p.shader->createBindingSet();
                    }
                }
                catch (const std::exception& e)
                {
                    context->log(
                        "mrv::mrvusd_window", e.what(), log::Type::Error);
                }
            }
        }

        void usd_window::prepare_mesh()
        {
            TLRENDER_P();

            const math::Size2i renderSize(pixel_w(), pixel_h());
            
            const auto& mesh =
                geom::box(math::Box2i(0, 0, renderSize.w, renderSize.h));
            if (!p.vbo)
            {
                p.vbo = vlk::VBO::create(
                    mesh.triangles.size() * 3, vlk::VBOType::Pos2_F32_UV_U16);
                p.vao.reset();
            }
            if (p.vbo)
            {
                p.vbo->copy(convert(mesh, vlk::VBOType::Pos2_F32_UV_U16));
            }

            if (!p.vao && p.vbo)
            {
                p.vao = vlk::VAO::create(ctx);
            }
        }
        
        void usd_window::prepare()
        {
            TLRENDER_P();
            
            prepare_shaders();
            prepare_mesh();
            prepare_render_pass();
            prepare_pipeline_layout(); // Main shader layout
            prepare_pipeline();
        }

        void usd_window::destroy()
        {
            TLRENDER_P();
            
            p.buffer.reset();

            if (p.pipeline_layout != VK_NULL_HANDLE)
            {
                vkDestroyPipelineLayout(device(), p.pipeline_layout, nullptr);
                p.pipeline_layout = VK_NULL_HANDLE;
            }

            if (pipeline() != VK_NULL_HANDLE)
            {
                vkDestroyPipeline(device(), pipeline(), nullptr);
                pipeline() = VK_NULL_HANDLE;
            }

            p.vbo.reset();
            p.vao.reset();
        }

        void usd_window::hide()
        {
            TLRENDER_P();

            wait_device();
            
            // Destroy main render
            p.render.reset();

            // Destroy buffers
            p.buffer.reset();

            // Destroy shaders
            p.shader.reset();

            // Destroy meshes
            p.vbo.reset();
            p.vao.reset();

            VkWindow::hide();
        }

        void usd_window::draw()
        {
            TLRENDER_P();
            
            const math::Size2i renderSize(pixel_w(), pixel_h());
            
            VkCommandBuffer cmd = getCurrentCommandBuffer();

            if (renderSize.isValid())
            {
                vlk::OffscreenBufferOptions offscreenBufferOptions;
                offscreenBufferOptions.colorType =
                    image::PixelType::RGBA_U8;
                offscreenBufferOptions.clear = true;
                offscreenBufferOptions.clearColor =
                    p.style->getColorRole(ui::ColorRole::Window);
                if (vlk::doCreate(
                        p.buffer, renderSize, offscreenBufferOptions))
                { 
                    p.buffer = vlk::OffscreenBuffer::create(
                        ctx, renderSize, offscreenBufferOptions);
                }
            }
            else
            {
                wait_device();
                        
                p.buffer.reset();
            }

            if (p.render && p.buffer)
            {
                p.buffer->transitionToColorAttachment(cmd);
                        
                timeline::RenderOptions renderOptions;
                renderOptions.clear = true;
                renderOptions.clearColor =
                    p.style->getColorRole(ui::ColorRole::Window);

                // Clear color in new render pass.
                p.render->begin(
                    cmd, p.buffer, m_currentFrameIndex, renderSize,
                    renderOptions);
                const math::Matrix4x4f ortho = math::ortho(
                    0.F, static_cast<float>(renderSize.w),
                    static_cast<float>(renderSize.h), 0.F,
                    -1.F, 1.F);
                p.render->setTransform(ortho);

                const auto& ocioOptions = p.ui->uiView->getOCIOOptions();
                p.render->setOCIOOptions(ocioOptions);
                        
                const auto& lutOptions = p.ui->uiView->lutOptions();
                p.render->setLUTOptions(lutOptions);
                        
                p.render->setClipRectEnabled(true);
                        
                ui::DrawEvent drawEvent(
                    p.style, p.render, p.fontSystem);
                p.render->beginLoadRenderPass();
                _drawEvent(
                    p.timelineWindow,
                    math::Box2i(0, 0, renderSize.w, renderSize.h),
                    drawEvent);
                p.render->endRenderPass();
                        
                p.render->setClipRectEnabled(false);
                p.render->end();
            }
        
        }

        p.buffer->transitionToShaderRead(cmd);
                        
        begin_render_pass(cmd);
    
        p.shader->bind(m_currentFrameIndex);
        const auto pm = math::ortho(
            0.F, static_cast<float>(renderSize.w),
            0.F, static_cast<float>(renderSize.h), -1.F, 1.F);
        p.shader->setUniform("transform.mvp", pm, vlk::kShaderVertex);
        p.shader->setFBO("textureSampler", p.buffer);


        // Bind the main composition pipeline (created/managed outside
        // this draw loop)
        vkCmdBindPipeline(
            cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline());
                
        VkDescriptorSet descriptorSet = p.shader->getDescriptorSet();
        vkCmdBindDescriptorSets(
            cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, p.pipeline_layout, 0,
            1, &descriptorSet, 0, nullptr);
                
        const float opacity = 1.0;
        vkCmdPushConstants(cmd, p.pipeline_layout,
                           p.shader->getPushStageFlags(), 0,
                           sizeof(float), &opacity);

        VkViewport viewport = {};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(pixel_w());
        viewport.height = static_cast<float>(pixel_h());
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(cmd, 0, 1, &viewport);

        VkRect2D scissor = {};
        scissor.offset = {0, 0};
        scissor.extent.width = pixel_w();
        scissor.extent.height = pixel_h();
        vkCmdSetScissor(cmd, 0, 1, &scissor);

        if (p.vao && p.vbo)
        {
            const VkColorComponentFlags allMask[] =
                { VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                  VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT };
            ctx.vkCmdSetColorWriteMaskEXT(cmd, 0, 1, allMask);
            
            p.vao->bind(m_currentFrameIndex);
            p.vao->draw(cmd, p.vbo);
        }
    }

    void usd_window::prepare_pipeline_layout()
    {
        TLRENDER_P();

        VkResult result;

        VkPipelineLayoutCreateInfo pPipelineLayoutCreateInfo = {};
        pPipelineLayoutCreateInfo.sType =
            VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pPipelineLayoutCreateInfo.pNext = NULL;
        pPipelineLayoutCreateInfo.setLayoutCount = 1;
        VkDescriptorSetLayout setLayout = p.shader->getDescriptorSetLayout();
        pPipelineLayoutCreateInfo.pSetLayouts = &setLayout;

        VkPushConstantRange pushConstantRange = {};
            std::size_t pushSize = p.shader->getPushSize();
            if (pushSize > 0)
            {
                pushConstantRange.stageFlags = p.shader->getPushStageFlags();
                pushConstantRange.offset = 0;
                pushConstantRange.size = pushSize;

                pPipelineLayoutCreateInfo.pushConstantRangeCount = 1;
                pPipelineLayoutCreateInfo.pPushConstantRanges =
                    &pushConstantRange;
            }

            result = vkCreatePipelineLayout(
                device(), &pPipelineLayoutCreateInfo, NULL, &p.pipeline_layout);
        }

        void usd_window::prepare_pipeline()
        {
            TLRENDER_P();
            
            // Elements of new Pipeline (fill with mesh info)
            vlk::VertexInputStateInfo vi;
            vi.bindingDescriptions = p.vbo->getBindingDescription();
            vi.attributeDescriptions = p.vbo->getAttributes();
            
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
            
            vlk::DepthStencilStateInfo ds;
            ds.depthTestEnable = VK_FALSE;
            ds.depthWriteEnable = VK_FALSE;
            ds.stencilTestEnable = VK_FALSE;
            
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
            shaderStages[0].name = p.shader->getName();
            shaderStages[0].module = p.shader->getVertex();
            shaderStages[0].entryPoint = "main";

            shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
            shaderStages[1].name = p.shader->getName();
            shaderStages[1].module = p.shader->getFragment();
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
            pipelineState.layout = p.pipeline_layout;

            pipeline() = pipelineState.create(device());
            if (pipeline() == VK_NULL_HANDLE)
            {
                throw std::runtime_error("Composition pipeline failed");
            }
        }


    } // namespace vulkan

} // namespace mrv
