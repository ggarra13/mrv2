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
            VkRenderPass     loadRenderPass = VK_NULL_HANDLE;
            VkPipelineLayout pipeline_layout = VK_NULL_HANDLE;
            VkPipelineLayout annotation_pipeline_layout = VK_NULL_HANDLE;
            VkPipeline       annotation_pipeline = VK_NULL_HANDLE;
            VkPipeline       overlay_pipeline = VK_NULL_HANDLE;

            image::PixelType colorBufferType = image::PixelType::RGBA_F32;

            //! Offscreen renderers
            std::shared_ptr<timeline_vlk::Render> render;
            std::shared_ptr<timeline_vlk::Render> annotationRender;
            std::shared_ptr<timeline_vlk::Render> overlayRender;

            //! Offscreen buffers
            std::shared_ptr<tl::vlk::OffscreenBuffer> buffer;
            std::shared_ptr<tl::vlk::OffscreenBuffer> stereoBuffer;
            std::shared_ptr<tl::vlk::OffscreenBuffer> annotation;
            std::shared_ptr<tl::vlk::OffscreenBuffer> overlay;
            std::shared_ptr<tl::vlk::OffscreenBuffer> overlayPBO;

            std::shared_ptr<tl::image::Image> annotationImage; // only used on APPLE
            // Compositing shaders
            std::shared_ptr<vlk::Shader> shader;
            std::shared_ptr<vlk::Shader> annotationShader;

            // Main mesh
            std::shared_ptr<vlk::VBO> vbo;
            std::shared_ptr<vlk::VAO> vao;

            // Annotation mesh
            std::shared_ptr<vlk::VBO> avbo;
            std::shared_ptr<vlk::VAO> avao;
            
            // Lines drawn into the rendered annotations
            std::shared_ptr<vulkan::Lines> lines;

            // Lines drawn into the viewport (safe areas, selection, cursor)
            std::shared_ptr<vulkan::Lines> viewport;

            bool init_debug = false;
        };

    } // namespace vulkan

//! Define a variable, "gl", that references the private implementation.
#define MRV2_VK() auto& vk = *_vk

} // namespace mrv
