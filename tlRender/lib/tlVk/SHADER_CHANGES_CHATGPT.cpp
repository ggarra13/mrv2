struct ShaderBindingSet
{
    std::vector<VkDescriptorSet> descriptorSets; // size = MAX_FRAMES_IN_FLIGHT

    // Optional: buffers, textures, memories etc., if you're managing lifetime
    // here
};

void Render::bindDescriptorSets(
    const std::string& pipelineName, const std::string& shaderName,
    VkDescriptorSet externalSet)
{
    TLRENDER_P();

    const std::string pipelineLayoutName = pipelineName + "_" + shaderName;
    if (!p.pipelineLayouts[pipelineLayoutName])
        throw std::runtime_error("Missing pipeline layout");

    vkCmdBindDescriptorSets(
        p.cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
        p.pipelineLayouts[pipelineLayoutName], 0, 1, &externalSet, 0, nullptr);
}

void Render::drawMesh(
    const geom::TriangleMesh2& mesh, const math::Vector2i& position,
    const image::Color4f& color, const ShaderBindingSet& bindingSet)
{
    TLRENDER_P();
    ... p.shaders["mesh"]->bind(p.frameIndex);
    p.shaders["mesh"]->setUniform("transform.mvp", transform); // per mesh value

    createPipeline(...);
    bindDescriptorSets(
        "timeline", "mesh", bindingSet.descriptorSets[p.frameIndex]);
    ...
}

ShaderBindingSet createBindingSet(std::shared_ptr<Shader> shader)
{
    ShaderBindingSet set;
    // Clone shader's descriptor layout and allocate one set per frame
    // Populate descriptorSets, bind textures/UBOs, etc.
    return set;
}

ShaderBindingSet meshBindings = shader->createBindingSet();
meshBindings.setUniform("transform.mvp", matrix, frameIndex);

shader->bind(frameIndex);

// UBO
bindingSet->updateUniform(
    ctx.device, "transform.mvp", &transform, sizeof(transform), frameIndex);

// Texture
bindingSet->updateTexture(
    ctx.device, "textureSampler", bindingSet->get(frameIndex), myTexture);

bindDescriptorSets(..., bindingSet->get(frameIndex));
