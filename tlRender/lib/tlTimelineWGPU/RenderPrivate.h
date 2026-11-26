// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// Copyright (c) 2025-Present Gonzalo Garramuño
// All rights reserved.

#pragma once

// #include <tlTimelineWGPU/HDRPeakDetection.h>
#include <tlTimelineWGPU/RenderOptions.h>
#include <tlTimelineWGPU/Render.h>

#include <tlWGPU/Mesh.h>
#include <tlWGPU/OffscreenBuffer.h>
#include <tlWGPU/PipelineCreationState.h>
#include <tlWGPU/Shader.h>
#include <tlWGPU/Texture.h>
#include <tlWGPU/TextureAtlas.h>
#include <tlWGPU/VAOPool.h>

#include <tlCore/StatsSystem.h>

#if defined(TLRENDER_OCIO)
#    include <OpenColorIO/OpenColorIO.h>
#endif // TLRENDER_OCIO

#include <array>
#include <list>
#include <map>
#include <string>

#if defined(TLRENDER_OCIO)
namespace OCIO = OCIO_NAMESPACE;
#endif // TLRENDER_OCIO

namespace tl
{
    namespace timeline_wgpu
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


        std::string vertexSource();
        std::string vertex2Source();
        std::string vertex2NoUVsSource();

        std::string meshFragmentSource();
        std::string colorMeshVertexSource();
        std::string colorMeshFragmentSource();
        std::string textFragmentSource();
        std::string textureFragmentSource();
        std::string differenceFragmentSource();
        std::string multiplyFragmentSource();
        std::string addFragmentSource();
        std::string butterflyFragmentSource();
        std::string debandingFragmentSource(const float threshold = 48.F,
                                            const float range = 16.F,
                                            const int iterations = 1,
                                            const float grain = 32.F);

        // For HDR peak detection
        std::string computeHDRPeakDetection();

        // For pixel conversions
        std::string computeRGB_F16_To_RGBA_F16();
        std::string computeRGB_F32_To_RGBA_F32();

        // For annotations
        std::string softFragmentSource();
        std::string hardFragmentSource();

        // For display
        std::string imageFragmentSource();
        std::string displayVertexSource();
        std::string displayFragmentSource(
            const std::string& ocioICSDef, const std::string& ocioICS,
            const std::string& ocioDef, const std::string& ocio,
            const std::string& lutDef, const std::string& lut,
            timeline::LUTOrder, const std::string& toneMapDef,
            const std::string& toneMap,
            const std::string& debandingDef = "",
            const std::string& debanding = "");

        std::vector<std::shared_ptr<wgpu::Texture> > getTextures(
            Fl_WebGPU_Context&, const image::Info&,
            const timeline::ImageFilters&,
            size_t offset = 0);

        void setActiveTextures(
            const image::Info& info,
            const std::vector<std::shared_ptr<wgpu::Texture> >&,
            size_t offset = 0);

#if defined(TLRENDER_OCIO)
        struct OCIOData
        {
            ~OCIOData();

            OCIO::ConstConfigRcPtr config;
            OCIO::DisplayViewTransformRcPtr transform;
            OCIO::LegacyViewingPipelineRcPtr lvp;
            OCIO::ConstProcessorRcPtr processor;
            OCIO::ConstGPUProcessorRcPtr gpuProcessor;
            OCIO::GpuShaderDescRcPtr icsDesc;
            OCIO::GpuShaderDescRcPtr shaderDesc;
            std::vector<std::shared_ptr<wgpu::Texture> > textures;
        };

        struct OCIOLUTData
        {
            ~OCIOLUTData();

            OCIO::ConstConfigRcPtr config;
            OCIO::FileTransformRcPtr transform;
            OCIO::ConstProcessorRcPtr processor;
            OCIO::ConstGPUProcessorRcPtr gpuProcessor;
            OCIO::GpuShaderDescRcPtr shaderDesc;
            std::vector<std::shared_ptr<wgpu::Texture> > textures;
        };

#endif // TLRENDER_OCIO

#if defined(TLRENDER_LIBPLACEBO)
        struct LibPlaceboData
        {
            LibPlaceboData(Fl_WebGPU_Context& ctx,
                           const bool peak_detection = false);
            ~LibPlaceboData();

            // Libplacebo variables
            pl_log log;
            pl_gpu gpu;
            pl_shader shader;
            pl_shader_obj state = nullptr;
            const pl_shader_res* res = nullptr;

            std::vector<pl_shader_var> pcUBOvars;
            void* pcUBOData = nullptr;
            size_t pcUBOSize = 0;

            // Peak detection variables
            float maxPeak = 0;
            float avgPeak = 0;


            // Vulkan variables
            Fl_WebGPU_Context& ctx;
            // std::vector<WGPUFence> ssboFences;
            // std::vector<WGPUCommandBuffer> ssboCmds;
            // WGPUCommandPool ssboCmdPool = VK_NULL_HANDLE;

            std::vector<std::shared_ptr<wgpu::Texture> > textures;
        };
#endif

        struct Render::Private
        {
            // WebGPU variables
            WGPUCommandEncoder cmd;
            WGPURenderPassEncoder renderPass;
            std::shared_ptr<wgpu::OffscreenBuffer> fbo;
            uint64_t frameIndex = 0;

            math::Size2i renderSize;
            timeline::OCIOOptions ocioOptions;
            timeline::LUTOptions lutOptions;
            timeline::HDROptions hdrOptions;
            timeline::ShaderOptions shaderOptions;
            unsigned bindingIndex = 7;
            timeline::RenderOptions renderOptions;


            struct ImageSlot
            {
                std::vector<std::shared_ptr<wgpu::Texture> > textures;
                std::shared_ptr<image::Image> last;
            };
            std::map<std::string, ImageSlot> imageSlots;
            std::map<const void*, int> imageOrdinal;

            //! Variable to cache source code to avoid recreation of shader
            //! when possible
            std::string oldSource;

#if defined(TLRENDER_OCIO)
            std::unique_ptr<OCIOData> ocioData;
            std::unique_ptr<OCIOLUTData> lutData;
#endif // TLRENDER_OCIO

#if defined(TLRENDER_LIBPLACEBO)
            std::unique_ptr<LibPlaceboData> placeboData;
#endif

            math::Box2i viewport;
            math::Matrix4x4f transform;
            bool clipRectEnabled = false;
            math::Box2i clipRect;
            std::string currentPipeline;

            //! Pipelines are cached by name AND by this group, so a single
            //! renderer can draw to targets with different render passes
            //! (video, annotations, overlay) without recreating pipelines
            //! every time the target changes.  Empty for the main target.
            std::string pipelineGroup;

            //! State of the outer pass, saved by beginPass().
            struct PassState
            {
                WGPUCommandEncoder cmd = nullptr;
                std::shared_ptr<wgpu::OffscreenBuffer> fbo;
                WGPURenderPassEncoder renderPass = nullptr;
                math::Size2i renderSize;
                math::Matrix4x4f transform;
                math::Box2i viewport;
                bool clipRectEnabled = false;
                math::Box2i clipRect;
                std::string pipelineGroup;
                std::string currentPipeline;
                timeline::RenderOptions renderOptions;
            };
            std::vector<PassState> passStack;

            std::unordered_map<std::string, std::shared_ptr<wgpu::Shader> > shaders;  // Vertex / Fragment
            std::unordered_map<std::string, std::shared_ptr<wgpu::Shader> > compute;  // Compute
            std::unordered_map<std::string, std::shared_ptr<wgpu::OffscreenBuffer> >
                buffers;
            std::unordered_map<std::string, WGPUPipelineLayout> pipelineLayouts;
            std::unordered_map<
                std::string, std::pair<wgpu::PipelineCreationState, WGPURenderPipeline>>
                pipelines;
            std::shared_ptr<TextureCache> textureCache;
            std::shared_ptr<wgpu::TextureAtlas> glyphTextureAtlas;
            std::map<image::GlyphInfo, wgpu::TextureAtlasID> glyphIDs;
            std::unordered_map<std::string, std::shared_ptr<wgpu::VBO> > vbos;
            std::shared_ptr<wgpu::VAOPool> vaoPool;

            //! Flag for using display shader on SDR.
            bool sdrOutput = false;

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
            std::chrono::steady_clock::time_point logTimer;
            std::shared_ptr<system::StatsSystem> statsSystem;

            void createTextMesh(Fl_WebGPU_Context& ctx, const geom::TriangleMesh2&);
        };
    } // namespace timeline_wgpu
} // namespace tl
