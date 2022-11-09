#pragma once


// mrViewer includes
#include <mrvGL/mrvTimelineViewport.h>

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

        //! HUD controls
        bool getHudActive() const;
        void setHudActive( const bool active );
        void setHudDisplay( const HudDisplay value );

        HudDisplay getHudDisplay() const noexcept;

    protected:
        void initializeGL();

        void _drawCropMask( const imaging::Size& renderSize );

        void _drawHUD();

        virtual
        void _readPixel( imaging::Color4f& rgba ) const noexcept override;

        //! Get a pixel value from an image (the raw data)
        void _getPixelValue( imaging::Color4f& rgba,
                             const std::shared_ptr<imaging::Image>& image,
                             const math::Vector2i& pos ) const;

    private:
        void _drawText( const std::vector<std::shared_ptr<imaging::Glyph> >&,
                        math::Vector2i&,
                        const int16_t lineHeight,
                        const imaging::Color4f&);

    private:
        struct GLPrivate;
        std::unique_ptr<GLPrivate> _gl;
    };
}
