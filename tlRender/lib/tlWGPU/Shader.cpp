// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// Copyright (c) 2025-Present Gonzalo Garramuño
// All rights reserved.

#include <tlWGPU/Shader.h>

#include <tlCore/Color.h>
#include <tlCore/String.h>
#include <tlCore/StringFormat.h>

#include <algorithm>
#include <atomic>
#include <cassert>
#include <chrono>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <thread>

namespace tl
{
    namespace wgpu
    {
        namespace
        {
            std::atomic<size_t> objectCount = 0;

            constexpr uint64_t alignUp(uint64_t value, uint64_t alignment)
            {
                return (value + alignment - 1) / alignment * alignment;
            }

            // Process pending callbacks (shader compilation info).
            // Define TLRENDER_WGPU_DAWN when building against Dawn.
            void pumpEvents(WGPUInstance instance)
            {
                wgpuInstanceProcessEvents(instance);
// #if defined(TLRENDER_WGPU_DAWN)
//                 wgpuDeviceTick(device);
// #else
//                 wgpuDevicePoll(device, false, nullptr);
// #endif
            }

            std::string toString(const WGPUStringView& view)
            {
                if (!view.data)
                    return std::string();
                if (WGPU_STRLEN == view.length)
                    return std::string(view.data);
                return std::string(view.data, view.length);
            }

            struct CompileMessage
            {
                WGPUCompilationMessageType type;
                uint64_t line = 0;
                uint64_t column = 0;
                std::string text;
            };

            struct CompileState
            {
                std::atomic<int> done{0};
                std::vector<CompileMessage> messages;
            };

            void onCompilationInfo(
                WGPUCompilationInfoRequestStatus, const WGPUCompilationInfo* info,
                void* userdata1, void*)
            {
                auto* sp = static_cast<std::shared_ptr<CompileState>*>(userdata1);
                CompileState& state = **sp;
                if (info)
                {
                    for (size_t i = 0; i < info->messageCount; ++i)
                    {
                        const WGPUCompilationMessage& m = info->messages[i];
                        CompileMessage out;
                        out.type = m.type;
                        out.line = m.lineNum;
                        out.column = m.linePos;
                        out.text = toString(m.message);
                        state.messages.push_back(out);
                    }
                }
                state.done = 1;
                delete sp;
            }

            // Namespace scope, next to onCompilationInfo():
            void onCompilationInfoLog(
                WGPUCompilationInfoRequestStatus, const WGPUCompilationInfo* info,
                void* userdata1, void*)
            {
                auto* name = static_cast<std::string*>(userdata1);
                if (info)
                {
                    for (size_t i = 0; i < info->messageCount; ++i)
                    {
                        const WGPUCompilationMessage& m = info->messages[i];
                        std::cerr << *name
                                  << (WGPUCompilationMessageType_Error == m.type
                                      ? " error " : " warning ")
                                  << m.lineNum << ":" << m.linePos << ": "
                                  << toString(m.message) << std::endl;
                    }
                }
                delete name;
            }

            //! Create a module from WGSL and report compiler diagnostics.
            //! Throws (after printing the numbered source) on errors.
            //!
            //! WebGPU reports shader errors asynchronously; without this
            //! round trip an invalid shader would only surface later as an
            //! opaque pipeline-creation failure.
            WGPUShaderModule createModule(
                WGPUInstance instance,
                WGPUDevice device, const std::string& shaderName,
                const char* stage, const std::string& source)
            {
                WGPUShaderSourceWGSL wgsl = {};
                wgsl.chain.sType = WGPUSType_ShaderSourceWGSL;
                wgsl.code = {source.data(), source.size()};

                const std::string label = shaderName + " (" + stage + ")";
                WGPUShaderModuleDescriptor desc = {};
                desc.nextInChain = &wgsl.chain;
                desc.label = {label.c_str(), label.size()};

                WGPUShaderModule module =
                    wgpuDeviceCreateShaderModule(device, &desc);
                if (!module)
                {
                    throw std::runtime_error(
                        shaderName + ": failed to create " + stage +
                        " shader module");
                }
#if defined(__EMSCRIPTEN__)
                // Just log diagnostics whenever they arrive.
                WGPUCompilationInfoCallbackInfo logCb = {};
                logCb.mode = WGPUCallbackMode_AllowSpontaneous;
                logCb.callback = onCompilationInfoLog;
                logCb.userdata1 = new std::string(shaderName + " " + stage);
                wgpuShaderModuleGetCompilationInfo(module, logCb);
                return module;
#endif

                auto state = std::make_shared<CompileState>();
                WGPUCompilationInfoCallbackInfo cb = {};
                cb.mode = WGPUCallbackMode_AllowProcessEvents;
                cb.callback = onCompilationInfo;
                cb.userdata1 = new std::shared_ptr<CompileState>(state);
                wgpuShaderModuleGetCompilationInfo(module, cb);

                // set to false the first time the callback never arrives.
                static std::atomic<bool> compilationInfoWorks = true;
                const auto start = std::chrono::steady_clock::now();
                while (0 == state->done)
                {
                    pumpEvents(instance);
                    if (0 != state->done)
                        break;
                    if (std::chrono::steady_clock::now() - start >
                        std::chrono::milliseconds(250))
                    {
                        // No diagnostics available; carry on.
                        compilationInfoWorks = false;
                        return module;
                    }
                    std::this_thread::sleep_for(std::chrono::microseconds(200));
                }

                std::string firstError;
                for (const auto& m : state->messages)
                {
                    const bool isError = (WGPUCompilationMessageType_Error == m.type);
                    std::cerr << shaderName << " " << stage
                              << (isError ? " error " : " warning ")
                              << m.line << ":" << m.column << ": " << m.text
                              << std::endl;
                    if (isError && firstError.empty())
                        firstError = m.text;
                }

                if (!firstError.empty())
                {
                    std::cerr << shaderName << " failed " << stage
                              << " compilation for" << std::endl;
                    auto lines = tl::string::split(
                        source, '\n', string::SplitOptions::KeepEmpty);
                    uint32_t i = 1;
                    for (const auto& line : lines)
                    {
                        std::cerr << i++ << ": " << line << std::endl;
                    }
                    wgpuShaderModuleRelease(module);
                    throw std::runtime_error(
                        shaderName + " failed " + stage +
                        " compilation: " + firstError);
                }
                return module;
            }

            WGPUTextureSampleType getSampleType(ShaderTextureType type)
            {
                switch (type)
                {
                case ShaderTextureType::UnfilterableFloat:
                    return WGPUTextureSampleType_UnfilterableFloat;
                case ShaderTextureType::Uint:
                    return WGPUTextureSampleType_Uint;
                case ShaderTextureType::Sint:
                    return WGPUTextureSampleType_Sint;
                case ShaderTextureType::Depth:
                    return WGPUTextureSampleType_Depth;
                case ShaderTextureType::Float:
                default:
                    return WGPUTextureSampleType_Float;
                }
            }

            // A filtering sampler binding is only valid with filterable
            // float textures; everything else uses a non-filtering one.
            WGPUSamplerBindingType getSamplerBindingType(ShaderTextureType type)
            {
                return ShaderTextureType::Float == type
                           ? WGPUSamplerBindingType_Filtering
                           : WGPUSamplerBindingType_NonFiltering;
            }

            WGPUTextureViewDimension getViewDimension(ShaderTextureDimension d)
            {
                switch (d)
                {
                case ShaderTextureDimension::_1D:
                    return WGPUTextureViewDimension_1D;
                case ShaderTextureDimension::_3D:
                    return WGPUTextureViewDimension_3D;
                case ShaderTextureDimension::_2D:
                default:
                    return WGPUTextureViewDimension_2D;
                }
            }

            WGPUStorageTextureAccess getStorageAccess(ShaderStorageAccess a)
            {
                switch (a)
                {
                case ShaderStorageAccess::ReadOnly:
                    return WGPUStorageTextureAccess_ReadOnly;
                case ShaderStorageAccess::ReadWrite:
                    return WGPUStorageTextureAccess_ReadWrite;
                case ShaderStorageAccess::WriteOnly:
                default:
                    return WGPUStorageTextureAccess_WriteOnly;
                }
            }

            // Read-write storage buffers are not allowed in the vertex stage.
            WGPUBufferBindingType getStorageBufferType(WGPUShaderStage visibility)
            {
                return (visibility & WGPUShaderStage_Vertex)
                           ? WGPUBufferBindingType_ReadOnlyStorage
                           : WGPUBufferBindingType_Storage;
            }

            const char* stageNames(WGPUShaderStage s, char* buf, size_t n)
            {
                std::snprintf(
                    buf, n, "%s%s%s",
                    (s & WGPUShaderStage_Vertex) ? "vertex " : "",
                    (s & WGPUShaderStage_Fragment) ? "fragment " : "",
                    (s & WGPUShaderStage_Compute) ? "compute " : "");
                return buf;
            }
        } // namespace

        size_t Shader::getObjectCount()
        {
            return objectCount;
        }

        struct Shader::Private
        {
            WGPUShaderModule vertex = nullptr;
            std::string vertexSource = "unavailable";
            std::string vertexEntry = "main";

            WGPUShaderModule fragment = nullptr;
            std::string fragmentSource = "unavailable";
            std::string fragmentEntry = "main";

            // True when vertex and fragment are the same module.
            bool sharedModule = false;

            WGPUComputePipeline computePipeline = nullptr;
            WGPUPipelineLayout pipelineLayout = nullptr;
            WGPUShaderModule compute = nullptr;
            std::string computeSource = "unavailable";
            std::string computeEntry = "main";
        };

        void Shader::_createVertexShader(const std::string& source)
        {
            TLRENDER_P();
            p.vertex = createModule(_instance,
                                    _device,
                                    shaderName,
                                    "vertex",
                                    source);
            p.vertexSource = source;
        }

        void Shader::_createFragmentShader(const std::string& source)
        {
            TLRENDER_P();
            p.fragment = createModule(_instance,
                                      _device,
                                      shaderName,
                                      "fragment",
                                      source);
            p.fragmentSource = source;
        }

        void Shader::_createComputeShader(const std::string& source)
        {
            TLRENDER_P();
            p.compute = createModule(_instance,
                                     _device,
                                     shaderName,
                                     "compute",
                                     source);
            p.computeSource = source;
        }

        void Shader::_init(
            const std::string& vertexSource, const std::string& fragmentSource,
            const std::string& name)
        {
            TLRENDER_P();

            shaderName = name;

            _createVertexShader(vertexSource);
            _createFragmentShader(fragmentSource);

            ++objectCount;
        }

        void Shader::_init(
            const std::string& source, const std::string& vertexEntry,
            const std::string& fragmentEntry, const std::string& name)
        {
            TLRENDER_P();

            shaderName = name;

            p.vertex = createModule(_instance,
                                    _device,
                                    shaderName,
                                    "vertex/fragment",
                                    source);
            p.fragment = p.vertex;
            p.sharedModule = true;
            p.vertexSource = p.fragmentSource = source;
            p.vertexEntry = vertexEntry;
            p.fragmentEntry = fragmentEntry;

            ++objectCount;
        }

        void Shader::_init(
            const std::string& computeSource, const std::string& name)
        {
            TLRENDER_P();

            shaderName = name;

            _createComputeShader(computeSource);

            ++objectCount;
        }

        Shader::Shader(Fl_WebGPU_Context& ctx) :
            _ctx(ctx),
            _instance(ctx.instance),
            _device(ctx.device),
            _queue(ctx.queue()),
            _p(new Private)
        {
        }

        Shader::~Shader()
        {
            TLRENDER_P();

            // Release only drops our reference; pipelines built from these
            // modules, and in-flight work, keep what they need alive.
            if (p.fragment && !p.sharedModule)
                wgpuShaderModuleRelease(p.fragment);

            if (p.vertex)
                wgpuShaderModuleRelease(p.vertex);

            if (p.compute)
                wgpuShaderModuleRelease(p.compute);

            if (p.computePipeline)
                wgpuComputePipelineRelease(p.computePipeline);

            if (p.pipelineLayout)
                wgpuPipelineLayoutRelease(p.pipelineLayout);

            activeBindingSet.reset();

            --objectCount;
        }

        std::shared_ptr<Shader> Shader::create(Fl_WebGPU_Context& ctx,
                                               const std::string& computeSource,
                                               const std::string& name)
        {
            auto out = std::make_shared<Shader>(ctx);
            out->_init(computeSource, name);
            return out;
        }

        std::shared_ptr<Shader> Shader::create(
            Fl_WebGPU_Context& ctx,
            const std::string& vertexSource,
            const std::string& fragmentSource,
            const std::string& name)
        {
            auto out = std::make_shared<Shader>(ctx);
            out->_init(vertexSource, fragmentSource, name);
            return out;
        }

        std::shared_ptr<Shader> Shader::create(
            Fl_WebGPU_Context& ctx,
            const std::string& source,
            const std::string& vertexEntry, const std::string& fragmentEntry,
            const std::string& name)
        {
            auto out = std::make_shared<Shader>(ctx);
            out->_init(source, vertexEntry, fragmentEntry, name);
            return out;
        }

        const std::string& Shader::getName() const
        {
            return shaderName;
        }

        const WGPUShaderModule& Shader::getVertex() const
        {
            return _p->vertex;
        }

        const WGPUShaderModule& Shader::getFragment() const
        {
            return _p->fragment;
        }

        const std::string& Shader::getVertexEntryPoint() const
        {
            return _p->vertexEntry;
        }

        const std::string& Shader::getFragmentEntryPoint() const
        {
            return _p->fragmentEntry;
        }

        const std::string& Shader::getComputeEntryPoint() const
        {
            return _p->computeEntry;
        }

        void Shader::setVertexEntryPoint(const std::string& value)
        {
            _p->vertexEntry = value;
        }

        void Shader::setFragmentEntryPoint(const std::string& value)
        {
            _p->fragmentEntry = value;
        }

        void Shader::setComputeEntryPoint(const std::string& value)
        {
            _p->computeEntry = value;
        }

        const std::string& Shader::getVertexSource() const
        {
            return _p->vertexSource;
        }

        const std::string& Shader::getFragmentSource() const
        {
            return _p->fragmentSource;
        }

        const std::string& Shader::getComputeSource() const
        {
            return _p->computeSource;
        }

        void Shader::useBindingSet(const std::shared_ptr<ShaderBindingSet> value)
        {
            activeBindingSet = value;
        }

        WGPUBindGroup Shader::getBindGroup()
        {
            if (!activeBindingSet)
                throw std::runtime_error(
                    "No activeBindingSet.  Call useBindingSet first");
            return activeBindingSet->getBindGroup();
        }

        WGPUBindGroupLayout Shader::getBindGroupLayout() const
        {
            if (!activeLayout || !activeLayout->handle)
            {
                throw std::runtime_error(
                    "No active bind group layout.  Call createBindingSet first");
            }
            return activeLayout->handle;
        }

        WGPUPipelineLayout Shader::getPipelineLayout()
        {
            if (!_p->pipelineLayout)
                createPipelineLayout();
            return _p->pipelineLayout;
        }

        void Shader::bind(uint64_t value)
        {
            // Nothing per-frame is selected any more; this only marks the
            // start of a new frame for the push-block allocator.
            if (value != frameIndex)
            {
                frameIndex = value;
                beginFrame();
            }
        }

        void Shader::beginFrame()
        {
            // Safe without per-frame copies: queue writes made for the next
            // frame are ordered after the previous frame's submit.
            if (pushRing)
                pushRing->offset = 0;
        }

        void Shader::addStorageBuffer(
            const std::string& name, const ShaderFlags stageFlags)
        {
            StorageBufferBinding t;
            t.binding = current_binding_index++;
            t.visibility = getWGPUShaderFlags(stageFlags);
            storageBufferBindings.insert(std::make_pair(name, t));
            layoutDirty = true;
        }

        void Shader::addStorageImage(
            const std::string& name, const ShaderFlags stageFlags,
            const WGPUTextureFormat format, const ShaderStorageAccess access)
        {
            StorageImageBinding t;
            t.binding = current_binding_index++;
            t.visibility = getWGPUShaderFlags(stageFlags);
            t.format = format;
            t.access = access;
            storageImageBindings.insert(std::make_pair(name, t));
            layoutDirty = true;
        }

        void Shader::addTexture(
            const std::string& name, const ShaderFlags stageFlags,
            const ShaderTextureType type,
            const ShaderTextureDimension dimension)
        {
            TextureBinding t;
            t.binding = current_binding_index++;
            t.samplerBinding = current_binding_index++; // WebGPU: separate
            t.visibility = getWGPUShaderFlags(stageFlags);
            t.type = type;
            t.dimension = dimension;
            textureBindings.insert(std::make_pair(name, t));
            layoutDirty = true;
        }

        void Shader::setTexture(
            const std::string& name, const std::shared_ptr<Texture>& texture,
            const ShaderFlags stageFlags)
        {
            if (!activeBindingSet)
                throw std::runtime_error(
                    "No activeBindingSet.  Call create/useBindingSet first");
            activeBindingSet->updateTexture(name, texture);
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
                throw std::runtime_error(
                    "No activeBindingSet.  Call create/useBindingSet first");
            activeBindingSet->updateFBO(name, fbo);
        }

        void Shader::setStorageBuffer(
            const std::string& name, const uint8_t* data,
            const std::size_t size)
        {
            if (!activeBindingSet)
                throw std::runtime_error(
                    "No activeBindingSet for Shader " + name);

            // We pass the specific data and size to the binding set
            activeBindingSet->updateStorageBuffer(name, data, size);
        }

        void Shader::setStorageImage(
            const std::string& name, const std::shared_ptr<Texture>& texture)
        {
            if (!activeBindingSet)
                throw std::runtime_error(
                    "No activeBindingSet for Shader " + name);

            activeBindingSet->updateStorageImage(name, texture);
        }

        void Shader::debugVertexDescriptorSets()
        {
            // UBOs
            for (const auto& [name, ubo] : ubos)
            {
                if ((ubo.visibility & WGPUShaderStage_Vertex) == 0)
                    continue;
                std::cerr << name << std::endl;
                std::cerr << "\t@binding(" << ubo.binding
                          << ") = uniform size=" << ubo.size << std::endl;
            }

            // Textures
            for (const auto& [name, texture] : textureBindings)
            {
                if ((texture.visibility & WGPUShaderStage_Vertex) == 0)
                    continue;
                std::cerr << name << std::endl;
                std::cerr << "\t@binding(" << texture.binding
                          << ") = texture, @binding(" << texture.samplerBinding
                          << ") = sampler" << std::endl;
            }
        }

        void Shader::debugFragmentDescriptorSets()
        {
            // UBOs
            for (const auto& [name, ubo] : ubos)
            {
                if ((ubo.visibility & WGPUShaderStage_Fragment) == 0)
                    continue;
                std::cerr << name << std::endl;
                std::cerr << "\t@binding(" << ubo.binding
                          << ") = uniform size=" << ubo.size << std::endl;
            }

            // Textures
            for (const auto& [name, texture] : textureBindings)
            {
                if ((texture.visibility & WGPUShaderStage_Fragment) == 0)
                    continue;
                std::cerr << name << std::endl;
                std::cerr << "\t@binding(" << texture.binding
                          << ") = texture, @binding(" << texture.samplerBinding
                          << ") = sampler" << std::endl;
            }
        }

        void Shader::debugDescriptorSets()
        {
            if (ubos.empty() && textureBindings.empty() && ssbos.empty() &&
                storageBufferBindings.empty() && storageImageBindings.empty() &&
                0 == pushSize)
            {
                std::cerr << "Shader (" << shaderName << "): has NO bindings"
                          << std::endl;
                return;
            }
            else
            {
                std::cerr << "Shader (" << shaderName
                          << "): has these bindings (@group(0))" << std::endl;
            }

            char buf[64];

            // UBOs
            for (const auto& [name, ubo] : ubos)
            {
                std::cerr << name << std::endl;
                std::cerr << "\t@binding(" << ubo.binding
                          << ") = uniform size=" << ubo.size << std::endl;
                std::cerr << "\t        "
                          << stageNames(ubo.visibility, buf, sizeof(buf))
                          << std::endl;
            }

            // Textures
            for (const auto& [name, texture] : textureBindings)
            {
                std::cerr << name << std::endl;
                std::cerr << "\t@binding(" << texture.binding
                          << ") = texture, @binding(" << texture.samplerBinding
                          << ") = sampler" << std::endl;
                std::cerr << "\t        "
                          << stageNames(texture.visibility, buf, sizeof(buf))
                          << std::endl;
            }

            // Storage buffers
            for (const auto& [name, sb] : storageBufferBindings)
            {
                std::cerr << name << std::endl;
                std::cerr << "\t@binding(" << sb.binding << ") = storage buffer"
                          << std::endl;
                std::cerr << "\t        "
                          << stageNames(sb.visibility, buf, sizeof(buf))
                          << std::endl;
            }

            // Storage images
            for (const auto& [name, si] : storageImageBindings)
            {
                std::cerr << name << std::endl;
                std::cerr << "\t@binding(" << si.binding << ") = storage texture"
                          << std::endl;
                std::cerr << "\t        "
                          << stageNames(si.visibility, buf, sizeof(buf))
                          << std::endl;
            }

            // SSBOs
            for (const auto& [name, ssbo] : ssbos)
            {
                std::cerr << name << std::endl;
                std::cerr << "\t@binding(" << ssbo.binding << ") = SSBO"
                          << std::endl;
                std::cerr << "\t        "
                          << stageNames(ssbo.visibility, buf, sizeof(buf))
                          << std::endl;
            }

            // Push block
            if (pushSize > 0)
            {
                std::cerr << "push block" << std::endl;
                std::cerr << "\t@binding(" << pushBinding
                          << ") = uniform (dynamic offset) size=" << pushSize
                          << std::endl;
                std::cerr << "\t        "
                          << stageNames(pushStageFlags, buf, sizeof(buf))
                          << std::endl;
            }
        }

        void Shader::createPush(
            const std::string& name, const std::size_t size,
            const ShaderFlags stageFlags)
        {
            // A push block now occupies a real binding.  Re-declaring it
            // keeps the binding it already has.
            if (0 == pushSize)
                pushBinding = current_binding_index++;
            pushSize = size;
            pushStageFlags = getWGPUShaderFlags(stageFlags);
            layoutDirty = true;
        }

        void Shader::setPushCapacity(uint64_t bytes)
        {
            pushCapacity = bytes;
            layoutDirty = true;
        }

        uint32_t Shader::_pushAllocate(const void* data)
        {
            if (!pushRing || !pushRing->buffer)
                throw std::runtime_error(
                    "Shader " + shaderName +
                    ": push data given but no push block exists "
                    "(createPush + createBindingSet first)");

            PushRing& ring = *pushRing;
            const uint64_t offset = alignUp(ring.offset, ring.alignment);
            if (offset + ring.entrySize > ring.capacity)
            {
                throw std::runtime_error(
                    "Shader " + shaderName +
                    ": push block ring is full.  Call Shader::bind() with a "
                    "new frame index (or beginFrame()) once per frame, or "
                    "increase setPushCapacity().");
            }

            std::vector<uint8_t> tmp(ring.entrySize, 0);
            std::memcpy(tmp.data(), data, pushSize);
            wgpuQueueWriteBuffer(
                _queue, ring.buffer, offset, tmp.data(), tmp.size());

            ring.offset = offset + ring.entrySize;
            lastPushOffset = static_cast<uint32_t>(offset);
            return lastPushOffset;
        }

        void Shader::bindToPass(
            WGPURenderPassEncoder pass, const void* pushData,
            uint32_t groupIndex)
        {
            WGPUBindGroup group = getBindGroup();
            if (pushSize > 0)
            {
                const uint32_t offset =
                    pushData ? _pushAllocate(pushData) : lastPushOffset;
                wgpuRenderPassEncoderSetBindGroup(
                    pass, groupIndex, group, 1, &offset);
            }
            else
            {
                wgpuRenderPassEncoderSetBindGroup(
                    pass, groupIndex, group, 0, nullptr);
            }
        }

        void Shader::bindToPass(
            WGPUComputePassEncoder pass, const void* pushData,
            uint32_t groupIndex)
        {
            WGPUBindGroup group = getBindGroup();
            if (pushSize > 0)
            {
                const uint32_t offset =
                    pushData ? _pushAllocate(pushData) : lastPushOffset;
                wgpuComputePassEncoderSetBindGroup(
                    pass, groupIndex, group, 1, &offset);
            }
            else
            {
                wgpuComputePassEncoderSetBindGroup(
                    pass, groupIndex, group, 0, nullptr);
            }
        }

        // ====================================================================
        //  Layout
        // ====================================================================

        void Shader::_ensureLayout()
        {
            TLRENDER_P();

            if (activeLayout && !layoutDirty)
                return;

            WGPULimits limits = {};
            if (wgpuDeviceGetLimits(_device, &limits) != WGPUStatus_Success)
            {
                limits = {};
                limits.maxUniformBufferBindingSize = 64u * 1024;
                limits.minUniformBufferOffsetAlignment = 256;
                limits.maxBufferSize = 256ull * 1024 * 1024;
            }

            // Push block ring.
            if (0 == pushSize)
            {
                pushRing.reset();
            }
            else
            {
                const uint64_t entrySize = alignUp(pushSize, 16);
                if (entrySize > limits.maxUniformBufferBindingSize)
                {
                    std::ostringstream os;
                    os << "tl::webgpu::Shader " << shaderName
                       << ": push block size (" << pushSize
                       << " bytes) exceeds this device's "
                          "maxUniformBufferBindingSize ("
                       << limits.maxUniformBufferBindingSize << " bytes).";
                    throw std::runtime_error(os.str());
                }

                const uint32_t alignment =
                    std::max<uint32_t>(limits.minUniformBufferOffsetAlignment, 4);
                const uint64_t capacity = alignUp(
                    std::max<uint64_t>(pushCapacity, alignUp(entrySize, alignment)),
                    alignment);

                if (!pushRing || pushRing->entrySize != entrySize ||
                    pushRing->capacity != capacity)
                {
                    auto ring = std::make_shared<PushRing>();
                    ring->entrySize = entrySize;
                    ring->alignment = alignment;
                    ring->capacity = capacity;

                    const std::string label = shaderName + " push ring";
                    WGPUBufferDescriptor bd = {};
                    bd.label = {label.c_str(), label.size()};
                    bd.usage = WGPUBufferUsage_Uniform | WGPUBufferUsage_CopyDst;
                    bd.size = capacity;
                    ring->buffer = wgpuDeviceCreateBuffer(_device, &bd);
                    if (!ring->buffer)
                        throw std::runtime_error(
                            shaderName + ": failed to create push ring");
                    pushRing = ring;
                }
            }

            std::vector<WGPUBindGroupLayoutEntry> entries;

            // StorageBuffers
            for (const auto& [name, sbb] : storageBufferBindings)
            {
                WGPUBindGroupLayoutEntry e = {};
                e.binding = sbb.binding;
                e.visibility = sbb.visibility;
                e.buffer.type = getStorageBufferType(sbb.visibility);
                entries.push_back(e);
            }

            // StorageImages
            for (const auto& [name, si] : storageImageBindings)
            {
                WGPUBindGroupLayoutEntry e = {};
                e.binding = si.binding;
                e.visibility = si.visibility;
                e.storageTexture.access = getStorageAccess(si.access);
                e.storageTexture.format = si.format;
                e.storageTexture.viewDimension = WGPUTextureViewDimension_2D;
                entries.push_back(e);
            }

            // UBOs
            for (const auto& [_, ubo] : ubos)
            {
                WGPUBindGroupLayoutEntry e = {};
                e.binding = ubo.binding;
                e.visibility = ubo.visibility;
                e.buffer.type = WGPUBufferBindingType_Uniform;
                e.buffer.hasDynamicOffset = false;
                entries.push_back(e);
            }

            // Textures: a texture binding and a sampler binding.
            for (const auto& [_, texture] : textureBindings)
            {
                WGPUBindGroupLayoutEntry te = {};
                te.binding = texture.binding;
                te.visibility = texture.visibility;
                te.texture.sampleType = getSampleType(texture.type);
                te.texture.viewDimension = getViewDimension(texture.dimension);
                te.texture.multisampled = false;
                entries.push_back(te);

                WGPUBindGroupLayoutEntry se = {};
                se.binding = texture.samplerBinding;
                se.visibility = texture.visibility;
                se.sampler.type = getSamplerBindingType(texture.type);
                entries.push_back(se);
            }

            // SSBOs
            for (const auto& [_, ssbo] : ssbos)
            {
                WGPUBindGroupLayoutEntry e = {};
                e.binding = ssbo.binding;
                e.visibility = ssbo.visibility;
                e.buffer.type = getStorageBufferType(ssbo.visibility);
                entries.push_back(e);
            }

            // Push block (dynamic offset uniform).
            if (pushSize > 0)
            {
                WGPUBindGroupLayoutEntry e = {};
                e.binding = pushBinding;
                e.visibility = pushStageFlags;
                e.buffer.type = WGPUBufferBindingType_Uniform;
                e.buffer.hasDynamicOffset = true;
                entries.push_back(e);
            }

            const std::string label = shaderName + " bind group layout";
            WGPUBindGroupLayoutDescriptor desc = {};
            desc.label = {label.c_str(), label.size()};
            desc.entryCount = entries.size();
            desc.entries = entries.data();

            WGPUBindGroupLayout layout =
                wgpuDeviceCreateBindGroupLayout(_device, &desc);
            if (!layout)
            {
                throw std::runtime_error(
                    "failed to create bind group layout for " + shaderName);
            }

            activeLayout = std::make_shared<BindGroupLayout>(layout);
            layoutDirty = false;

            // A pipeline layout built from the old layout is stale.
            if (p.pipelineLayout)
            {
                wgpuPipelineLayoutRelease(p.pipelineLayout);
                p.pipelineLayout = nullptr;
            }
        }

        void Shader::createPipelineLayout()
        {
            TLRENDER_P();

            _ensureLayout();

            if (p.pipelineLayout)
            {
                wgpuPipelineLayoutRelease(p.pipelineLayout);
                p.pipelineLayout = nullptr;
            }

            const std::string label = shaderName + " pipeline layout";
            WGPUPipelineLayoutDescriptor desc = {};
            desc.label = {label.c_str(), label.size()};
            desc.bindGroupLayoutCount = 1;
            desc.bindGroupLayouts = &activeLayout->handle;

            p.pipelineLayout = wgpuDeviceCreatePipelineLayout(_device, &desc);
            if (!p.pipelineLayout)
            {
                throw std::runtime_error("Failed to create pipeline layout!");
            }
        }

        void Shader::createComputePipeline()
        {
            TLRENDER_P();

            if (!p.compute)
                throw std::runtime_error("Compute shader module not initialized.");

            // Ensure layout exists
            WGPUPipelineLayout layout = getPipelineLayout();

            const std::string label = shaderName + " compute pipeline";
            WGPUComputePipelineDescriptor desc = {};
            desc.label = {label.c_str(), label.size()};
            desc.layout = layout;
            desc.compute.module = p.compute;
            desc.compute.entryPoint = {
                p.computeEntry.c_str(), p.computeEntry.size()};

            WGPUComputePipeline pipeline =
                wgpuDeviceCreateComputePipeline(_device, &desc);
            if (!pipeline)
            {
                throw std::runtime_error("Failed to create compute pipeline!");
            }

            if (p.computePipeline)
                wgpuComputePipelineRelease(p.computePipeline);
            p.computePipeline = pipeline;
        }

        void Shader::dispatch(
            WGPUComputePassEncoder pass, uint32_t groupCountX,
            uint32_t groupCountY, uint32_t groupCountZ, const void* pushData)
        {
            TLRENDER_P();

            if (!p.computePipeline)
                throw std::runtime_error(
                    "Shader " + shaderName +
                    ": call createComputePipeline() before dispatch()");

            // Bind the compute pipeline
            wgpuComputePassEncoderSetPipeline(pass, p.computePipeline);

            // Bind the bind group (and push data)
            bindToPass(pass, pushData);

            // Run the shader
            wgpuComputePassEncoderDispatchWorkgroups(
                pass, groupCountX, groupCountY, groupCountZ);
        }

        void Shader::dispatch(
            WGPUCommandEncoder encoder, uint32_t groupCountX,
            uint32_t groupCountY, uint32_t groupCountZ, const void* pushData)
        {
            const std::string label = shaderName + " compute pass";
            WGPUComputePassDescriptor desc = {};
            desc.label = {label.c_str(), label.size()};

            WGPUComputePassEncoder pass =
                wgpuCommandEncoderBeginComputePass(encoder, &desc);
            dispatch(pass, groupCountX, groupCountY, groupCountZ, pushData);
            wgpuComputePassEncoderEnd(pass);
            wgpuComputePassEncoderRelease(pass);
        }

        void* Shader::mapSSBO(const std::string& name)
        {
            if (!activeBindingSet)
                throw std::runtime_error("No activeBindingSet for Shader " + shaderName);
            return activeBindingSet->mapSSBO(name);
        }

        void Shader::clearSSBO(WGPUCommandEncoder encoder, const std::string& name)
        {
            if (!activeBindingSet)
                throw std::runtime_error("No activeBindingSet for Shader " + shaderName);
            activeBindingSet->clearSSBO(encoder, name);
        }

        void Shader::unmapSSBO(const std::string& name)
        {
            if (!activeBindingSet)
                throw std::runtime_error("No activeBindingSet for Shader " + shaderName);
            activeBindingSet->unmapSSBO(name);
        }

        std::shared_ptr<ShaderBindingSet> Shader::createBindingSet()
        {
            _ensureLayout();

            WGPULimits limits = {};
            if (wgpuDeviceGetLimits(_device, &limits) != WGPUStatus_Success)
            {
                limits = {};
                limits.maxUniformBufferBindingSize = 64u * 1024;
                limits.maxStorageBufferBindingSize = 128u * 1024 * 1024;
            }

            auto bindingSet = std::make_shared<ShaderBindingSet>(_ctx);
            bindingSet->shaderName = shaderName;
            bindingSet->bindGroupLayout = activeLayout;

            auto makeBuffer = [&](const std::string& name, WGPUBufferUsage usage,
                                  uint64_t size)
            {
                const std::string label = shaderName + " " + name;
                WGPUBufferDescriptor bd = {};
                bd.label = {label.c_str(), label.size()};
                bd.usage = usage;
                bd.size = size;
                WGPUBuffer buffer = wgpuDeviceCreateBuffer(_device, &bd);
                if (!buffer)
                    throw std::runtime_error(
                        "failed to create buffer " + label);
                return buffer;
            };

            // UBOs.  One buffer each - no per-frame copies (see
            // ShaderBindingSet).  Buffers start zero-initialized.
            for (const auto& [name, uboTemplate] : ubos)
            {
                if (uboTemplate.size > limits.maxUniformBufferBindingSize)
                {
                    throw std::runtime_error(
                        "Uniform '" + name + "' of shader " + shaderName +
                        " exceeds maxUniformBufferBindingSize (" +
                        std::to_string(limits.maxUniformBufferBindingSize) + ")");
                }
                ShaderBindingSet::UniformParameter ubo = uboTemplate;
                ubo.buffer = makeBuffer(
                    name, WGPUBufferUsage_Uniform | WGPUBufferUsage_CopyDst,
                    alignUp(std::max<size_t>(ubo.size, 4), 16));
                bindingSet->uniforms[name] = ubo;
            }

            // Storage buffers: created by the first setStorageBuffer().
            for (const auto& [name, sbb] : storageBufferBindings)
            {
                ShaderBindingSet::StorageBufferParameter sb = sbb;
                sb.buffer = nullptr;
                sb.currentSize = 0;
                bindingSet->storageBuffers[name] = sb;
            }

            // Texture bindings: set by setTexture/setFBO.
            for (const auto& [name, texBinding] : textureBindings)
            {
                ShaderBindingSet::TextureParameter t = texBinding;
                t.texture.reset();
                t.fbo.reset();
                t.view = nullptr;
                t.sampler = nullptr;
                bindingSet->textures[name] = t;
            }

            // Storage image bindings: set by setStorageImage.
            for (const auto& [name, imageBinding] : storageImageBindings)
            {
                ShaderBindingSet::StorageImageParameter si = imageBinding;
                si.texture.reset();
                bindingSet->storageImages[name] = si;
            }

            // SSBOs.
            for (const auto& [name, ssboTemplate] : ssbos)
            {
                if (ssboTemplate.size > limits.maxStorageBufferBindingSize)
                {
                    throw std::runtime_error(
                        "SSBO '" + name + "' of shader " + shaderName +
                        " exceeds maxStorageBufferBindingSize (" +
                        std::to_string(limits.maxStorageBufferBindingSize) + ")");
                }
                ShaderBindingSet::SSBOParameter ssbo = ssboTemplate;
                ssbo.staging = nullptr;
                ssbo.mapped = false;
                ssbo.buffer = makeBuffer(
                    name,
                    WGPUBufferUsage_Storage | WGPUBufferUsage_CopyDst |
                        WGPUBufferUsage_CopySrc,
                    alignUp(std::max<size_t>(ssbo.size, 4), 4));
                bindingSet->ssbos[name] = ssbo;
            }

            // Push block: shares the shader's ring buffer.
            if (pushSize > 0)
            {
                bindingSet->hasPush = true;
                bindingSet->pushBinding = pushBinding;
                bindingSet->pushVisibility = pushStageFlags;
                bindingSet->pushRing = pushRing;
            }

            // The bind group itself is built lazily by getBindGroup(), once
            // every texture / storage buffer has been set.

            // Make this set active
            activeBindingSet = bindingSet;

            return bindingSet;
        }

    } // namespace webgpu
} // namespace tl
