
#include <tlTimelineVk/RenderPrivate.h>

#include <string>

#define DEBUG_PIPELINE_USE 0

namespace tl
{
    namespace timeline_vlk
    {
        void Render::_createBindingSet(const std::shared_ptr<vlk::Shader>& shader)
        {
            TLRENDER_P();
            const auto bindingSet = shader->createBindingSet();
            shader->useBindingSet(bindingSet);
            p.garbage[p.frameIndex].bindingSets.push_back(bindingSet);
        }
        
        
        VkPipelineLayout Render::_createPipelineLayout(
            const std::string& pipelineLayoutName,
            const std::shared_ptr<vlk::Shader> shader)
        {
            TLRENDER_P();
            
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
            
            VkPipelineLayout pipelineLayout;
            VkResult result = vkCreatePipelineLayout(ctx.device, &pPipelineLayoutCreateInfo, NULL, &pipelineLayout);
            VK_CHECK(result);
            p.pipelineLayouts[pipelineLayoutName] = pipelineLayout;
            return pipelineLayout;
        }
        
        void Render::createPipeline(const std::string& pipelineName,
                                    const std::string& pipelineLayoutName,
                                    const VkRenderPass renderPass,
                                    const std::shared_ptr<vlk::Shader>& shader,
                                    const std::shared_ptr<vlk::VBO>& mesh,
                                    const vlk::ColorBlendStateInfo& cb,
                                    const vlk::DepthStencilStateInfo& ds,
                                    const vlk::MultisampleStateInfo& ms)
        {
            
            TLRENDER_P();            
            if (!shader)
                throw std::runtime_error(
                    "createPipeline failed with unknown shader '" +
                    shader->getName() + "'");

            if (!mesh)
                throw std::runtime_error(
                    "createPipeline failed with unknown mesh");

            if (renderPass == VK_NULL_HANDLE)
                throw std::runtime_error(
                    "createPipeline failed with renderPass == VK_NULL_HANDLE");
                

            
            VkPipelineLayout pipelineLayout = p.pipelineLayouts[pipelineLayoutName];
            if (!pipelineLayout)
            {
#if DEBUG_PIPELINE_USE
                std::cerr << "CREATING   pipelineLayout " << pipelineLayoutName << std::endl;
#endif
                pipelineLayout = _createPipelineLayout(pipelineLayoutName,
                                                       shader);
            }
            else
            {
#if DEBUG_PIPELINE_USE
                std::cerr << "REUSING    pipelineLayout " << pipelineLayoutName << std::endl;
#endif
            }
            
            if (pipelineLayout == VK_NULL_HANDLE)
                throw std::runtime_error(
                    "createPipeline failed with pipelineLayout == VK_NULL_HANDLE");

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
                VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};

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
#if DEBUG_PIPELINE_USE
                std::cerr << "CREATING   pipeline " << pipelineName << std::endl;
#endif
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
#if DEBUG_PIPELINE_USE
                    std::cerr << "RECREATING pipeline " << pipelineName << std::endl;
#endif
                    p.garbage[p.frameIndex].pipelines.push_back(
                        oldPipeline);
                    pipeline = pipelineState.create(device);
                    auto pair = std::make_pair(pipelineState, pipeline);
                    p.pipelines[pipelineName] = pair;
                }
                else
                {
#if DEBUG_PIPELINE_USE
                    std::cerr << "REUSING    pipeline " << pipelineName << std::endl;
#endif
                    pipeline = pair.second;
                }
            }

            // Enable the pipeline.
            vkCmdBindPipeline(p.cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
        }
        
        void Render::createPipeline(
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
            const VkBlendOp alphaBlendOp)
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
            cb.attachments.push_back(colorBlendAttachment);
            
            vlk::DepthStencilStateInfo ds;
            ds.depthTestEnable = fbo->hasDepth() ? VK_TRUE : VK_FALSE;
            ds.depthWriteEnable = fbo->hasDepth() ? VK_TRUE : VK_FALSE;
            ds.stencilTestEnable = fbo->hasStencil() ? VK_TRUE : VK_FALSE;
            
            vlk::MultisampleStateInfo ms;
            ms.rasterizationSamples = fbo->getSampleCount();
            
            createPipeline(pipelineName, pipelineLayoutName, fbo->getRenderPass(),
                           shader, mesh, cb, ds, ms);
            
            fbo->setupViewportAndScissor(p.cmd);
            if (p.clipRectEnabled)
            {
                setClipRect(p.clipRect);
            }
        }
        
        void Render::_setViewportAndScissor(const math::Size2i& viewportSize)
        {
            TLRENDER_P();
            
            VkViewport viewport = {};
            viewport.x =  0.F;
            viewport.y = static_cast<float>(viewportSize.h);
            viewport.width = static_cast<float>(viewportSize.w);
            viewport.height = -static_cast<float>(viewportSize.h);
            viewport.minDepth = 0.0f;
            viewport.maxDepth = 1.0f;
            vkCmdSetViewport(p.cmd, 0, 1, &viewport);

            VkRect2D scissor = {};
            scissor.extent.width = viewportSize.w;
            scissor.extent.height = viewportSize.h;
            vkCmdSetScissor(p.cmd, 0, 1, &scissor);
        }

        void Render::_bindDescriptorSets(
            const std::string& pipelineLayoutName, const std::string& shaderName)
        {
            TLRENDER_P();
            if (!p.shaders[shaderName])
            {
                throw std::runtime_error("Undefined shader " + shaderName);
            }
            if (!p.pipelineLayouts[pipelineLayoutName])
            {
                throw std::runtime_error(
                    "Undefined pipelineLayout " + pipelineLayoutName);
            }
            VkDescriptorSet descriptorSet = p.shaders[shaderName]->getDescriptorSet();
            
            vkCmdBindDescriptorSets(
                p.cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
                p.pipelineLayouts[pipelineLayoutName], 0, 1,
                &descriptorSet, 0, nullptr);
        }
        
        void Render::_vkDraw(const std::string& meshName)
        {
            TLRENDER_P();

            if (!p.vaos[meshName])
                throw std::runtime_error("p.vaos[" + meshName + "] not created");
            if (!p.vbos[meshName])
                throw std::runtime_error("p.vbos[" + meshName + "] not created");
            
            p.vaos[meshName]->bind(p.frameIndex);
            p.vaos[meshName]->draw(p.cmd, p.vbos[meshName]);
            p.garbage[p.frameIndex].vaos.push_back(p.vaos[meshName]);
        }

        VkRenderPass Render::getRenderPass() const
        {
            TLRENDER_P();
            
            if (p.fbo) return p.fbo->getRenderPass();
            return VK_NULL_HANDLE;
        }
    }
}
