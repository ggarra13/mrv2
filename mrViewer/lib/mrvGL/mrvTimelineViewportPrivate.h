#pragma once

class ViewerUI;
class Fl_Menu_Button;

namespace mrv
{

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
        bool frameView = false;
        int  event_x, event_y;
        math::Vector2i mousePos;
        math::Vector2i mousePress;
        math::Vector2i viewPosMousePress;

        //! Main ui pointer
        ViewerUI* ui = nullptr;

        //! Video frame and data
        std::vector<tl::timeline::VideoData> videoData;

        std::shared_ptr<imaging::FontSystem> fontSystem;
        std::unique_ptr<Fl_Menu_Button>      popupMenu;

        //! HUD display flags (ORed together)
        bool       hudActive = true;
        HudDisplay hud = HudDisplay::kNone;

        //! Masking
        float      masking = 0.F;

        bool pixelBar     = false;
        bool fullScreen   = false;
        bool presentation = false;
    };

}
