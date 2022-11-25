#pragma once

#include <mrvGL/mrvGLDefines.h>
#include "mrvGL/mrvTimelineViewport.h"

namespace mrv
{


    //
    // This class implements a viewport using OpenGL
    //
    class GLViewport : public TimelineViewport
    {
        TLRENDER_NON_COPYABLE(GLViewport);

    public:
        GLViewport( int X, int Y, int W, int H, const char* L = 0 );
        GLViewport( int W, int H, const char* L = 0 );

        ~GLViewport();

        //! Virual draw method
        virtual void draw() override;

        void setContext(
            const std::weak_ptr<system::Context>& context);

        const imaging::Color4f* image() const;

        //! HUD controls
        bool getHudActive() const;
        void setHudActive( const bool active );
        void setHudDisplay( const HudDisplay value );

        HudDisplay getHudDisplay() const noexcept;

        void calculateColorAreaInfo( const math::BBox2i& box, mrv::area::Info& info );

    protected:
        void initializeGL();

        void _drawCropMask( const imaging::Size& renderSize );

        void _drawHUD();

        void _drawAnnotations(math::Matrix4x4f& mvp);

#ifdef USE_OPENGL2
        void _drawAnnotationsGL2();
#endif
        
        virtual
        void _readPixel( imaging::Color4f& rgba ) const noexcept override;

        //! Get a pixel value from an image (the raw data)
        void _getPixelValue( imaging::Color4f& rgba,
                             const std::shared_ptr<imaging::Image>& image,
                             const math::Vector2i& pos ) const;
        
        void
        _drawRectangleOutline( const math::BBox2i& box,
                               const imaging::Color4f& color,
                               const math::Matrix4x4f& mvp ) const noexcept;
        void _drawText( const std::vector<std::shared_ptr<imaging::Glyph> >&,
                        math::Vector2i&,
                        const int16_t lineHeight,
                        const imaging::Color4f&);
        void _drawSafeAreas() const noexcept;
        void _drawSafeAreas(
            const float percentX, const float percentY,
            const float pixelAspectRatio,
            const imaging::Color4f& color,
            const math::Matrix4x4f& mvp,
            const char* label = "" ) const noexcept;

    private:
        struct GLPrivate;
        std::unique_ptr<GLPrivate> _gl;
    };
}
