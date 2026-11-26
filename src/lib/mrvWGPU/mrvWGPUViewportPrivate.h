
// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <memory>

#include <tlTimelineWGPU/Render.h>

#include <tlWGPU/Mesh.h>
#include <tlWGPU/OffscreenBuffer.h>
#include <tlWGPU/Shader.h>

// #include "mrvWGPU/mrvWGPUDefines.h"
// #include "mrvWGPU/mrvWGPUErrors.h"
// #include "mrvWGPU/mrvWGPULines.h"
// #include "mrvWGPU/mrvWGPUViewport.h"

namespace mrv
{
    namespace webgpu
    {
        struct Viewport::WGPUPrivate
        {
            std::weak_ptr<system::Context> context;


            bool pickPending = false;

            //! Boolean used to turn on dynamic read pixels.
            //! We must not read pixels when wgpu.buffer is just created.
            bool readPixels = false;

            // WGPU variables
            //! Webgpu Offscreen buffers
            image::PixelType colorBufferType = image::PixelType::RGBA_F32;

            std::shared_ptr<timeline_wgpu::Render> render;
            std::shared_ptr<tl::wgpu::OffscreenBuffer> buffer;
            std::shared_ptr<tl::wgpu::OffscreenBuffer> stereoBuffer;
            std::shared_ptr<tl::wgpu::OffscreenBuffer> annotation;
            std::shared_ptr<tl::image::Image> annotationImage;

            WGPURenderPipeline pipeline = nullptr;
            WGPURenderPipeline annotation_pipeline = nullptr;

            std::shared_ptr<wgpu::Shader> shader;
            std::shared_ptr<wgpu::ShaderBindingSet> bindingSet;

            std::shared_ptr<wgpu::Shader> annotationShader;

            std::shared_ptr<wgpu::VBO> vbo;
            std::shared_ptr<wgpu::VAO> vao;

            std::shared_ptr<webgpu::Lines> lines;

            bool init_debug = false;
        };

    }

//! Define a variable, "wgpu", that references the private implementation.
#define MRV2_WGPU() auto& wgpu = *_wgpu

} // namespace mrv
