// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// Copyright (c) 2025-Present Gonzalo Garramuño
// All rights reserved.

#pragma once

#include <tlVk/Texture.h>
#include <tlVk/OffscreenBuffer.h>
#include <tlVk/Vk.h>

#include <tlCore/Color.h>
#include <tlCore/Matrix.h>
#include <tlCore/Util.h>
#include <tlCore/Vector.h>

#include <memory>
#include <string>
#include <vector>

namespace tl
{
    namespace vlk
    {
        enum ShaderFlags
        {
            kShaderVertex   = 1,
            kShaderFragment = 2,
        };
        
        const int MAX_FRAMES_IN_FLIGHT = 10;
        
        inline VkShaderStageFlags getVulkanShaderFlags(ShaderFlags stageFlags)
        {
            VkShaderStageFlags out = 0;
            out  = (stageFlags & kShaderFragment ? VK_SHADER_STAGE_FRAGMENT_BIT : 0);
            out |= (stageFlags & kShaderVertex ? VK_SHADER_STAGE_VERTEX_BIT : 0);
            return out;
        }
        
        //! Vulkan shader.
        class Shader : public std::enable_shared_from_this<Shader>
        {
            TLRENDER_NON_COPYABLE(Shader);

        protected:
            void _init();

            Shader(Fl_Vk_Context& ctx);

        public:
            ~Shader();

            //! Create a new shader.
            static std::shared_ptr<Shader> create(
                Fl_Vk_Context& ctx, const std::string& vertexSource,
                const std::string& fragmentSource,
                const std::string& name = "");

            //! Get the vertex shader source.
            const std::string& getVertexSource() const;

            //! Get the fragment shader source.
            const std::string& getFragmentSource() const;

            //! Get the Vulkan vertex module.
            const VkShaderModule& getVertex() const;

            //! Get the Vulkan fragment module.
            const VkShaderModule& getFragment() const;

            //! Get the Vulkan description set for current frame.
            const VkDescriptorSet& getDescriptorSet(int frameIndex) const;

            //! Get the Vulkan description set layout for current frame.
            const VkDescriptorSetLayout& getDescriptorSetLayout() const;

            //! Get the Vulkan description pool for current frame.
            const VkDescriptorPool& getDescriptorPool(int frameIndex) const;

            //! Bind the shader.
            void bind();

            //! \name Uniform
            //! Set uniform values.
            ///@{

            //! Create a uniform UBO variable.
            template <typename T>
            void createUniform(
                const std::string&, const T& value,
                const ShaderFlags stageFlags = kShaderFragment);

            //! Set and upload a uniform UBO variable 
            template <typename T>
            void setUniform(
                const std::string&, const T& value,
                const int frameIndex = 0,
                const ShaderFlags stageFlags = kShaderFragment);
            ///@}

            //! Add a textire to shader parameters.
            void addTexture(
                const std::string& name, 
                const ShaderFlags stageFlags = kShaderFragment);

            //! Attach and upload a texture to shader parameters.
            void setTexture(
                const std::string& name,
                const std::shared_ptr<Texture>& texture,
                const int frameIndex = 0,
                const ShaderFlags stageFlags = kShaderFragment);

            //! Add and FBO to list of shader parameters.
            void addFBO(
                const std::string& name,
                const ShaderFlags stageFlags = kShaderFragment);

            //! Attach an FBO and updata shader parameters.
            void setFBO(
                const std::string& name,
                const std::shared_ptr<OffscreenBuffer>&,
                const int frameIndex = 0,
                const ShaderFlags stageFlags = kShaderFragment);
            
            //! Create desciptor set bindings for all frames
            void createDescriptorSets();

            //! Print out a list of descriptor set bindings.
            void debugDescriptorSets();

            //! Print out a list of pointers for all frames.
            void debugPointers();

            //! Print out all debug info.
            void debug();
            
        private:
            Fl_Vk_Context& ctx;
            uint32_t current_binding_index = 0;

            VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
            std::vector<VkDescriptorPool> descriptorPools;
            std::vector<VkDescriptorSet> descriptorSets;

            struct UBO
            {
                VkDescriptorSetLayoutBinding layoutBinding;
                

                std::vector<VkBuffer> buffers;
                std::vector<VkDeviceMemory> memories;
                std::vector<VkDescriptorBufferInfo> bufferInfos;
                
                size_t size;
            };
            std::map<std::string, UBO> ubos;

            struct TextureBinding
            {
                uint32_t binding;
                VkShaderStageFlags stageFlags;
                std::shared_ptr<Texture> texture; // texture can be shared
            };

            std::map<std::string, TextureBinding> textureBindings;
            
            struct FBOBinding
            {
                uint32_t binding;
                VkShaderStageFlags stageFlags;
                std::shared_ptr<OffscreenBuffer> fbo;  // fbo can be shared
            };

            std::map<std::string, FBOBinding> fboBindings;

            TLRENDER_PRIVATE();
        };
    } // namespace vlk
} // namespace tl

#include <tlVk/ShaderInline.h>
