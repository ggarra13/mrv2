#pragma once

#include <tlTimeline/IRender.h>

// FLTK includes
#include <FL/Fl_Gl_Window.H>

class ViewerUI;

namespace mrv
{
    using namespace tl;

    class TimelinePlayer;

    class TimelineViewport : public Fl_Gl_Window
    {
        TLRENDER_NON_COPYABLE(TimelineViewport);

    public:
        TimelineViewport( int X, int Y, int W, int H, const char* L );
        ~TimelineViewport();

        //! Virtual handle method
        virtual int handle( int event ) override;
        virtual void resize( int X, int Y, int W, int H ) override;

        //! Store main ui
        void main( ViewerUI* m );

        //! Handle scrubbing
        void scrub();

        //! Set the color configuration.
        void setColorConfigOptions(const timeline::ColorConfigOptions&);

        //! Set the LUT options.
        void setLUTOptions(const timeline::LUTOptions&);

        //! Set the image options.
        void setImageOptions(const std::vector<timeline::ImageOptions>&);

        //! Set the display options.
        void setDisplayOptions(const std::vector<timeline::DisplayOptions>&);

        //! Set the comparison options.
        void setCompareOptions(const timeline::CompareOptions&);

        //! Set the timeline players.
        void setTimelinePlayers(const std::vector<TimelinePlayer*>&);

        TimelinePlayer* getTimelinePlayer(const int index = 0) const;

        //! Get the view position.
        const math::Vector2i& viewPos() const;

        //! Get the view zoom.
        float viewZoom() const;

        //! Get whether the view is framed.
        bool hasFrameView() const;

        //! Set the view position and zoom.
        void setViewPosAndZoom(const tl::math::Vector2i&, float);

        //! Set the view zoom.
        void setViewZoom(float,
                         const math::Vector2i& focus = math::Vector2i());

        //! Resize the window to screen
        void resizeWindow();

        //! Frame the view.
        void frameView();

        //! Set the view zoom to 1:1.
        void viewZoom1To1();

        //! Playback controls
        void start();

        void framePrev();

        void playBackwards();

        void stop();

        void frameNext();

        void playForwards();

        void end();

        //Q_SLOTS
        void videoCallback(const tl::timeline::VideoData&,
                           const TimelinePlayer* sender );

        //! Set the color configuration.
        void updateColorConfigOptions();

        //! Set the image options.
        void updateImageOptions( int idx = -1 );

        //! Set the display options.
        void updateDisplayOptions( int idx = -1 );

        //Q_SIGNALS:
        //! This signal is emitted when the position and zoom change.
        void viewPosAndZoomChanged(const tl::math::Vector2i&, float);

        //! This signal is emitted when the view is framed.
        void frameViewActivated();

    protected:
        imaging::Size _getRenderSize() const;
        imaging::Size _getViewportSize() const;
        std::vector<imaging::Size> _getTimelineSizes() const;
        math::Vector2i _getViewportCenter() const;
        math::Vector2i _getFocus( int X, int Y ) const;
        math::Vector2i _getFocus() const;
        void _updateCoords() const;
        void _frameView();
        void _mouseMove();

        TLRENDER_PRIVATE(); //!<- protected really
    };
}
