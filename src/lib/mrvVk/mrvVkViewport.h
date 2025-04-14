// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <tlVk/OffscreenBuffer.h>

#include <mrvDraw/Shape.h>

#include "mrvVk/mrvVkDefines.h"
#include "mrvVk/mrvTimelineViewport.h"

namespace mrv
{

    //
    // This class implements a viewport using OpenGL
    //
    class Viewport : public TimelineViewport
    {
        TLRENDER_NON_COPYABLE(Viewport);

    public:
        Viewport(int X, int Y, int W, int H, const char* L = 0);
        ~Viewport();
        
        //! Virual draw method
        void draw() override;

        //! Virtual handle event method
        int handle(int event) override;

        //! Set the internal system context for the widget.
        void setContext(const std::weak_ptr<system::Context>& context);

        //! Refresh window by clearing the associated resources.
        void refresh() override;

    protected:
        void _initializeGL();
        void _initializeGLResources();

        void _createPBOs(const math::Size2i& renderSize);
        void _createOverlayPBO(const math::Size2i& renderSize);
        
        void _createCubicEnvironmentMap();

        void _createSphericalEnvironmentMap();

        math::Matrix4x4f _createEnvironmentMap();

        math::Matrix4x4f _createTexturedRectangle();

        void _calculateColorArea(mrv::area::Info& info);

        void _drawAnaglyph(int, int) const noexcept;

        void _drawCheckerboard(int, int) const noexcept;

        void _drawColumns(int, int) const noexcept;

        void _drawScanlines(int, int) const noexcept;

        void _drawStereoOpenGL() noexcept;

        void _drawStereo3D() const noexcept;

        void _drawDataWindow() const noexcept;

        void _drawDisplayWindow() const noexcept;

        void _drawMissingFrame(const math::Size2i& renderSize) const noexcept;

        //! Crop mask, data window and display window
        void _drawOverlays(const math::Size2i& renderSize) const noexcept;

        void _drawCropMask(const math::Size2i& renderSize) const noexcept;

        void _drawHUD(float alpha) const noexcept;

        void _drawCursor(const math::Matrix4x4f& mvp) const noexcept;

        void _drawAnnotations(
            const std::shared_ptr<tl::vk::OffscreenBuffer>& overlay,
            const math::Matrix4x4f& renderMVP,
            const otime::RationalTime& time,
            const std::vector<std::shared_ptr<draw::Annotation>>& annotations,
            const math::Size2i& renderSize);

#ifdef USE_OPENGL2
        void _drawGL1TextShapes(const tl::math::Matrix4x4f&,
                                const double viewZoom);
#endif

        void _pushAnnotationShape(const std::string& cmd) const override;

        void _readPixel(image::Color4f& rgba) const noexcept override;

        void _drawHelpText() const noexcept;

        void _drawRectangleOutline(
            const math::Box2i& box, const image::Color4f& color,
            const math::Matrix4x4f& mvp) const noexcept;
        void _drawText(
            const std::vector<std::shared_ptr<image::Glyph> >&, math::Vector2i&,
            const int16_t lineHeight, const image::Color4f&) const noexcept;
        void _drawSafeAreas() const noexcept;
        void _drawSafeAreas(
            const float percentX, const float percentY,
            const float pixelAspectRatio, const image::Color4f& color,
            const math::Matrix4x4f& mvp, const char* label = "") const noexcept;

        void _mapBuffer() const noexcept;
        void _unmapBuffer() const noexcept;

        void _drawShape(
            const std::shared_ptr< draw::Shape >& shape,
            const float alphamult = 1.F) noexcept;

        void _calculateColorAreaFullValues(area::Info& info) noexcept;

        void _drawWindowArea(const std::string&) const noexcept;

        void _compositeAnnotations(
            const math::Matrix4x4f& shaderMatrix,
            const math::Size2i& viewportSize);
        
        void _compositeAnnotations(
            const std::shared_ptr<tl::vk::OffscreenBuffer>&,
            const math::Matrix4x4f& orthoMatrix,
            const math::Size2i& viewportSize);
        
        void _compositeOverlay(const std::shared_ptr<tl::vk::OffscreenBuffer>&,
                               const math::Matrix4x4f& identity,
                               const math::Size2i& viewportSize);
        
    private:
        struct VKPrivate;
        std::unique_ptr<VKPrivate> _gl;
    };
} // namespace mrv
