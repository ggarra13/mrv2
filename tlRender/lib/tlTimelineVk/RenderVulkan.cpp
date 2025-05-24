
#include <tlTimelineVk/RenderPrivate.h>

#include <iostream>
#include <string>

#define DEBUG_PIPELINE_USE 0
#define DEBUG_PIPELINE_LAYOUT_USE 0

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
    namespace timeline_vlk
    {
        void Render::_createBindingSet(const std::shared_ptr<vlk::Shader>& shader)
        {
            TLRENDER_P();
            if (!shader)
                throw std::runtime_error("_createBindingSet shader nullptr");
            auto bindingSet = shader->createBindingSet();
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
            
            VkPipelineLayout pipelineLayout = p.pipelineLayouts[pipelineLayoutName];
            if (!pipelineLayout)
            {
                DEBUG_PIPELINE_LAYOUT("CREATING   pipelineLayout " << pipelineLayoutName);
                pipelineLayout = _createPipelineLayout(pipelineLayoutName,
                                                       shader);
            }
            else
            {
                DEBUG_PIPELINE_LAYOUT("REUSING    pipelineLayout " << pipelineLayoutName);
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
                    DEBUG_PIPELINE("REUSING    pipeline " << pipelineName);
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

            createPipeline(pipelineName, pipelineLayoutName,
                           fbo->getLoadRenderPass(), shader, mesh, cb, ds, ms);
            
            fbo->setupViewportAndScissor(p.cmd);
            if (p.clipRectEnabled)
            {
                setClipRect(p.clipRect);
            }
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
        
        void Render::_vkDraw(const std::string& meshName)
        {
            TLRENDER_P();
            
            p.vaos[meshName]->bind(p.frameIndex);
            p.vaos[meshName]->draw(p.cmd, p.vbos[meshName]);
        }

        VkRenderPass Render::getRenderPass() const
        {
            return _p->renderPass;
        }

        void Render::setRenderPass(VkRenderPass value)
        {
            _p->renderPass = value;
        }
    }
}
