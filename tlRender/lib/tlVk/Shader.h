// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// Copyright (c) 2025-Present Gonzalo Garramuño
// All rights reserved.

#pragma once

#include <tlVk/Texture.h>
#include <tlVk/OffscreenBuffer.h>
#include <tlVk/ShaderBindingSet.h>
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
        };

        inline VkShaderStageFlags getVulkanShaderFlags(ShaderFlags stageFlags)
        {
            VkShaderStageFlags out = 0;
            out =
                (stageFlags & kShaderFragment ? VK_SHADER_STAGE_FRAGMENT_BIT
                                              : 0);
            out |=
                (stageFlags & kShaderVertex ? VK_SHADER_STAGE_VERTEX_BIT : 0);
            return out;
        }

        //! Vulkan shader.
        class Shader : public std::enable_shared_from_this<Shader>
        {
            TLRENDER_NON_COPYABLE(Shader);
            
        public:
            Shader(Fl_Vk_Context& ctx);
            ~Shader();
            
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


            //! Create a new shader.
            static std::shared_ptr<Shader> create(
                Fl_Vk_Context& ctx,
                const std::string& vertexSource,
                const std::string& fragmentSource,
                const std::string& name = "");
            
            //! Create a new shader.
            static std::shared_ptr<Shader> create(
                Fl_Vk_Context& ctx,
                const uint32_t* vertexBytes,
                const uint32_t vertexLength,
                const std::string& fragmentSource,
                const std::string& name = "");
            
            //! Create a new shader.
            static std::shared_ptr<Shader> create(
                Fl_Vk_Context& ctx,
                const uint32_t* vertexBytes,
                const uint32_t vertexLength,
                const uint32_t* fragmentBytes,
                const uint32_t fragmentLength,
                const std::string& name = "");

            //! Get the shader name.
            const std::string& getName() const;

            //! Get the vertex shader source.
            const std::string& getVertexSource() const;

            //! Get the fragment shader source.
            const std::string& getFragmentSource() const;

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

            //! Set and upload a uniform UBO variable
            template <typename T>
            void setUniform(
                const std::string&, const T& value,
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
            VkShaderStageFlags getPushStageFlags() { return pushStageFlags; }

            //! Get the push size.
            std::size_t getPushSize() { return pushSize; }

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

            //! Create desciptor set bindings for all frames
            void createDescriptorSets();

            //! Create a new shader binding set.
            std::shared_ptr<ShaderBindingSet> createBindingSet();

            //! Bind a ShaderBindingSet to this shader.
            void useBindingSet(const std::shared_ptr<ShaderBindingSet>);
            
            //! Print out a list of descriptor set bindings for vertex shader.
            void debugVertexDescriptorSets();

            //! Print out a list of descriptor set bindings for fragment shader.
            void debugFragmentDescriptorSets();

            //! Print out a list of descriptor set bindings for both shaders
            //! types.
            void debugDescriptorSets();

            //! Print out all debug info.
            void debug();

        private:
            void _createVertexShader();
            void _createFragmentShader();
            
            Fl_Vk_Context& ctx;
            
            std::string shaderName = "Shader";

            //! Counter used in binding UBOs, Textures and FBOs.
            uint32_t current_binding_index = 0;

            //! Descriptor set layout shared across sets and pools.
            VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;

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

            std::shared_ptr<ShaderBindingSet> activeBindingSet;

            TLRENDER_PRIVATE();
        };
    } // namespace vlk
} // namespace tl

#include <tlVk/ShaderInline.h>
