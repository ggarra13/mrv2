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

        enum HudDisplay {
            kHudNone          = 0,
            kHudFilename      = 1 << 0,
            kHudDirectory     = 1 << 1,
            kHudFrame         = 1 << 2,
            kHudFrameRange    = 1 << 3,
            kHudFrameCount    = 1 << 4,
            kHudResolution    = 1 << 5,
            kHudFPS           = 1 << 6,
            kHudAttributes    = 1 << 7,
            kHudAVDifference  = 1 << 8,
            kHudTimecode      = 1 << 9,
            kHudWipe          = 1 << 10,
            kHudMemoryUse     = 1 << 11,
            kHudCenter        = 1 << 12,
        };


    public:
        GLViewport( int X, int Y, int W, int H, const char* L = 0 );

        ~GLViewport();

        //! Virual draw method
        virtual void draw() override;

        void setContext(
            const std::weak_ptr<system::Context>& context);


    protected:
        void initializeGL();
        void _drawHUD();
        void _readPixel( imaging::Color4f& rgba ) const noexcept override;


    private:
        struct GLPrivate;
        std::unique_ptr<GLPrivate> _gl;
    };
}
