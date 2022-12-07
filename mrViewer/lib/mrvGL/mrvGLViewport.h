// SPDX-License-Identifier: BSD-3-Clause
// mrv2 (mrViewer2)
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <tlGL/OffscreenBuffer.h>

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
        Viewport( int X, int Y, int W, int H, const char* L = 0 );
        ~Viewport();

        //! Virual draw method
        virtual void draw() override;
        virtual int handle( int event ) override;

        //! Set the internal system context for the widget.
        void setContext(
            const std::weak_ptr<system::Context>& context);


    protected:
        void _initializeGL();

        void _calculateColorArea( mrv::area::Info& info );

        void _drawCropMask( const imaging::Size& renderSize ) const noexcept;

        void _drawHUD() const noexcept;

        void _drawCursor(const math::Matrix4x4f& mvp) const noexcept;

        void _drawAnnotations(math::Matrix4x4f& mvp);

#ifdef USE_OPENGL2
        void _drawAnnotationsGL2();
#endif

        virtual
        void _readPixel( imaging::Color4f& rgba ) const noexcept override;

        void
        _drawRectangleOutline( const math::BBox2i& box,
                               const imaging::Color4f& color,
                               const math::Matrix4x4f& mvp ) const noexcept;
        void _drawText( const std::vector<std::shared_ptr<imaging::Glyph> >&,
                        math::Vector2i&,
                        const int16_t lineHeight,
                        const imaging::Color4f&) const noexcept;
        void _drawSafeAreas() const noexcept;
        void _drawSafeAreas(
            const float percentX, const float percentY,
            const float pixelAspectRatio,
            const imaging::Color4f& color,
            const math::Matrix4x4f& mvp,
            const char* label = "" ) const noexcept;

        void _mapBuffer() const noexcept;
        void _unmapBuffer() const noexcept;

        void _calculateColorAreaFullValues( area::Info& info ) noexcept;
        
    private:
        struct GLPrivate;
        std::unique_ptr<GLPrivate> _gl;
    };
}
