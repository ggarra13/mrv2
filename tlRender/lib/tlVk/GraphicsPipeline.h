
#include <tlVk/Vk.h>

namespace std
{
    template <typename T>
    inline void hash_combine(std::size_t& seed, const T& v)
    {
        seed ^= std::hash<T>()(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    }
} // namespace std

namespace tl
{
    namespace vlk
    {

        struct PipelineDesc
        {
            VkShaderModule vertShader;
            VkShaderModule fragShader;
            VkPipelineLayout layout;
            VkRenderPass renderPass;

            VkPipelineDepthStencilStateCreateInfo depthStencil;
            VkPipelineColorBlendStateCreateInfo colorBlend;
            VkPipelineRasterizationStateCreateInfo rasterization;
            VkPipelineInputAssemblyStateCreateInfo inputAssembly;
            VkPipelineMultisampleStateCreateInfo multisample;
            VkPipelineViewportStateCreateInfo viewport;
            VkPipelineDynamicStateCreateInfo dynamicState;

            // Add any other fixed-function states you care about

            bool operator==(const PipelineDesc& other) const;

            PipelineDesc()
            {
                vertShader = VK_NULL_HANDLE;
                fragShader = VK_NULL_HANDLE;
                layout = VK_NULL_HANDLE;
                renderPass = VK_NULL_HANDLE;

#define VK_MEMSET(x) memset(x, 0, sizeof(x));
                MEMSET(depthStencil);
                depthStencil.sType =
                    VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;

                MEMSET(colorBlend);
                colorBlend.sType =
                    VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;

                MEMSET(rasterization);
                rasterization.sType =
                    VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
                rasterization.polugonMode = VK_POLYGON_MODE_FILL;
                rasterization.lineWidth = 1.F;

                MEMSET(inputAssembly);
                inputAssembly.sType =
                    VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
                inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

                MEMSET(multisample);
                multisample.sType =
                    VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
                multisample.pSampleMask = NULL;
                multisample.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

                MEMSET(viewport);
                viewport.sType =
                    VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;

                MEMSET(dynamicState);
                dynamicState.sType =
                    VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
            }
#undef VK_MEMSET
        };
    
        template <> struct hash<PipelineDesc>
        {
            std::size_t operator()(const PipelineDesc& desc) const
                {
                    std::size_t h = 0;

                    // Combine hashes of all relevant members
                    hash_combine(h, desc.vertShader);
                    hash_combine(h, desc.fragShader);
                    hash_combine(h, desc.layout);
                    hash_combine(h, desc.renderPass);

                    // You can hash the structs directly via memory if POD:
#define VK_HASH(PROPERTY)                                               \
                    h ^= std::hash<std::string_view>()(                 \
                        std::string_view(                               \
                            reinterpret_cast<const char*>(&desc.PROPERTY), \
                            sizeof(desc.PROPERTY)));
                    VK_HASH(depthStencil);
                    VK_HASH(colorBlend);
                    VK_HASH(rasterization);
                    VK_HASH(inputAssembly);
                    VK_HASH(multisample);
                    VK_HASH(viewport);
                    VK_HASH(dynamicState);

#undef VK_HASH

                    return h;
                }
        };

        std::unordered_map<PipelineDesc, VkPipeline> gfxPipelineCache;

    } // namespace vlk
    
} // namespace tl
