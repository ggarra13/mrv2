// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTimeline/BackgroundOptions.h>
#include <tlTimeline/CompareOptions.h>
#include <tlTimeline/DisplayOptions.h>
#include <tlTimeline/HDROptions.h>
#include <tlTimeline/LUTOptions.h>
#include <tlTimeline/OCIOOptions.h>
#include <tlTimeline/RenderOptions.h>
#include <tlTimeline/Video.h>

#include <tlCore/Context.h>
#include <tlCore/FontSystem.h>
#include <tlCore/Mesh.h>
#include <tlCore/Size.h>

namespace tl
{
    namespace timeline
    {
        //! Text information.
        struct TextInfo
        {
            TextInfo(const geom::TriangleMesh2& mesh, unsigned id) :
                mesh(mesh),
                textureId(id)
                {
                };
            
            geom::TriangleMesh2 mesh;
            uint8_t textureId;
        };
        
        //! Base class for renderers.
        class IRender : public std::enable_shared_from_this<IRender>
        {
        protected:
            void _init(const std::shared_ptr<system::Context>&);

            IRender();

        public:
            virtual ~IRender() = 0;

            //! Start a render.
            virtual void begin(
                const math::Size2i&,
                const RenderOptions& = RenderOptions()) = 0;

            //! Finish a render.
            virtual void end() = 0;

            //! Get the render size.
            virtual math::Size2i getRenderSize() const = 0;

            //! Set the render size.
            virtual void setRenderSize(const math::Size2i&) = 0;

            //! Get the viewport.
            virtual math::Box2i getViewport() const = 0;

            //! Set the viewport.
            virtual void setViewport(const math::Box2i&) = 0;

            //! Clear the viewport.
            virtual void clearViewport(const image::Color4f&) = 0;

            //! Get whether the clipping rectangle is enabled.
            virtual bool getClipRectEnabled() const = 0;

            //! Set whether the clipping rectangle is enabled.
            virtual void setClipRectEnabled(bool) = 0;

            //! Get the clipping rectangle.
            virtual math::Box2i getClipRect() const = 0;

            //! Set the clipping rectangle.
            virtual void setClipRect(const math::Box2i&) = 0;

            //! Get the transformation matrix.
            virtual math::Matrix4x4f getTransform() const = 0;

            //! Set the transformation matrix.
            virtual void setTransform(const math::Matrix4x4f&) = 0;

            //! Set the OpenColorIO options.
            virtual void setOCIOOptions(const OCIOOptions&) = 0;

            //! Set the LUT options.
            virtual void setLUTOptions(const LUTOptions&) = 0;

            //! Set the HDR options.
            virtual void setHDROptions(const HDROptions&) = 0;

            //! Draw a rectangle.
            virtual void
            drawRect(const math::Box2i&, const image::Color4f&,
                     const std::string& pipelineName = "",
                     const bool enableBlending = true) = 0;

            //! Draw a triangle mesh.
            virtual void drawMesh(
                const geom::TriangleMesh2&, const math::Vector2i& position,
                const image::Color4f&, const std::string& meshName = "") = 0;

            //! Draw a triangle mesh with vertex color information.
            virtual void drawColorMesh(
                const geom::TriangleMesh2&, const math::Vector2i& position,
                const image::Color4f&) = 0;
           
            //! Create text meshes to speed up drawing
            virtual void appendText(
                std::vector<TextInfo>& textInfos,
                const std::vector<std::shared_ptr<image::Glyph> >& glyphs,
                const math::Vector2i& position) = 0;
            
            //! Draw a text mesh after having been created with appendText.
            virtual void drawText(
                const TextInfo& info,
                const math::Vector2i& position,
                const image::Color4f&, const std::string& meshName = "") = 0;

            //! Draw a texture.
            virtual void drawTexture(
                unsigned int, const math::Box2i&,
                const image::Color4f& = image::Color4f(1.F, 1.F, 1.F)) = 0;

            //! Draw an image.
            virtual void drawImage(
                const std::shared_ptr<image::Image>&, const math::Box2i&,
                const image::Color4f& = image::Color4f(1.F, 1.F, 1.F),
                const ImageOptions& = ImageOptions()) = 0;

            //! Draw timeline video data.
            virtual void drawVideo(
                const std::vector<timeline::VideoData>&,
                const std::vector<math::Box2i>&,
                const std::vector<ImageOptions>& = {},
                const std::vector<DisplayOptions>& = {},
                const CompareOptions& = CompareOptions(),
                const BackgroundOptions& = BackgroundOptions()) = 0;

        protected:
            std::weak_ptr<system::Context> _context;
        };
    } // namespace timeline
} // namespace tl
