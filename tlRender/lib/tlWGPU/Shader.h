// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// Copyright (c) 2025-Present Gonzalo Garramuño
// All rights reserved.

#pragma once

#include <tlWGPU/OffscreenBuffer.h>
#include <tlWGPU/ShaderBindingSet.h>
#include <tlWGPU/Texture.h>
#include <tlWGPU/WGPU.h>

#include <tlCore/Color.h>
#include <tlCore/Matrix.h>
#include <tlCore/Util.h>
#include <tlCore/Vector.h>

#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace tl
{
    namespace wgpu
    {

        enum ShaderFlags {
            kShaderVertex = 1,
            kShaderFragment = 2,
            kShaderCompute = 4,
            kShaderGeometry = 8, // No geometry shaders in WebGPU: ignored.
        };

        inline ShaderFlags operator|(ShaderFlags a, ShaderFlags b)
        {
            return static_cast<ShaderFlags>(
                static_cast<int>(a) | static_cast<int>(b));
        }

        inline WGPUShaderStage getWGPUShaderFlags(ShaderFlags stageFlags)
        {
            WGPUShaderStage out = 0;
            out |= (stageFlags & kShaderFragment ? WGPUShaderStage_Fragment : 0);
            out |= (stageFlags & kShaderVertex ? WGPUShaderStage_Vertex : 0);
            out |= (stageFlags & kShaderCompute ? WGPUShaderStage_Compute : 0);
            return out;
        }

        //! WebGPU shader (WGSL).
        //!
        //! Resource binding contract (everything lives in @group(0), and
        //! binding numbers are handed out in declaration order, exactly as in
        //! the Vulkan version):
        //!
        //!   createUniform / addSSBO / addStorageBuffer /
        //!   addStorageImage / createPush        1 binding each
        //!   addTexture                          2 bindings: the texture at
        //!                                       N and its sampler at N+1
        //!
        //! debugDescriptorSets() prints the resulting numbers.
        //!
        //! The WGSL entry point is "main" unless changed with the
        //! set*EntryPoint() functions (or passed to the combined-source
        //! create()).
        class Shader : public std::enable_shared_from_this<Shader>
        {
            TLRENDER_NON_COPYABLE(Shader);

        public:
            //! \p instance, \p device and \p queue are not owned and must
            //! outlive the shader.
            Shader(Fl_WebGPU_Context&);
            ~Shader();

            void _init(
                const std::string& computeSource, const std::string& name);
            void _init(
                const std::string& vertexSource,
                const std::string& fragmentSource, const std::string& name);
            void _init(
                const std::string& source, const std::string& vertexEntry,
                const std::string& fragmentEntry, const std::string& name);

            //! Create a new compute shader from WGSL source.
            static std::shared_ptr<Shader> create(
                Fl_WebGPU_Context&,
                const std::string& computeSource, const std::string& name);

            //! Create a new vertex and fragment shader from WGSL source
            //! (two modules).
            static std::shared_ptr<Shader> create(
                Fl_WebGPU_Context&,
                const std::string& vertexSource,
                const std::string& fragmentSource, const std::string& name);

            //! Create a new vertex and fragment shader from a single WGSL
            //! module containing both entry points.
            static std::shared_ptr<Shader> create(
                Fl_WebGPU_Context&,
                const std::string& source,
                const std::string& vertexEntry,
                const std::string& fragmentEntry, const std::string& name);

            //! Get the shader name.
            const std::string& getName() const;

            //! Get the vertex module.
            const WGPUShaderModule& getVertex() const;

            //! Get the fragment module.
            const WGPUShaderModule& getFragment() const;

            //! Get the entry points (default "main").
            const std::string& getVertexEntryPoint() const;
            const std::string& getFragmentEntryPoint() const;
            const std::string& getComputeEntryPoint() const;
            void setVertexEntryPoint(const std::string&);
            void setFragmentEntryPoint(const std::string&);
            void setComputeEntryPoint(const std::string&);

            //! Get the vertex source if available.
            const std::string& getVertexSource() const;

            //! Get the fragment source if available.
            const std::string& getFragmentSource() const;

            //! Get the compute source if available.
            const std::string& getComputeSource() const;

            //! Get the bind group of the active binding set (rebuilt if a
            //! resource changed).
            WGPUBindGroup getBindGroup();

            //! Get the bind group layout (for building render pipelines).
            WGPUBindGroupLayout getBindGroupLayout() const;

            //! Get a pipeline layout containing the bind group layout, for
            //! render or compute pipelines.  Created on first use.
            WGPUPipelineLayout getPipelineLayout();

            //! Start of frame.  Resets the push-block allocator when the
            //! value differs from the previous call, so it is safe to call
            //! from several places per frame.  The value is no longer used
            //! to select per-frame resources (see ShaderBindingSet).
            void bind(uint64_t value);

            //! Unconditionally reset the push-block allocator.  Call once
            //! per frame (after the previous frame was submitted) if you do
            //! not use bind() with a changing index.
            void beginFrame();

            //! \name Uniform
            //! Set uniform values.
            ///@{

            //! Create a uniform UBO variable.
            template <typename T>
            void createUniform(
                const std::string&, const T& value,
                const ShaderFlags stageFlags = kShaderFragment);

            // Create uniform UBO from a size
            inline void createUniformData(
                const std::string&, const size_t size,
                const ShaderFlags stageFlags = kShaderFragment);

            //! Set and upload a uniform UBO variable
            template <typename T>
            void setUniform(
                const std::string&, const T& value,
                const ShaderFlags stageFlags = kShaderFragment);

            // Set and upload a uniform UBO from a void* and size
            inline void setUniformData(
                const std::string&, const void* data, const size_t size,
                const ShaderFlags stageFlags = kShaderFragment);
            ///@}

            //! \name Push block
            //! WebGPU has no push constants.  They are emulated with a
            //! uniform buffer bound with a dynamic offset, so every draw
            //! still sees its own data.  Declare it in WGSL as an ordinary
            //! `var<uniform>` at the binding number it was given (declaration
            //! order).  Pass the data to bindToPass().
            ///@{

            //! Add a push block.
            template <typename T>
            void addPush(
                const std::string&, const T& value,
                const ShaderFlags stageFlags = kShaderFragment);

            //! Create a push block given a size.
            void createPush(
                const std::string& name, const std::size_t size,
                const ShaderFlags stageFlags);

            //! Get the push stage flags.
            inline WGPUShaderStage getPushStageFlags() { return pushStageFlags; }

            //! Get the push size.
            inline std::size_t getPushSize() { return pushSize; }

            //! Set the capacity (in bytes) of the push-block ring.  Each push
            //! uses its size rounded up to the device's
            //! minUniformBufferOffsetAlignment (256 by default), so the
            //! default 2 MiB allows ~8000 pushes per frame.  Call before
            //! createBindingSet().
            void setPushCapacity(uint64_t bytes);

            ///@}

            //! Bind the active binding set (and push data, if the shader has
            //! a push block) to a pass.  Replaces vkCmdBindDescriptorSets
            //! plus vkCmdPushConstants.  If \p pushData is null the previous
            //! push data is reused.
            void bindToPass(
                WGPURenderPassEncoder pass, const void* pushData = nullptr,
                uint32_t groupIndex = 0);
            void bindToPass(
                WGPUComputePassEncoder pass, const void* pushData = nullptr,
                uint32_t groupIndex = 0);

            //! Add a texture to shader parameters.  Occupies two bindings
            //! (texture, then sampler).  \p type and \p dimension are baked
            //! into the layout and must match the texture set later.
            void addTexture(
                const std::string& name,
                const ShaderFlags stageFlags = kShaderFragment,
                const ShaderTextureType type = ShaderTextureType::Float,
                const ShaderTextureDimension dimension =
                    ShaderTextureDimension::_2D);

            //! Attach a texture to shader parameters.
            void setTexture(
                const std::string& name,
                const std::shared_ptr<Texture>& texture,
                const ShaderFlags stageFlags = kShaderFragment);

            //! Attach an FBO and update shader parameters.
            void setFBO(
                const std::string& name,
                const std::shared_ptr<OffscreenBuffer>&,
                const ShaderFlags stageFlags = kShaderFragment);

            //! Add a Storage Buffer to shader.
            void addStorageBuffer(
                const std::string& name,
                const ShaderFlags stageFlags = kShaderCompute);

            //! Attach a Storage Buffer and update shader parameters.
            void setStorageBuffer(
                const std::string& name, const uint8_t* data,
                const std::size_t size);

            //! Add a Storage Image to shader.  \p format and \p access are
            //! baked into the layout; the texture must have been created
            //! with WGPUTextureUsage_StorageBinding and the same format.
            void addStorageImage(
                const std::string& name,
                const ShaderFlags stageFlags = kShaderCompute,
                const WGPUTextureFormat format = WGPUTextureFormat_RGBA32Float,
                const ShaderStorageAccess access =
                    ShaderStorageAccess::WriteOnly);

            //! Attach a texture as a storage image.
            void setStorageImage(
                const std::string& name,
                const std::shared_ptr<Texture>& texture);

            //! Attach a SSBO (Shader Storage Buffer Object).  Bindings that
            //! include the vertex stage are read-only storage buffers.
            template <typename T>
            void addSSBO(
                const std::string& name, const T& value,
                const ShaderFlags stageFlags = kShaderCompute);

            //! Set a SSBO (Shader Storage Buffer Object) initial value.
            template <typename T>
            void setSSBO(const std::string& name, const T& value);

            //! Record a clear of the SSBO to 0.  Outside of a pass.
            void clearSSBO(WGPUCommandEncoder encoder, const std::string& name);

            //! Copy the SSBO back to the CPU and return a read-only pointer
            //! (nullptr on failure).  Blocks until the GPU has finished the
            //! work already submitted.
            void* mapSSBO(const std::string& name);

            //! Unmap the SSBO data.
            void unmapSSBO(const std::string& name);

            //! Create a new shader binding set.
            std::shared_ptr<ShaderBindingSet> createBindingSet();

            //! Bind a ShaderBindingSet to this shader.
            void useBindingSet(const std::shared_ptr<ShaderBindingSet>);

            //! Create a pipeline layout (rebuilt from the current layout).
            void createPipelineLayout();

            //! Create a compute pipeline from this shader.
            //! Must be called after createBindingSet.
            void createComputePipeline();

            //! Dispatch compute shader in its own compute pass.
            void dispatch(
                WGPUCommandEncoder encoder, uint32_t groupCountX,
                uint32_t groupCountY, uint32_t groupCountZ = 1,
                const void* pushData = nullptr);

            //! Dispatch compute shader into an existing compute pass.
            void dispatch(
                WGPUComputePassEncoder pass, uint32_t groupCountX,
                uint32_t groupCountY, uint32_t groupCountZ = 1,
                const void* pushData = nullptr);

            //! Print out a list of bindings for vertex shader.
            void debugVertexDescriptorSets();

            //! Print out a list of bindings for fragment shader.
            void debugFragmentDescriptorSets();

            //! Print out a list of bindings for all shader types.
            void debugDescriptorSets();

            //! Get object count.
            static size_t getObjectCount();

        private:
            void _createVertexShader(const std::string&);
            void _createFragmentShader(const std::string&);
            void _createComputeShader(const std::string&);
            void _ensureLayout();
            uint32_t _pushAllocate(const void* data);

            Fl_WebGPU_Context& _ctx;
            WGPUInstance _instance = nullptr;
            WGPUDevice _device = nullptr;
            WGPUQueue _queue = nullptr;

            std::string shaderName = "Shader";

            //! Counter used in binding UBOs, Textures and FBOs.
            uint32_t current_binding_index = 0;

            //! Set when a resource was added after the layout was built.
            bool layoutDirty = true;

            //! Bind group layout shared across binding sets.
            std::shared_ptr<BindGroupLayout> activeLayout;

            //! Last value passed to bind().
            uint64_t frameIndex = 0;

            //! Push size (if shader has a push block).
            std::size_t pushSize = 0;

            //! Push stage flags (if shader has a push block).
            WGPUShaderStage pushStageFlags =
                WGPUShaderStage_Vertex | WGPUShaderStage_Fragment;
            uint32_t pushBinding = 0;
            uint64_t pushCapacity = 2ull * 1024 * 1024;
            uint32_t lastPushOffset = 0;
            std::shared_ptr<PushRing> pushRing;

            typedef ShaderBindingSet::UniformParameter UBOBinding;
            std::map<std::string, UBOBinding> ubos;

            typedef ShaderBindingSet::TextureParameter TextureBinding;
            std::map<std::string, TextureBinding> textureBindings;

            typedef ShaderBindingSet::StorageBufferParameter StorageBufferBinding;
            std::map<std::string, StorageBufferBinding> storageBufferBindings;

            typedef ShaderBindingSet::StorageImageParameter StorageImageBinding;
            std::map<std::string, StorageImageBinding> storageImageBindings;

            typedef ShaderBindingSet::SSBOParameter SSBOBinding;
            std::map<std::string, SSBOBinding> ssbos;

            std::shared_ptr<ShaderBindingSet> activeBindingSet;

            TLRENDER_PRIVATE();
        };
    } // namespace webgpu
} // namespace tl

#include <tlWGPU/ShaderInline.h>
