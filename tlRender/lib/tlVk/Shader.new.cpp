std::shared_ptr<ShaderBindingSet> activeBindingSet;

void Shader::useBindingSet(const std::shared_ptr<ShaderBindingSet>& set)
{
    activeBindingSet = set;
}

const VkDescriptorSet& Shader::getDescriptorSet() const
{
    if (activeBindingSet)
        return activeBindingSet->get(frameIndex);
    return descriptorSets[frameIndex]; // fallback for static/global set
}

template <typename T>
void Shader::setUniform(
    const std::string& name, const T& value, const ShaderFlags stageFlags)
{
    if (!activeBindingSet)
        throw std::runtime_error("No active ShaderBindingSet!");

    activeBindingSet->updateUniform(
        ctx.device, name, &value, sizeof(T), frameIndex);
}

void Shader::setTexture(
    const std::string& name, const std::shared_ptr<Texture>& tex,
    const ShaderFlags stageFlags)
{
    if (!activeBindingSet)
        throw std::runtime_error("No active ShaderBindingSet!");
    activeBindingSet->updateTexture(
        ctx.device, name, activeBindingSet->get(frameIndex), tex);
}

void Shader::setFBO(
    const std::string& name, const std::shared_ptr<OffscreenBuffer>& tex,
    const ShaderFlags stageFlags)
{
    if (!activeBindingSet)
        throw std::runtime_error("No active ShaderBindingSet!");
    activeBindingSet->updateFBO(
        ctx.device, name, activeBindingSet->get(frameIndex), tex);
}

std::shared_ptr<ShaderBindingSet> Shader::createBindingSet()
{
    auto bindingSet = std::make_shared<ShaderBindingSet>();
    VkDevice device = ctx.device;
    VkPhysicalDevice gpu = ctx.gpu;

    // === 1. Allocate descriptor pool ===
    std::vector<VkDescriptorPoolSize> poolSizes;
    for (const auto& [_, ubo] : ubos)
    {
        VkDescriptorPoolSize poolSize{};
        poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        poolSize.descriptorCount = MAX_FRAMES_IN_FLIGHT;
        poolSizes.push_back(poolSize);
    }

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = MAX_FRAMES_IN_FLIGHT;

    bindingSet->descriptorPools.resize(MAX_FRAMES_IN_FLIGHT);
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
    {
        VK_CHECK(vkCreateDescriptorPool(
            device, &poolInfo, nullptr, &bindingSet->descriptorPools[i]));
    }

    // === 2. Allocate descriptor sets ===
    bindingSet->descriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
    std::vector<VkDescriptorSetLayout> layouts(
        MAX_FRAMES_IN_FLIGHT, descriptorSetLayout);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
    {
        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = bindingSet->descriptorPools[i];
        allocInfo.descriptorSetCount = 1;
        allocInfo.pSetLayouts = &descriptorSetLayout;

        VK_CHECK(vkAllocateDescriptorSets(
            device, &allocInfo, &bindingSet->descriptorSets[i]));
    }

    // === 3. Create UBOs for each binding ===
    for (const auto& [name, uboTemplate] : ubos)
    {
        ShaderBindingSet::Uniform uniform;
        uniform.size = uboTemplate.size;
        uniform.layoutBinding = uboTemplate.layoutBinding;
        uniform.buffers.resize(MAX_FRAMES_IN_FLIGHT);
        uniform.memories.resize(MAX_FRAMES_IN_FLIGHT);
        uniform.infos.resize(MAX_FRAMES_IN_FLIGHT);

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
        {
            VkBufferCreateInfo bufferInfo{};
            bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
            bufferInfo.size = uniform.size;
            bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
            bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

            VK_CHECK(vkCreateBuffer(
                device, &bufferInfo, nullptr, &uniform.buffers[i]));

            VkMemoryRequirements memReqs;
            vkGetBufferMemoryRequirements(device, uniform.buffers[i], &memReqs);

            VkMemoryAllocateInfo allocInfo{};
            allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            allocInfo.allocationSize = memReqs.size;
            allocInfo.memoryTypeIndex = findMemoryType(
                gpu, memReqs.memoryTypeBits,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                    VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

            VK_CHECK(vkAllocateMemory(
                device, &allocInfo, nullptr, &uniform.memories[i]));
            VK_CHECK(vkBindBufferMemory(
                device, uniform.buffers[i], uniform.memories[i], 0));

            uniform.infos[i].buffer = uniform.buffers[i];
            uniform.infos[i].offset = 0;
            uniform.infos[i].range = uniform.size;

            // Write to descriptor set
            VkWriteDescriptorSet write{};
            write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            write.dstSet = bindingSet->descriptorSets[i];
            write.dstBinding = uniform.layoutBinding.binding;
            write.dstArrayElement = 0;
            write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            write.descriptorCount = 1;
            write.pBufferInfo = &uniform.infos[i];

            vkUpdateDescriptorSets(device, 1, &write, 0, nullptr);
        }

        bindingSet->uniforms[name] = std::move(uniform);

        // Textures
        for (const auto& [name, texture] : textureBindings)
        {
            ShaderBindingSet::Texture textureInfo;
            textureInfo.binding = texBinding.binding;
            textureInfo.stageFlags = texBinding.stageFlags;
            bindingSet->textures[name] = textureInfo;
        }

        // FBOs
        for (const auto& [_, element] : fboBindings)
        {
            ShaderBindingSet::FBO fboInfo;
            fboInfo.binding = element.binding;
            fboInfo.stageFlags = element.stageFlags;
        }
    }

    return bindingSet;
}
