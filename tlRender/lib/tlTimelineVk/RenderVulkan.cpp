
#include <tlTimelineVk/RenderPrivate.h>

#include <string>

namespace tl
{
    namespace timeline_vlk
    {
        
        void Render::_createPipeline(
            const std::shared_ptr<vlk::OffscreenBuffer>& fbo,
            const std::string& pipelineName,
            const std::string& pipelineLayoutName,
            const std::string& shaderName,
            const std::string& meshName, const bool enableBlending,
            const VkBlendFactor srcColorBlendFactor,
            const VkBlendFactor dstColorBlendFactor,
            const VkBlendFactor srcAlphaBlendFactor,
            const VkBlendFactor dstAlphaBlendFactor,
            const VkBlendOp colorBlendOp,
            const VkBlendOp alphaBlendOp)
        {
            TLRENDER_P();

            VkDevice device = ctx.device;

            const auto& shader = p.shaders[shaderName];
            const auto& mesh = p.vbos[meshName];

            if (!shader)
                throw std::runtime_error(
                    "createPipeline failed with unknown shader '" + shaderName +
                    "'");

            if (!mesh)
                throw std::runtime_error(
                    "createPipeline failed with unknown mesh '" + meshName +
                    "'");

            VkPipelineLayout pipelineLayout = p.pipelineLayouts[pipelineLayoutName];
            if (!pipelineLayout)
            {
                pipelineLayout = _createPipelineLayout(pipelineLayoutName, shaderName);;
            }

            // Elements of new Pipeline
            vlk::VertexInputStateInfo vi;
            vi.bindingDescriptions = mesh->getBindingDescription();
            vi.attributeDescriptions = mesh->getAttributes();

            // Defaults are fine
            vlk::InputAssemblyStateInfo ia;

            // Defaults are fine
            vlk::ColorBlendStateInfo cb;

            // Defaults are fine
            vlk::RasterizationStateInfo rs;

            vlk::DepthStencilStateInfo ds;

            bool has_depth = fbo->hasDepth();     // Check if FBO has depth
            bool has_stencil = fbo->hasStencil(); // Check if FBO has stencil

            ds.depthTestEnable = has_depth ? VK_TRUE : VK_FALSE;
            ds.depthWriteEnable = has_depth ? VK_TRUE : VK_FALSE;
            ds.stencilTestEnable = has_stencil ? VK_TRUE : VK_FALSE;

            vlk::ViewportStateInfo vp;

            vlk::MultisampleStateInfo ms;

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

            vlk::ColorBlendAttachmentStateInfo colorBlendAttachment;
            if (enableBlending)
            {
                colorBlendAttachment.blendEnable = VK_TRUE;
            }

            cb.attachments.push_back(colorBlendAttachment);

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
            pipelineState.renderPass = fbo->getRenderPass();
            pipelineState.layout = pipelineLayout;

            VkPipeline pipeline;
            if (p.pipelines.count(pipelineName) == 0)
            {
                pipeline = pipelineState.create(device);
                std::cerr << "created pipeline " << pipelineName << " at "
                          << pipeline << std::endl;
                p.pipelines[pipelineName] = std::make_pair(pipelineState,
                                                           pipeline);
            }
            else
            {
                const auto& pair = p.pipelines[pipelineName];
                const auto& oldPipelineState = pair.first;
                VkPipeline oldPipeline = pair.second;
                if (pipelineState != oldPipelineState ||
                    oldPipeline == VK_NULL_HANDLE)
                {
                    if (oldPipeline != VK_NULL_HANDLE)
                    {
                        std::cerr << "send to garbage pipeline "
                                  << pipelineName
                                  << " "
                                  << oldPipeline << std::endl;
                        p.garbage[p.frameIndex].pipelines.push_back(
                            oldPipeline);
                    }
                    pipeline = pipelineState.create(device);
                    auto pair = std::make_pair(pipelineState, pipeline);
                    p.pipelines[pipelineName] = pair;
                }
                else
                {
                    pipeline = pair.second;
                    std::cerr << "reusing pipeline " << pipeline << " "
                              << pipelineName << std::endl;
                }
            }

            vkCmdBindPipeline(p.cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

            fbo->setupViewportAndScissor(p.cmd);
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
    }
}
