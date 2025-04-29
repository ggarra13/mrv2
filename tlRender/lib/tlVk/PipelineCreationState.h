#include <tlVk/Vk.h>

#include <array>
#include <optional>
#include <vector>

// Helper for comparing VkStencilOpState
inline bool operator==(const VkStencilOpState& a, const VkStencilOpState& b)
{
    return a.failOp == b.failOp && a.passOp == b.passOp &&
           a.depthFailOp == b.depthFailOp && a.compareOp == b.compareOp &&
           a.compareMask == b.compareMask && a.writeMask == b.writeMask &&
           a.reference == b.reference;
}
inline bool operator!=(const VkStencilOpState& a, const VkStencilOpState& b)
{
    return !(a == b);
}

namespace tl
{
    namespace vlk
    {

        struct VertexInputStateInfo
        {
            // Corresponds to VkPipelineVertexInputStateCreateInfo
            std::vector<VkVertexInputBindingDescription> bindingDescriptions;
            std::vector<VkVertexInputAttributeDescription>
                attributeDescriptions;

            // Add comparison operators (==, !=)
            bool operator==(const VertexInputStateInfo& other) const
            {
                if (bindingDescriptions.size() !=
                    other.bindingDescriptions.size())
                    return false;
                if (attributeDescriptions.size() !=
                    other.attributeDescriptions.size())
                    return false;

                // Compare VkVertexInputBindingDescription elements field by
                // field
                for (size_t i = 0; i < bindingDescriptions.size(); ++i)
                {
                    const auto& b1 = bindingDescriptions[i];
                    const auto& b2 = other.bindingDescriptions[i];
                    if (b1.binding != b2.binding || b1.stride != b2.stride ||
                        b1.inputRate != b2.inputRate)
                    {
                        return false;
                    }
                }

                // Compare VkVertexInputAttributeDescription elements field by
                // field
                for (size_t i = 0; i < attributeDescriptions.size(); ++i)
                {
                    const auto& a1 = attributeDescriptions[i];
                    const auto& a2 = other.attributeDescriptions[i];
                    if (a1.location != a2.location ||
                        a1.binding != a2.binding || a1.format != a2.format ||
                        a1.offset != a2.offset)
                    {
                        return false;
                    }
                }
                return true;
            }
            bool operator!=(const VertexInputStateInfo& other) const
            {
                return !(*this == other);
            }
        };

        struct InputAssemblyStateInfo
        {
            // Corresponds to VkPipelineInputAssemblyStateCreateInfo
            VkPrimitiveTopology topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
            VkBool32 primitiveRestartEnable = VK_FALSE;

            // Add comparison operators (==, !=)
            bool operator==(const InputAssemblyStateInfo& other) const
            {
                return topology == other.topology &&
                       primitiveRestartEnable == other.primitiveRestartEnable;
            }
            bool operator!=(const InputAssemblyStateInfo& other) const
            {
                return !(*this == other);
            }
        };

        struct ViewportStateInfo
        {
            // Corresponds to VkPipelineViewportStateCreateInfo
            uint32_t viewportCount = 1;
            uint32_t scissorCount = 1;

            // Add comparison operators (==, !=)
            bool operator==(const ViewportStateInfo& other) const
            {
                return viewportCount == other.viewportCount &&
                       scissorCount == other.scissorCount;
            }
            bool operator!=(const ViewportStateInfo& other) const
            {
                return !(*this == other);
            }
        };

        struct RasterizationStateInfo
        {
            // Corresponds to VkPipelineRasterizationStateCreateInfo
            VkBool32 depthClampEnable = VK_FALSE;
            VkBool32 rasterizerDiscardEnable = VK_FALSE;
            VkPolygonMode polygonMode = VK_POLYGON_MODE_FILL;
            VkCullModeFlags cullMode = VK_CULL_MODE_NONE;
            VkFrontFace frontFace = VK_FRONT_FACE_CLOCKWISE;
            VkBool32 depthBiasEnable = VK_FALSE;
            float depthBiasConstantFactor = 0.F;
            float depthBiasClamp = VK_FALSE;
            float depthBiasSlopeFactor = 0.F;
            float lineWidth = 1.F;

            // Add comparison operators (==, !=)
            bool operator==(const RasterizationStateInfo& other) const
            {
                return depthClampEnable == other.depthClampEnable &&
                       rasterizerDiscardEnable ==
                           other.rasterizerDiscardEnable &&
                       polygonMode == other.polygonMode &&
                       cullMode == other.cullMode &&
                       frontFace == other.frontFace &&
                       depthBiasEnable == other.depthBiasEnable &&
                       depthBiasConstantFactor ==
                           other.depthBiasConstantFactor &&
                       depthBiasClamp == other.depthBiasClamp &&
                       depthBiasSlopeFactor == other.depthBiasSlopeFactor &&
                       lineWidth == other.lineWidth;
            }
            bool operator!=(const RasterizationStateInfo& other) const
            {
                return !(*this == other);
            }
        };

        struct ColorBlendAttachmentStateInfo
        {
            // Corresponds to VkPipelineColorBlendAttachmentState
            VkBool32 blendEnable = VK_FALSE;
            VkBlendFactor srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
            VkBlendFactor dstColorBlendFactor =
                VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
            VkBlendOp colorBlendOp = VK_BLEND_OP_ADD;
            VkBlendFactor srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
            VkBlendFactor dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
            VkBlendOp alphaBlendOp = VK_BLEND_OP_ADD;
            VkColorComponentFlags colorWriteMask =
                VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

            // Add comparison operators (==, !=)
            bool operator==(const ColorBlendAttachmentStateInfo& other) const
            {
                return blendEnable == other.blendEnable &&
                       srcColorBlendFactor == other.srcColorBlendFactor &&
                       dstColorBlendFactor == other.dstColorBlendFactor &&
                       colorBlendOp == other.colorBlendOp &&
                       srcAlphaBlendFactor == other.srcAlphaBlendFactor &&
                       dstAlphaBlendFactor == other.dstAlphaBlendFactor &&
                       alphaBlendOp == other.alphaBlendOp &&
                       colorWriteMask == other.colorWriteMask;
            }
            bool operator!=(const ColorBlendAttachmentStateInfo& other) const
            {
                return !(*this == other);
            }
        };

        struct ColorBlendStateInfo
        {
            // Corresponds to VkPipelineColorBlendStateCreateInfo
            VkBool32 logicOpEnable = VK_FALSE;
            VkLogicOp logicOp = {};
            std::array<float, 4> blendConstants;
            std::vector<ColorBlendAttachmentStateInfo>
                attachments; // Store our custom attachment info

            // Add comparison operators (==, !=)
            bool operator==(const ColorBlendStateInfo& other) const
            {
                for (int i = 0; i < 4; ++i)
                {
                    if (blendConstants[i] != other.blendConstants[i])
                        return false;
                }

                return logicOpEnable == other.logicOpEnable &&
                       logicOp == other.logicOp &&
                       attachments ==
                           other.attachments; // std::vector comparison works if
                                              // element type is comparable
            }
            bool operator!=(const ColorBlendStateInfo& other) const
            {
                return !(*this == other);
            }
        };

        struct DynamicStateInfo
        {
            // Corresponds to VkPipelineDynamicStateCreateInfo
            std::vector<VkDynamicState> dynamicStates;

            // Add comparison operators (==, !=)
            bool operator==(const DynamicStateInfo& other) const
            {
                return dynamicStates == other.dynamicStates;
            }
            bool operator!=(const DynamicStateInfo& other) const
            {
                return !(*this == other);
            }
        };

        struct MultisampleStateInfo
        {
            // Corresponds to VkPipelineMultisampleStateInfo
            VkPipelineMultisampleStateCreateFlags flags = 0;
            VkSampleCountFlagBits rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
            VkBool32 sampleShadingEnable = VK_FALSE;
            float minSampleShading = 0;
            std::vector<VkSampleMask> sampleMasks;
            VkBool32 alphaToCoverageEnable = 0;
            VkBool32 alphaToOneEnable = 0;

            // Add comparison operators (==, !=)
            bool operator==(const MultisampleStateInfo& other) const
            {
                return (
                    flags == other.flags &&
                    rasterizationSamples == other.rasterizationSamples &&
                    sampleShadingEnable == other.sampleShadingEnable &&
                    minSampleShading == other.minSampleShading &&
                    sampleMasks == other.sampleMasks &&
                    alphaToCoverageEnable == other.alphaToCoverageEnable &&
                    alphaToOneEnable == other.alphaToOneEnable);
            }
            bool operator!=(const MultisampleStateInfo& other) const
            {
                return !(*this == other);
            }
        };

        struct DepthStencilStateInfo
        {
            // Corresponds to VkPipelineDepthStencilStateInfo
            VkPipelineDepthStencilStateCreateFlags flags = 0;
            VkBool32 depthTestEnable = VK_TRUE;
            VkBool32 depthWriteEnable = VK_FALSE;
            VkCompareOp depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
            VkBool32 depthBoundsTestEnable = VK_FALSE;
            VkBool32 stencilTestEnable = VK_TRUE;
            VkStencilOpState front = {};
            VkStencilOpState back = {};
            float minDepthBounds = 0.F;
            float maxDepthBounds = 0.F;

            // Add comparison operators (==, !=)
            bool operator==(const DepthStencilStateInfo& other) const
            {
                return (
                    flags == other.flags &&
                    depthTestEnable == other.depthTestEnable &&
                    depthWriteEnable == other.depthWriteEnable &&
                    depthCompareOp == other.depthCompareOp &&
                    depthBoundsTestEnable == other.depthBoundsTestEnable &&
                    stencilTestEnable == other.stencilTestEnable &&
                    front == other.front && back == other.back &&
                    minDepthBounds == other.minDepthBounds &&
                    maxDepthBounds == other.maxDepthBounds);
            }
            bool operator!=(const DepthStencilStateInfo& other) const
            {
                return !(*this == other);
            }
        };

        struct PipelineCreationState
        {
            // Corresponds to VkGraphicsPipelineCreateInfo and its pointers

            // Shader Stages (needs to store info about the shaders)
            // You might store file paths, source code strings, or identifiers
            // here, rather than VkShaderModule handles if shader compilation is
            // part of the process
            struct ShaderStageInfo
            {
                std::string name;
                VkShaderStageFlagBits stage;
                std::string entryPoint;
                // Could add info here to identify the shader source/SPIR-V if
                // needed for recreation e.g., std::string filename; or
                // std::vector<char> spirvCode;
                VkShaderModule
                    module; // Or an identifier/path to look up the module

                // Add comparison operators (==, !=) - be careful if comparing
                // raw VkShaderModule handles Comparing identifiers/paths is
                // safer if they represent unique shader code
                bool operator==(const ShaderStageInfo& other) const
                {
                    return stage == other.stage &&
                           entryPoint == other.entryPoint &&
                           name == other.name; // WARNING: comparing raw handles
                                               // might not be what you want if
                                               // the handle could change but
                                               // the shader is the same. Use a
                                               // shader identifier if possible.
                }
                bool operator!=(const ShaderStageInfo& other) const
                {
                    return !(*this == other);
                }
            };
            std::vector<ShaderStageInfo> stages;

            // Sub-states (using the structs defined above)
            std::optional<VertexInputStateInfo>
                vertexInputState; // Use optional as not always present
            std::optional<InputAssemblyStateInfo> inputAssemblyState;
            std::optional<ViewportStateInfo> viewportState;
            std::optional<RasterizationStateInfo> rasterizationState;
            std::optional<MultisampleStateInfo>
                multisampleState; // Define MultisampleStateInfo
            std::optional<DepthStencilStateInfo>
                depthStencilState; // Define DepthStencilStateInfo
            std::optional<ColorBlendStateInfo> colorBlendState;
            std::optional<DynamicStateInfo> dynamicState;

            // Fixed state not covered above
            // VkPipelineTessellationStateCreateInfo* pTessellationState; // If
            // applicable, add struct and optional member

            // Layout and Render Pass
            VkPipelineLayout layout = VK_NULL_HANDLE; // Store the handle
            VkRenderPass renderPass = VK_NULL_HANDLE; // Store the handle
            uint32_t subpass = 0;

            // Base pipeline (if using derivation)
            // VkPipeline                     basePipelineHandle; // Store the
            // handle int32_t                        basePipelineIndex;

            // Flags
            VkPipelineCreateFlags flags = 0;

            // --- Comparison Operator (CRUCIAL for detecting changes) ---
            // This operator needs to compare *every relevant field*
            bool operator==(const PipelineCreationState& other) const
            {
                // Compare all members recursively, including optional states
                // and vectors
                std::cerr << "pre stages" << std::endl;
                if (!(stages == other.stages))
                    return false;
                std::cerr << "past stages" << std::endl;
                if (!(vertexInputState == other.vertexInputState))
                    return false;
                std::cerr << "past vi" << std::endl;
                if (!(inputAssemblyState == other.inputAssemblyState))
                    return false;
                std::cerr << "past ia" << std::endl;
                if (!(viewportState == other.viewportState))
                    return false;
                std::cerr << "past vs" << std::endl;
                if (!(rasterizationState == other.rasterizationState))
                    return false;
                std::cerr << "past rs" << std::endl;
                if (!(multisampleState == other.multisampleState))
                    return false;
                std::cerr << "past ms" << std::endl;
                if (!(depthStencilState == other.depthStencilState))
                    return false;
                std::cerr << "past ds" << std::endl;
                if (!(colorBlendState == other.colorBlendState))
                    return false;
                std::cerr << "past cb" << std::endl;
                if (!(dynamicState == other.dynamicState))
                    return false;
                std::cerr << "past dynamicState" << std::endl;

                // Compare other direct members
                if (layout != other.layout)
                    return false; // WARNING: Comparing handles might not be
                                  // robust
                std::cerr << "past layout" << std::endl;

                if (renderPass != other.renderPass)
                    return false; // WARNING: Comparing handles might not be
                                  // robust
                std::cerr << "past renderPass" << std::endl;

                if (subpass != other.subpass)
                    return false;
                std::cerr << "past subpass" << std::endl;

                // if (basePipelineHandle != other.basePipelineHandle) return
                // false; // WARNING: Comparing handles might not be robust if
                // (basePipelineIndex != other.basePipelineIndex) return false;
                if (flags != other.flags)
                    return false;
                std::cerr << "past flags is EQUAL!" << std::endl;

                return true; // All compared fields match
            }

            bool operator!=(const PipelineCreationState& other) const
            {
                return !(*this == other);
            }

            // --- Function to convert to VkGraphicsPipelineCreateInfo ---
            // This function will assemble the Vulkan struct using the stored
            // data
            VkPipeline create(VkDevice device) const
            {
                VkGraphicsPipelineCreateInfo createInfo{};
                createInfo.sType =
                    VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
                // Declare a temporary VkPipelineVertexInputStateCreateInfo
                // struct This struct will live on the stack and only needs to
                // exist for the duration of the vkCreateGraphicsPipelines call.
                VkPipelineVertexInputStateCreateInfo tempVertexInputStateInfo{};

                if (vertexInputState.has_value())
                {
                    // The optional has a value, so this state is enabled.
                    const auto& vi_info =
                        this->vertexInputState
                            .value(); // Get the VertexInputStateInfo

                    // Populate the temporary Vulkan struct using data from our
                    // stored info
                    tempVertexInputStateInfo.sType =
                        VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
                    tempVertexInputStateInfo.pNext =
                        nullptr; // No pNext for this basic struct
                    tempVertexInputStateInfo.vertexBindingDescriptionCount =
                        static_cast<uint32_t>(
                            vi_info.bindingDescriptions.size());
                    // Get a pointer to the start of the data in our std::vector
                    // .data() returns a pointer to the first element, or
                    // nullptr if the vector is empty (which is correct for
                    // count 0)
                    tempVertexInputStateInfo.pVertexBindingDescriptions =
                        vi_info.bindingDescriptions.data();

                    tempVertexInputStateInfo.vertexAttributeDescriptionCount =
                        static_cast<uint32_t>(
                            vi_info.attributeDescriptions.size());
                    // Get a pointer to the start of the data in our std::vector
                    tempVertexInputStateInfo.pVertexAttributeDescriptions =
                        vi_info.attributeDescriptions.data();

                    // Set the pVertexInputState pointer in the main createInfo
                    // to point to our temporary struct
                    createInfo.pVertexInputState = &tempVertexInputStateInfo;
                }
                else
                {
                    createInfo.pVertexInputState = nullptr;
                }

                VkPipelineInputAssemblyStateCreateInfo
                    tempInputAssemblyStateInfo{};
                if (inputAssemblyState.has_value())
                {
                    // The optional has a value, so this state is enabled.
                    const auto& ia = this->inputAssemblyState.value();
                    tempInputAssemblyStateInfo.sType =
                        VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
                    tempInputAssemblyStateInfo.pNext = nullptr;
                    tempInputAssemblyStateInfo.topology = ia.topology;
                    tempInputAssemblyStateInfo.primitiveRestartEnable =
                        ia.primitiveRestartEnable;
                    createInfo.pInputAssemblyState =
                        &tempInputAssemblyStateInfo;
                }
                else
                {
                    createInfo.pInputAssemblyState = nullptr;
                }
                VkPipelineViewportStateCreateInfo tempViewportStateInfo{};
                if (viewportState.has_value())
                {
                    // The optional has a value, so this state is enabled.
                    const auto& vp = this->viewportState.value();
                    tempViewportStateInfo.sType =
                        VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
                    tempViewportStateInfo.pNext = nullptr;
                    tempViewportStateInfo.viewportCount = vp.viewportCount;
                    tempViewportStateInfo.scissorCount = vp.scissorCount;

                    createInfo.pViewportState = &tempViewportStateInfo;
                }
                else
                {
                    createInfo.pViewportState = nullptr;
                }

                VkPipelineRasterizationStateCreateInfo
                    tempRasterizationStateInfo{};
                if (rasterizationState.has_value())
                {
                    const auto& rs = this->rasterizationState.value();
                    tempRasterizationStateInfo.sType =
                        VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
                    tempRasterizationStateInfo.pNext = nullptr;
                    tempRasterizationStateInfo.depthClampEnable =
                        rs.depthClampEnable;
                    tempRasterizationStateInfo.rasterizerDiscardEnable =
                        rs.rasterizerDiscardEnable;
                    tempRasterizationStateInfo.polygonMode = rs.polygonMode;
                    tempRasterizationStateInfo.cullMode = rs.cullMode;
                    tempRasterizationStateInfo.frontFace = rs.frontFace;
                    tempRasterizationStateInfo.depthBiasEnable =
                        rs.depthBiasEnable;
                    tempRasterizationStateInfo.depthBiasConstantFactor =
                        rs.depthBiasConstantFactor;
                    tempRasterizationStateInfo.depthBiasClamp =
                        rs.depthBiasClamp;
                    tempRasterizationStateInfo.depthBiasSlopeFactor =
                        rs.depthBiasSlopeFactor;
                    tempRasterizationStateInfo.lineWidth = rs.lineWidth;

                    createInfo.pRasterizationState =
                        &tempRasterizationStateInfo;
                }
                else
                {
                    createInfo.pRasterizationState = nullptr;
                }

                VkPipelineMultisampleStateCreateInfo tempMultisampleStateInfo =
                    {};
                if (multisampleState.has_value())
                {
                    const auto& ms = this->multisampleState.value();
                    tempMultisampleStateInfo.sType =
                        VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
                    tempMultisampleStateInfo.rasterizationSamples =
                        ms.rasterizationSamples;
                    tempMultisampleStateInfo.sampleShadingEnable =
                        ms.sampleShadingEnable;
                    tempMultisampleStateInfo.minSampleShading =
                        ms.minSampleShading;
                    tempMultisampleStateInfo.pSampleMask =
                        ms.sampleMasks.empty() ? nullptr
                                               : ms.sampleMasks.data();
                    tempMultisampleStateInfo.alphaToCoverageEnable =
                        ms.alphaToCoverageEnable;
                    tempMultisampleStateInfo.alphaToOneEnable =
                        ms.alphaToOneEnable;

                    createInfo.pMultisampleState = &tempMultisampleStateInfo;
                }
                else
                {
                    createInfo.pMultisampleState = nullptr;
                }

                VkPipelineDepthStencilStateCreateInfo
                    tempDepthStencilStateInfo = {};
                if (depthStencilState.has_value())
                {
                    const auto& ds = this->depthStencilState.value();
                    tempDepthStencilStateInfo.sType =
                        VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
                    tempDepthStencilStateInfo.flags = ds.flags;
                    tempDepthStencilStateInfo.depthTestEnable =
                        ds.depthTestEnable;
                    tempDepthStencilStateInfo.depthWriteEnable =
                        ds.depthWriteEnable;
                    tempDepthStencilStateInfo.depthCompareOp =
                        ds.depthCompareOp;
                    tempDepthStencilStateInfo.depthBoundsTestEnable =
                        ds.depthBoundsTestEnable;
                    tempDepthStencilStateInfo.stencilTestEnable =
                        ds.stencilTestEnable;
                    tempDepthStencilStateInfo.front = ds.front;
                    tempDepthStencilStateInfo.back = ds.back;
                    tempDepthStencilStateInfo.minDepthBounds =
                        ds.minDepthBounds;
                    tempDepthStencilStateInfo.maxDepthBounds =
                        ds.maxDepthBounds;

                    createInfo.pDepthStencilState = &tempDepthStencilStateInfo;
                }
                else
                {
                    createInfo.pDepthStencilState = nullptr;
                }

                VkPipelineColorBlendStateCreateInfo tempColorBlendStateInfo =
                    {};
                std::vector<VkPipelineColorBlendAttachmentState>
                    tempColorBlendAttachments;
                if (colorBlendState.has_value())
                {
                    const auto& cb = this->colorBlendState.value();
                    tempColorBlendStateInfo.sType =
                        VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
                    tempColorBlendStateInfo.logicOpEnable = cb.logicOpEnable;
                    tempColorBlendStateInfo.logicOp = cb.logicOp;
                    for (int i = 0; i < 4; ++i)
                        tempColorBlendStateInfo.blendConstants[i] =
                            cb.blendConstants[i];

                    tempColorBlendAttachments.reserve(cb.attachments.size());
                    for (const auto& a : cb.attachments)
                    {
                        VkPipelineColorBlendAttachmentState tmpState;
                        tmpState.blendEnable = a.blendEnable;
                        tmpState.srcColorBlendFactor = a.srcColorBlendFactor;
                        tmpState.dstColorBlendFactor = a.dstColorBlendFactor;
                        tmpState.colorBlendOp = a.colorBlendOp;
                        tmpState.srcAlphaBlendFactor = a.srcAlphaBlendFactor;
                        tmpState.dstAlphaBlendFactor = a.dstAlphaBlendFactor;
                        tmpState.alphaBlendOp = a.alphaBlendOp;
                        tmpState.colorWriteMask = a.colorWriteMask;

                        tempColorBlendAttachments.push_back(tmpState);
                    }
                    tempColorBlendStateInfo.attachmentCount =
                        static_cast<uint32_t>(tempColorBlendAttachments.size());
                    tempColorBlendStateInfo.pAttachments =
                        tempColorBlendAttachments.data();

                    createInfo.pColorBlendState = &tempColorBlendStateInfo;
                }
                else
                {
                    createInfo.pColorBlendState = nullptr;
                }

                VkPipelineDynamicStateCreateInfo tempDynamicStateInfo = {};
                if (dynamicState.has_value())
                {
                    const auto& ds = this->dynamicState.value();
                    tempDynamicStateInfo.sType =
                        VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
                    tempDynamicStateInfo.pNext = nullptr;
                    tempDynamicStateInfo.dynamicStateCount =
                        static_cast<uint32_t>(ds.dynamicStates.size());
                    tempDynamicStateInfo.pDynamicStates =
                        ds.dynamicStates.data();

                    createInfo.pDynamicState = &tempDynamicStateInfo;
                }
                else
                {
                    createInfo.pDynamicState = nullptr;
                }

                std::vector<VkPipelineShaderStageCreateInfo> tempShaderStages;
                tempShaderStages.reserve(stages.size());
                for (const auto& shaderStage : stages)
                {
                    VkPipelineShaderStageCreateInfo tempShaderStage = {};
                    tempShaderStage.sType =
                        VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
                    tempShaderStage.stage = shaderStage.stage;
                    tempShaderStage.module = shaderStage.module;
                    tempShaderStage.pName = shaderStage.entryPoint.c_str();

                    tempShaderStages.push_back(tempShaderStage);
                }

                createInfo.stageCount =
                    static_cast<uint32_t>(tempShaderStages.size());
                createInfo.pStages = tempShaderStages.data();

                // Actual variables that should have been filled before.
                createInfo.renderPass = renderPass;
                createInfo.layout = layout;

                // Create a temporary pipeline cache
                VkResult result;

                VkPipelineCacheCreateInfo pipelineCacheCreateInfo{};
                pipelineCacheCreateInfo.sType =
                    VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;

                VkPipelineCache pipelineCache;
                result = vkCreatePipelineCache(
                    device, &pipelineCacheCreateInfo, NULL, &pipelineCache);
                VK_CHECK(result);

                VkPipeline graphicsPipeline;
                result = vkCreateGraphicsPipelines(
                    device, pipelineCache, 1, &createInfo, NULL,
                    &graphicsPipeline);
                VK_CHECK(result);

                vkDestroyPipelineCache(device, pipelineCache, nullptr);

                return graphicsPipeline;
            }
        };

    } // namespace vlk
} // namespace tl
