// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#if defined(TLRENDER_LIBPLACEBO)
extern "C"
{
#    include <libplacebo/dummy.h>
#    include <libplacebo/shaders/colorspace.h>
}
#endif

#include <tlTimelineGL/Render.h>

#include <tlGL/Mesh.h>
#include <tlGL/OffscreenBuffer.h>
#include <tlGL/Shader.h>
#include <tlGL/TextureAtlas.h>

#if defined(TLRENDER_OCIO)
#    include <OpenColorIO/OpenColorIO.h>
#endif // TLRENDER_OCIO

#include <list>

#if defined(TLRENDER_OCIO)
namespace OCIO = OCIO_NAMESPACE;
#endif // TLRENDER_OCIO

namespace tl
{
    namespace timeline_gl
    {
        std::string vertexSource();
        std::string meshFragmentSource();
        std::string colorMeshVertexSource();
        std::string colorMeshFragmentSource();
        std::string textFragmentSource();
        std::string textureFragmentSource();
        std::string imageFragmentSource();
        std::string displayFragmentSource(
            const std::string& ocioICSDef, const std::string& ocioICS,
            const std::string& ocioDef, const std::string& ocio,
            const std::string& lutDef, const std::string& lut,
            timeline::LUTOrder, const std::string& toneMapDef,
            const std::string& toneMap);
        std::string differenceFragmentSource();

        std::vector<std::shared_ptr<gl::Texture> > getTextures(
            const image::Info&, const timeline::ImageFilters&,
            size_t offset = 0);

        void copyTextures(
            const std::shared_ptr<image::Image>&,
            const std::vector<std::shared_ptr<gl::Texture> >&,
            size_t offset = 0);

        void setActiveTextures(
            const image::Info& info,
            const std::vector<std::shared_ptr<gl::Texture> >&,
            size_t offset = 0);

#if defined(TLRENDER_OCIO) || defined(TLRENDER_LIBPLACEBO)
        struct OCIOTexture
        {
            OCIOTexture(
                unsigned id, std::string name, std::string sampler,
                unsigned type);

            unsigned id = -1;
            std::string name;
            std::string sampler;
            unsigned type = -1;
        };
#endif

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
            std::vector<OCIOTexture> textures;
        };

        struct OCIOLUTData
        {
            ~OCIOLUTData();

            OCIO::ConstConfigRcPtr config;
            OCIO::FileTransformRcPtr transform;
            OCIO::ConstProcessorRcPtr processor;
            OCIO::ConstGPUProcessorRcPtr gpuProcessor;
            OCIO::GpuShaderDescRcPtr shaderDesc;
            std::vector<OCIOTexture> textures;
        };

#endif // TLRENDER_OCIO

#if defined(TLRENDER_LIBPLACEBO)
        struct LibPlaceboData
        {
            LibPlaceboData();
            ~LibPlaceboData();

            pl_log log;
            pl_gpu gpu;
            pl_shader_obj state = nullptr;
            std::vector<OCIOTexture> textures;
        };
#endif

        struct Render::Private
        {
            math::Size2i renderSize;
            timeline::OCIOOptions ocioOptions;
            timeline::LUTOptions lutOptions;
            timeline::HDROptions hdrOptions;
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

            std::map<std::string, std::shared_ptr<gl::Shader> > shaders;
            std::map<std::string, std::shared_ptr<gl::OffscreenBuffer> >
                buffers;
            std::shared_ptr<TextureCache> textureCache;
            std::shared_ptr<gl::TextureAtlas> glyphTextureAtlas;
            std::map<image::GlyphInfo, gl::TextureAtlasID> glyphIDs;
            std::map<std::string, std::shared_ptr<gl::VBO> > vbos;
            std::map<std::string, std::shared_ptr<gl::VAO> > vaos;

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

            void drawTextMesh(const geom::TriangleMesh2&);
        };
    } // namespace timeline_gl
} // namespace tl
