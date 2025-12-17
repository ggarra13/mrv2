// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2025-Present Gonzalo Garramuño
// All rights reserved.

#include <FL/Fl_Vk_Utils.H>

namespace tl
{
    namespace vlk
    {

        template <typename T>
        void Shader::createUniform(
            const std::string& name, const T& value,
            const ShaderFlags stageFlags)
        {
            auto it = ubos.find(name);
            if (it != ubos.end())
            {
                throw std::runtime_error(
                    name + " for shader " + shaderName + " already created.");
            }

            UBOBinding ubo;
            ubo.size = sizeof(value);

            ubo.layoutBinding.binding = current_binding_index++;
            ubo.layoutBinding.descriptorCount = 1;
            ubo.layoutBinding.descriptorType =
                VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            ubo.layoutBinding.stageFlags = getVulkanShaderFlags(stageFlags);
            ubo.layoutBinding.pImmutableSamplers = nullptr;

            ubos[name] = ubo;
        }

        template <typename T>
        void Shader::setUniform(
            const std::string& name, const T& value,
            const ShaderFlags stageFlags)
        {
            if (!activeBindingSet)
                throw std::runtime_error("No activeBindingSet for Shader " + name);
            activeBindingSet->updateUniform(name, &value, sizeof(value), frameIndex);
        }

        template <typename T>
        void Shader::addPush(
            const std::string& name, const T& value,
            const ShaderFlags stageFlags)
        {
            pushSize = sizeof(T);
            pushStageFlags = getVulkanShaderFlags(stageFlags);
        }

        inline
        void Shader::setStorageBuffer(
            const std::string& name, 
            VkBuffer buffer, 
            VkDeviceSize size)
        {
            if (!activeBindingSet)
                throw std::runtime_error("No activeBindingSet for Shader " + name);
    
            // We pass the specific buffer and size to the binding set
            activeBindingSet->updateStorageBuffer(name, 
                                                  activeBindingSet->getDescriptorSet(frameIndex), 
                                                  buffer, 
                                                  size);
        }

        inline
        void Shader::setStorageImage(
            const std::string& name, 
            const std::shared_ptr<Texture>& texture)
        {
            if (!activeBindingSet)
                throw std::runtime_error("No activeBindingSet for Shader " + name);
    
            activeBindingSet->updateStorageImage(name, 
                                                 activeBindingSet->getDescriptorSet(frameIndex), 
                                                 texture);
        }
        
    } // namespace vlk
} // namespace tl
