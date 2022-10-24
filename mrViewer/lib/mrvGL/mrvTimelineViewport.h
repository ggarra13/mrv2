#pragma once

#include <tlTimeline/IRender.h>

// FLTK includes
#ifdef USE_METAL
#  include <FL/Fl_Metal_Window.H>
#  define Fl_SuperClass Fl_Metal_Window
#else
#  include <FL/Fl_Gl_Window.H>
#  define Fl_SuperClass Fl_Gl_Window
#endif


class ViewerUI;

namespace mrv
{
    using namespace tl;

    enum PixelDisplay
    {
        kRGBA_Float,
        kRGBA_Hex,
        kRGBA_Decimal
    };

    enum PixelValue
    {
        kFull,
        kOriginal
    };

    enum HudDisplay
    {
        kNone          = 0,
        kFilename      = 1 << 0,
        kDirectory     = 1 << 1,
        kFrame         = 1 << 2,
        kFrameRange    = 1 << 3,
        kFrameCount    = 1 << 4,
        kResolution    = 1 << 5,
        kFPS           = 1 << 6,
        kAttributes    = 1 << 7,
        kTimecode      = 1 << 8
    };

    class TimelinePlayer;

    class TimelineViewport : public Fl_SuperClass
    {
        TLRENDER_NON_COPYABLE(TimelineViewport);

    public:
        TimelineViewport( int X, int Y, int W, int H, const char* L );
        ~TimelineViewport();

        //! Virtual FLTK methods
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

        //! Get the image options.
        timeline::ImageOptions& getImageOptions( int idx ) noexcept;

        //! Get the image options.
        std::vector< timeline::ImageOptions>& getImageOptions() noexcept;

        //! Set the display options.
        void setDisplayOptions(const std::vector<timeline::DisplayOptions>&);

        //! Get the display options.
        timeline::DisplayOptions& getDisplayOptions( int idx ) noexcept;

        //! Get the display options.
        std::vector< timeline::DisplayOptions >& getDisplayOptions() noexcept;

        //! Set the comparison options.
        void setCompareOptions(const timeline::CompareOptions&);
        
        //! Set the timeline players.
        void setTimelinePlayers(const std::vector<TimelinePlayer*>&);

        //! Get one of the timeline players.  Index is not checked.
        mrv::TimelinePlayer* getTimelinePlayer(int idx = 0) const;

        //! Return all timeline playrers associatied to this view.
        std::vector<mrv::TimelinePlayer*>& getTimelinePlayers() const;

        //! Return the crop mask
        float getMask() const;

        //! Set the crop mask
        void setMask( float f );

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
        void resizeWindow() noexcept;

        //! Frame the view.
        void frameView();

        //! Center the view without changing the zoom.
        void centerView() noexcept;

        //! Set the view zoom to 1:1.
        void viewZoom1To1();

        //! Playback controls
        void startFrame();

        void framePrev();

        void playBackwards();

        void stop();

        void frameNext();

        void playForwards();

        void togglePlayback();

        void endFrame();

        //Q_SLOTS
        void videoCallback(const tl::timeline::VideoData&,
                           const TimelinePlayer* sender ) noexcept;

        //! Set the color configuration from the GUI.
        void updateColorConfigOptions() noexcept;

        //! Updatee the image options from the GUI.
        void updateImageOptions( int idx = -1 ) noexcept;

        //! Update the display options from the GUI.
        void updateDisplayOptions( int idx = -1 ) noexcept;

        //! Update the video layer from the GUI.
        void updateVideoLayers( int idx = 0 ) noexcept;

        //Q_SIGNALS:
        //! This signal is emitted when the position and zoom change.
        void viewPosAndZoomChanged(const tl::math::Vector2i&, float);

        //! This signal is emitted when the view is framed.
        void frameViewActivated();

        //! Toggle a display channel between it and the color channel
        //! in a timeline or in all timelines if idx = -1.
        void toggleDisplayChannel( const timeline::Channels &,
                                   int idx = -1 ) noexcept;

        //! Set or unset the window to full screen and hide/show all bars
        void setPresentationMode( bool active = true );

        //! Set or unset the window to full screen but don't hide any bars
        void setFullScreenMode( bool active = true );

        //! Handle a drag and drop of files to load
        void dragAndDrop( const std::string& text );

        void updatePixelBar() noexcept;

    protected:
        virtual void _readPixel( imaging::Color4f& rgba ) const noexcept = 0;
        imaging::Size _getRenderSize() const noexcept;
        imaging::Size _getViewportSize() const noexcept;
        std::vector<imaging::Size> _getTimelineSizes() const noexcept;
        math::Vector2i _getViewportCenter() const noexcept;
        math::Vector2i _getFocus( int X, int Y ) const noexcept;
        math::Vector2i _getFocus() const noexcept;
        //! Call redraw and a flush to force a redraw
        void _refresh() noexcept;
        void _updateCoords() const noexcept;
        void _frameView() noexcept;

        void
        _updateDisplayOptions( int idx,
                               const timeline::DisplayOptions& d ) noexcept;
        void _updateImageOptions( int idx,
                                  const timeline::ImageOptions& d ) noexcept;
        TLRENDER_PRIVATE();
    };
}
