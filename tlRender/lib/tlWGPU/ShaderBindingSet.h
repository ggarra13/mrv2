// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <tlWGPU/OffscreenBuffer.h>
#include <tlWGPU/Texture.h>
#include <tlWGPU/WGPU.h> // must pull in <webgpu/webgpu.h>

#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace tl
{
    namespace wgpu
    {
        //! Sample type a texture binding is declared with.  Unlike Vulkan,
        //! WebGPU bakes this into the bind group layout, and the texture
        //! bound later must be compatible with it:
        //!   Float             unorm / half-float formats (filterable)
        //!   UnfilterableFloat 32-bit float formats (without the
        //!                     float32-filterable feature)
        //!   Uint / Sint       integer formats
        //!   Depth             depth formats
        enum class ShaderTextureType {
            Float,
            UnfilterableFloat,
            Uint,
            Sint,
            Depth
        };

        //! View dimension a texture binding is declared with.
        enum class ShaderTextureDimension { _1D, _2D, _3D };

        //! Access mode for storage textures.
        enum class ShaderStorageAccess { WriteOnly, ReadOnly, ReadWrite };

        //! RAII wrapper for a bind group layout (replaces DescriptorSetLayout).
        struct BindGroupLayout
        {
            WGPUBindGroupLayout handle = nullptr;

            explicit BindGroupLayout(WGPUBindGroupLayout h) : handle(h) {}
            ~BindGroupLayout()
            {
                if (handle)
                    wgpuBindGroupLayoutRelease(handle);
            }
            BindGroupLayout(const BindGroupLayout&) = delete;
            BindGroupLayout& operator=(const BindGroupLayout&) = delete;
        };

        //! Replacement for push constants (WebGPU has none).  A uniform
        //! buffer that is bound with a *dynamic offset*, so every draw can
        //! see its own copy of the data, just like vkCmdPushConstants.
        //! Owned by the Shader and shared with its binding sets.
        struct PushRing
        {
            WGPUBuffer buffer = nullptr;
            uint64_t capacity = 0;  //< Buffer size in bytes.
            uint64_t offset = 0;    //< Bump-allocator cursor.
            uint64_t entrySize = 0; //< Bytes per push (padded to 16).
            uint32_t alignment = 256; //< minUniformBufferOffsetAlignment.

            PushRing() = default;
            ~PushRing()
            {
                if (buffer)
                    wgpuBufferRelease(buffer);
            }
            PushRing(const PushRing&) = delete;
            PushRing& operator=(const PushRing&) = delete;
        };

        //! A set of resources bound to a shader (one WGPUBindGroup).
        //!
        //! Differences from the Vulkan version:
        //!
        //!  * No MAX_FRAMES_IN_FLIGHT copies.  Vulkan needed one descriptor
        //!    set / UBO / SSBO per frame in flight because the CPU could
        //!    overwrite memory the GPU was still reading.  WebGPU orders
        //!    wgpuQueueWriteBuffer() against queue submissions, so a write
        //!    made for frame N+1 can never be seen by frame N's commands.
        //!  * Bind groups are immutable.  Setting a texture, FBO, storage
        //!    buffer (when it grows) or storage image marks the set dirty and
        //!    the bind group is rebuilt the next time getBindGroup() is
        //!    called.  Bind groups already recorded into a command buffer are
        //!    unaffected.
        //!  * The set keeps the textures / FBOs bound to it alive.
        //!
        //! As before, uniforms are per binding set, not per draw: all draws
        //! recorded before a submit see the last value written.  Use one
        //! binding set per distinct draw state (or the push block).
        //!
        //! All allocation of these structures happens in
        //! Shader::createBindingSet().
        struct ShaderBindingSet
        {
            std::string shaderName;

            std::shared_ptr<BindGroupLayout> bindGroupLayout;

            struct UniformParameter
            {
                uint32_t binding = 0;
                WGPUShaderStage visibility = 0;
                std::size_t size = 0;
                WGPUBuffer buffer = nullptr;
            };
            std::map<std::string, UniformParameter> uniforms;

            //! A texture takes TWO consecutive bindings: the texture at
            //! `binding` and its sampler at `samplerBinding` (WebGPU has no
            //! combined image sampler).
            struct TextureParameter
            {
                uint32_t binding = 0;
                uint32_t samplerBinding = 0;
                WGPUShaderStage visibility = 0;
                ShaderTextureType type = ShaderTextureType::Float;
                ShaderTextureDimension dimension = ShaderTextureDimension::_2D;

                std::shared_ptr<Texture> texture;
                std::shared_ptr<OffscreenBuffer> fbo;
                WGPUTextureView view = nullptr;
                WGPUSampler sampler = nullptr;
            };
            std::map<std::string, TextureParameter> textures;

            struct StorageBufferParameter
            {
                uint32_t binding = 0;
                WGPUShaderStage visibility = 0;
                WGPUBuffer buffer = nullptr;

                // Track the current allocated size to avoid unnecessary
                // reallocation.
                uint64_t currentSize = 0;
            };
            std::map<std::string, StorageBufferParameter> storageBuffers;

            struct StorageImageParameter
            {
                uint32_t binding = 0;
                WGPUShaderStage visibility = 0;
                WGPUTextureFormat format = WGPUTextureFormat_RGBA32Float;
                ShaderStorageAccess access = ShaderStorageAccess::WriteOnly;
                std::shared_ptr<Texture> texture;
            };
            std::map<std::string, StorageImageParameter> storageImages;

            struct SSBOParameter
            {
                uint32_t binding = 0;
                WGPUShaderStage visibility = 0;
                std::size_t size = 0;
                WGPUBuffer buffer = nullptr;

                // Created on demand by mapSSBO() (storage buffers cannot be
                // mapped in WebGPU, so reads go through a staging copy).
                WGPUBuffer staging = nullptr;
                bool mapped = false;

                // Non-blocking read-back state (used on Emscripten, where
                // mapSSBO() cannot block).  Type-erased
                // std::shared_ptr<MapState> owned by ShaderBindingSet.cpp.
                std::shared_ptr<void> mapState;
            };
            std::map<std::string, SSBOParameter> ssbos;

            // Push block emulation (see PushRing).
            bool hasPush = false;
            uint32_t pushBinding = 0;
            WGPUShaderStage pushVisibility = 0;
            std::shared_ptr<PushRing> pushRing;

            WGPUInstance instance = nullptr;
            WGPUDevice device = nullptr;
            WGPUQueue queue = nullptr;

            ShaderBindingSet(Fl_WebGPU_Context&);
            ~ShaderBindingSet();

            ShaderBindingSet(const ShaderBindingSet&) = delete;
            ShaderBindingSet& operator=(const ShaderBindingSet&) = delete;

            void updateStorageBuffer(
                const std::string& name, const uint8_t* data,
                const uint64_t size);

            void updateStorageImage(
                const std::string& name,
                const std::shared_ptr<Texture>& texture);

            void updateUniform(
                const std::string& name, const void* data, size_t size);

            void updateSSBO(
                const std::string& name, const void* data, const size_t size);

            void updateTexture(
                const std::string& name,
                const std::shared_ptr<Texture>& texture);

            void updateFBO(
                const std::string& name,
                const std::shared_ptr<OffscreenBuffer>& fbo);

            void destroy();

            //! Record a clear (to zero) of the SSBO.  Must be called outside
            //! of a render/compute pass.  No barriers are needed.
            void clearSSBO(WGPUCommandEncoder encoder, const std::string& name);

            //! Copy the SSBO to a staging buffer and return a READ-ONLY
            //! pointer to its contents.  Only work already *submitted* is
            //! seen.  Use setSSBO() to write.
            //!
            //! Native builds: blocks (bounded) until the GPU is done and
            //! returns nullptr on timeout/failure.
            //!
            //! Emscripten builds: a browser cannot block on the GPU (map
            //! callbacks are delivered by the JS event loop), so this NEVER
            //! blocks.  The first call starts the copy and the map and
            //! returns nullptr; keep calling it once per frame until it
            //! returns non-null, use the data, then call unmapSSBO().
            void* mapSSBO(const std::string& name);

            //! Release the pointer returned by mapSSBO().
            void unmapSSBO(const std::string& name);

            //! Get the bind group, (re)building it if any resource changed.
            //! Throws if a texture, storage image or storage buffer was
            //! declared but never set.
            WGPUBindGroup getBindGroup();

        private:
            WGPUBindGroup _bindGroup = nullptr;
            bool _dirty = true;
            WGPULimits _limits = {};
        };

    } // namespace webgpu
} // namespace tl
