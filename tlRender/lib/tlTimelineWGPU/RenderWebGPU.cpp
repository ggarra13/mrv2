// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2025-Present Gonzalo Garramuño
// All rights reserved.

#include <tlTimelineWGPU/RenderPrivate.h>

#include <iostream>
#include <mutex>
#include <stdexcept>
#include <string>

#if DEBUG_PIPELINE_USE
#define DEBUG_PIPELINE(x) std::cerr << x << std::endl;
#else
#define DEBUG_PIPELINE(x)
#endif

#if DEBUG_PIPELINE_LAYOUT_USE
#define DEBUG_PIPELINE_LAYOUT(x) std::cerr << x << std::endl;
#else
#define DEBUG_PIPELINE_LAYOUT(x)
#endif

namespace tl
{
    namespace timeline_wgpu
    {
        namespace
        {
            //! Pipelines are cached by name and by pipeline group (the
            //! render target they were built for).  The main target has an
            //! empty group, so its keys are the plain pipeline names.
            std::string pipelineKey(const std::string& name,
                                    const std::string& group)
            {
                return group.empty() ? name : name + "@" + group;
            }

            //! Build the pipeline's vertex input from the mesh's VBO layout.
            wgpu::VertexInputStateInfo
            makeVertexInput(const std::shared_ptr<wgpu::VBO>& mesh)
            {
                const WGPUVertexBufferLayout& layout = mesh->getVertexLayout();

                wgpu::VertexBufferLayoutInfo buffer;
                buffer.arrayStride = layout.arrayStride;
                buffer.stepMode = layout.stepMode;
                buffer.attributes = mesh->getAttributes();

                wgpu::VertexInputStateInfo out;
                out.buffers.push_back(buffer);
                return out;
            }
        } // namespace

        void Render::_createBindingSet(const std::shared_ptr<wgpu::Shader> shader)
        {
            // Shader::createBindingSet() also makes the new set the active
            // one, so there is nothing else to do.
            shader->createBindingSet();
        }

        WGPUPipelineLayout Render::_createPipelineLayout(
            const std::string& pipelineLayoutName,
            const std::shared_ptr<wgpu::Shader> shader)
        {
            TLRENDER_P();

            // The Shader owns its pipeline layout (bind group layout + push
            // ring), so we only keep a NON-OWNING handle in the map.  Never
            // wgpuPipelineLayoutRelease() the values of p.pipelineLayouts.
            // Always refresh it: it changes if the shader's layout was
            // rebuilt.
            DEBUG_PIPELINE_LAYOUT("REFRESHING pipelineLayout " << pipelineLayoutName);
            WGPUPipelineLayout pipelineLayout = shader->getPipelineLayout();
            p.pipelineLayouts[pipelineLayoutName] = pipelineLayout;
            return pipelineLayout;
        }

        void Render::createPipeline(
            const std::string& pipelineName,
            const std::string& pipelineLayoutName,
            const std::shared_ptr<wgpu::OffscreenBuffer>& fbo,
            const std::shared_ptr<wgpu::Shader> shader,
            const std::shared_ptr<wgpu::VBO>& mesh,
            const wgpu::ColorTargetStateInfo& colorTarget,
            const wgpu::DepthStencilStateInfo& depthStencil,
            const wgpu::MultisampleStateInfo& multisample)
        {
            TLRENDER_P();

            if (!fbo || !shader || !mesh)
            {
                throw std::runtime_error(
                    "Render::createPipeline(" + pipelineName +
                    "): null fbo, shader or mesh");
            }
            if (!p.renderPass)
            {
                throw std::runtime_error(
                    "Render::createPipeline(" + pipelineName +
                    "): no active render pass");
            }

            WGPUPipelineLayout pipelineLayout =
                _createPipelineLayout(pipelineLayoutName, shader);

            //
            // Pass pipeline creation parameters to pipelineState.
            //
            // Unlike Vulkan, a WebGPU pipeline is not tied to a render pass
            // object; it only has to be *compatible* with the pass it is used
            // in: same color format, depth/stencil format and sample count.
            // So all of those come from the target fbo.
            //
            // Viewport and scissor are pass state (set below), blend
            // constants, stencil reference etc. are pass state too, and
            // write masks / blend / stencil ops are baked into the
            // pipeline (no dynamic state equivalents).
            //
            wgpu::PipelineCreationState pipelineState;
            pipelineState.label = pipelineName;

            wgpu::ShaderStageInfo vs;
            vs.name = shader->getName();
            vs.module = shader->getVertex();
            vs.entryPoint = shader->getVertexEntryPoint();
            pipelineState.vertexStage = vs;

            wgpu::ShaderStageInfo fs;
            fs.name = shader->getName();
            fs.module = shader->getFragment();
            fs.entryPoint = shader->getFragmentEntryPoint();
            pipelineState.fragmentStage = fs;

            pipelineState.vertexInputState = makeVertexInput(mesh);
            pipelineState.primitiveState = wgpu::PrimitiveStateInfo();

            wgpu::ColorTargetStateInfo target = colorTarget;
            target.format = fbo->getFormat();
            pipelineState.colorTargets.push_back(target);

            // The pipeline must declare a depth/stencil state if (and only
            // if) the pass has a depth/stencil attachment.
            const WGPUTextureFormat depthFormat = fbo->getDepthFormat();
            if (depthFormat != WGPUTextureFormat_Undefined)
            {
                wgpu::DepthStencilStateInfo ds = depthStencil;
                ds.format = depthFormat;
                pipelineState.depthStencilState = ds;
            }

            wgpu::MultisampleStateInfo ms = multisample;
            ms.count = fbo->getSampleCount();
            pipelineState.multisampleState = ms;

            pipelineState.layout = pipelineLayout;

            const std::string key = pipelineKey(pipelineName, p.pipelineGroup);
            WGPURenderPipeline pipeline = nullptr;
            auto it = p.pipelines.find(key);
            if (it == p.pipelines.end())
            {
                DEBUG_PIPELINE("CREATING   pipeline " << pipelineName);
                pipeline = pipelineState.create(ctx.device);
                p.pipelines[key] = std::make_pair(pipelineState, pipeline);
            }
            else if (pipelineState != it->second.first)
            {
                DEBUG_PIPELINE("RECREATING pipeline " << pipelineName);
                pipeline = pipelineState.create(ctx.device);

                // WebGPU objects are reference counted and a recorded
                // command keeps what it uses alive, so the old pipeline can
                // be released right away even if it was used earlier in
                // this frame.
                if (it->second.second)
                    wgpuRenderPipelineRelease(it->second.second);
                it->second = std::make_pair(pipelineState, pipeline);
            }
            else
            {
                pipeline = it->second.second;
            }

            // Enable the pipeline.
            // WebGPU and are NOT touched here, so a clip rect set earlier in
            // the pass survives pipeline changes.)
            wgpuRenderPassEncoderSetPipeline(p.renderPass, pipeline);
            p.currentPipeline = key;
        }

        void Render::createPipeline(
            const std::shared_ptr<wgpu::OffscreenBuffer>& fbo,
            const std::string& pipelineName,
            const std::string& pipelineLayoutName,
            const std::string& shaderName,
            const std::string& meshName,
            const bool enableBlending,
            const WGPUBlendFactor srcColorBlendFactor,
            const WGPUBlendFactor dstColorBlendFactor,
            const WGPUBlendFactor srcAlphaBlendFactor,
            const WGPUBlendFactor dstAlphaBlendFactor,
            const WGPUBlendOperation colorBlendOp,
            const WGPUBlendOperation alphaBlendOp)
        {
            TLRENDER_P();

            const auto& shader = p.shaders[shaderName];
            const auto& mesh = p.vbos[meshName];

            wgpu::ColorTargetStateInfo ct;
            if (enableBlending)
            {
                wgpu::BlendStateInfo blend;
                blend.color.operation = colorBlendOp;
                blend.color.srcFactor = srcColorBlendFactor;
                blend.color.dstFactor = dstColorBlendFactor;
                blend.alpha.operation = alphaBlendOp;
                blend.alpha.srcFactor = srcAlphaBlendFactor;
                blend.alpha.dstFactor = dstAlphaBlendFactor;
                ct.blend = blend;
            }

            // depthCompare/depthWrite are only used when the fbo has a depth
            // attachment (createPipeline() drops them otherwise).  Stencil
            // stays at "always / keep": WebGPU has no stencil enable bit.
            wgpu::DepthStencilStateInfo ds;
            if (fbo->hasDepth())
            {
                ds.depthWriteEnabled = WGPUOptionalBool_True;
                ds.depthCompare = WGPUCompareFunction_LessEqual;
            }

            createPipeline(pipelineName, pipelineLayoutName, fbo, shader,
                           mesh, ct, ds);

            // Viewport and scissor are per pass in WebGPU.
            fbo->setupViewportAndScissor(p.renderPass);
            if (p.clipRectEnabled && p.fbo == fbo)
            {
                setClipRect(p.clipRect);
            }
        }

        void Render::usePipeline(const std::string& pipelineName)
        {
            TLRENDER_P();

            const std::string key = pipelineKey(pipelineName, p.pipelineGroup);
            if (p.currentPipeline == key)
                return;

            const auto pipelineIt = p.pipelines.find(key);
            if (pipelineIt == p.pipelines.end())
                return;
            WGPURenderPipeline pipeline = pipelineIt->second.second;

            // Enable the pipeline.
            wgpuRenderPassEncoderSetPipeline(p.renderPass, pipeline);
            p.currentPipeline = key;

            ++(p.currentStats.pipelineChanges);
        }

        void Render::_bindDescriptorSets(
            WGPURenderPassEncoder pass,
            const std::shared_ptr<wgpu::Shader> shader,
            const void* pushData)
        {
            TLRENDER_P();

            // Replaces vkCmdBindDescriptorSets + vkCmdPushConstants.  The
            // push block is a uniform buffer bound with a dynamic offset,
            // so each draw still sees its own copy.  If pushData is null
            // the previous push data is reused.
            shader->bindToPass(pass, pushData);
        }

        void Render::_bindComputeDescriptorSets(
            WGPUComputePassEncoder pass,
            const std::shared_ptr<wgpu::Shader> shader,
            const void* pushData)
        {
            shader->bindToPass(pass, pushData);
        }

        void Render::_vkDraw(const std::string& meshName)
        {
            TLRENDER_P();

            // Upload the vertex data into the pool and draw immediately.
            // The pool selects a slot with enough room, creating a new slot
            // when the current one is full.  Uploads are queue writes, which
            // land before the whole command buffer runs, so every draw gets
            // its own range (no overwriting within a frame).
            const wgpu::VAOAllocation alloc =
                p.vaoPool->upload(p.vbos[meshName]);
            p.vaoPool->draw(p.renderPass, alloc);
        }

        WGPURenderPassEncoder Render::getRenderPass() const
        {
            return _p->renderPass;
        }

        void Render::setRenderPass(WGPURenderPassEncoder value)
        {
            _p->renderPass = value;
            _p->currentPipeline.clear();
        }
    } // namespace timeline_wgpu
} // namespace tl
