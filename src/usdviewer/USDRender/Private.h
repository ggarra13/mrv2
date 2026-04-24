// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// Copyright (c) 2025-Present Gonzalo Garramuño
// All rights reserved.

#pragma once


#include "engine/options.h"

#include "USDRender/Render.h"

#include "Vk/VAOPool.h"

#include <tlVk/OffscreenBuffer.h>
#include <tlVk/PipelineCreationState.h>
#include <tlVk/Shader.h>
#include <tlVk/Texture.h>

#include <array>
#include <list>
#include <map>
#include <string>

namespace tl
{
    namespace usd
    {
        // Shaders as text
        std::string vertexDummy();
        std::string fragmentDummy();
        std::string vertexDummy_Color();
        std::string fragmentDummy_Color();

        std::string vertexSTs();
        std::string fragmentSTs();

        std::string vertexUSD_UV();
        std::string vertexUSD_UV_Color();
        std::string vertexUSD_UV_Normal();
        std::string vertexUSD_UV_Normal_Color();

        std::string fragmentUSD(bool hasNormal = false,
                                bool hasColor = false,
                                bool hasOIT = false);
        

        struct Render::Private
        {
            // Vulkan variables
            VkCommandBuffer cmd;
            std::shared_ptr<vlk::OffscreenBuffer> fbo;
            VkRenderPass renderPass;

            // Current frame in the swapchain
            int32_t frameIndex; // must be an int32_t not an uint32_t.
            std::shared_ptr<vlk::VAOPool> vaoPool;
            
            
            math::Size2i renderSize;
            timeline::RenderOptions renderOptions;

            std::weak_ptr<system::Context> context;
            math::Box2i viewport;

            math::Matrix4x4f transform;    // vp
            math::Matrix4x4f viewMatrix;   // view
            
            bool clipRectEnabled = false;
            math::Box2i clipRect;
            std::string currentPipeline;

            struct FrameGarbage
            {
                std::vector<VkPipeline> pipelines;
                std::vector<VkPipelineLayout> pipelineLayouts;
                std::vector<std::shared_ptr<vlk::ShaderBindingSet> > bindingSets;
            };
            std::array<FrameGarbage, vlk::MAX_FRAMES_IN_FLIGHT> garbage;

            // Active resources
            std::shared_ptr<vlk::Texture> accum[vlk::MAX_FRAMES_IN_FLIGHT];
            std::shared_ptr<vlk::Texture> reveal[vlk::MAX_FRAMES_IN_FLIGHT];
            VkFramebuffer oitFramebuffer[vlk::MAX_FRAMES_IN_FLIGHT];
            VkRenderPass  oitRenderPass = VK_NULL_HANDLE;
            
            std::unordered_map<std::string, std::shared_ptr<vlk::Shader> > shaders;  // Vertex / Fragment
            std::unordered_map<std::string, std::shared_ptr<vlk::Shader> > compute;  // Compute
            std::unordered_map<std::string, VkPipelineLayout> pipelineLayouts;
            std::unordered_map<
                std::string, std::pair<vlk::PipelineCreationState, VkPipeline>>
                pipelines;
            std::unordered_map<std::string, std::shared_ptr<vlk::VBO> > vbos;
            std::unordered_map<std::string, std::shared_ptr<vlk::VAO> > vaos;
        };
        
    } // namespace usd
} // namespace tl
