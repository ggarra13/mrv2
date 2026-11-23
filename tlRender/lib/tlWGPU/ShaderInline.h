// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2025-Present Gonzalo Garramuño
// All rights reserved.

#pragma once

#include <stdexcept>

namespace tl
{
    namespace wgpu
    {

        void Shader::createUniformData(
            const std::string& name, const size_t size,
            const ShaderFlags stageFlags)
        {
            auto it = ubos.find(name);
            if (it != ubos.end())
            {
                throw std::runtime_error(
                    name + " for shader " + shaderName + " already created.");
            }

            UBOBinding ubo;
            ubo.size = size;
            ubo.binding = current_binding_index++;
            ubo.visibility = getWGPUShaderFlags(stageFlags);

            ubos[name] = ubo;
            layoutDirty = true;
        }

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

            createUniformData(name, sizeof(value), stageFlags);
        }

        template <typename T>
        void Shader::setUniform(
            const std::string& name, const T& value,
            const ShaderFlags stageFlags)
        {
            if (!activeBindingSet)
                throw std::runtime_error(
                    "No activeBindingSet for Shader " + shaderName +
                    " parameter " + name);
            activeBindingSet->updateUniform(name, &value, sizeof(value));
        }

        void Shader::setUniformData(
            const std::string& name, const void* data, const size_t size,
            const ShaderFlags stageFlags)
        {
            if (!activeBindingSet)
                throw std::runtime_error(
                    "No activeBindingSet for Shader " + shaderName +
                    " parameter " + name);
            activeBindingSet->updateUniform(name, data, size);
        }

        template <typename T>
        void Shader::addPush(
            const std::string& name, const T& value,
            const ShaderFlags stageFlags)
        {
            createPush(name, sizeof(T), stageFlags);
        }

        //! Attach a SSBO (Shader Storage Buffer Object).
        template <typename T>
        void Shader::addSSBO(
            const std::string& name, const T& value,
            const ShaderFlags stageFlags)
        {
            auto it = ssbos.find(name);
            if (it != ssbos.end())
            {
                throw std::runtime_error(
                    "SSBO " + name + " for shader " + shaderName +
                    " already created.");
            }

            SSBOBinding ssbo;
            ssbo.size = sizeof(value);
            ssbo.binding = current_binding_index++;
            ssbo.visibility = getWGPUShaderFlags(stageFlags);

            ssbos[name] = ssbo;
            layoutDirty = true;
        }

        //! Set a SSBO (Shader Storage Buffer Object) initial value.
        template <typename T>
        void Shader::setSSBO(const std::string& name, const T& value)
        {
            if (!activeBindingSet)
                throw std::runtime_error(
                    "No activeBindingSet for Shader " + shaderName +
                    " parameter " + name);
            activeBindingSet->updateSSBO(name, &value, sizeof(value));
        }

    } // namespace webgpu
} // namespace tl
