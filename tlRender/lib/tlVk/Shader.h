// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// Copyright (c) 2025-Present Gonzalo Garramuño
// All rights reserved.

#pragma once

#include <tlVk/Texture.h>
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
                const std::string& fragmentSource);

            //! Get the vertex shader source.
            const std::string& getVertexSource() const;

            //! Get the fragment shader source.
            const std::string& getFragmentSource() const;

            //! Get the Vulkan vertex module.
            const VkShaderModule& getVertex() const;

            //! Get the Vulkan fragment module.
            const VkShaderModule& getFragment() const;

            //! Get the Vulkan description set.
            const VkDescriptorSet& getDescriptorSet() const;

            //! Get the Vulkan description set.
            const VkDescriptorSetLayout& getDescriptorSetLayout() const;

            //! Get the Vulkan description set.
            const VkDescriptorPool& getDescriptorPool() const;

            //! Bind the shader.
            void bind();

            //! \name Uniform
            //! Set uniform values.
            ///@{

            template <typename T>
            void createUniform(
                const std::string&, const T& value,
                const VkShaderStageFlags stageFlags =
                    VK_SHADER_STAGE_FRAGMENT_BIT);

            template <typename T>
            void setUniform(
                const std::string&, const T& value,
                const VkShaderStageFlags stageFlags =
                    VK_SHADER_STAGE_FRAGMENT_BIT);
            ///@}

            void addTexture(
                const std::string& name, 
                const std::shared_ptr<Texture>& texture,
                const VkShaderStageFlags stageFlags =
                VK_SHADER_STAGE_FRAGMENT_BIT);
            void setTexture(
                const std::string& name,
                const std::shared_ptr<Texture>& texture,
                const VkShaderStageFlags stageFlags =
                VK_SHADER_STAGE_FRAGMENT_BIT);

            void createDescriptorSet();

        private:
            Fl_Vk_Context& ctx;
            uint32_t current_binding_index = 0;

            VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
            VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
            VkDescriptorSet descriptorSet = VK_NULL_HANDLE;

            struct UBO
            {
                VkDescriptorBufferInfo bufferInfo;
                VkDescriptorSetLayoutBinding layoutBinding;

                VkBuffer buffer;
                VkDeviceMemory memory;
                size_t size;
            };
            std::map<std::string, UBO> ubos;

            struct TextureBinding
            {
                uint32_t binding;
                VkShaderStageFlags stageFlags;
                std::shared_ptr<Texture> texture;
            };

            std::map<std::string, TextureBinding> textureBindings;

            TLRENDER_PRIVATE();
        };
    } // namespace vlk
} // namespace tl

#include <tlVk/ShaderInline.h>
