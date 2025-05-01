
1. Implement dynamic descriptor allocation with two strategies :

    // Option 1: Descriptor set per material
    std::unordered_map<std::string, VkDescriptorSet>
        materialDescriptorSets;

// Option 2: Pool-based allocation
VkDescriptorPoolCreateInfo poolInfo{};
poolInfo.maxSets = 100; // Adjust based on scene complexity
poolInfo.poolSizeCount = /* ... */;

Key modifications:

Remove MAX_FRAMES_IN_FLIGHT arrays for descriptor sets

Add descriptor set caching mechanism

Implement LRU cache eviction for unused sets


----------------
2. Decouple descriptor sets from pipeline state:

struct PipelineLayout
{
    VkPipelineLayout layout;
    VkDescriptorSetLayout setLayout;
    std::vector<VkDescriptorSet> descriptorSets;
};

std::vector<PipelineLayout> pipelineLayouts;

-- -- -- -- -- -- -- -- -3. Implement late binding mechanism :

    void
    Shader::bindDescriptorSet(VkCommandBuffer cmdBuffer, VkDescriptorSet set)
{
    vkCmdBindDescriptorSets(
        cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &set,
        0, nullptr);
}

-- -- -- -- -- -- -- -- -4. Add upload tracking system :

    struct ResourceUpload
{
    size_t frameLastUsed;
    VkDescriptorSet set;
    bool locked;
};

std::unordered_map<MeshID, ResourceUpload> uploadTracker;

-- -- -- -- -- -- -- -- -- -- -- -- -- -- -5. Implement set recycling queue :

    std::queue<VkDescriptorSet>
        availableSets;
std::vector<VkDescriptorSet> inUseSets;

VkDescriptorSet allocateSet()
{
    if (!availableSets.empty())
    {
        VkDescriptorSet set = availableSets.front();
        availableSets.pop();
        return set;
    }
    // Create new set...
}

6.  Add push constant handling for per-mesh data:


void Shader::addPushConstantRange(
    VkShaderStageFlags stageFlags,
    uint32_t size)
{

    VkPushConstantRange range{};
    range.stageFlags = stageFlags;
    range.offset = pushConstantOffset;
    range.size = size;
    pushConstantRanges.push_back(range);
    pushConstantOffset += size;
}

-- -- -- -- -- -- -- -- -- --Usage-- -- -- -- -- -- -- -- -- --

                           // Per mesh setup
                           VkDescriptorSet meshSet = shader->allocateSet();
shader->updateSet(meshSet, meshResources);
shader->bindDescriptorSet(cmdBuffer, meshSet);
vkCmdDrawIndexed(cmdBuffer, ...);

// Frame cleanup
shader->releaseUnusedSets(currentFrame);
