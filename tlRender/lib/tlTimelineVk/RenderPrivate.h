// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#if defined(TLRENDER_LIBPLACEBO)
extern "C"
{
#    include <libplacebo/dummy.h>
#    include <libplacebo/shaders.h>
}
#endif

#include <tlTimelineVk/RenderOptions.h>
#include <tlTimelineVk/Render.h>

#include <tlVk/Mesh.h>
#include <tlVk/OffscreenBuffer.h>
#include <tlVk/PipelineCreationState.h>
#include <tlVk/Shader.h>
#include <tlVk/Texture.h>
#include <tlVk/TextureAtlas.h>

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
    namespace timeline_vlk
    {
        // For drawing
        std::string vertexSource();
        std::string vertex2Source();
        std::string vertex2NoUVsSource();
        std::string meshFragmentSource();
        std::string colorMeshVertexSource();
        std::string colorMeshFragmentSource();
        std::string textFragmentSource();
        std::string textureFragmentSource();
        std::string differenceFragmentSource();

        // For annotations
        std::string softFragmentSource();
        std::string hardFragmentSource();

        // For display
        std::string imageFragmentSource();
        std::string displayFragmentSource(
            const std::string& ocioICSDef, const std::string& ocioICS,
            const std::string& ocioDef, const std::string& ocio,
            const std::string& lutDef, const std::string& lut,
            timeline::LUTOrder, const std::string& toneMapDef,
            const std::string& toneMap);

        std::vector<std::shared_ptr<vlk::Texture> > getTextures(
            Fl_Vk_Context&, const image::Info&, const timeline::ImageFilters&,
            size_t offset = 0);

        void copyTextures(
            const std::shared_ptr<image::Image>&,
            const std::vector<std::shared_ptr<vlk::Texture> >&,
            size_t offset = 0);

        void setActiveTextures(
            const image::Info& info,
            const std::vector<std::shared_ptr<vlk::Texture> >&,
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
            std::vector<std::shared_ptr<vlk::Texture> > textures;
        };

        struct OCIOLUTData
        {
            ~OCIOLUTData();

            OCIO::ConstConfigRcPtr config;
            OCIO::FileTransformRcPtr transform;
            OCIO::ConstProcessorRcPtr processor;
            OCIO::ConstGPUProcessorRcPtr gpuProcessor;
            OCIO::GpuShaderDescRcPtr shaderDesc;
            std::vector<std::shared_ptr<vlk::Texture> > textures;
        };

#endif // TLRENDER_OCIO

#if defined(TLRENDER_LIBPLACEBO)
        struct LibPlaceboData
        {
            LibPlaceboData();
            ~LibPlaceboData();

            pl_log log;
            pl_gpu gpu;
            pl_shader shader;
            const pl_shader_res* res = nullptr;
            std::vector<std::shared_ptr<vlk::Texture> > textures;
        };
#endif

        struct Render::Private
        {
            // Vulkan variables
            VkCommandBuffer cmd;
            std::shared_ptr<vlk::OffscreenBuffer> fbo;
            VkRenderPass renderPass;

            // Current frame in the swapchain
            int32_t frameIndex; // must be an int32_t not an uint32_t.

            bool hdrMonitorFound = false;

            math::Size2i renderSize;
            timeline::OCIOOptions ocioOptions;
            timeline::LUTOptions lutOptions;
            timeline::HDROptions hdrOptions;
            unsigned bindingIndex = 7;
            timeline::RenderOptions renderOptions;

#if defined(TLRENDER_OCIO)
            //! \todo Add a cache for OpenColorIO data.
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

            struct FrameGarbage
            {
                std::vector<VkPipeline> pipelines;
                std::vector<VkPipelineLayout> pipelineLayouts;
                std::vector<std::shared_ptr<vlk::ShaderBindingSet> > bindingSets;
                std::vector<std::shared_ptr<vlk::Shader> > shaders;
            };
            std::array<FrameGarbage, vlk::MAX_FRAMES_IN_FLIGHT> garbage;

            std::map<std::string, std::shared_ptr<vlk::Shader> > shaders;
            std::map<std::string, std::shared_ptr<vlk::OffscreenBuffer> >
                buffers;
            std::map<std::string, VkPipelineLayout> pipelineLayouts;
            std::map<
                std::string, std::pair<vlk::PipelineCreationState, VkPipeline>>
                pipelines;
            std::shared_ptr<TextureCache> textureCache;
            std::shared_ptr<vlk::TextureAtlas> glyphTextureAtlas;
            std::map<image::GlyphInfo, vlk::TextureAtlasID> glyphIDs;
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
            };
            Stats currentStats;
            std::list<Stats> stats;
            std::chrono::steady_clock::time_point logTimer;

            void createTextMesh(Fl_Vk_Context& ctx, const geom::TriangleMesh2&);
        };
    } // namespace timeline_vlk
} // namespace tl
