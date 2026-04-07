// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// Copyright (c) 2025-Present Gonzalo Garramuño
// All rights reserved.

#pragma once

#include <tlVk/Texture.h>
#include <tlVk/OffscreenBuffer.h>
#include <tlVk/ShaderBindingSet.h>
#include <tlVk/DescriptorSetLayout.h>
#include <tlVk/Vk.h>

#include <tlCore/Color.h>
#include <tlCore/Matrix.h>
#include <tlCore/Util.h>
#include <tlCore/Vector.h>

#include <memory>
#include <string>
#include <vector>
#include <cstdint>

namespace tl
{
    namespace vlk
    {

        enum ShaderFlags {
            kShaderVertex = 1,
            kShaderFragment = 2,
            kShaderCompute = 4,
        };

        inline VkShaderStageFlags getVulkanShaderFlags(ShaderFlags stageFlags)
        {
            VkShaderStageFlags out = 0;
            out =
                (stageFlags & kShaderFragment ? VK_SHADER_STAGE_FRAGMENT_BIT
                                              : 0);
            out |=
                (stageFlags & kShaderVertex ? VK_SHADER_STAGE_VERTEX_BIT : 0);
            out |=
                (stageFlags & kShaderCompute ? VK_SHADER_STAGE_COMPUTE_BIT : 0);
            return out;
        }

        //! Vulkan shader.
        class Shader : public std::enable_shared_from_this<Shader>
        {
            TLRENDER_NON_COPYABLE(Shader);
            
        public:
            Shader(Fl_Vk_Context& ctx);
            ~Shader();
            
            void _init(const std::string& computeSource,
                       const std::string& name);
            void _init(const uint32_t* computeBytes,
                       const uint32_t computeLength,
                       const std::string& name);
            void _init(const std::string& vertexSource,
                       const std::string& fragmentSource,
                       const std::string& name);
            void _init(const uint32_t* vertexBytes, const uint32_t vertexLength,
                       const std::string& fragmentSource,
                       const std::string& name);
            void _init(const uint32_t* vertexBytes, const uint32_t vertexLength,
                       const uint32_t* fragmentBytes,
                       const uint32_t fragmentLength,
                       const std::string& name);


            //! Create a new compute shader.
            static std::shared_ptr<Shader> create(
                Fl_Vk_Context& ctx,
                const std::string& computeSource,
                const std::string& name);
            
            //! Create a new compute shader from SPIRV.
            static std::shared_ptr<Shader> create(
                Fl_Vk_Context& ctx,
                const uint32_t* computeBytes,
                const uint32_t computeLength,
                const std::string& name);
            
            //! Create a new vertex and fragment shader from source.
            static std::shared_ptr<Shader> create(
                Fl_Vk_Context& ctx,
                const std::string& vertexSource,
                const std::string& fragmentSource,
                const std::string& name);
            
            //! Create a new vertex shader from SPIRV and fragment from source.
            static std::shared_ptr<Shader> create(
                Fl_Vk_Context& ctx,
                const uint32_t* vertexBytes,
                const uint32_t vertexLength,
                const std::string& fragmentSource,
                const std::string& name);
            
            //! Create a new vertex and fragment shaders from SPIRV.
            static std::shared_ptr<Shader> create(
                Fl_Vk_Context& ctx,
                const uint32_t* vertexBytes,
                const uint32_t vertexLength,
                const uint32_t* fragmentBytes,
                const uint32_t fragmentLength,
                const std::string& name = "");

            //! Get the shader name.
            const std::string& getName() const;

            //! Get the Vulkan vertex module.
            const VkShaderModule& getVertex() const;

            //! Get the Vulkan fragment module.
            const VkShaderModule& getFragment() const;

            //! Get the Vulkan description set for current frame.
            const VkDescriptorSet getDescriptorSet() const;

            //! Get the Vulkan description set layout for current frame.
            const VkDescriptorSetLayout getDescriptorSetLayout() const;

            //! Get the Vulkan description pool for current frame.
            const VkDescriptorPool getDescriptorPool() const;

            //! Bind the shader.
            void bind(uint64_t value);

            //! \name Uniform
            //! Set uniform values.
            ///@{

            //! Create a uniform UBO variable.
            template <typename T>
            void createUniform(
                const std::string&, const T& value,
                const ShaderFlags stageFlags = kShaderFragment);
            
            // Create uniform UBO from a size
            inline void createUniformData(const std::string&, const size_t size,
                                   const ShaderFlags stageFlags = kShaderFragment);

            //! Set and upload a uniform UBO variable
            template <typename T>
            void setUniform(
                const std::string&, const T& value,
                const ShaderFlags stageFlags = kShaderFragment);

            // Set and upload a uniform UBO from a void* and size
            inline void setUniformData(const std::string&, const void* data, const size_t size,
                                       const ShaderFlags stageFlags = kShaderFragment);
            ///@}

            //! Add a push block.
            template <typename T>
            void addPush(
                const std::string&, const T& value,
                const ShaderFlags stageFlags = kShaderFragment);

            //! Createa a push block given a size.
            void createPush(const std::string& name, const std::size_t size,
                            const ShaderFlags stageFlags);
            
            //! Get the push stage flags.
            inline VkShaderStageFlags getPushStageFlags() { return pushStageFlags; }

            //! Get the push size.
            inline std::size_t getPushSize() { return pushSize; }

            //! Add a textire to shader parameters.
            void addTexture(
                const std::string& name,
                const ShaderFlags stageFlags = kShaderFragment);

            //! Attach and upload a texture to shader parameters.
            void setTexture(
                const std::string& name,
                const std::shared_ptr<Texture>& texture,
                const ShaderFlags stageFlags = kShaderFragment);
            
            //! Add and FBO to list of shader parameters.
            void addFBO(
                const std::string& name,
                const ShaderFlags stageFlags = kShaderFragment);

            //! Attach an FBO and updata shader parameters.
            void setFBO(
                const std::string& name,
                const std::shared_ptr<OffscreenBuffer>&,
                const ShaderFlags stageFlags = kShaderFragment);

            //! Add a Storage Buffer to shader.
            void addStorageBuffer(const std::string& name, 
                                  const ShaderFlags stageFlags = kShaderCompute);

            //! Attach an Storage Buffer and updata shader parameters.
            void setStorageBuffer(
                const std::string& name,
                const uint8_t* data,
                const std::size_t size);
            
            //! Add a Storage Image to shader.
            void addStorageImage(const std::string& name, 
                                 const ShaderFlags stageFlags = kShaderCompute);
            
            //! Attach and upload a texture to shader parameters.
            void setStorageImage(
                const std::string& name,
                const std::shared_ptr<Texture>& texture);

            //! Attach a SSBO (Shader Storage Buffer Object).
            template <typename T>
            void addSSBO(const std::string& name,
                         const T& value,
                         const ShaderFlags stageFlags = kShaderCompute);

            //! Set a SSBO (Shader Storage Buffer Object) initial value.
            template <typename T>
            void setSSBO(const std::string& name, const T& value);

            //! Clear SSBO values to 0
            void clearSSBO(
                VkCommandBuffer cmd,
                const std::string& name);

            //! Map the SSBO data.
            void* mapSSBO(const std::string& name);

            //! Unmap the SSBO data.
            void unmapSSBO(const std::string& name);
            
            //! Create a new shader binding set.
            std::shared_ptr<ShaderBindingSet> createBindingSet();

            //! Bind a ShaderBindingSet to this shader.
            void useBindingSet(const std::shared_ptr<ShaderBindingSet>);

            //! Create a pipelineLayout
            void createPipelineLayout();
            
            //! Create a compute pipeline from this shader.
            //! Must be called after createBindingSet.
            void createComputePipeline();

            //! Dispatch compute shader.
            void dispatch(VkCommandBuffer cmd,
                          uint32_t groupCountX, uint32_t groupCountY,
                          uint32_t groupCountZ = 1);

            //! Print out a list of descriptor set bindings for vertex shader.
            void debugVertexDescriptorSets();

            //! Print out a list of descriptor set bindings for fragment shader.
            void debugFragmentDescriptorSets();

            //! Print out a list of descriptor set bindings for both shaders
            //! types.
            void debugDescriptorSets();

        private:
            void _createVertexShader(const std::string&);
            void _createFragmentShader(const std::string&);
            void _createComputeShader(const std::string&);
            
            Fl_Vk_Context& ctx;
            
            std::string shaderName = "Shader";

            //! Counter used in binding UBOs, Textures and FBOs.
            uint32_t current_binding_index = 0;

            //! Descriptor set layout shared across sets and pools.
            std::shared_ptr<DescriptorSetLayout> activeLayout;

            //! Internal variable used to point all resources to the right
            //! frame in flight.
            uint64_t frameIndex = 0;

            //! Push size (if shader has a push parameter).
            std::size_t pushSize = 0;

            //! Push flags (if shader has a push parameter).
            VkShaderStageFlags pushStageFlags =
                VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

            typedef ShaderBindingSet::UniformParameter UBOBinding;
            std::map<std::string, UBOBinding> ubos;

            typedef ShaderBindingSet::TextureParameter TextureBinding;
            std::map<std::string, TextureBinding> textureBindings;

            typedef ShaderBindingSet::FBOParameter FBOBinding;
            std::map<std::string, FBOBinding> fboBindings;

            // Add these maps to your Shader class members
            typedef ShaderBindingSet::StorageBufferParameter StorageBufferBinding;
            std::map<std::string, StorageBufferBinding> storageBufferBindings;
            
            typedef ShaderBindingSet::StorageImageParameter StorageImageBinding;
            std::map<std::string, StorageImageBinding> storageImageBindings;

            typedef ShaderBindingSet::SSBOParameter SSBOBinding;
            std::map<std::string, SSBOBinding> ssbos;
            
            std::shared_ptr<ShaderBindingSet> activeBindingSet;

            TLRENDER_PRIVATE();
        };
    } // namespace vlk
} // namespace tl

#include <tlVk/ShaderInline.h>
