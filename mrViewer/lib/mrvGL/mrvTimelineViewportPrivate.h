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
        float viewZoom;
        bool frameView = false;
        int  event_x, event_y;
        math::Vector2i mousePos;
        math::Vector2i mousePress;
        math::Vector2i viewPosMousePress;


        //! Show annotations
        bool showAnnotations = true;

        short ghostPrevious = 5;
        short ghostNext     = 5;

        //! Last annotation undones
        std::shared_ptr< draw::Annotation > undoAnnotation = nullptr;

        //! Main ui pointer
        ViewerUI* ui = nullptr;

        //! Video frame and data
        std::vector<tl::timeline::VideoData> videoData;

        std::shared_ptr<imaging::FontSystem> fontSystem;
        Fl_Menu_Button*     popupMenu = nullptr;

        //! HUD display flags (ORed together)
        bool       hudActive = true;
        HudDisplay hud = HudDisplay::kNone;

        //! Action Mode
        static ActionMode                    actionMode;

        //! Rectangle selection ( Color area )
        static math::BBox2i                   selection;

        //! Color area information
        area::Info                 colorAreaInfo;
        
        //! Masking
        float      masking = 0.F;

        bool fullScreen   = false;
        bool presentation = false;
    };

}
