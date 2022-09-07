#pragma once

class ViewerUI;

namespace mrv
{
    enum PixelDisplay
    {
        kRGBA_Float,
        kRGBA_Hex,
        kRGBA_Decimal
    };

    struct TimelineViewport::Private
    {
        std::vector<TimelinePlayer*> timelinePlayers;

        imaging::ColorConfig colorConfig;
        std::vector<tl::timeline::ImageOptions> imageOptions;
        std::vector<tl::timeline::DisplayOptions> displayOptions;
        timeline::CompareOptions compareOptions;

        math::Vector2i viewPos;
        float viewZoom = 1.F;
        bool frameView = true;
        bool mouseInside = false;
        math::Vector2i mousePos;
        math::Vector2i mousePress;
        math::Vector2i viewPosMousePress;

        //! Main ui pointer
        ViewerUI* ui = nullptr;

        //! OpenGL Offscreen buffer
        std::shared_ptr<tl::gl::OffscreenBuffer> buffer = nullptr;

        //! Video frame and data
        std::vector<tl::timeline::VideoData> videoData;
    };

}
