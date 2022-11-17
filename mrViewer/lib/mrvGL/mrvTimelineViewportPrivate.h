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


        //! Show annotations
        bool showAnnotations = true;
        
        //! List of annotations ( drawings per frame )
        draw::AnnotationList annotations;

        //! Last annotation undones
        std::shared_ptr< draw::Annotation > undoAnnotation = nullptr;

        //! Annotations ghosting
        int ghostNext = 5;
        int ghostPrevious = 5;

        //! Main ui pointer
        ViewerUI* ui = nullptr;

        //! Video frame and data
        std::vector<tl::timeline::VideoData> videoData;

        std::shared_ptr<imaging::FontSystem> fontSystem;
        std::unique_ptr<Fl_Menu_Button>      popupMenu;

        //! HUD display flags (ORed together)
        bool       hudActive = true;
        HudDisplay hud = HudDisplay::kNone;

        //! Action Mode
        ActionMode                    actionMode = ActionMode::kScrub;

        //! Rectangle selection ( Color area )
        math::BBox2i                   selection;

        //! Color area information
        area::Info                 colorAreaInfo;
        
        //! Masking
        float      masking = 0.F;

        //bool pixelBar     = true;
        bool fullScreen   = false;
        bool presentation = false;
    };

}
