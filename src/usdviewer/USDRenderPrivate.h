// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// Copyright (c) 2025-Present Gonzalo Garramuño
// All rights reserved.

#pragma once


#include <USDRenderOptions.h>
#include <USDRender.h>

#include <tlVk/Mesh.h>
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
        // For drawing
        std::string vertexDummy();
        std::string fragmentDummy();

        std::string vertexSTs();
        std::string fragmentSTs();

        std::string vertexUSD();
        std::string fragmentUSD();

        std::string vertexPBR();
        std::string fragmentPBR();
        



        struct Render::Private
        {
            // Vulkan variables
            VkCommandBuffer cmd;
            std::shared_ptr<vlk::OffscreenBuffer> fbo;
            VkRenderPass renderPass;

            // Current frame in the swapchain
            int32_t frameIndex; // must be an int32_t not an uint32_t.

            math::Size2i renderSize;
            timeline::RenderOptions renderOptions;

            std::weak_ptr<system::Context> context;
            math::Box2i viewport;
            math::Matrix4x4f transform;
            bool clipRectEnabled = false;
            math::Box2i clipRect;
            std::string currentPipeline;

            struct FrameGarbage
            {
                std::vector<VkPipeline> pipelines;
                std::vector<VkPipelineLayout> pipelineLayouts;
                std::vector<std::shared_ptr<vlk::ShaderBindingSet> > bindingSets;
                std::vector<std::shared_ptr<vlk::OffscreenBuffer> > buffers;
            };
            std::array<FrameGarbage, vlk::MAX_FRAMES_IN_FLIGHT> garbage;

            std::unordered_map<std::string, std::shared_ptr<vlk::Shader> > shaders;  // Vertex / Fragment
            std::unordered_map<std::string, std::shared_ptr<vlk::Shader> > compute;  // Compute
            std::unordered_map<std::string, std::shared_ptr<vlk::OffscreenBuffer> >
                buffers;
            std::unordered_map<std::string, VkPipelineLayout> pipelineLayouts;
            std::unordered_map<
                std::string, std::pair<vlk::PipelineCreationState, VkPipeline>>
                pipelines;
            std::unordered_map<std::string, std::shared_ptr<vlk::VBO> > vbos;
            std::unordered_map<std::string, std::shared_ptr<vlk::VAO> > vaos;

            std::chrono::steady_clock::time_point timer;
            struct Stats
            {
                int time = 0;
                size_t rects = 0;
                size_t meshes = 0;
                size_t meshTriangles = 0;
                size_t text = 0;
                size_t textTriangles = 0;
                size_t textures = 0;
                size_t images = 0;
                size_t pipelineChanges = 0;
            };
            Stats currentStats;
            std::list<Stats> stats;
            std::chrono::steady_clock::time_point logTimer;
        };
        
    } // namespace usd
} // namespace tl
