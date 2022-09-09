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
        timeline::ColorConfigOptions colorConfigOptions;
        timeline::LUTOptions lutOptions;
        std::vector<tl::timeline::ImageOptions> imageOptions;
        std::vector<tl::timeline::DisplayOptions> displayOptions;
        timeline::CompareOptions compareOptions;

        std::vector<TimelinePlayer*> timelinePlayers;

        math::Vector2i viewPos;
        float viewZoom = 1.F;
        bool frameView = true;
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
