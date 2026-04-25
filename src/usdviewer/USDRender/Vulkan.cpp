// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2025-Present Gonzalo Garramuño
// All rights reserved.

#include "USDTextureSlots.h"

#include "USDRender/Private.h"

#include <FL/vk_enum_string_helper.h>

#include <iostream>
#include <string>

#define USE_DEPTH 1   // \@bug: turning this to one AND drawing to accum/reveal
                      // results in VK_ERROR_DEVICE_LOST

#if DEBUG_PIPELINE_USE
#define DEBUG_PIPELINE(x) std::cerr << x << std::endl;
#else
#define DEBUG_PIPELINE(x)
#endif

#if DEBUG_PIPELINE_LAYOUT_USE
#define DEBUG_PIPELINE_LAYOUT(x) std::cerr << x << std::endl;
#else
#define DEBUG_PIPELINE_LAYOUT(x)
#endif

namespace tl
{
    namespace usd
    {
        void Render::_createBindingSet(const std::shared_ptr<vlk::Shader>& shader)
        {
            TLRENDER_P();
            
            auto bindingSet = shader->createBindingSet();
            p.garbage[p.frameIndex].bindingSets.push_back(bindingSet);
        }
        
        
        VkPipelineLayout Render::_createPipelineLayout(
            const std::string& pipelineLayoutName,
            const std::shared_ptr<vlk::Shader> shader)
        {
            TLRENDER_P();

            VkPipelineLayout pipelineLayout = p.pipelineLayouts[pipelineLayoutName];
            if (pipelineLayout)
            {
                p.garbage[p.frameIndex].pipelineLayouts.push_back(pipelineLayout);
            }
            
            VkPipelineLayoutCreateInfo pPipelineLayoutCreateInfo = {};
            pPipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
            pPipelineLayoutCreateInfo.pNext = NULL;
            pPipelineLayoutCreateInfo.setLayoutCount = 1;
            VkDescriptorSetLayout setLayout = shader->getDescriptorSetLayout(); // Get layout from shader
            pPipelineLayoutCreateInfo.pSetLayouts = &setLayout;

            VkPushConstantRange pushConstantRange = {};
            std::size_t pushSize = shader->getPushSize();
            if (pushSize > 0)
            {
                pushConstantRange.stageFlags = shader->getPushStageFlags();
                pushConstantRange.offset = 0;
                pushConstantRange.size = pushSize;
                pPipelineLayoutCreateInfo.pushConstantRangeCount = 1;
                pPipelineLayoutCreateInfo.pPushConstantRanges = &pushConstantRange;
            }
            
            VkResult result = vkCreatePipelineLayout(ctx.device, &pPipelineLayoutCreateInfo, NULL, &pipelineLayout);
            VK_CHECK(result);
            
            p.pipelineLayouts[pipelineLayoutName] = pipelineLayout;
            return pipelineLayout;
        }
        
        void Render::_createPipeline(const std::string& pipelineName,
                                     const std::string& pipelineLayoutName,
                                     const VkRenderPass renderPass,
                                     const std::shared_ptr<vlk::Shader>& shader,
                                     const std::shared_ptr<vlk::VBO>& mesh,
                                     const vlk::ColorBlendStateInfo& cb,
                                     const vlk::DepthStencilStateInfo& ds,
                                     const vlk::MultisampleStateInfo& ms)
        {
            TLRENDER_P();
            
            VkPipelineLayout pipelineLayout = p.pipelineLayouts[pipelineLayoutName];
            if (!pipelineLayout)
            {
                DEBUG_PIPELINE_LAYOUT("CREATING   pipelineLayout " << pipelineLayoutName);
                pipelineLayout = _createPipelineLayout(pipelineLayoutName,
                                                       shader);
            }

            VkDevice device = ctx.device;
            
            // Elements of new Pipeline (fill with mesh info)
            vlk::VertexInputStateInfo vi;
            vi.bindingDescriptions = mesh->getBindingDescription();
            vi.attributeDescriptions = mesh->getAttributes();

            // Defaults are fine
            vlk::InputAssemblyStateInfo ia;

            // Defaults are fine
            vlk::RasterizationStateInfo rs;

            // Defaults are fine
            vlk::ViewportStateInfo vp;

            // Defaults are fine
            vlk::DynamicStateInfo dynamicState;
            dynamicState.dynamicStates = {
                VK_DYNAMIC_STATE_VIEWPORT,
                VK_DYNAMIC_STATE_SCISSOR,
                
#if USE_DYNAMIC_RGBA_WRITE_MASKS
                // For dynamic R/G/B/A masks
                VK_DYNAMIC_STATE_COLOR_WRITE_MASK_EXT,
#endif

                // For dynamic stencils
#if USE_DYNAMIC_STENCILS
                VK_DYNAMIC_STATE_STENCIL_TEST_ENABLE_EXT,
                VK_DYNAMIC_STATE_STENCIL_COMPARE_MASK,
                VK_DYNAMIC_STATE_STENCIL_WRITE_MASK,
                VK_DYNAMIC_STATE_STENCIL_REFERENCE,
                VK_DYNAMIC_STATE_STENCIL_OP_EXT
#endif
            };

            // Get the vertex and fragment shaders
            std::vector<vlk::PipelineCreationState::ShaderStageInfo>
                shaderStages(2);

            shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
            shaderStages[0].name = shader->getName();
            shaderStages[0].module = shader->getVertex();
            shaderStages[0].entryPoint = "main";

            shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
            shaderStages[1].name = shader->getName();
            shaderStages[1].module = shader->getFragment();
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
            pipelineState.renderPass = renderPass;
            pipelineState.layout = pipelineLayout;

            VkPipeline pipeline;
            if (p.pipelines.count(pipelineName) == 0)
            {
                DEBUG_PIPELINE("CREATING   pipeline " << pipelineName);
                pipeline = pipelineState.create(device);
                p.pipelines[pipelineName] = std::make_pair(pipelineState,
                                                           pipeline);
            }
            else
            {
                const auto& pair = p.pipelines[pipelineName];
                const auto& oldPipelineState = pair.first;
                VkPipeline oldPipeline = pair.second;
                if (pipelineState != oldPipelineState)
                {
                    DEBUG_PIPELINE("RECREATING pipeline " << pipelineName);
                    p.garbage[p.frameIndex].pipelines.push_back(
                        oldPipeline);
                    pipeline = pipelineState.create(device);
                    auto pair = std::make_pair(pipelineState, pipeline);
                    p.pipelines[pipelineName] = pair;
                }
                else
                {
                    pipeline = pair.second;
                }
            }

            // Enable the pipeline.
            vkCmdBindPipeline(p.cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
            p.currentPipeline = pipelineName;
        }
        
        void Render::_createPipeline(
            const std::shared_ptr<vlk::OffscreenBuffer>& fbo,
            const std::string& pipelineName,
            const std::string& pipelineLayoutName,
            const std::string& shaderName,
            const std::string& meshName,
            const bool enableBlending,
            const VkBlendFactor srcColorBlendFactor,
            const VkBlendFactor dstColorBlendFactor,
            const VkBlendFactor srcAlphaBlendFactor,
            const VkBlendFactor dstAlphaBlendFactor,
            const VkBlendOp colorBlendOp,
            const VkBlendOp alphaBlendOp,
            const VkBool32 depthTest,
            const VkBool32 depthWrite)
        {
            TLRENDER_P();

            const auto& shader = p.shaders[shaderName];
            const auto& mesh = p.vbos[meshName];
            
            vlk::ColorBlendStateInfo cb;
            vlk::ColorBlendAttachmentStateInfo colorBlendAttachment;
            if (enableBlending)
            {
                colorBlendAttachment.blendEnable = VK_TRUE;
                colorBlendAttachment.srcColorBlendFactor = srcColorBlendFactor;
                colorBlendAttachment.dstColorBlendFactor = dstColorBlendFactor;
                colorBlendAttachment.srcAlphaBlendFactor = srcAlphaBlendFactor;
                colorBlendAttachment.dstAlphaBlendFactor = dstAlphaBlendFactor;
                colorBlendAttachment.colorBlendOp = colorBlendOp;
                colorBlendAttachment.alphaBlendOp = alphaBlendOp;
            }
            else
            {
                colorBlendAttachment.blendEnable = VK_FALSE;
            }
            cb.attachments.push_back(colorBlendAttachment);
            
            vlk::DepthStencilStateInfo ds;
            ds.depthTestEnable = fbo->hasDepth() ? depthTest : VK_FALSE;
            ds.depthWriteEnable = fbo->hasDepth() ? depthWrite : VK_FALSE;
            ds.stencilTestEnable = fbo->hasStencil() ? VK_TRUE : VK_FALSE;
            
            vlk::MultisampleStateInfo ms;
            ms.rasterizationSamples = fbo->getSampleCount();

            _createPipeline(pipelineName, pipelineLayoutName,
                            fbo->getLoadRenderPass(), shader, mesh, cb, ds, ms);
            
            fbo->setupViewportAndScissor(p.cmd);
            if (p.clipRectEnabled)
            {
                setClipRect(p.clipRect);
            }
        }

        void Render::_usePipeline(const std::string& pipelineName)
        {
            TLRENDER_P();

            if (p.currentPipeline == pipelineName)
                return;
            
            const auto& pair = p.pipelines[pipelineName];
            VkPipeline pipeline = pair.second;

            // Enable the pipeline.
            vkCmdBindPipeline(p.cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
            p.currentPipeline = pipelineName;
        }
        
        void Render::_bindDescriptorSets(
            const std::string& pipelineLayoutName, const std::string& shaderName)
        {
            TLRENDER_P();
            
            VkDescriptorSet descriptorSet = p.shaders[shaderName]->getDescriptorSet();
            
            vkCmdBindDescriptorSets(
                p.cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
                p.pipelineLayouts[pipelineLayoutName], 0, 1,
                &descriptorSet, 0, nullptr);
        }        
        
        void Render::createOIT()
        {
            TLRENDER_P();

            VkDevice device = ctx.device;

            // Only create the render pass once (it doesn't depend on frame size, 
            // only on formats which don't change).
            if (p.oitRenderPass == VK_NULL_HANDLE)
            {

                std::vector<VkAttachmentDescription> attachments;

                // Accum attachment
                VkAttachmentDescription attachment = {};
                attachment.format = p.accum[p.frameIndex]->getInternalFormat();
                attachment.samples = VK_SAMPLE_COUNT_1_BIT;
                attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
                attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
                attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
                attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
                attachment.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                attachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                attachments.push_back(attachment);

                // Reveal attachment
                attachment.format = p.reveal[p.frameIndex]->getInternalFormat();
                attachment.samples = VK_SAMPLE_COUNT_1_BIT;
                attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
                attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
                attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
                attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
                attachment.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                attachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                attachments.push_back(attachment);

#if USE_DEPTH
                
                // We reuse depth from opaque pass (in p.fbo).
                attachment.format = p.fbo->getDepthFormat();
                attachment.samples = p.fbo->getSampleCount();
                attachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;   // keep depth!
                attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
                attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
                attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
                attachment.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
                attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
                attachments.push_back(attachment);
#endif
                
                std::vector<VkAttachmentReference> colorRefs;

                VkAttachmentReference colorRef = {};
         
                colorRef.attachment = colorRefs.size();
                colorRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                colorRefs.push_back(colorRef);

                colorRef.attachment = colorRefs.size();
                colorRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                colorRefs.push_back(colorRef);

#if USE_DEPTH
                VkAttachmentReference depthRef = {};
                depthRef.attachment = colorRefs.size();
                depthRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
#endif
                VkSubpassDescription subpass = {};
                subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
                subpass.colorAttachmentCount = colorRefs.size();
                subpass.pColorAttachments = colorRefs.data();
#if USE_DEPTH
                subpass.pDepthStencilAttachment = &depthRef;
#endif

                // Comprehensive dual dependencies to prevent layout transition races
                std::vector<VkSubpassDependency> dependencies(2);

                // External → Subpass 0 (opaque → OIT)
                dependencies[0].srcSubpass      = VK_SUBPASS_EXTERNAL;
                dependencies[0].dstSubpass      = 0;
                dependencies[0].srcStageMask    = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
                dependencies[0].srcAccessMask   = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
                dependencies[0].dstStageMask    = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
                dependencies[0].dstAccessMask   = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT |
                                                  VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

               // Subpass 0 → External (for later shader read of accum/reveal)
                dependencies[1].srcSubpass      = 0;
                dependencies[1].dstSubpass      = VK_SUBPASS_EXTERNAL;
                dependencies[1].srcStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
                dependencies[1].srcAccessMask   = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
                dependencies[1].dstStageMask    = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
                dependencies[1].dstAccessMask   = VK_ACCESS_SHADER_READ_BIT;

                VkRenderPassCreateInfo rpInfo = {};
                rpInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
                rpInfo.attachmentCount = attachments.size();
                rpInfo.pAttachments = attachments.data();
                rpInfo.subpassCount = 1;
                rpInfo.pSubpasses = &subpass;
                rpInfo.dependencyCount = dependencies.size();
                rpInfo.pDependencies = dependencies.data();

                VK_CHECK(vkCreateRenderPass(device, &rpInfo, nullptr,
                                            &p.oitRenderPass));

                p.renderPass = p.oitRenderPass;
            }

            // Recreate the framebuffer only when the size or attachments change.
            // Wait for the slot to be free (your existing fence/semaphore scheme 
            // guarantees this at frame-begin, so it is safe to destroy here).
            if (p.oitFramebuffer[p.frameIndex] != VK_NULL_HANDLE)
            {
                vkDestroyFramebuffer(device, p.oitFramebuffer[p.frameIndex], nullptr);
                p.oitFramebuffer[p.frameIndex] = VK_NULL_HANDLE;
            }


#if USE_DEPTH
            p.fbo->transitionDepthToStencilAttachment(p.cmd);
#endif
            
            p.accum[p.frameIndex]->transitionToColorAttachment(p.cmd);
            p.reveal[p.frameIndex]->transitionToColorAttachment(p.cmd);

            std::vector<VkImageView> views;
            views.push_back(p.accum[p.frameIndex]->getImageView());
            views.push_back(p.reveal[p.frameIndex]->getImageView());

#if USE_DEPTH
            views.push_back(p.fbo->getDepthImageView());
#endif

            VkFramebufferCreateInfo fbInfo = {};
            fbInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            fbInfo.renderPass = p.oitRenderPass;
            fbInfo.attachmentCount = views.size();
            fbInfo.pAttachments = views.data();
            fbInfo.width = p.fbo->getWidth();
            fbInfo.height = p.fbo->getHeight();
            fbInfo.layers = 1;

            VK_CHECK(vkCreateFramebuffer(device, &fbInfo, nullptr,
                                         &p.oitFramebuffer[p.frameIndex]));
        }

        void Render::beginOITRenderPass()
        {
            TLRENDER_P();
            
            std::vector<VkClearValue> clears;

            VkClearValue clear = {};
            
            // Accum = 0
            clear.color = {0.f, 0.f, 0.f, 0.f};
            clears.push_back(clear);

            // Reveal = 1
            clear.color = {1.f, 0.f, 0.f, 0.f};
            clears.push_back(clear);

#if USE_DEPTH
            // Depth = don't care (we LOAD it)
            clear.color = {1.f, 0.f, 0.f, 0.f};
            clear.depthStencil = {1.0f, 0};
            clears.push_back(clear);
#endif
            
            VkRenderPassBeginInfo beginInfo = {};
            beginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            beginInfo.renderPass = p.oitRenderPass;
            beginInfo.framebuffer = p.oitFramebuffer[p.frameIndex];
            beginInfo.renderArea.extent = {
                static_cast<uint32_t>(p.fbo->getWidth()),
                static_cast<uint32_t>(p.fbo->getHeight())
            };
            beginInfo.clearValueCount = clears.size();
            beginInfo.pClearValues = clears.data();

            vkCmdBeginRenderPass(p.cmd, &beginInfo, VK_SUBPASS_CONTENTS_INLINE);

            VkViewport viewport = {};
            viewport.x = 0.0f;
            // Set the y origin to the bottom of the framebuffer
            viewport.y = static_cast<float>(p.fbo->getHeight());
            viewport.width = static_cast<float>(p.fbo->getWidth());
            // Use a negative height to flip the y-axis
            viewport.height = -static_cast<float>(p.fbo->getHeight());
            viewport.minDepth = 0.0f;
            viewport.maxDepth = 1.0f;

            VkRect2D scissor = {};
            scissor = {};
            scissor.offset = {0, 0};
            scissor.extent = {
                static_cast<uint32_t>(p.fbo->getWidth()),
                static_cast<uint32_t>(p.fbo->getHeight())};
            vkCmdSetViewport(p.cmd, 0, 1, &viewport);
            vkCmdSetScissor(p.cmd, 0, 1, &scissor);
        }
        
        VkRenderPass Render::getRenderPass() const
        {
            return _p->renderPass;
        }

        void Render::setRenderPass(VkRenderPass value)
        {
            _p->renderPass = value;
        }
        
        void Render::drawMeshOIT(const geom::TriangleMesh3& geom,
                                 const usd::MeshOptimization& meshOptimization,
                                 const math::Matrix4x4f& model,
                                 const image::Color4f& color,
                                 const std::string& shaderId,
                                 const std::unordered_map<int, std::shared_ptr<vlk::Texture> >& textures,
                                 const Material& material)
        {
            TLRENDER_P();

            std::string pipelineName;
            std::string pipelineLayoutName;
            std::string shaderName = "usd_oit";
            const std::string meshName = "TransparentMeshes";
                
            pipelineName = "oit_blending";
            pipelineName += "_depth_test";
            pipelineName += "_no_depth_write";
            
            _create3DMesh(meshName, geom, meshOptimization);
            
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
                
            i = textures.find(USD_OpacityMap) ;
            p.shaders[shaderName]->setTexture("u_OpacityMap", i->second);
                
            i = textures.find(USD_OpacityThresholdMap);
            p.shaders[shaderName]->setTexture("u_OpacityThresholdMap", i->second);
                
            i = textures.find(USD_IorMap);
            p.shaders[shaderName]->setTexture("u_IorMap", i->second);
                
            vlk::ColorBlendStateInfo cb;
            
            vlk::ColorBlendAttachmentStateInfo accumBlend;
            accumBlend.blendEnable = VK_TRUE;
            accumBlend.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
            accumBlend.dstColorBlendFactor = VK_BLEND_FACTOR_ONE;
            accumBlend.colorBlendOp = VK_BLEND_OP_ADD;

            accumBlend.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
            accumBlend.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
            accumBlend.alphaBlendOp = VK_BLEND_OP_ADD;

            accumBlend.colorWriteMask =
                VK_COLOR_COMPONENT_R_BIT |
                VK_COLOR_COMPONENT_G_BIT |
                VK_COLOR_COMPONENT_B_BIT |
                VK_COLOR_COMPONENT_A_BIT;
            cb.attachments.push_back(accumBlend);

            vlk::ColorBlendAttachmentStateInfo revealBlend;
            revealBlend.blendEnable = VK_TRUE;
            revealBlend.srcColorBlendFactor = VK_BLEND_FACTOR_ZERO;
            revealBlend.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;  // safer according to ChatGPT
            // revealBlend.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
            revealBlend.colorBlendOp = VK_BLEND_OP_ADD;

            revealBlend.srcAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
            revealBlend.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
            revealBlend.alphaBlendOp = VK_BLEND_OP_ADD;
            
            revealBlend.colorWriteMask = VK_COLOR_COMPONENT_R_BIT;

            cb.attachments.push_back(revealBlend);
            
            cb.logicOpEnable = VK_FALSE;
            cb.logicOp = VK_LOGIC_OP_COPY;
            
            cb.blendConstants[0] = 0.0f;
            cb.blendConstants[1] = 0.0f;
            cb.blendConstants[2] = 0.0f;
            cb.blendConstants[3] = 0.0f;
                
            vlk::DepthStencilStateInfo ds;
            ds.depthTestEnable = VK_TRUE;
            ds.depthWriteEnable = VK_FALSE;
            ds.stencilTestEnable = VK_FALSE;
            
            vlk::MultisampleStateInfo ms;
            ms.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

            auto shader = p.shaders["usd_oit"];
            auto mesh = p.vbos[meshName];

            _createPipeline(pipelineName, pipelineLayoutName,
                            p.oitRenderPass, shader, mesh, cb, ds, ms);
            
            
            const auto mvp = p.transform * model;
            _emitMeshDraw(pipelineLayoutName, shaderName, meshName, mvp,
                          model, color);
        }

        void Render::endOITRenderPass()
        {
            TLRENDER_P();
            vkCmdEndRenderPass(p.cmd);
    
            // Tell the C++ tracking what the render pass did for us automatically.
            // This prevents transition() from inserting a wrong barrier next frame.
            p.accum[p.frameIndex]->setCurrentLayout(
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
            p.reveal[p.frameIndex]->setCurrentLayout(
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
            p.fbo->setDepthLayout(VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
        }

        
    }
}
