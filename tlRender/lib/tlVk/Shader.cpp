// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// Copyright (c) 2025-Present Gonzalo Garramuño
// All rights reserved.

#include <tlVk/Shader.h>

#include <tlCore/Color.h>
#include <tlCore/String.h>
#include <tlCore/StringFormat.h>

#include <FL/Fl_Vk_Utils.H>

#include <cassert>
#include <iostream>
#include <stdexcept>

namespace tl
{
    namespace vlk
    {
        struct Shader::Private
        {
            std::string vertexSource = "Compiled SPIRV code";
            std::string fragmentSource = "Compiled SPIRV code";
            std::string computeSource = "Compiled SPIRV code";
            
            VkShaderModule vertex = VK_NULL_HANDLE;
            VkShaderModule fragment = VK_NULL_HANDLE;
            VkShaderModule compute = VK_NULL_HANDLE;
        };

        void Shader::_createVertexShader()
        {
            TLRENDER_P();

            try
            {
                std::vector<uint32_t> spirv = compile_glsl_to_spirv(
                    p.vertexSource,
                    shaderc_vertex_shader, // Shader type
                    "vertex_shader.glsl"       // Filename for error reporting
                );

                // Assuming you have a VkDevice 'device' already created
                p.vertex = create_shader_module(ctx.device, spirv);
            }
            catch (const std::exception& e)
            {
                std::cerr << shaderName << " failed vertex compilation " << std::endl
                          << e.what() << " for " << std::endl;
                auto lines = tl::string::split(
                    p.vertexSource, '\n', string::SplitOptions::KeepEmpty);
                uint32_t i = 1;
                for (const auto& line : lines)
                {
                    std::cerr << i++ << ": " << line << std::endl;
                }
                p.vertex = VK_NULL_HANDLE;
                throw e;
            }
        }
        
        void Shader::_createFragmentShader()
        {
            TLRENDER_P();

            try
            {
                std::vector<uint32_t> spirv = compile_glsl_to_spirv(
                    p.fragmentSource,
                    shaderc_fragment_shader, // Shader type
                    "frag_shader.glsl"       // Filename for error reporting
                );

                // Assuming you have a VkDevice 'device' already created
                p.fragment = create_shader_module(ctx.device, spirv);
            }
            catch (const std::exception& e)
            {
                std::cerr << shaderName << " failed fragment compilation " << std::endl
                          << e.what() << " for " << std::endl;
                auto lines = tl::string::split(
                    p.fragmentSource, '\n', string::SplitOptions::KeepEmpty);
                uint32_t i = 1;
                for (const auto& line : lines)
                {
                    std::cerr << i++ << ": " << line << std::endl;
                }
                p.fragment = VK_NULL_HANDLE;
                throw e;
            }
        }
        
        void Shader::_createComputeShader()
        {
            TLRENDER_P();

            try
            {
                std::vector<uint32_t> spirv = compile_glsl_to_spirv(
                    p.computeSource,
                    shaderc_compute_shader, // Shader type
                    "compute_shader.glsl"       // Filename for error reporting
                );

                // Assuming you have a VkDevice 'device' already created
                p.compute = create_shader_module(ctx.device, spirv);
            }
            catch (const std::exception& e)
            {
                std::cerr << shaderName << " failed fragment compilation " << std::endl
                          << e.what() << " for " << std::endl;
                auto lines = tl::string::split(
                    p.computeSource, '\n', string::SplitOptions::KeepEmpty);
                uint32_t i = 1;
                for (const auto& line : lines)
                {
                    std::cerr << i++ << ": " << line << std::endl;
                }
                p.compute = VK_NULL_HANDLE;
                throw e;
            }
        }
        
        void Shader::_init(const uint32_t* vertexBytes,
                           const uint32_t vertexLength,
                           const std::string& fragmentSource,
                           const std::string& name)
        {
            TLRENDER_P();
            
            p.vertex = create_shader_module(ctx.device, vertexBytes, vertexLength);

            p.fragmentSource = fragmentSource;
            shaderName = name;
            
            _createFragmentShader();
        }
        
        void Shader::_init(const uint32_t* vertexBytes,
                           const uint32_t vertexLength,
                           const uint32_t* fragmentBytes,
                           const uint32_t fragmentLength,
                           const std::string& name)
        {
            TLRENDER_P();
            
            p.vertex = create_shader_module(ctx.device, vertexBytes, vertexLength);
            p.fragment = create_shader_module(ctx.device, fragmentBytes, fragmentLength);
            shaderName = name;
        }
        
        void Shader::_init(const std::string& vertexSource,
                           const std::string& fragmentSource,
                           const std::string& name)
        {
            TLRENDER_P();

            p.vertexSource = vertexSource;
            p.fragmentSource = fragmentSource;

            shaderName = name;
            
            _createVertexShader();
            _createFragmentShader();
        }
        
        void Shader::_init(const std::string& computeSource,
                           const std::string& name)
        {
            TLRENDER_P();

            p.computeSource = computeSource;

            shaderName = name;
            
            _createComputeShader();
        }

        Shader::Shader(Fl_Vk_Context& context) :
            _p(new Private),
            ctx(context)
        {
        }

        Shader::~Shader()
        {
            TLRENDER_P();

            VkDevice device = ctx.device;

            if (p.fragment != VK_NULL_HANDLE)
                vkDestroyShaderModule(device, p.fragment, nullptr);

            if (p.vertex != VK_NULL_HANDLE)
                vkDestroyShaderModule(device, p.vertex, nullptr);

            if (descriptorSetLayout != VK_NULL_HANDLE)
            {
                vkDestroyDescriptorSetLayout(
                    device, descriptorSetLayout, nullptr);
                descriptorSetLayout = VK_NULL_HANDLE;
            }

            activeBindingSet.reset();
        }

        std::shared_ptr<Shader> Shader::create(
            Fl_Vk_Context& ctx, const std::string& computeSource,
            const std::string& name)
        {
            auto out = std::make_shared<Shader>(ctx);
            out->_init(computeSource, name);
            return out;
        }
        
        std::shared_ptr<Shader> Shader::create(
            Fl_Vk_Context& ctx, const std::string& vertexSource,
            const std::string& fragmentSource, const std::string& name)
        {
            auto out = std::make_shared<Shader>(ctx);
            out->_init(vertexSource, fragmentSource, name);
            return out;
        }

        //! Create a new shader.
        std::shared_ptr<Shader> Shader::create(
            Fl_Vk_Context& ctx,
            const uint32_t* vertexBytes,
            const uint32_t vertexLength,
            const std::string& fragmentSource,
            const std::string& name)
        {
            auto out = std::make_shared<Shader>(ctx);
            out->_init(vertexBytes, vertexLength, fragmentSource, name);
            return out;
        }
            
        //! Create a new shader.
        std::shared_ptr<Shader> Shader::create(
            Fl_Vk_Context& ctx,
            const uint32_t* vertexBytes,
            const uint32_t vertexLength,
            const uint32_t* fragmentBytes,
            const uint32_t fragmentLength,
            const std::string& name)
        {
            auto out = std::make_shared<Shader>(ctx);
            out->_init(vertexBytes, vertexLength, fragmentBytes,
                       fragmentLength, name);
            return out;
        }

        const std::string& Shader::getName() const
        {
            return shaderName;
        }

        const VkShaderModule& Shader::getVertex() const
        {
            return _p->vertex;
        }

        const VkShaderModule& Shader::getFragment() const
        {
            return _p->fragment;
        }

        const std::string& Shader::getVertexSource() const
        {
            return _p->vertexSource;
        }

        const std::string& Shader::getFragmentSource() const
        {
            return _p->fragmentSource;
        }

        void Shader::useBindingSet(const std::shared_ptr<ShaderBindingSet> value)
        {
            activeBindingSet = value;
        }
        
        const VkDescriptorSet Shader::getDescriptorSet() const
        {
            if (!activeBindingSet)
                throw std::runtime_error("No activeBindingSet.  Call useBindingSet first");
            return activeBindingSet->getDescriptorSet(frameIndex);
        }
        
        const VkDescriptorSetLayout Shader::getDescriptorSetLayout() const
        {
            return descriptorSetLayout;
        }

        const VkDescriptorPool Shader::getDescriptorPool() const
        {            
            if (!activeBindingSet)
                throw std::runtime_error("No activeBindingSet.  Call create/useBindingSet first");
            return activeBindingSet->getDescriptorPool(frameIndex);
        }

        void Shader::bind(uint64_t value)
        {
            frameIndex = value;
        }

        void Shader::addStorageBuffer(
            const std::string& name, const ShaderFlags stageFlags)
        {
            StorageBufferBinding t;
            t.binding = current_binding_index++;
            t.stageFlags = getVulkanShaderFlags(stageFlags);
            storageBufferBindings.insert(std::make_pair(name, t));
        }

        void Shader::addStorageImage(
            const std::string& name, const ShaderFlags stageFlags)
        {
            StorageImageBinding t;
            t.binding = current_binding_index++;
            t.stageFlags = getVulkanShaderFlags(stageFlags);
            storageImageBindings.insert(std::make_pair(name, t));
        }

        void Shader::addTexture(
            const std::string& name, const ShaderFlags stageFlags)
        {
            TextureBinding t;
            t.binding = current_binding_index++;
            t.stageFlags = getVulkanShaderFlags(stageFlags);
            textureBindings.insert(std::make_pair(name, t));
        }

        void Shader::setTexture(
            const std::string& name, const std::shared_ptr<Texture>& texture,
            const ShaderFlags stageFlags)
        {
            if (!activeBindingSet)
                throw std::runtime_error("No activeBindingSet.  Call create/useBindingSet first");
            activeBindingSet->updateTexture(name,
                                            activeBindingSet->getDescriptorSet(frameIndex),
                                            texture);
        }

        void
        Shader::addFBO(const std::string& name, const ShaderFlags stageFlags)
        {
            FBOBinding binding;
            binding.binding = current_binding_index++;
            binding.stageFlags = getVulkanShaderFlags(stageFlags);
            fboBindings.insert(std::make_pair(name, binding));
        }

        void Shader::setFBO(
            const std::string& name,
            const std::shared_ptr<OffscreenBuffer>& fbo,
            const ShaderFlags stageFlags)
        {
            if (!fbo)
            {
                throw std::runtime_error(
                    tl::string::Format(
                        "Cannot set FBO '{0}': provided FBO pointer is null.")
                        .arg(name));
            }

            if (!activeBindingSet)
                throw std::runtime_error("No activeBindingSet.  Call create/useBindingSet first");
            activeBindingSet->updateFBO(name,
                                        activeBindingSet->getDescriptorSet(frameIndex),
                                        fbo);
        }

        void Shader::createDescriptorSets()
        {
        }

        void Shader::debugVertexDescriptorSets()
        {

            // UBOs
            for (const auto& [name, ubo] : ubos)
            {
                if ((ubo.layoutBinding.stageFlags &
                     VK_SHADER_STAGE_VERTEX_BIT) == 0)
                    continue;
                std::cerr << name << std::endl;
                std::cerr << "\tbinding " << ubo.layoutBinding.binding
                          << " = UNIFORM_BUFFER size=" << ubo.size << std::endl;
            }

            // Textures
            for (const auto& [name, texture] : textureBindings)
            {
                if ((texture.stageFlags & VK_SHADER_STAGE_VERTEX_BIT) == 0)
                    continue;
                std::cerr << name << std::endl;
                std::cerr << "\tbinding " << texture.binding
                          << " = COMBINED_IMAGE_SAMPLER" << std::endl;
            }

            // FBOs
            for (const auto& [name, fbo] : fboBindings)
            {
                if ((fbo.stageFlags & VK_SHADER_STAGE_VERTEX_BIT) == 0)
                    continue;
                std::cerr << name << std::endl;
                std::cerr << "\tbinding " << fbo.binding
                          << " = COMBINED_IMAGE_SAMPLER" << std::endl;
            }
        }

        void Shader::debugFragmentDescriptorSets()
        {
            // UBOs
            for (const auto& [name, ubo] : ubos)
            {
                if ((ubo.layoutBinding.stageFlags &
                     VK_SHADER_STAGE_FRAGMENT_BIT) == 0)
                    continue;
                std::cerr << name << std::endl;
                std::cerr << "\tbinding " << ubo.layoutBinding.binding
                          << " = UNIFORM_BUFFER size=" << ubo.size << std::endl;
            }

            // Textures
            for (const auto& [name, texture] : textureBindings)
            {
                if ((texture.stageFlags & VK_SHADER_STAGE_FRAGMENT_BIT) == 0)
                    continue;
                std::cerr << name << std::endl;
                std::cerr << "\tbinding " << texture.binding
                          << " = COMBINED_IMAGE_SAMPLER" << std::endl;
            }

            // FBOs
            for (const auto& [name, fbo] : fboBindings)
            {
                if ((fbo.stageFlags & VK_SHADER_STAGE_FRAGMENT_BIT) == 0)
                    continue;
                std::cerr << name << std::endl;
                std::cerr << "\tbinding " << fbo.binding
                          << " = COMBINED_IMAGE_SAMPLER" << std::endl;
            }
        }

        void Shader::debugDescriptorSets()
        {
            if (ubos.empty() && textureBindings.empty() && fboBindings.empty())
            {
                std::cerr << "Shader (" << this << "): has NO bindings"
                          << std::endl;
                return;
            }
            else
            {
                std::cerr << "Shader (" << this << "): has these bindings"
                          << std::endl;
            }

            // UBOs
            for (const auto& [name, ubo] : ubos)
            {
                std::cerr << name << std::endl;
                std::cerr << "\tbinding " << ubo.layoutBinding.binding
                          << " = UNIFORM_BUFFER" << std::endl;
                std::cerr << "\t        "
                          << (ubo.layoutBinding.stageFlags &
                                      VK_SHADER_STAGE_VERTEX_BIT
                                  ? "vertex"
                                  : " ")
                          << (ubo.layoutBinding.stageFlags &
                                      VK_SHADER_STAGE_FRAGMENT_BIT
                                  ? "fragment"
                                  : " ")
                          << std::endl;
            }

            // Textures
            for (const auto& [name, texture] : textureBindings)
            {
                std::cerr << name << std::endl;
                std::cerr << "\tbinding " << texture.binding
                          << " = COMBINED_IMAGE_SAMPLER" << std::endl;
                std::cerr << "\t        "
                          << (texture.stageFlags & VK_SHADER_STAGE_VERTEX_BIT
                                  ? "vertex"
                                  : " ")
                          << (texture.stageFlags & VK_SHADER_STAGE_FRAGMENT_BIT
                                  ? "fragment"
                                  : " ")
                          << std::endl;
            }

            // FBOs
            for (const auto& [name, fbo] : fboBindings)
            {
                std::cerr << name << std::endl;
                std::cerr << "\tbinding " << fbo.binding
                          << " = COMBINED_IMAGE_SAMPLER" << std::endl;
                std::cerr << "\t        "
                          << (fbo.stageFlags & VK_SHADER_STAGE_VERTEX_BIT
                                  ? "vertex"
                                  : " ")
                          << (fbo.stageFlags & VK_SHADER_STAGE_FRAGMENT_BIT
                                  ? "fragment"
                                  : " ")
                          << std::endl;
            }
        }

        void Shader::createPush(const std::string& name, const std::size_t size,
                                const ShaderFlags stageFlags)
        {
            pushSize = size;
            pushStageFlags = getVulkanShaderFlags(stageFlags);
        }
        
        void Shader::debug()
        {
            TLRENDER_P();

            std::cerr << shaderName
                      << " ================================" << std::endl;
            std::cerr << getVertexSource() << std::endl;
            debugVertexDescriptorSets();
            std::cerr << "------------------------------------" << std::endl;
            std::cerr << getFragmentSource() << std::endl;
            std::cerr << "------------------------------------" << std::endl;
            debugFragmentDescriptorSets();
            if (pushSize > 0)
            {
                std::cerr << "------------------------------------" << std::endl;
                std::cerr << "pushSize = " << pushSize << std::endl;
            }
        }
        
        std::shared_ptr<ShaderBindingSet> Shader::createBindingSet()
        {
            VkDevice device = ctx.device;
            VkPhysicalDevice gpu = ctx.gpu;

#ifdef MRV2_NO_VMA
            auto bindingSet = std::make_shared<ShaderBindingSet>(device);
#else
            auto bindingSet = std::make_shared<ShaderBindingSet>(device,
                                                                 ctx.allocator);
#endif
            bindingSet->shaderName = shaderName;
            
            std::vector<VkDescriptorSetLayoutBinding> bindings;
            std::vector<VkDescriptorPoolSize> poolSizes;

            // StorageBuffers
            for (const auto& [name, sb] : storageBufferBindings) {
                VkDescriptorSetLayoutBinding layoutBinding = {};
                layoutBinding.binding = sb.binding;
                layoutBinding.descriptorCount = 1;
                layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                layoutBinding.stageFlags = sb.stageFlags;
                bindings.push_back(layoutBinding);

                VkDescriptorPoolSize poolSize = {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, MAX_FRAMES_IN_FLIGHT};
                poolSizes.push_back(poolSize);
            }

            // StorageImages
            for (const auto& [name, si] : storageImageBindings) {
                VkDescriptorSetLayoutBinding layoutBinding = {};
                layoutBinding.binding = si.binding;
                layoutBinding.descriptorCount = 1;
                layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
                layoutBinding.stageFlags = si.stageFlags;
                bindings.push_back(layoutBinding);

                VkDescriptorPoolSize poolSize = {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, MAX_FRAMES_IN_FLIGHT};
                poolSizes.push_back(poolSize);
            }

            // UBOs
            for (const auto& [_, ubo] : ubos)
            {
                bindings.push_back(ubo.layoutBinding);
                VkDescriptorPoolSize poolSize = {};
                poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                poolSize.descriptorCount =
                    MAX_FRAMES_IN_FLIGHT; // or more if you support arrays
                poolSizes.push_back(poolSize);
            }

            // Textures
            for (const auto& [_, texture] : textureBindings)
            {
                VkDescriptorSetLayoutBinding layoutBinding = {};

                layoutBinding.binding = texture.binding;
                layoutBinding.descriptorCount = 1;
                layoutBinding.descriptorType =
                    VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                layoutBinding.stageFlags = texture.stageFlags;
                layoutBinding.pImmutableSamplers = nullptr;

                bindings.push_back(layoutBinding);

                VkDescriptorPoolSize poolSize{};
                poolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

                // One texture descriptor per frame per binding
                poolSize.descriptorCount = MAX_FRAMES_IN_FLIGHT;
                poolSizes.push_back(poolSize);
            }

            // FBOs
            for (const auto& [_, element] : fboBindings)
            {
                VkDescriptorSetLayoutBinding layoutBinding = {};

                layoutBinding.binding = element.binding;
                layoutBinding.descriptorCount = 1;
                layoutBinding.descriptorType =
                    VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                layoutBinding.stageFlags = element.stageFlags;
                layoutBinding.pImmutableSamplers = nullptr;

                bindings.push_back(layoutBinding);

                VkDescriptorPoolSize poolSize{};
                poolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                // One FBO descriptor per frame per binding
                poolSize.descriptorCount = MAX_FRAMES_IN_FLIGHT;
                poolSizes.push_back(poolSize);
            }

            // Create descriptor pool. (One pool per frame)
            // maxSets should be 1 if allocating 1 set per pool.
            VkDescriptorPoolCreateInfo poolInfo = {};
            poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
            poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
            poolInfo.pPoolSizes = poolSizes.data();
            poolInfo.maxSets = 1;

            // Create a descriptor pool for each frame
            for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
            {
                if (vkCreateDescriptorPool(
                        device, &poolInfo, nullptr, &bindingSet->descriptorPools[i]) !=
                    VK_SUCCESS)
                {
                    throw std::runtime_error(
                        "failed to create descriptor pool!");
                }
            }

            // Create descriptor set layout. (Layout can be shared across
            // frames)
            VkDescriptorSetLayoutCreateInfo layout_info = {};
            layout_info.sType =
                VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            layout_info.bindingCount = static_cast<uint32_t>(bindings.size());
            layout_info.pBindings = bindings.data();

            if (descriptorSetLayout != VK_NULL_HANDLE)
            {
                vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
                descriptorSetLayout = VK_NULL_HANDLE;
            }
            
            if (vkCreateDescriptorSetLayout(
                    device, &layout_info, nullptr, &descriptorSetLayout) !=
                VK_SUCCESS)
            {
                throw std::runtime_error(
                    "failed to create descriptor set layout!");
            }

            // Allocate descriptor sets for each frame, from their respective
            // pools
            std::vector<VkDescriptorSetLayout> layouts(
                MAX_FRAMES_IN_FLIGHT, descriptorSetLayout);
            VkDescriptorSetAllocateInfo allocInfo{};
            allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
            allocInfo.descriptorSetCount = 1; // Allocating one set at a time
            allocInfo.pSetLayouts = layouts.data(); // Still use the same layout

            for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
            {
                allocInfo.descriptorPool = bindingSet->descriptorPools[i];
                if (vkAllocateDescriptorSets(
                        device, &allocInfo, &bindingSet->descriptorSets[i]) != VK_SUCCESS)
                {
                    throw std::runtime_error(
                        "failed to allocate descriptor set for frame " +
                        std::to_string(i) + "!");
                }
            }

            // Initial Update of descriptor sets for UBOs
            // Textures and FBOs will be updated by setTexture/setFBO when the
            // resources are ready. An initial write is often required to
            // transition the descriptor set to a valid state. For image
            // descriptors, you could potentially write a dummy image
            // view/sampler if unbound descriptors are not supported or you need
            // to avoid validation errors. However, the simplest is to ensure
            // setTexture/setFBO is called for each frame's descriptor set
            // before that set is first bound in a command buffer.
        
            // Step 3: populate UBOs for each uniform
            for (const auto& [name, uboTemplate] : ubos)
            {
                ShaderBindingSet::UniformParameter ubo;
                ubo.size = uboTemplate.size;
                ubo.layoutBinding = uboTemplate.layoutBinding;

                ubo.buffers.resize(MAX_FRAMES_IN_FLIGHT);
                ubo.infos.resize(MAX_FRAMES_IN_FLIGHT);
#ifdef MRV2_NO_VMA
                ubo.memories.resize(MAX_FRAMES_IN_FLIGHT);
#else
                ubo.allocation.resize(MAX_FRAMES_IN_FLIGHT);
#endif

                for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
                {
                    VkBufferCreateInfo bufferInfo = {};
                    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
                    bufferInfo.size = ubo.size;
                    bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
                    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

#ifdef MRV2_NO_VMA
                    VK_CHECK(vkCreateBuffer(device, &bufferInfo, nullptr, &ubo.buffers[i]));

                    VkMemoryRequirements memReqs;
                    vkGetBufferMemoryRequirements(device, ubo.buffers[i], &memReqs);

                    VkMemoryAllocateInfo allocInfo = {};
                    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
                    allocInfo.allocationSize = memReqs.size;
                    allocInfo.memoryTypeIndex = findMemoryType(
                        gpu,
                        memReqs.memoryTypeBits,
                        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
            
                    VK_CHECK(vkAllocateMemory(device, &allocInfo, nullptr, &ubo.memories[i]));
                    VK_CHECK(vkBindBufferMemory(device, ubo.buffers[i], ubo.memories[i], 0));
#else
                    VmaAllocationCreateInfo allocInfo = {};
                    allocInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;  // or VMA_MEMORY_USAGE_AUTO_PREFER_HOST
                    
                    VmaAllocation allocation;
                    vmaCreateBuffer(ctx.allocator, &bufferInfo, &allocInfo,
                                    &ubo.buffers[i], &allocation,
                                    nullptr);
                    ubo.allocation[i] = allocation;
#endif
                    ubo.infos[i].buffer = ubo.buffers[i];
                    ubo.infos[i].offset = 0;
                    ubo.infos[i].range = ubo.size;

                    // Descriptor write to binding
                    VkWriteDescriptorSet write{};
                    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                    write.dstSet = bindingSet->descriptorSets[i];
                    write.dstBinding = ubo.layoutBinding.binding;
                    write.dstArrayElement = 0;
                    write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                    write.descriptorCount = 1;
                    write.pBufferInfo = &ubo.infos[i];

                    vkUpdateDescriptorSets(device, 1, &write, 0, nullptr);
                }

                bindingSet->uniforms[name] = std::move(ubo);
            }

            // Step 4: Texture bindings
            for (const auto& [name, texBinding] : textureBindings)
            {
                ShaderBindingSet::TextureParameter textureInfo;
                textureInfo.binding = texBinding.binding;
                textureInfo.stageFlags = texBinding.stageFlags;
                bindingSet->textures[name] = textureInfo;
            }
            
            // Step 5: FBO bindings
            for (const auto& [name, fboBinding] : fboBindings)
            {
                ShaderBindingSet::FBOParameter fboInfo;
                fboInfo.binding = fboBinding.binding;
                fboInfo.stageFlags = fboBinding.stageFlags;
                bindingSet->fbos[name] = fboInfo;
            }

            // Make this set active
            activeBindingSet = bindingSet;
            
            return bindingSet;
        }

    } // namespace vlk
} // namespace tl
