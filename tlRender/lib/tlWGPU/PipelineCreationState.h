// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2025-Present Gonzalo Garramuño
// All rights reserved.

#pragma once

#include <tlWGPU/WGPU.h>

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

namespace tl
{
    namespace wgpu
    {
        namespace detail
        {
            inline WGPUStringView makeStringView(const std::string& str)
            {
                return WGPUStringView{
                    str.empty() ? nullptr : str.data(),
                    str.empty() ? WGPU_STRLEN : str.size()};
            }
        } // namespace detail

        struct VertexBufferLayoutInfo
        {
            // Corresponds to WGPUVertexBufferLayout
            uint64_t arrayStride = 0;
            WGPUVertexStepMode stepMode = WGPUVertexStepMode_Vertex;
            std::vector<WGPUVertexAttribute> attributes;

            bool operator==(const VertexBufferLayoutInfo& other) const
            {
                if (arrayStride != other.arrayStride ||
                    stepMode != other.stepMode ||
                    attributes.size() != other.attributes.size())
                {
                    return false;
                }

                for (std::size_t i = 0; i < attributes.size(); ++i)
                {
                    const auto& a1 = attributes[i];
                    const auto& a2 = other.attributes[i];
                    if (a1.format != a2.format || a1.offset != a2.offset ||
                        a1.shaderLocation != a2.shaderLocation)
                    {
                        return false;
                    }
                }
                return true;
            }
            bool operator!=(const VertexBufferLayoutInfo& other) const
            {
                return !(*this == other);
            }
        };

        struct VertexInputStateInfo
        {
            // Corresponds to WGPUVertexState buffer layouts
            std::vector<VertexBufferLayoutInfo> buffers;

            bool operator==(const VertexInputStateInfo& other) const
            {
                return buffers == other.buffers;
            }
            bool operator!=(const VertexInputStateInfo& other) const
            {
                return !(*this == other);
            }
        };

        struct PrimitiveStateInfo
        {
            // Corresponds to WGPUPrimitiveState
            WGPUPrimitiveTopology topology = WGPUPrimitiveTopology_TriangleList;
            WGPUIndexFormat stripIndexFormat = WGPUIndexFormat_Undefined;
            WGPUFrontFace frontFace = WGPUFrontFace_CCW;
            WGPUCullMode cullMode = WGPUCullMode_None;
            WGPUBool unclippedDepth = false;

            bool operator==(const PrimitiveStateInfo& other) const
            {
                return topology == other.topology &&
                       stripIndexFormat == other.stripIndexFormat &&
                       frontFace == other.frontFace &&
                       cullMode == other.cullMode &&
                       unclippedDepth == other.unclippedDepth;
            }
            bool operator!=(const PrimitiveStateInfo& other) const
            {
                return !(*this == other);
            }
        };

        struct BlendComponentInfo
        {
            // Corresponds to WGPUBlendComponent
            WGPUBlendOperation operation = WGPUBlendOperation_Add;
            WGPUBlendFactor srcFactor = WGPUBlendFactor_One;
            WGPUBlendFactor dstFactor = WGPUBlendFactor_Zero;

            bool operator==(const BlendComponentInfo& other) const
            {
                return operation == other.operation &&
                       srcFactor == other.srcFactor &&
                       dstFactor == other.dstFactor;
            }
            bool operator!=(const BlendComponentInfo& other) const
            {
                return !(*this == other);
            }
        };

        struct BlendStateInfo
        {
            // Corresponds to WGPUBlendState
            BlendComponentInfo color{
                WGPUBlendOperation_Add,
                WGPUBlendFactor_SrcAlpha,
                WGPUBlendFactor_OneMinusSrcAlpha};
            BlendComponentInfo alpha{
                WGPUBlendOperation_Add,
                WGPUBlendFactor_One,
                WGPUBlendFactor_Zero};

            bool operator==(const BlendStateInfo& other) const
            {
                return color == other.color && alpha == other.alpha;
            }
            bool operator!=(const BlendStateInfo& other) const
            {
                return !(*this == other);
            }
        };

        struct ColorTargetStateInfo
        {
            // Corresponds to WGPUColorTargetState
            WGPUTextureFormat format = WGPUTextureFormat_Undefined;
            std::optional<BlendStateInfo> blend; // nullopt => blending disabled
            WGPUColorWriteMask writeMask = WGPUColorWriteMask_All;

            bool operator==(const ColorTargetStateInfo& other) const
            {
                return format == other.format && blend == other.blend &&
                       writeMask == other.writeMask;
            }
            bool operator!=(const ColorTargetStateInfo& other) const
            {
                return !(*this == other);
            }
        };

        struct MultisampleStateInfo
        {
            // Corresponds to WGPUMultisampleState
            uint32_t count = 1;
            uint32_t mask = 0xFFFFFFFF;
            WGPUBool alphaToCoverageEnabled = false;

            bool operator==(const MultisampleStateInfo& other) const
            {
                return count == other.count && mask == other.mask &&
                       alphaToCoverageEnabled == other.alphaToCoverageEnabled;
            }
            bool operator!=(const MultisampleStateInfo& other) const
            {
                return !(*this == other);
            }
        };

        struct StencilFaceStateInfo
        {
            // Corresponds to WGPUStencilFaceState
            WGPUCompareFunction compare = WGPUCompareFunction_Always;
            WGPUStencilOperation failOp = WGPUStencilOperation_Keep;
            WGPUStencilOperation depthFailOp = WGPUStencilOperation_Keep;
            WGPUStencilOperation passOp = WGPUStencilOperation_Keep;

            bool operator==(const StencilFaceStateInfo& other) const
            {
                return compare == other.compare && failOp == other.failOp &&
                       depthFailOp == other.depthFailOp &&
                       passOp == other.passOp;
            }
            bool operator!=(const StencilFaceStateInfo& other) const
            {
                return !(*this == other);
            }
        };

        struct DepthStencilStateInfo
        {
            // Corresponds to WGPUDepthStencilState
            WGPUTextureFormat format = WGPUTextureFormat_Undefined;
            WGPUOptionalBool depthWriteEnabled = WGPUOptionalBool_False;
            WGPUCompareFunction depthCompare = WGPUCompareFunction_Always;
            StencilFaceStateInfo stencilFront{};
            StencilFaceStateInfo stencilBack{};
            uint32_t stencilReadMask = 0xFFFFFFFF;
            uint32_t stencilWriteMask = 0xFFFFFFFF;
            int32_t depthBias = 0;
            float depthBiasSlopeScale = 0.0f;
            float depthBiasClamp = 0.0f;

            bool operator==(const DepthStencilStateInfo& other) const
            {
                return format == other.format &&
                       depthWriteEnabled == other.depthWriteEnabled &&
                       depthCompare == other.depthCompare &&
                       stencilFront == other.stencilFront &&
                       stencilBack == other.stencilBack &&
                       stencilReadMask == other.stencilReadMask &&
                       stencilWriteMask == other.stencilWriteMask &&
                       depthBias == other.depthBias &&
                       depthBiasSlopeScale == other.depthBiasSlopeScale &&
                       depthBiasClamp == other.depthBiasClamp;
            }
            bool operator!=(const DepthStencilStateInfo& other) const
            {
                return !(*this == other);
            }
        };

        struct ShaderStageInfo
        {
            std::string name;
            WGPUShaderModule module = nullptr;
            std::string entryPoint = "main";

            bool operator==(const ShaderStageInfo& other) const
            {
                return name == other.name && entryPoint == other.entryPoint;
            }
            bool operator!=(const ShaderStageInfo& other) const
            {
                return !(*this == other);
            }
        };

        struct PipelineCreationState
        {
            std::string label;

            std::optional<ShaderStageInfo> vertexStage;
            std::optional<ShaderStageInfo> fragmentStage;

            std::optional<VertexInputStateInfo> vertexInputState;
            std::optional<PrimitiveStateInfo> primitiveState;
            std::optional<MultisampleStateInfo> multisampleState;
            std::optional<DepthStencilStateInfo> depthStencilState;
            std::vector<ColorTargetStateInfo> colorTargets;

            WGPUPipelineLayout layout = nullptr;

            bool operator==(const PipelineCreationState& other) const
            {
                return label == other.label &&
                       vertexStage == other.vertexStage &&
                       fragmentStage == other.fragmentStage &&
                       vertexInputState == other.vertexInputState &&
                       primitiveState == other.primitiveState &&
                       multisampleState == other.multisampleState &&
                       depthStencilState == other.depthStencilState &&
                       colorTargets == other.colorTargets &&
                       layout == other.layout;
            }

            bool operator!=(const PipelineCreationState& other) const
            {
                return !(*this == other);
            }

            WGPURenderPipeline create(WGPUDevice device) const
            {
                if (!device)
                {
                    throw std::runtime_error("WGPUDevice is null");
                }
                if (!vertexStage.has_value() || !vertexStage->module)
                {
                    throw std::runtime_error(
                        "Vertex shader stage and module are required");
                }

                WGPURenderPipelineDescriptor desc{};
                desc.label = detail::makeStringView(label);
                desc.layout = layout; // nullptr => auto layout

                // 1. Vertex stage & vertex buffer layouts
                std::vector<std::vector<WGPUVertexAttribute>>
                    ownedAttributeArrays;
                std::vector<WGPUVertexBufferLayout> wgpuBufferLayouts;

                if (vertexInputState.has_value())
                {
                    const auto& buffers = vertexInputState->buffers;
                    ownedAttributeArrays.resize(buffers.size());
                    wgpuBufferLayouts.resize(buffers.size());

                    for (std::size_t i = 0; i < buffers.size(); ++i)
                    {
                        ownedAttributeArrays[i] = buffers[i].attributes;
                        wgpuBufferLayouts[i].arrayStride =
                            buffers[i].arrayStride;
                        wgpuBufferLayouts[i].stepMode = buffers[i].stepMode;
                        wgpuBufferLayouts[i].attributeCount =
                            ownedAttributeArrays[i].size();
                        wgpuBufferLayouts[i].attributes =
                            ownedAttributeArrays[i].data();
                    }
                }

                desc.vertex.module = vertexStage->module;
                desc.vertex.entryPoint =
                    detail::makeStringView(vertexStage->entryPoint);
                desc.vertex.bufferCount = wgpuBufferLayouts.size();
                desc.vertex.buffers =
                    wgpuBufferLayouts.empty() ? nullptr : wgpuBufferLayouts.data();

                // 2. Primitive state
                desc.primitive.topology = WGPUPrimitiveTopology_TriangleList;
                desc.primitive.stripIndexFormat = WGPUIndexFormat_Undefined;
                desc.primitive.frontFace = WGPUFrontFace_CCW;
                desc.primitive.cullMode = WGPUCullMode_None;
                desc.primitive.unclippedDepth = false;

                if (primitiveState.has_value())
                {
                    const auto& ps = *primitiveState;
                    desc.primitive.topology = ps.topology;
                    desc.primitive.stripIndexFormat = ps.stripIndexFormat;
                    desc.primitive.frontFace = ps.frontFace;
                    desc.primitive.cullMode = ps.cullMode;
                    desc.primitive.unclippedDepth = ps.unclippedDepth;
                }

                // 3. Multisample state
                desc.multisample.count = 1;
                desc.multisample.mask = 0xFFFFFFFF;
                desc.multisample.alphaToCoverageEnabled = false;

                if (multisampleState.has_value())
                {
                    const auto& ms = *multisampleState;
                    desc.multisample.count = ms.count;
                    desc.multisample.mask = ms.mask;
                    desc.multisample.alphaToCoverageEnabled =
                        ms.alphaToCoverageEnabled;
                }

                // 4. Depth/stencil state
                WGPUDepthStencilState depthStencil{};
                if (depthStencilState.has_value())
                {
                    const auto& ds = *depthStencilState;
                    if (ds.format == WGPUTextureFormat_Undefined)
                    {
                        throw std::runtime_error(
                            "DepthStencilStateInfo requires a valid texture "
                            "format");
                    }
                    depthStencil.format = ds.format;
                    depthStencil.depthWriteEnabled = ds.depthWriteEnabled;
                    depthStencil.depthCompare = ds.depthCompare;
                    depthStencil.stencilFront.compare =
                        ds.stencilFront.compare;
                    depthStencil.stencilFront.failOp = ds.stencilFront.failOp;
                    depthStencil.stencilFront.depthFailOp =
                        ds.stencilFront.depthFailOp;
                    depthStencil.stencilFront.passOp = ds.stencilFront.passOp;
                    depthStencil.stencilBack.compare = ds.stencilBack.compare;
                    depthStencil.stencilBack.failOp = ds.stencilBack.failOp;
                    depthStencil.stencilBack.depthFailOp =
                        ds.stencilBack.depthFailOp;
                    depthStencil.stencilBack.passOp = ds.stencilBack.passOp;
                    depthStencil.stencilReadMask = ds.stencilReadMask;
                    depthStencil.stencilWriteMask = ds.stencilWriteMask;
                    depthStencil.depthBias = ds.depthBias;
                    depthStencil.depthBiasSlopeScale = ds.depthBiasSlopeScale;
                    depthStencil.depthBiasClamp = ds.depthBiasClamp;
                    desc.depthStencil = &depthStencil;
                }
                else
                {
                    desc.depthStencil = nullptr;
                }

                // 5. Fragment state & color targets
                WGPUFragmentState fragment{};
                std::vector<WGPUBlendState> ownedBlendStates;
                std::vector<WGPUColorTargetState> wgpuColorTargets;

                if (fragmentStage.has_value())
                {
                    if (!fragmentStage->module)
                    {
                        throw std::runtime_error(
                            "Fragment shader stage is set but module is null");
                    }

                    ownedBlendStates.reserve(colorTargets.size());
                    wgpuColorTargets.reserve(colorTargets.size());

                    for (const auto& ct : colorTargets)
                    {
                        WGPUColorTargetState target{};
                        target.format = ct.format;
                        target.writeMask = ct.writeMask;

                        if (ct.blend.has_value())
                        {
                            WGPUBlendState blend{};
                            blend.color.operation = ct.blend->color.operation;
                            blend.color.srcFactor = ct.blend->color.srcFactor;
                            blend.color.dstFactor = ct.blend->color.dstFactor;
                            blend.alpha.operation = ct.blend->alpha.operation;
                            blend.alpha.srcFactor = ct.blend->alpha.srcFactor;
                            blend.alpha.dstFactor = ct.blend->alpha.dstFactor;
                            ownedBlendStates.push_back(blend);
                            target.blend = &ownedBlendStates.back();
                        }
                        else
                        {
                            target.blend = nullptr;
                        }

                        wgpuColorTargets.push_back(target);
                    }

                    fragment.module = fragmentStage->module;
                    fragment.entryPoint =
                        detail::makeStringView(fragmentStage->entryPoint);
                    fragment.targetCount = wgpuColorTargets.size();
                    fragment.targets =
                        wgpuColorTargets.empty() ? nullptr : wgpuColorTargets.data();
                    desc.fragment = &fragment;
                }
                else
                {
                    desc.fragment = nullptr;
                }

                WGPURenderPipeline pipeline =
                    wgpuDeviceCreateRenderPipeline(device, &desc);
                if (!pipeline)
                {
                    throw std::runtime_error(
                        "Failed to create WGPURenderPipeline");
                }
                return pipeline;
            }
        };

    } // namespace wgpu
} // namespace tl
