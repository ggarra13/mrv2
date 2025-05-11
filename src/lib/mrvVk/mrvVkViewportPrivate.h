// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <memory>

#include <tlVk/Mesh.h>
#include <tlVk/OffscreenBuffer.h>
#include <tlTimelineVk/Render.h>
#include <tlVk/ShaderBindingSet.h>
#include <tlVk/Shader.h>

#include "mrvVk/mrvVkDefines.h"
// #include "mrvVk/mrvVkErrors.h"
#include "mrvVk/mrvVkLines.h"
#include "mrvVk/mrvVkViewport.h"

namespace mrv
{
    namespace vulkan
    {

        struct Viewport::VKPrivate
        {
            std::weak_ptr<system::Context> context;
            
            // Main command buffer, stored for convenience.
            VkCommandBuffer  cmd = VK_NULL_HANDLE;

            //! This is for compositing pipelines
            VkPipelineLayout pipeline_layout = VK_NULL_HANDLE;
            VkPipelineLayout annotation_pipeline_layout = VK_NULL_HANDLE;
            VkPipeline       annotation_pipeline = VK_NULL_HANDLE;

            // tlRender variables
            //! Vulkan Offscreen buffers
            image::PixelType colorBufferType = image::PixelType::RGBA_F32;

            std::shared_ptr<timeline_vlk::Render> render;
            std::shared_ptr<timeline_vlk::Render> annotationRender;
            std::shared_ptr<tl::vlk::OffscreenBuffer> buffer;
            std::shared_ptr<tl::vlk::OffscreenBuffer> stereoBuffer;
            std::shared_ptr<tl::vlk::OffscreenBuffer> annotation;
            std::shared_ptr<tl::image::Image> annotationImage; // only used on APPLE
            std::shared_ptr<tl::vlk::OffscreenBuffer> overlay;
            std::shared_ptr<vlk::Shader> shader;
            std::shared_ptr<vlk::Shader> annotationShader;

            uint32_t overlayPBO = 0;
            VkFence overlayFence;
            std::shared_ptr<vlk::VBO> vbo;
            std::shared_ptr<vlk::VAO> vao;
            
            std::shared_ptr<vlk::VBO> avbo;
            std::shared_ptr<vlk::VAO> avao;

            std::shared_ptr<vulkan::Lines> lines;

            bool init_debug = false;
        };

    } // namespace vulkan

//! Define a variable, "gl", that references the private implementation.
#define MRV2_VK() auto& vk = *_vk

} // namespace mrv
