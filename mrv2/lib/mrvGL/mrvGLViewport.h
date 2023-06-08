// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <tlGL/OffscreenBuffer.h>

#include <mrvDraw/Shape.h>

#include "mrvGL/mrvGLDefines.h"
#include "mrvGL/mrvTimelineViewport.h"

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
        int handle(int event) override;

        //! Set the internal system context for the widget.
        void setContext(const std::weak_ptr<system::Context>& context);

        //! Refresh window by clearing the associated resources.
        virtual void refresh() override;

    protected:
        void _initializeGL();

        void _createCubicEnvironmentMap();

        void _createSphericalEnvironmentMap();

        math::Matrix4x4f _createEnvironmentMap();

        math::Matrix4x4f _createTexturedRectangle();

        void _calculateColorArea(mrv::area::Info& info);

        void _drawAnaglyph(int, int) const noexcept;

        void _drawCheckerboard(int, int) const noexcept;

        void _drawColumns(int, int) const noexcept;

        void _drawScanlines(int, int) const noexcept;

        void _drawStereoOpenGL() const noexcept;

        void _drawStereo3D() const noexcept;

        void _drawMissingFrame(const imaging::Size& renderSize) const noexcept;

        void _drawCropMask(const imaging::Size& renderSize) const noexcept;

        void _drawHUD() const noexcept;

        void _drawCursor(const math::Matrix4x4f& mvp) const noexcept;

        void _drawAnnotations(const math::Matrix4x4f& mvp);

#ifdef USE_OPENGL2
        void _drawGL2TextShapes();
#endif

        void _pushAnnotationShape(const std::string& cmd) const override;

        void _readPixel(imaging::Color4f& rgba) const noexcept override;

        void _drawHelpText();
        void _drawRectangleOutline(
            const math::BBox2i& box, const imaging::Color4f& color,
            const math::Matrix4x4f& mvp) const noexcept;
        void _drawText(
            const std::vector<std::shared_ptr<imaging::Glyph> >&,
            math::Vector2i&, const int16_t lineHeight,
            const imaging::Color4f&) const noexcept;
        void _drawSafeAreas() const noexcept;
        void _drawSafeAreas(
            const float percentX, const float percentY,
            const float pixelAspectRatio, const imaging::Color4f& color,
            const math::Matrix4x4f& mvp, const char* label = "") const noexcept;

        void _mapBuffer() const noexcept;
        void _unmapBuffer() const noexcept;

        void _drawShape(
            const std::shared_ptr< tl::draw::Shape >& shape,
            const float alphamult = 1.F) noexcept;

        void _calculateColorAreaFullValues(area::Info& info) noexcept;

    private:
        struct GLPrivate;
        std::unique_ptr<GLPrivate> _gl;
    };
} // namespace mrv
