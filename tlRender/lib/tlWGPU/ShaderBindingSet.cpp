// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <tlWGPU/ShaderBindingSet.h>

#include <emscripten/emscripten.h>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstring>
#include <stdexcept>
#include <thread>

// A browser cannot block waiting for a GPU callback: callbacks are
// delivered by the JS event loop, which a spinning main thread starves, so a
// wait loop always runs into its timeout.  Define TLRENDER_WGPU_CAN_BLOCK=1
// to force the blocking paths (e.g. with JSPI / Asyncify).
#ifndef TLRENDER_WGPU_CAN_BLOCK
#    if defined(__EMSCRIPTEN__)
#        define TLRENDER_WGPU_CAN_BLOCK 0
#    else
#        define TLRENDER_WGPU_CAN_BLOCK 1
#    endif
#endif

namespace tl
{
    namespace wgpu
    {
        namespace
        {
            constexpr uint64_t alignUp(uint64_t value, uint64_t alignment)
            {
                return (value + alignment - 1) / alignment * alignment;
            }

            // Process pending callbacks (buffer maps).
            // Define TLRENDER_WGPU_DAWN when building against Dawn.
            [[maybe_unused]] void pumpEvents(WGPUInstance instance)
            {
                wgpuInstanceProcessEvents(instance);
// #if defined(TLRENDER_WGPU_DAWN)
//                 wgpuDeviceTick(device);
// #else
//                 wgpuDevicePoll(device, false, nullptr);
// #endif
            }

            struct MapState
            {
                std::atomic<int> status{0}; // 0 = pending, 1 = ok, 2 = failed
            };

            void onBufferMapped(
                WGPUMapAsyncStatus status, WGPUStringView, void* userdata1,
                void*)
            {
                auto* state = static_cast<std::shared_ptr<MapState>*>(userdata1);
                (*state)->status = (WGPUMapAsyncStatus_Success == status) ? 1 : 2;
                delete state;
            }

            //! Start mapping a MapRead buffer.  The result is reported through
            //! @p state.
            void requestMap(
                WGPUBuffer buffer, uint64_t size,
                const std::shared_ptr<MapState>& state)
            {
                WGPUBufferMapCallbackInfo info = {};
#if TLRENDER_WGPU_CAN_BLOCK
                info.mode = WGPUCallbackMode_AllowProcessEvents;
#else
                // Delivered by the browser's event loop; no pumping needed.
                info.mode = WGPUCallbackMode_AllowSpontaneous;
#endif
                info.callback = onBufferMapped;
                info.userdata1 = new std::shared_ptr<MapState>(state);
                wgpuBufferMapAsync(buffer, WGPUMapMode_Read, 0, size, info);
            }

#if TLRENDER_WGPU_CAN_BLOCK
            //! Map a MapRead buffer and wait (bounded) for it.
            bool mapAndWait(
                WGPUInstance instance, WGPUBuffer buffer, uint64_t size,
                int timeoutMs = 5000)
            {
                auto state = std::make_shared<MapState>();
                requestMap(buffer, size, state);

                const auto start = std::chrono::steady_clock::now();
                while (0 == state->status)
                {
                    pumpEvents(instance);
                    if (0 != state->status)
                        break;
                    if (std::chrono::steady_clock::now() - start >
                        std::chrono::milliseconds(timeoutMs))
                    {
                        return false;
                    }
                    std::this_thread::sleep_for(std::chrono::microseconds(200));
                }
                return 1 == state->status;
            }
#endif

            //! Copy @p src into the MapRead staging buffer and submit.  The
            //! copy is ordered after everything already submitted.
            void copyToStaging(
                WGPUDevice device, WGPUQueue queue, WGPUBuffer src,
                WGPUBuffer staging, uint64_t size)
            {
                WGPUCommandEncoder encoder =
                    wgpuDeviceCreateCommandEncoder(device, nullptr);
                wgpuCommandEncoderCopyBufferToBuffer(
                    encoder, src, 0, staging, 0, size);
                WGPUCommandBuffer commands =
                    wgpuCommandEncoderFinish(encoder, nullptr);
                wgpuQueueSubmit(queue, 1, &commands);
                wgpuCommandBufferRelease(commands);
                wgpuCommandEncoderRelease(encoder);
            }

            WGPUBuffer createBuffer(
                WGPUDevice device, const std::string& label, WGPUBufferUsage usage,
                uint64_t size)
            {
                WGPUBufferDescriptor desc = {};
                desc.label = {label.c_str(), label.size()};
                desc.usage = usage;
                desc.size = size;
                desc.mappedAtCreation = false;
                WGPUBuffer buffer = wgpuDeviceCreateBuffer(device, &desc);
                if (!buffer)
                {
                    throw std::runtime_error(
                        "tl::webgpu::ShaderBindingSet: failed to create "
                        "buffer " +
                        label);
                }
                return buffer;
            }

            //! Write to a buffer, padding the size up to a multiple of 4.
            void writeBuffer(
                WGPUQueue queue, WGPUBuffer buffer, uint64_t offset,
                const void* data, uint64_t size)
            {
                if (0 == size)
                    return;
                const uint64_t padded = alignUp(size, 4);
                if (padded == size)
                {
                    wgpuQueueWriteBuffer(queue, buffer, offset, data, size);
                }
                else
                {
                    std::vector<uint8_t> tmp(padded, 0);
                    std::memcpy(tmp.data(), data, size);
                    wgpuQueueWriteBuffer(
                        queue, buffer, offset, tmp.data(), padded);
                }
            }

            bool isUintFormat(WGPUTextureFormat f)
            {
                return f == WGPUTextureFormat_R32Uint ||
                       f == WGPUTextureFormat_RG32Uint ||
                       f == WGPUTextureFormat_RGBA32Uint;
            }

            bool isFloat32Format(WGPUTextureFormat f)
            {
                return f == WGPUTextureFormat_R32Float ||
                       f == WGPUTextureFormat_RG32Float ||
                       f == WGPUTextureFormat_RGBA32Float;
            }

            //! Check a texture format against the declared sample type so the
            //! user gets a readable error instead of a bind group validation
            //! failure.
            void checkTextureCompatible(
                WGPUDevice device, const std::string& name,
                ShaderTextureType type, WGPUTextureFormat format)
            {
                bool ok = true;
                if (ShaderTextureType::Depth == type ||
                    ShaderTextureType::Sint == type)
                {
                    return; // not checked
                }
                if (isUintFormat(format))
                {
                    ok = (ShaderTextureType::Uint == type);
                }
                else if (isFloat32Format(format))
                {
                    ok = (ShaderTextureType::UnfilterableFloat == type) ||
                         (ShaderTextureType::Float == type &&
                          wgpuDeviceHasFeature(
                              device, WGPUFeatureName_Float32Filterable));
                }
                else
                {
                    ok = (ShaderTextureType::Float == type ||
                          ShaderTextureType::UnfilterableFloat == type);
                }
                if (!ok)
                {
                    throw std::runtime_error(
                        "Texture binding '" + name +
                        "' was declared with a sample type that is not "
                        "compatible with the texture's format.  Pass "
                        "ShaderTextureType::UnfilterableFloat (32-bit float "
                        "without float32-filterable) or ::Uint (integer "
                        "formats) to addTexture().");
                }
            }
        } // namespace

        ShaderBindingSet::ShaderBindingSet(Fl_WebGPU_Context& ctx) :
            instance(ctx.instance),
            device(ctx.device),
            queue(ctx.queue())
        {
            if (wgpuDeviceGetLimits(device, &_limits) != WGPUStatus_Success)
            {
                // WebGPU defaults.
                _limits = {};
                _limits.maxStorageBufferBindingSize = 128u * 1024 * 1024;
                _limits.maxUniformBufferBindingSize = 64u * 1024;
            }
        }

        ShaderBindingSet::~ShaderBindingSet()
        {
            destroy();
        }

        void ShaderBindingSet::updateStorageBuffer(
            const std::string& name, const uint8_t* data, const uint64_t size)
        {
            auto it = storageBuffers.find(name);
            if (it == storageBuffers.end())
                throw std::runtime_error("Storage Buffer not found: " + name);

            auto& param = it->second;

            const uint64_t paddedSize = std::max<uint64_t>(alignUp(size, 4), 4);
            if (!param.buffer || param.currentSize < paddedSize)
            {
                // Grow with a factor so we don't reallocate constantly.
                uint64_t newSize = paddedSize;
                if (param.currentSize > 0)
                {
                    newSize = std::max(
                        paddedSize,
                        alignUp(param.currentSize + param.currentSize / 2, 4));
                }
                if (newSize > _limits.maxStorageBufferBindingSize)
                {
                    newSize = paddedSize;
                    if (newSize > _limits.maxStorageBufferBindingSize)
                    {
                        throw std::runtime_error(
                            "Storage Buffer '" + name + "' (" +
                            std::to_string(size) +
                            " bytes) exceeds maxStorageBufferBindingSize (" +
                            std::to_string(_limits.maxStorageBufferBindingSize) +
                            ")");
                    }
                }

                WGPUBuffer buffer = createBuffer(
                    device, shaderName + " " + name,
                    WGPUBufferUsage_Storage | WGPUBufferUsage_CopyDst |
                        WGPUBufferUsage_CopySrc,
                    newSize);

                // Release (not destroy): in-flight commands may still use it.
                if (param.buffer)
                    wgpuBufferRelease(param.buffer);
                param.buffer = buffer;
                param.currentSize = newSize;

                // Bind groups are immutable: rebuild on next use.
                _dirty = true;
            }

            writeBuffer(queue, param.buffer, 0, data, size);
        }

        void ShaderBindingSet::updateStorageImage(
            const std::string& name, const std::shared_ptr<Texture>& texture)
        {
            auto it = storageImages.find(name);
            if (it == storageImages.end())
                throw std::runtime_error(
                    "Storage Image not found: " + name +
                    " storageImages.size()=" +
                    std::to_string(storageImages.size()));
            if (!texture)
                throw std::runtime_error("Null storage image for: " + name);

            auto& param = it->second;

            if (!(texture->getOptions().usage & WGPUTextureUsage_StorageBinding))
            {
                throw std::runtime_error(
                    "Storage Image '" + name +
                    "': the texture was not created with "
                    "WGPUTextureUsage_StorageBinding in its TextureOptions");
            }
            if (texture->getInternalFormat() != param.format)
            {
                throw std::runtime_error(
                    "Storage Image '" + name +
                    "': the texture format does not match the format passed "
                    "to addStorageImage()");
            }

            if (param.texture != texture)
            {
                param.texture = texture;
                _dirty = true;
            }
        }

        void ShaderBindingSet::updateUniform(
            const std::string& name, const void* data, size_t size)
        {
            auto it = uniforms.find(name);
            if (it == uniforms.end())
            {
                throw std::runtime_error("Uniform Parameter not found: " + name);
            }
            if (it->second.size != size)
                throw std::runtime_error("Uniform size mismatch");

            // Queue-ordered: this lands after every previously submitted
            // frame and before the next submit.
            writeBuffer(queue, it->second.buffer, 0, data, size);
        }

        void ShaderBindingSet::updateSSBO(
            const std::string& name, const void* data, const size_t size)
        {
            auto it = ssbos.find(name);
            if (it == ssbos.end())
            {
                throw std::runtime_error("SSBO Parameter not found: " + name);
            }
            if (it->second.size != size)
            {
                throw std::runtime_error("SSBO size mismatch");
            }
            writeBuffer(queue, it->second.buffer, 0, data, size);
        }

        void ShaderBindingSet::updateTexture(
            const std::string& name, const std::shared_ptr<Texture>& texture)
        {
            auto it = textures.find(name);
            if (it == textures.end())
                throw std::runtime_error("Texture binding not found: " + name);
            if (!texture)
                throw std::runtime_error("Null texture for: " + name);

            auto& param = it->second;
            checkTextureCompatible(
                device, name, param.type, texture->getInternalFormat());

            WGPUTextureView view = texture->getTextureView();
            WGPUSampler sampler = texture->getSampler();
            if (view == param.view && sampler == param.sampler)
                return;

            param.texture = texture;
            param.fbo.reset();
            param.view = view;
            param.sampler = sampler;
            _dirty = true;
        }

        void ShaderBindingSet::updateFBO(
            const std::string& name,
            const std::shared_ptr<OffscreenBuffer>& fbo)
        {
            auto it = textures.find(name);
            if (it == textures.end())
                throw std::runtime_error("FBO binding not found: " + name);
            if (!fbo)
                throw std::runtime_error("updateFBO: Null FBO for: " + name);

            auto& param = it->second;
            checkTextureCompatible(device, name, param.type, fbo->getFormat());

            WGPUTextureView view = fbo->getTextureView();
            WGPUSampler sampler = fbo->getSampler();
            if (view == param.view && sampler == param.sampler)
                return;

            param.fbo = fbo;
            param.texture.reset();
            param.view = view;
            param.sampler = sampler;
            _dirty = true;
        }

        void ShaderBindingSet::destroy()
        {
            for (auto& [_, ubo] : uniforms)
            {
                if (ubo.buffer)
                    wgpuBufferRelease(ubo.buffer);
                ubo.buffer = nullptr;
            }

            for (auto& [_, sb] : storageBuffers)
            {
                if (sb.buffer)
                    wgpuBufferRelease(sb.buffer);
                sb.buffer = nullptr;
            }

            for (auto& [_, ssbo] : ssbos)
            {
                if (ssbo.mapped && ssbo.staging)
                    wgpuBufferUnmap(ssbo.staging);
                ssbo.mapped = false;
                ssbo.mapState.reset();
                if (ssbo.staging)
                    wgpuBufferRelease(ssbo.staging);
                ssbo.staging = nullptr;
                if (ssbo.buffer)
                    wgpuBufferRelease(ssbo.buffer);
                ssbo.buffer = nullptr;
            }

            if (_bindGroup)
                wgpuBindGroupRelease(_bindGroup);
            _bindGroup = nullptr;

            uniforms.clear();
            textures.clear();
            storageBuffers.clear();
            storageImages.clear();
            ssbos.clear();
            pushRing.reset();
        }

        void ShaderBindingSet::clearSSBO(
            WGPUCommandEncoder encoder, const std::string& name)
        {
            auto it = ssbos.find(name);
            if (it == ssbos.end())
            {
                throw std::runtime_error("SSBO Parameter not found: " + name);
            }

            // Reset SSBO to zero.  WebGPU inserts the barriers.
            wgpuCommandEncoderClearBuffer(
                encoder, it->second.buffer, 0, WGPU_WHOLE_SIZE);
        }

        void* ShaderBindingSet::mapSSBO(const std::string& name)
        {
            auto it = ssbos.find(name);
            if (it == ssbos.end())
            {
                throw std::runtime_error("SSBO Parameter not found: " + name);
            }
            auto& ssbo = it->second;
            const uint64_t size = alignUp(ssbo.size, 4);

            if (!ssbo.staging)
            {
                ssbo.staging = createBuffer(
                    device, shaderName + " " + name + " staging",
                    WGPUBufferUsage_MapRead | WGPUBufferUsage_CopyDst, size);
            }

#if TLRENDER_WGPU_CAN_BLOCK
            if (ssbo.mapped)
            {
                wgpuBufferUnmap(ssbo.staging);
                ssbo.mapped = false;
            }

            copyToStaging(device, queue, ssbo.buffer, ssbo.staging, size);

            if (!mapAndWait(instance, ssbo.staging, size))
                return nullptr;

            const void* ptr =
                wgpuBufferGetConstMappedRange(ssbo.staging, 0, size);
            ssbo.mapped = (ptr != nullptr);
            return const_cast<void*>(ptr);
#else
            // Non-blocking: start the request on the first call, then poll.
            if (!ssbo.mapState)
            {
                if (ssbo.mapped)
                {
                    wgpuBufferUnmap(ssbo.staging);
                    ssbo.mapped = false;
                }
                copyToStaging(device, queue, ssbo.buffer, ssbo.staging, size);

                auto state = std::make_shared<MapState>();
                ssbo.mapState = state;
                requestMap(ssbo.staging, size, state);
                return nullptr;
            }

            auto state = std::static_pointer_cast<MapState>(ssbo.mapState);
            const int status = state->status.load();
            if (0 == status)
                return nullptr; // still in flight, try again next frame
            if (1 != status)
            {
                ssbo.mapState.reset(); // failed: the next call retries
                return nullptr;
            }

            const void* ptr =
                wgpuBufferGetConstMappedRange(ssbo.staging, 0, size);
            if (!ptr)
            {
                ssbo.mapState.reset();
                return nullptr;
            }
            ssbo.mapped = true;
            return const_cast<void*>(ptr);
#endif
        }

        void ShaderBindingSet::unmapSSBO(const std::string& name)
        {
            auto it = ssbos.find(name);
            if (it == ssbos.end())
            {
                throw std::runtime_error("SSBO Parameter not found: " + name);
            }
            auto& ssbo = it->second;
            if (ssbo.mapped && ssbo.staging)
                wgpuBufferUnmap(ssbo.staging);
            ssbo.mapped = false;
            ssbo.mapState.reset();
        }

        WGPUBindGroup ShaderBindingSet::getBindGroup()
        {
            if (_bindGroup && !_dirty)
                return _bindGroup;

            if (!bindGroupLayout)
                throw std::runtime_error(
                    "ShaderBindingSet (" + shaderName + "): no layout");

            std::vector<WGPUBindGroupEntry> entries;

            for (const auto& [name, ubo] : uniforms)
            {
                WGPUBindGroupEntry e = {};
                e.binding = ubo.binding;
                e.buffer = ubo.buffer;
                e.offset = 0;
                e.size = WGPU_WHOLE_SIZE;
                entries.push_back(e);
            }

            for (const auto& [name, ssbo] : ssbos)
            {
                WGPUBindGroupEntry e = {};
                e.binding = ssbo.binding;
                e.buffer = ssbo.buffer;
                e.offset = 0;
                e.size = WGPU_WHOLE_SIZE;
                entries.push_back(e);
            }

            for (const auto& [name, sb] : storageBuffers)
            {
                if (!sb.buffer)
                    throw std::runtime_error(
                        "Shader " + shaderName + ": storage buffer '" + name +
                        "' was never set (call setStorageBuffer first)");
                WGPUBindGroupEntry e = {};
                e.binding = sb.binding;
                e.buffer = sb.buffer;
                e.offset = 0;
                e.size = WGPU_WHOLE_SIZE;
                entries.push_back(e);
            }

            for (const auto& [name, t] : textures)
            {
                if (!t.view || !t.sampler)
                    throw std::runtime_error(
                        "Shader " + shaderName + ": texture '" + name +
                        "' was never set (call setTexture/setFBO first)");
                WGPUBindGroupEntry te = {};
                te.binding = t.binding;
                te.textureView = t.view;
                entries.push_back(te);

                WGPUBindGroupEntry se = {};
                se.binding = t.samplerBinding;
                se.sampler = t.sampler;
                entries.push_back(se);
            }

            for (const auto& [name, si] : storageImages)
            {
                if (!si.texture)
                    throw std::runtime_error(
                        "Shader " + shaderName + ": storage image '" + name +
                        "' was never set (call setStorageImage first)");
                WGPUBindGroupEntry e = {};
                e.binding = si.binding;
                e.textureView = si.texture->getTextureView();
                entries.push_back(e);
            }

            if (hasPush)
            {
                if (!pushRing || !pushRing->buffer)
                    throw std::runtime_error(
                        "Shader " + shaderName + ": push block has no buffer");
                WGPUBindGroupEntry e = {};
                e.binding = pushBinding;
                e.buffer = pushRing->buffer;
                e.offset = 0;
                // The dynamic offset passed to SetBindGroup is added to this.
                e.size = pushRing->entrySize;
                entries.push_back(e);
            }

            const std::string label = shaderName + " bind group";
            WGPUBindGroupDescriptor desc = {};
            desc.label = {label.c_str(), label.size()};
            desc.layout = bindGroupLayout->handle;
            desc.entryCount = entries.size();
            desc.entries = entries.data();

            WGPUBindGroup group = wgpuDeviceCreateBindGroup(device, &desc);
            if (!group)
                throw std::runtime_error(
                    "Shader " + shaderName + ": failed to create bind group");

            // Release only drops our reference; command buffers that
            // already recorded the old group keep it alive.
            if (_bindGroup)
                wgpuBindGroupRelease(_bindGroup);
            _bindGroup = group;
            _dirty = false;
            return _bindGroup;
        }

    } // namespace webgpu
} // namespace tl
