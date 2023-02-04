// SPDX-License-Identifier: BSD-3-Clause
// mrv2 (mrViewer2)
// Copyright Contributors to the mrv2 Project. All rights reserved.


#include <memory>
#include <cmath>
#include <algorithm>

#include "mrViewer.h"

#include "mrvPanels/mrvAnnotationsPanel.h"
#include "mrvPanels/mrvPanelsCallbacks.h"

#include "mrvGL/mrvTimelineViewport.h"
#include "mrvGL/mrvTimelineViewportPrivate.h"

#include "mrvFl/mrvCallbacks.h"
#include "mrvFl/mrvTimelinePlayer.h"

#include "mrvCore/mrvUtil.h"
#include "mrvCore/mrvMath.h"
#include "mrvCore/mrvHotkey.h"
#include "mrvCore/mrvColorSpaces.h"

#include "mrvWidgets/mrvHorSlider.h"
#include "mrvWidgets/mrvMultilineInput.h"

#include "mrvApp/mrvSettingsObject.h"

#include "mrvFl/mrvIO.h"

#include <FL/platform.H>
#include <FL/Fl.H>

namespace {
    const char* kModule = "view";
}

namespace mrv
{
    using namespace tl;

    math::BBox2i  TimelineViewport::Private::selection;
    imaging::Size TimelineViewport::Private::videoSize;
    ActionMode   TimelineViewport::Private::actionMode = ActionMode::kScrub;
    float        TimelineViewport::Private::masking = 0.F;
    otio::RationalTime TimelineViewport::Private::lastTime;
    uint64_t     TimelineViewport::Private::skippedFrames = 0;
    bool         TimelineViewport::Private::safeAreas = false;
    bool         TimelineViewport::Private::hudActive = true;
    HudDisplay   TimelineViewport::Private::hud = HudDisplay::kNone;

    TimelineViewport::TimelineViewport(
        int X, int Y, int W, int H, const char* L ) :
        Fl_SuperClass( X, Y, W, H, L ),
        _p( new Private )
    {
    }

    TimelineViewport::TimelineViewport( int W, int H, const char* L ) :
        Fl_SuperClass( W, H, L ),
        _p( new Private )
    {
    }

    TimelineViewport::~TimelineViewport()
    {
        _unmapBuffer();
    }

    void TimelineViewport::main( ViewerUI* m ) noexcept
    {
        TLRENDER_P();
        p.ui = m;
    }


    void TimelineViewport::undo()
    {
        TLRENDER_P();

        const auto player = getTimelinePlayer();
        if ( ! player ) return;

        player->undoAnnotation();

        auto annotation = player->getAnnotation();
        if ( !annotation )
        {
            p.ui->uiUndoDraw->deactivate();
            p.ui->uiRedoDraw->deactivate();
            redrawWindows();
            return;
        }

        auto numShapes = annotation->shapes().size();
        if ( numShapes == 0 )
        {
            p.ui->uiUndoDraw->deactivate();
        }
        redrawWindows();
    }

    void TimelineViewport::redo()
    {
        TLRENDER_P();

        const auto player = getTimelinePlayer();
        if ( ! player ) return;

        player->redoAnnotation();
        auto annotation = player->getAnnotation();
        if ( !annotation )
        {
            p.ui->uiUndoDraw->deactivate();
            p.ui->uiRedoDraw->deactivate();
            redrawWindows();
            return;
        }

        auto numShapes = annotation->undo_shapes().size();
        if ( numShapes == 0 )
        {
            p.ui->uiRedoDraw->deactivate();
        }

        redrawWindows();
    }


    void TimelineViewport::setActionMode(const ActionMode& mode) noexcept
    {
        TLRENDER_P();

        if ( mode == p.actionMode ) return;

        //! Turn off all buttons
        p.ui->uiScrub->value(0);
        p.ui->uiSelection->value(0);
        p.ui->uiDraw->value(0);
        p.ui->uiErase->value(0);
        p.ui->uiCircle->value(0);
        p.ui->uiRectangle->value(0);
        p.ui->uiArrow->value(0);
        p.ui->uiText->value(0);

        if ( mode != kSelection )
        {
            p.selection.min = p.selection.max;
        }

        if ( p.actionMode == kText && mode != kText )
        {
            acceptMultilineInput();
        }

        p.actionMode = mode;

        switch( mode )
        {
        case kScrub:
            p.ui->uiScrub->value(1);
            p.ui->uiStatus->copy_label( _("Scrub") );
            break;
        case kSelection:
            p.ui->uiSelection->value(1);
            p.ui->uiStatus->copy_label( _("Selection") );
            break;
        case kDraw:
            p.ui->uiDraw->value(1);
            p.ui->uiStatus->copy_label( _("Draw") );
            break;
        case kErase:
            p.ui->uiErase->value(1);
            p.ui->uiStatus->copy_label( _("Erase") );
            break;
        case kCircle:
            p.ui->uiCircle->value(1);
            p.ui->uiStatus->copy_label( _("Circle") );
            break;
        case kRectangle:
            p.ui->uiRectangle->value(1);
            p.ui->uiStatus->copy_label( _("Rectangle") );
            break;
        case kArrow:
            p.ui->uiArrow->value(1);
            p.ui->uiStatus->copy_label( _("Arrow") );
            break;
        case kText:
            p.ui->uiText->value(1);
            p.ui->uiStatus->copy_label( _("Text") );
            break;
        case kRotate:
            p.ui->uiStatus->copy_label( _("Rotate") );
            break;
        }

        _updateCursor();

        // We refresh the window to clear the OpenGL drawing cursor
        redraw();
    }

    void TimelineViewport::cursor( Fl_Cursor x ) const noexcept
    {
        window()->cursor( x );
    }

    void TimelineViewport::scrub() noexcept
    {
        TLRENDER_P();

        if (p.timelinePlayers.empty()) return;

        const auto t = p.timelinePlayers[0]->currentTime();
        const int X = Fl::event_x() * pixels_per_unit();

        const float scale =
            p.ui->uiPrefs->uiPrefsScrubbingSensitivity->value();

        float dx = ( X - p.mousePress.x );
        dx /= scale;

        const auto& player = p.timelinePlayers[0];
        const auto&   time = t + otime::RationalTime(dx, t.rate());
        DBGM2( "dx= " << dx << " X=" << X << " p.mousePress.x="
               << p.mousePress.x
               << " t= " << t << " seek " << time );
        player->seek(time);
        p.mousePress.x = X;

    }

    void TimelineViewport::resize( int X, int Y, int W, int H )
    {
        Fl_SuperClass::resize( X, Y, W, H );
        if ( hasFrameView() )
        {
            _frameView();
        }
    }

    void TimelineViewport::startFrame() noexcept
    {
        TLRENDER_P();
        for (const auto& i : p.timelinePlayers)
        {
            i->start();
        }
    }

    void TimelineViewport::framePrev() noexcept
    {
        TLRENDER_P();
        for (const auto& i : p.timelinePlayers)
        {
            i->framePrev();
        }
    }

    void TimelineViewport::playBackwards() noexcept
    {
        TLRENDER_P();
        for (const auto& i : p.timelinePlayers)
        {
            i->setPlayback( timeline::Playback::Reverse );
        }
        p.ui->uiMain->fill_menu( p.ui->uiMenuBar );
    }

    void TimelineViewport::stop() noexcept
    {
        TLRENDER_P();
        for (const auto& i : p.timelinePlayers)
        {
            i->setPlayback( timeline::Playback::Stop );
        }
        p.ui->uiMain->fill_menu( p.ui->uiMenuBar );
    }

    void TimelineViewport::playForwards() noexcept
    {
        TLRENDER_P();
        for (const auto& i : p.timelinePlayers)
        {
            i->setPlayback( timeline::Playback::Forward );
        }
        p.ui->uiMain->fill_menu( p.ui->uiMenuBar );
    }

    void TimelineViewport::togglePlayback() noexcept
    {
        TLRENDER_P();
        for (const auto& i : p.timelinePlayers)
        {
            i->togglePlayback();
        }
        p.ui->uiMain->fill_menu( p.ui->uiMenuBar );
    }

    void TimelineViewport::frameNext() noexcept
    {
        TLRENDER_P();
        for (const auto& i : p.timelinePlayers)
        {
            i->frameNext();
        }
    }

    void TimelineViewport::endFrame() noexcept
    {
        TLRENDER_P();
        for (const auto& i : p.timelinePlayers)
        {
            i->end();
        }
    }

    const area::Info& TimelineViewport::getColorAreaInfo() noexcept
    {
        return _p->colorAreaInfo;
    }

    const timeline::ColorConfigOptions&
    TimelineViewport::getColorConfigOptions() noexcept
    {
        return _p->colorConfigOptions;
    }

    void TimelineViewport::setColorConfigOptions(
        const timeline::ColorConfigOptions& value) noexcept
    {
        TLRENDER_P();
        if (value == p.colorConfigOptions)
            return;
        p.colorConfigOptions = value;
        redraw();
    }

    timeline::LUTOptions& TimelineViewport::lutOptions() noexcept
    {
        return _p->lutOptions;
    }

    void
    TimelineViewport::setLUTOptions(const timeline::LUTOptions& value) noexcept
    {
        TLRENDER_P();
        if (value == p.lutOptions)
            return;
        p.lutOptions = value;
        redraw();
    }

    void TimelineViewport::setImageOptions(
        const std::vector<timeline::ImageOptions>& value) noexcept
    {
        TLRENDER_P();
        if (value == p.imageOptions)
            return;
        p.imageOptions = value;
        redraw();
    }

    void TimelineViewport::setDisplayOptions(
        const std::vector<timeline::DisplayOptions>& value) noexcept
    {
        TLRENDER_P();
        if (value == p.displayOptions)
            return;
        p.displayOptions = value;
        redraw();
    }

    const timeline::CompareOptions&
    TimelineViewport::getCompareOptions() noexcept
    {
        return _p->compareOptions;
    }

    void
    TimelineViewport::setCompareOptions(
        const timeline::CompareOptions& value) noexcept
    {
        TLRENDER_P();
        if (value == p.compareOptions)
            return;
        p.compareOptions = value;
        redraw();
    }

    void TimelineViewport::setTimelinePlayers(
        const std::vector<TimelinePlayer*>& value, const bool primary) noexcept
    {
        TLRENDER_P();
        p.timelinePlayers = value;
        updateVideoLayers();
        p.videoData.resize( value.size() );
        int index = 0;
        for (const auto i : p.timelinePlayers)
        {
            if ( primary ) i->setTimelineViewport( this );
            else           i->setSecondaryViewport( this );
            const auto& video = i->currentVideo();
            if ( time::isValid( video.time ) )
            {
                p.videoData[index] = video;
            }
            ++index;
        }
        if (p.frameView)
        {
            frameView();
        }
        if ( p.ui->uiColorChannel->children() == 0 )
        {
            p.ui->uiColorChannel->copy_label( _("(no image)") );
        }
        else
        {
            const Fl_Menu_Item* m = p.ui->uiColorChannel->child(0);
            p.ui->uiColorChannel->copy_label( m->text );
        }

        p.ui->uiColorChannel->redraw();

    }

    mrv::TimelinePlayer*
    TimelineViewport::getTimelinePlayer( int idx ) const noexcept
    {
        if ( idx >= _p->timelinePlayers.size() ) return nullptr;  // needed
        return _p->timelinePlayers[idx];
    }

    std::vector< mrv::TimelinePlayer* >&
    TimelineViewport::getTimelinePlayers() const noexcept
    {
        return _p->timelinePlayers;
    }

    const math::Vector2i& TimelineViewport::viewPos() const noexcept
    {
        return _p->viewPos;
    }

    float TimelineViewport::viewZoom() const noexcept
    {
        return _p->viewZoom;
    }

    void TimelineViewport::setFrameView( bool active ) noexcept
    {
        _p->frameView = active;
    }

    bool TimelineViewport::hasFrameView() const noexcept
    {
        return _p->frameView;
    }

    //! Return the safe areas status
    bool TimelineViewport::getSafeAreas() const noexcept
    {
        return _p->safeAreas;
    }

    //! Set the crop masking
    void TimelineViewport::setSafeAreas( bool value ) noexcept
    {
        if ( value == _p->safeAreas ) return;
        _p->safeAreas = value;
        redrawWindows();
    }

    //! Return the crop masking
    float TimelineViewport::getMask() const noexcept
    {
        return _p->masking;
    }

    //! Set the crop masking
    void TimelineViewport::setMask( float value ) noexcept
    {
        if ( value == _p->masking ) return;
        _p->masking = value;
        redrawWindows();
    }


    bool TimelineViewport::getHudActive() const
    {
        return _p->hudActive;
    }

    void TimelineViewport::setHudActive( const bool active )
    {
        _p->hudActive = active;
        redrawWindows();
    }

    void TimelineViewport::setHudDisplay( const HudDisplay hud )
    {
        _p->hud = hud;
        redrawWindows();
    }

    HudDisplay TimelineViewport::getHudDisplay() const noexcept
    {
        return _p->hud;
    }

    void TimelineViewport::_updateCursor() const noexcept
    {
        TLRENDER_P();
        if ( p.actionMode == ActionMode::kScrub ||
             p.actionMode == ActionMode::kSelection ||
             p.actionMode == ActionMode::kRotate )
            cursor( FL_CURSOR_CROSS );
        // else if ( p.actionMode == ActionMode::kRotate )
        //     cursor( FL_CURSOR_MOVE );
        else if ( p.actionMode == ActionMode::kText )
            cursor( FL_CURSOR_INSERT );
        else
            cursor( FL_CURSOR_NONE );
    }

    void TimelineViewport::setViewPosAndZoom(const math::Vector2i& pos,
                                             float zoom) noexcept
    {
        TLRENDER_P();
        if (pos == p.viewPos && zoom == p.viewZoom)
            return;
        p.viewPos = pos;
        p.viewZoom = zoom;
        _updateZoom();
        redraw();
        auto m = getMultilineInput();
        if (!m) return;
        float pixels_unit = pixels_per_unit();
        redraw();
    }

    void TimelineViewport::setViewZoom(float zoom,
                                       const math::Vector2i& focus) noexcept
    {
        TLRENDER_P();
        math::Vector2i pos;
        pos.x = focus.x + (p.viewPos.x - focus.x) * (zoom / p.viewZoom);
        pos.y = focus.y + (p.viewPos.y - focus.y) * (zoom / p.viewZoom);
        setViewPosAndZoom(pos, zoom);
    }

    void TimelineViewport::frameView() noexcept
    {
        TLRENDER_P();
        _frameView();
        _refresh();
        _updateZoom();
        _updateCoords();
    }

    void TimelineViewport::viewZoom1To1() noexcept
    {
        TLRENDER_P();
        const auto viewportSize = _getViewportCenter();
        const auto renderSize = getRenderSize();
        const math::Vector2i c(renderSize.w / 2, renderSize.h / 2);
        p.viewPos.x = viewportSize.x - c.x;
        p.viewPos.y = viewportSize.y - c.y;
        setViewPosAndZoom(p.viewPos, 1.F );
    }


    void TimelineViewport::currentVideoCallback(
        const timeline::VideoData& value,
        const TimelinePlayer* sender ) noexcept
    {
        TLRENDER_P();
        const auto i = std::find(p.timelinePlayers.begin(),
                                 p.timelinePlayers.end(), sender);
        if (i != p.timelinePlayers.end())
        {
            const size_t index = i - p.timelinePlayers.begin();
            p.videoData[index] = value;
            if ( index == 0 )
            {
                if ( p.selection.min != p.selection.max )
                {
                    if ( ! value.layers.empty() )
                    {
                        const auto& image = value.layers[0].image;
                        if ( image && image->isValid() )
                        {
                            const auto& videoSize = image->getSize();
                            if ( p.videoSize != videoSize )
                            {
                                p.selection.min = p.selection.max;
                                p.videoSize = videoSize;
                            }
                        }
                    }
                }
                if ( p.ui->uiBottomBar->visible() )
                {
                    TimelineClass* c = p.ui->uiTimeWindow;
                    c->uiTimeline->redraw();
                    c->uiFrame->setTime(value.time);
                }
            }
            redraw();
        }
    }

    
    bool  TimelineViewport::_isPlaybackStopped() const noexcept
    {
        TLRENDER_P();
        bool stopped = false;
        if ( !p.timelinePlayers.empty() )
        {
            auto player = p.timelinePlayers[0];
            stopped = ( player->playback() == timeline::Playback::Stop );
        }
        DBGM1( "is playback stopped= " << stopped );
        return stopped;
    }
    
    bool  TimelineViewport::_shouldUpdatePixelBar() const noexcept
    {
        TLRENDER_P();
        // Don't update the pixel bar here if we are playing the movie,
        // as we will update it in the draw() routine.
        bool update = _isPlaybackStopped();
        if ( !p.timelinePlayers.empty() )
        {
            auto player = p.timelinePlayers[0];

            // However, if the movie is a single frame long, we need to
            // update it
            if ( player->inOutRange().duration().to_frames() == 1 )
                update = true;
        }
        DBGM1( "should update pixel bar= " << update );
        return update;
    }

    void TimelineViewport::cacheChangedCallback() const noexcept
    {
        if ( ! _p->ui->uiBottomBar->visible() ) return;

        // This checks whether playback is stopped and if so redraws timeline
        bool update = _shouldUpdatePixelBar();
        if ( update )
        {
            TimelineClass* c = _p->ui->uiTimeWindow;
            c->uiTimeline->redraw();
        }
    }

    imaging::Size TimelineViewport::getViewportSize() const noexcept
    {
        TimelineViewport* t =
            const_cast< TimelineViewport* >( this );
        return imaging::Size( t->pixel_w(), t->pixel_h() );
    }

    std::vector<imaging::Size>
    TimelineViewport::_getTimelineSizes() const noexcept
    {
        TLRENDER_P();
        std::vector<imaging::Size> sizes;
        for (const auto& i : p.timelinePlayers)
        {
            const auto& ioInfo = i->ioInfo();
            if (!ioInfo.video.empty())
            {
                sizes.push_back(ioInfo.video[0].size);
            }
        }
        return sizes;
    }

    imaging::Size TimelineViewport::getRenderSize() const noexcept
    {
        return timeline::getRenderSize(_p->compareOptions.mode,
                                       _getTimelineSizes());
    }

    math::Vector2i TimelineViewport::_getViewportCenter() const noexcept
    {
        const auto viewportSize = getViewportSize();
        return math::Vector2i(viewportSize.w / 2, viewportSize.h / 2);
    }

    void TimelineViewport::centerView() noexcept
    {
        TLRENDER_P();
        const auto viewportSize = getViewportSize();
        const auto renderSize = getRenderSize();
        const math::Vector2i c(renderSize.w / 2, renderSize.h / 2);
        p.viewPos.x = viewportSize.w / 2.F - c.x * p.viewZoom;
        p.viewPos.y = viewportSize.h / 2.F - c.y * p.viewZoom;
        p.mousePos = _getFocus();
        _refresh();
        _updateCoords();
    }

    void TimelineViewport::_frameView() noexcept
    {
        TLRENDER_P();
        const auto viewportSize = getViewportSize();
        const auto renderSize = getRenderSize();
        float zoom = viewportSize.w / static_cast<float>(renderSize.w);
        if (zoom * renderSize.h > viewportSize.h)
        {
            zoom = viewportSize.h / static_cast<float>(renderSize.h);
        }
        const math::Vector2i c(renderSize.w / 2, renderSize.h / 2);
        p.viewPos.x = viewportSize.w / 2.F - c.x * zoom;
        p.viewPos.y = viewportSize.h / 2.F - c.y * zoom;
        p.viewZoom = zoom;
        p.mousePos = _getFocus();
        redraw();
    }

    void TimelineViewport::resizeWindow() noexcept
    {
        TLRENDER_P();
        auto renderSize = getRenderSize();
        DBG;

        if ( !renderSize.isValid() ) return;

        DBG;

        Fl_Double_Window* mw = p.ui->uiMain;
        int screen = mw->screen_num();
        float scale = Fl::screen_scale( screen );

        int W = renderSize.w;
        int H = renderSize.h;

        int minx, miny, maxW, maxH, posX, posY;
        Fl::screen_work_area( minx, miny, maxW, maxH, screen );

        PreferencesUI* uiPrefs = p.ui->uiPrefs;
        if ( uiPrefs->uiWindowFixedPosition->value() )
        {
            posX = (int) uiPrefs->uiWindowXPosition->value();
            posY = (int) uiPrefs->uiWindowYPosition->value();

            maxW = maxW - posX + minx;
            maxH = maxH - posY + miny;

        }
        else
        {
            posX = minx;
            posY = miny;
        }



        int decW = mw->decorated_w();
        int decH = mw->decorated_h();
        DBG;

        int dW = decW - mw->w();
        int dH = decH - mw->h();

        DBG;
        maxW -= dW;
        maxH -= dH;
        posX += dW / 2;
#ifdef _WIN32
        posY += dH - dW / 2;
#else
        posY += dH;
#endif
        DBG;

        // Take into account the different UI bars
        if ( p.ui->uiMenuGroup->visible() )
            H += p.ui->uiMenuGroup->h();

        if ( p.ui->uiTopBar->visible() )
            H += p.ui->uiTopBar->h();

        if ( p.ui->uiPixelBar->visible() )
            H += p.ui->uiPixelBar->h();

        if ( p.ui->uiBottomBar->visible() )
            H += p.ui->uiBottomBar->h();

        if ( p.ui->uiToolsGroup->visible() )
            W += p.ui->uiToolsGroup->w();

        if ( p.ui->uiDockGroup->visible() )
            W += p.ui->uiDockGroup->w();

        bool alwaysFrameView = (bool)uiPrefs->uiPrefsAutoFitImage->value();
        p.frameView = alwaysFrameView;

        if ( uiPrefs->uiWindowFixedSize->value() )
        {
            W = (int) uiPrefs->uiWindowXSize->value();
            H = (int) uiPrefs->uiWindowYSize->value();
        }

        DBG;

        maxW = (int) (maxW / scale);
        if ( W < 690 )
        {
            p.frameView = true;
            W = 690;
        }
        else if ( W > maxW )
        {
            p.frameView = true;
            W = maxW;
        }

        maxH =  (int) (maxH / scale);
        if ( H < 590 ) {
            p.frameView = true;
            H =  590;
        }
        else if ( H > maxH )
        {
            p.frameView = true;
            H = maxH;
        }

        if ( W == renderSize.w )
        {
            p.frameView = true;
        }


        DBGM0("pos= "<< posX << ", " << posY << " WxH= " << W << "x" << H );

        mw->resize( posX, posY, W, H );

        if ( p.frameView )
        {
            frameView();
            p.frameView = alwaysFrameView;
        }
    }

    math::Vector2i TimelineViewport::_getFocus(int X, int Y ) const noexcept
    {
        TimelineViewport* self = const_cast< TimelineViewport* >( this );
        math::Vector2i pos;
        const float devicePixelRatio = self->pixels_per_unit();
        pos.x = X * devicePixelRatio;
        pos.y = ( h() - 1 - Y ) * devicePixelRatio;
        return pos;
    }


    math::Vector2i TimelineViewport::_getFocus() const noexcept
    {
        return _getFocus( _p->event_x, _p->event_y );
    }

    math::Vector2i
    TimelineViewport::_getRaster(int X, int Y) const noexcept
    {
        TLRENDER_P();
        math::Vector2i pos(( X - p.viewPos.x ) / p.viewZoom,
                           ( Y - p.viewPos.y ) / p.viewZoom );
        return pos;
    }

    math::Vector2i
    TimelineViewport::_getRaster() const noexcept
    {
        return _getRaster( _p->mousePos.x, _p->mousePos.y );
    }

    void
    TimelineViewport::_updateZoom() const noexcept
    {
        TLRENDER_P();
        char label[12];
        if ( p.viewZoom >= 1.0f )
            snprintf( label, 12, "x%.2g", p.viewZoom );
        else
            snprintf( label, 12, "1/%.3g", 1.0f/p.viewZoom );
        PixelToolBarClass* c = _p->ui->uiPixelWindow;
        c->uiZoom->copy_label( label );
    }

    void
    TimelineViewport::_updateCoords() const noexcept
    {
        char buf[40];
        const auto& pos = _getRaster();

        snprintf( buf, 40, "%5d, %5d", pos.x, pos.y );
        PixelToolBarClass* c = _p->ui->uiPixelWindow;
        c->uiCoord->value( buf );
    }


    //! Set the Annotation previous ghost frames.
    void TimelineViewport::setGhostPrevious( int x )
    {
        _p->ghostPrevious = x;
    }

    //! Set the Annotation previous ghost frames.
    void TimelineViewport::setGhostNext( int x )
    {
        _p->ghostNext = x;
    }

    // Cannot be const imaging::Color4f& rgba, as we clamp values
    void TimelineViewport::_updatePixelBar(
        imaging::Color4f& rgba) const noexcept
    {
        TLRENDER_P();

        PixelToolBarClass* c = _p->ui->uiPixelWindow;
        char buf[24];
        switch( c->uiAColorType->value() )
        {
        case kRGBA_Float:
            c->uiPixelR->value( float_printf( buf, rgba.r ) );
            c->uiPixelG->value( float_printf( buf, rgba.g ) );
            c->uiPixelB->value( float_printf( buf, rgba.b ) );
            c->uiPixelA->value( float_printf( buf, rgba.a ) );
            break;
        case kRGBA_Hex:
            c->uiPixelR->value( hex_printf( buf, rgba.r ) );
            c->uiPixelG->value( hex_printf( buf, rgba.g ) );
            c->uiPixelB->value( hex_printf( buf, rgba.b ) );
            c->uiPixelA->value( hex_printf( buf, rgba.a ) );
            break;
        case kRGBA_Decimal:
            c->uiPixelR->value( dec_printf( buf, rgba.r ) );
            c->uiPixelG->value( dec_printf( buf, rgba.g ) );
            c->uiPixelB->value( dec_printf( buf, rgba.b ) );
            c->uiPixelA->value( dec_printf( buf, rgba.a ) );
            break;
        }


        if ( rgba.r > 1.0f )      rgba.r = 1.0f;
        else if ( rgba.r < 0.0f ) rgba.r = 0.0f;
        if ( rgba.g > 1.0f )      rgba.g = 1.0f;
        else if ( rgba.g < 0.0f ) rgba.g = 0.0f;
        if ( rgba.b > 1.0f )      rgba.b = 1.0f;
        else if ( rgba.b < 0.0f ) rgba.b = 0.0f;

        uint8_t col[3];
        col[0] = uint8_t(rgba.r * 255.f);
        col[1] = uint8_t(rgba.g * 255.f);
        col[2] = uint8_t(rgba.b * 255.f);

        Fl_Color fltk_color( fl_rgb_color( col[0], col[1], col[2] ) );


        // In fltk color lookup? (0 != Fl_BLACK)
        if ( fltk_color == 0 )
            c->uiPixelView->color( FL_BLACK );
        else
            c->uiPixelView->color( fltk_color );
        c->uiPixelView->redraw();

        imaging::Color4f hsv;

        int cspace = c->uiBColorType->value() + 1;

        switch( cspace )
        {
        case color::kHSV:
            hsv = color::rgb::to_hsv( rgba );
            break;
        case color::kHSL:
            hsv = color::rgb::to_hsl( rgba );
            break;
        case color::kCIE_XYZ:
            hsv = color::rgb::to_xyz( rgba );
            break;
        case color::kCIE_xyY:
            hsv = color::rgb::to_xyY( rgba );
            break;
        case color::kCIE_Lab:
            hsv = color::rgb::to_lab( rgba );
            break;
        case color::kCIE_Luv:
            hsv = color::rgb::to_luv( rgba );
            break;
        case color::kYUV:
            hsv = color::rgb::to_yuv( rgba );
            break;
        case color::kYDbDr:
            hsv = color::rgb::to_YDbDr( rgba );
            break;
        case color::kYIQ:
            hsv = color::rgb::to_yiq( rgba );
            break;
        case color::kITU_601:
            hsv = color::rgb::to_ITU601( rgba );
            break;
        case color::kITU_709:
            hsv = color::rgb::to_ITU709( rgba );
            break;
        case color::kRGB:
        default:
            hsv = rgba;
            break;
        }

        c->uiPixelH->value( float_printf( buf, hsv.r ) );
        c->uiPixelS->value( float_printf( buf, hsv.g ) );
        c->uiPixelV->value( float_printf( buf, hsv.b ) );

        mrv::BrightnessType brightness_type = (mrv::BrightnessType)
                                              c->uiLType->value();
        hsv.a = calculate_brightness( rgba, brightness_type );

        c->uiPixelL->value( float_printf( buf, hsv.a ) );

    }

    void TimelineViewport::updatePixelBar() const noexcept
    {
        TLRENDER_P();
        const Fl_Widget* belowmouse = Fl::belowmouse();
        if ( !p.ui->uiPixelBar->visible() || !visible_r() ||
             belowmouse != this ) return;

        const imaging::Size& r = getRenderSize();

        p.mousePos = _getFocus();
        const auto& pos = _getRaster();

        constexpr float NaN = std::numeric_limits<float>::quiet_NaN();
        imaging::Color4f rgba( NaN, NaN, NaN, NaN );
        bool inside = true;
        if ( pos.x < 0 || pos.x >= r.w || pos.y < 0 || pos.y >= r.h )
            inside = false;

        if ( inside )
        {
            _readPixel( rgba );
        }

        _updatePixelBar( rgba );
    }


    std::vector< timeline::ImageOptions >&
    TimelineViewport::getImageOptions() noexcept
    {
        return _p->imageOptions;
    }


    timeline::ImageOptions&
    TimelineViewport::getImageOptions( int idx ) noexcept
    {
        TLRENDER_P();
        static timeline::ImageOptions empty;
        if ( p.imageOptions.empty() ) return empty;
        if ( idx < 0 ) return p.imageOptions[0];
        else           return p.imageOptions[idx];
    }

    std::vector< timeline::DisplayOptions >&
    TimelineViewport::getDisplayOptions() noexcept
    {
        return _p->displayOptions;
    }

    timeline::DisplayOptions&
    TimelineViewport::getDisplayOptions( int idx ) noexcept
    {
        TLRENDER_P();
        static timeline::DisplayOptions empty;
        if ( p.displayOptions.empty() ) return empty;
        if ( idx < 0 ) return p.displayOptions[0];
        else           return p.displayOptions[idx];
    }


    void
    TimelineViewport::updateImageOptions( int idx ) noexcept
    {
        TLRENDER_P();

        timeline::ImageOptions o;
        if ( idx < 0 ) o = p.imageOptions[0];
        else           o = p.imageOptions[idx];

        // @tood. get this from menus, gui or preferences
        //o.alphaBlend = Straight;   // Straight or Premultiplied
        const Fl_Menu_Item* item =
            p.ui->uiMenuBar->find_item(_("Render/Minify Filter/Linear") );
        timeline::ImageFilter min_filter = timeline::ImageFilter::Nearest;
        if ( item->value() ) min_filter = timeline::ImageFilter::Linear;

        item = p.ui->uiMenuBar->find_item(_("Render/Magnify Filter/Linear") );
        timeline::ImageFilter mag_filter = timeline::ImageFilter::Nearest;
        if ( item->value() ) mag_filter = timeline::ImageFilter::Linear;

        o.imageFilters.minify  = min_filter;
        o.imageFilters.magnify = mag_filter;

        _updateImageOptions( idx, o );
    }

    void
    TimelineViewport::_updateImageOptions(
        int idx, const timeline::ImageOptions& o ) noexcept
    {
        TLRENDER_P();
        if ( idx < 0 )
        {
            for( auto& imageOptions : p.imageOptions )
            {
                imageOptions = o;
            }
        }
        else
        {
            p.imageOptions[idx] = o;
        }
        redraw();
    }


    void
    TimelineViewport::updateColorConfigOptions() noexcept
    {
        TLRENDER_P();
        int inputIndex = p.ui->uiICS->value();
        std::string input = p.ui->uiICS->label();
        if ( inputIndex < 0 ) input = "";

        p.colorConfigOptions.fileName =
            p.ui->uiPrefs->uiPrefsOCIOConfig->value();
        p.colorConfigOptions.input = input;

        PopupMenu* menu = p.ui->OCIOView;


        int viewIndex = menu->value();
        const Fl_Menu_Item* w = nullptr;
        if ( viewIndex >=0 ) w = menu->mvalue();
        const char* lbl;
        if ( ! w ) {
            lbl = menu->label();
            for ( int i = 0; i < menu->children(); ++i )
            {
                const Fl_Menu_Item* c = menu->child(i);
                if ( (c->flags & FL_SUBMENU) || !c->label()) continue;
                if ( strcmp( lbl, c->label() ) == 0 )
                {
                    w = menu->child(i);
                    break;
                }
            }
        }
        else lbl = w->label();
        const Fl_Menu_Item* t = nullptr;
        const Fl_Menu_Item* c = nullptr;
        if ( menu->children() > 0 ) c = menu->child(0);
        for ( ; c && w && c != w; ++c )
        {
            if ( c && (c->flags & FL_SUBMENU) ) t = c;
        }
        if ( t && t->label() )
        {
            p.colorConfigOptions.display = t->label();
            if ( lbl && strcmp(lbl, t->label()) != 0 )
            {
                p.colorConfigOptions.view = lbl;
            }
        }
        else
        {
            if (!c || !c->label() ) return;
            std::string view = c->label();
            size_t pos = view.find( '(' );
            std::string display = view.substr( pos+1, view.size() );
            p.colorConfigOptions.view = view.substr( 0, pos-1 );
            pos = display.find( ')' );
            p.colorConfigOptions.display = display.substr( 0, pos );
        }

        menu->copy_label( p.colorConfigOptions.view.c_str() );

#if 0
        std::cerr << "p.colorConfigOptions.fileName="
                  << p.colorConfigOptions.fileName << "." << std::endl
                  << "p.colorConfigOptions.input="
                  << p.colorConfigOptions.input << "." << std::endl
                  << "p.colorConfigOptions.display="
                  << p.colorConfigOptions.display << "." << std::endl
                  << "p.colorConfigOptions.view="
                  << p.colorConfigOptions.view << "." << std::endl
                  << "p.colorConfigOptions.look="
                  << p.colorConfigOptions.look << "." << std::endl;
#endif
        if ( p.ui->uiSecondary && p.ui->uiSecondary->viewport() )
        {
            Viewport* view = p.ui->uiSecondary->viewport();
            view->setColorConfigOptions( p.colorConfigOptions );
        }
        TimelineClass* tc = p.ui->uiTimeWindow;
        tc->uiTimeline->setColorConfigOptions( p.colorConfigOptions );
        tc->uiTimeline->redraw(); // to refresh thumbnail
        redrawWindows();
    }

    inline float calculate_fstop( float exposure ) noexcept
    {
        float base = 3.0f; // for exposure 0 = f/8

        float seq1, seq2;

        float e = exposure * 0.5f;
        float v = (float) base + (float) int( -e );

        float f = fmod( fabs(exposure), 2.0f );
        if ( exposure >= 0 )
        {
            seq1 = 1.0f * powf( 2.0f, v);    // 8
            seq2 = 1.4f * powf( 2.0f, v-1);  // 5.6
        }
        else
        {
            seq1 = 1.0f * powf( 2.0f, v);  // 8
            seq2 = 1.4f * powf( 2.0f, v);  // 11
        }


        float fstop = seq1 * (1-f) + f * seq2;
        return fstop;
    }

    void
    TimelineViewport::updateDisplayOptions( int idx ) noexcept
    {
        TLRENDER_P();

        if ( p.displayOptions.empty() ) {
            p.ui->uiGain->value( 1.0f );
            p.ui->uiGainInput->value( 1.0f );
            p.ui->uiGamma->value( 1.0f );
            return;
        }

        timeline::DisplayOptions d;
        if ( idx < 1 ) d = p.displayOptions[0];
        else           d = p.displayOptions[idx];

        // Get these from the toggle menus


        // We toggle R,G,B,A channels from hotkeys

        float gamma = p.ui->uiGamma->value();
        if ( gamma != d.levels.gamma )
        {
            d.levels.gamma = gamma;
            d.levelsEnabled = true;
            redraw();
        }

        d.exrDisplayEnabled = false;
        if ( d.exrDisplay.exposure < 0.001F )
            d.exrDisplay.exposure = d.color.brightness.x;

        float gain = p.ui->uiGain->value();
        d.color.brightness.x = d.exrDisplay.exposure * gain;
        d.color.brightness.y = d.exrDisplay.exposure * gain;
        d.color.brightness.z = d.exrDisplay.exposure * gain;

        if ( ! mrv::is_equal( gain, 1.F ) )
        {
            d.colorEnabled = true;

            float exposure = ( logf(gain) / logf(2.0f) );
            float fstop = calculate_fstop( exposure );
            char buf[8];
            snprintf( buf, 8, "f/%1.1f", fstop );
            p.ui->uiFStop->copy_label( buf );
            p.ui->uiFStop->labelcolor( 0xFF800000 );
        }
        else
        {
            p.ui->uiFStop->copy_label( "f/8" );
            p.ui->uiFStop->labelcolor( p.ui->uiGain->labelcolor() );
        }


        //  @todo.    ask darby why image filters are both in display
        //            options and in imageoptions
        const Fl_Menu_Item* item =
            p.ui->uiMenuBar->find_item(_("Render/Minify Filter/Linear") );
        timeline::ImageFilter min_filter = timeline::ImageFilter::Nearest;
        if ( item && item->value() ) min_filter = timeline::ImageFilter::Linear;

        item = p.ui->uiMenuBar->find_item(_("Render/Magnify Filter/Linear") );
        timeline::ImageFilter mag_filter = timeline::ImageFilter::Nearest;
        if ( item && item->value() ) mag_filter = timeline::ImageFilter::Linear;

        d.imageFilters.minify  = min_filter;
        d.imageFilters.magnify = mag_filter;

        _updateDisplayOptions( idx, d );
    }

    void TimelineViewport::updateVideoLayers( int idx ) noexcept
    {
        TLRENDER_P();

        const auto& player = getTimelinePlayer();
        if ( !player ) return;

        const auto& info   = player->timelinePlayer()->getIOInfo();

        const auto& videos = info.video;

        p.ui->uiColorChannel->clear();

        std::string name;
        for ( const auto& video : videos )
        {
            if ( video.name == "A,B,G,R" || video.name == "B,G,R" )
                name = "Color";
            else
                name = video.name;
            p.ui->uiColorChannel->add( name.c_str() );
        }

        p.ui->uiColorChannel->menu_end();
    }

    // This function is needed to force the repositioning of the window/view
    // before querying, for example, the mouse coordinates.
    void TimelineViewport::_refresh() noexcept
    {
        redraw();
        Fl::flush(); // force the redraw
    }

    void TimelineViewport::toggleDisplayChannel(
        const timeline::Channels& channel, int idx ) noexcept
    {
        TLRENDER_P();
        timeline::DisplayOptions d;
        if ( idx < 0 ) d = p.displayOptions[0];
        else           d = p.displayOptions[idx];
        if ( d.channels == channel )
        {
            d.channels = timeline::Channels::Color;
        }
        else
        {
            d.channels = channel;
        }
        _updateDisplayOptions( idx, d );
        redraw();
    }

    bool TimelineViewport::getPresentationMode() const noexcept
    {
        return _p->presentation;
    }

    //! Set or unset the window to full screen and hide/show all bars
    void TimelineViewport::setPresentationMode( bool active ) noexcept
    {
        TLRENDER_P();

        if ( !active )
        {
            setFullScreenMode( active );
            p.presentation = false;
        }
        else
        {
            setFullScreenMode( active );
            hide_ui_state( p.ui );
            p.presentation = true; p.fullScreen = false;
        }
    }

    //! Set or unset the window to full screen but don't hide any bars
    void TimelineViewport::setFullScreenMode( bool active ) noexcept
    {
        TLRENDER_P();

        MainWindow* w = p.ui->uiMain;
        if ( !active )
        {
            if ( w->fullscreen_active() ) {
                w->fullscreen_off();
                restore_ui_state( p.ui );
                take_focus();
            }
            p.fullScreen = false;
        }
        else
        {
            if ( !p.presentation )
            {
                save_ui_state( p.ui );
                if ( ! w->fullscreen_active() ) {
                    w->fullscreen();
                    Fl_Group* bar = p.ui->uiToolsGroup;
                    bar->size( 45, bar->h() );
                    p.ui->uiViewGroup->init_sizes();
                    p.ui->uiViewGroup->redraw();
                }
            }
            else
            {
                restore_ui_state( p.ui );
            }
            p.fullScreen = true;
        }
        w->fill_menu( p.ui->uiMenuBar );
    }


    void TimelineViewport::_updateDisplayOptions(
        int idx, const timeline::DisplayOptions& d ) noexcept
    {
        TLRENDER_P();
        if ( idx < 0 )
        {
            idx = 0;
            for( auto& display : p.displayOptions )
            {
                display = d;
            }
        }
        else
        {
            p.displayOptions[idx] = d;
        }

        const TimelinePlayer* player = getTimelinePlayer(idx);
        if ( !player ) return;

        const auto& info   = player->timelinePlayer()->getIOInfo();

        const auto& videos = info.video;

        int layer = p.ui->uiColorChannel->value();
        if ( layer < 0 ) layer = 0;

        std::string name = videos[layer].name;
        if ( name == "A,B,G,R" || name == "B,G,R" ) name = "Color";

        switch ( d.channels )
        {
        case timeline::Channels::Red:
            name += " (R)";
            break;
        case timeline::Channels::Green:
            name += " (G)";
            break;
        case timeline::Channels::Blue:
            name += " (B)";
            break;
        case timeline::Channels::Alpha:
            name += " (A)";
            break;
        case timeline::Channels::Color:
        default:
            break;
        }

        p.ui->uiColorChannel->copy_label( name.c_str() );
        p.ui->uiColorChannel->redraw();
        redraw();
    }

    void TimelineViewport::hsv_to_info( const imaging::Color4f& hsv,
                                        area::Info& info ) const noexcept
    {
        info.hsv.mean.r += hsv.r;
        info.hsv.mean.g += hsv.g;
        info.hsv.mean.b += hsv.b;
        info.hsv.mean.a += hsv.a;

        if ( hsv.r < info.hsv.min.r ) info.hsv.min.r = hsv.r;
        if ( hsv.g < info.hsv.min.g ) info.hsv.min.g = hsv.g;
        if ( hsv.b < info.hsv.min.b ) info.hsv.min.b = hsv.b;
        if ( hsv.a < info.hsv.min.a ) info.hsv.min.a = hsv.a;

        if ( hsv.r > info.hsv.max.r ) info.hsv.max.r = hsv.r;
        if ( hsv.g > info.hsv.max.g ) info.hsv.max.g = hsv.g;
        if ( hsv.b > info.hsv.max.b ) info.hsv.max.b = hsv.b;
        if ( hsv.a > info.hsv.max.a ) info.hsv.max.a = hsv.a;
    }

    imaging::Color4f
    TimelineViewport::rgba_to_hsv( int hsv_colorspace,
                                   imaging::Color4f& rgba ) const noexcept
    {
        if      ( rgba.r < 0.F ) rgba.r = 0.F;
        else if ( rgba.r > 1.F ) rgba.r = 1.F;
        if      ( rgba.g < 0.F ) rgba.g = 0.F;
        else if ( rgba.g > 1.F ) rgba.g = 1.F;
        if      ( rgba.b < 0.F ) rgba.b = 0.F;
        else if ( rgba.b > 1.F ) rgba.b = 1.F;

        imaging::Color4f hsv;

        switch( hsv_colorspace )
        {
        case color::kHSV:
            hsv = color::rgb::to_hsv( rgba );
            break;
        case color::kHSL:
            hsv = color::rgb::to_hsl( rgba );
            break;
        case color::kCIE_XYZ:
            hsv = color::rgb::to_xyz( rgba );
            break;
        case color::kCIE_xyY:
            hsv = color::rgb::to_xyY( rgba );
            break;
        case color::kCIE_Lab:
            hsv = color::rgb::to_lab( rgba );
            break;
        case color::kCIE_Luv:
            hsv = color::rgb::to_luv( rgba );
            break;
        case color::kYUV:
            hsv = color::rgb::to_yuv( rgba );
            break;
        case color::kYDbDr:
            hsv = color::rgb::to_YDbDr( rgba );
            break;
        case color::kYIQ:
            hsv = color::rgb::to_yiq( rgba );
            break;
        case color::kITU_601:
            hsv = color::rgb::to_ITU601( rgba );
            break;
        case color::kITU_709:
            hsv = color::rgb::to_ITU709( rgba );
            break;
        case color::kRGB:
        default:
            hsv = rgba;
            break;
        }
        return hsv;
    }


    void TimelineViewport::_getPixelValue(
        imaging::Color4f& rgba,
        const std::shared_ptr<imaging::Image>& image,
        const math::Vector2i& pos ) const noexcept
    {
        TLRENDER_P();
        imaging::PixelType type = image->getPixelType();
        uint8_t channels = imaging::getChannelCount(type);
        uint8_t depth    = imaging::getBitDepth(type) / 8;
        const auto& info   = image->getInfo();
        imaging::VideoLevels  videoLevels = info.videoLevels;
        const math::Vector4f& yuvCoefficients =
            getYUVCoefficients( info.yuvCoefficients );
        imaging::Size size = image->getSize();
        const uint8_t*  data = image->getData();
        int X = pos.x;
        int Y = size.h - pos.y - 1;
        if ( p.displayOptions[0].mirror.x ) X = size.w - X - 1;
        if ( p.displayOptions[0].mirror.y ) Y = size.h - Y - 1;

        // Do some sanity check just in case
        if ( X < 0 || Y < 0 || X >= size.w || Y >= size.h )
            return;

        size_t offset = ( Y * size.w + X ) * depth;

        switch( type )
        {
        case imaging::PixelType::YUV_420P_U8:
        case imaging::PixelType::YUV_422P_U8:
        case imaging::PixelType::YUV_444P_U8:
            break;
        case imaging::PixelType::YUV_420P_U16:
        case imaging::PixelType::YUV_422P_U16:
        case imaging::PixelType::YUV_444P_U16:
            break;
        default:
            offset *= channels;
            break;
        }

        rgba.a = 1.0f;
        switch ( type )
        {
        case imaging::PixelType::L_U8:
            rgba.r = data[offset] / 255.0f;
            rgba.g = data[offset] / 255.0f;
            rgba.b = data[offset] / 255.0f;
            break;
        case imaging::PixelType::LA_U8:
            rgba.r = data[offset]   / 255.0f;
            rgba.g = data[offset]   / 255.0f;
            rgba.b = data[offset]   / 255.0f;
            rgba.a = data[offset+1] / 255.0f;
            break;
        case imaging::PixelType::L_U16:
        {
            uint16_t* f = (uint16_t*) (&data[offset]);
            rgba.r = f[0] / 65535.0f;
            rgba.g = f[0] / 65535.0f;
            rgba.b = f[0] / 65535.0f;
            break;
        }
        case imaging::PixelType::LA_U16:
        {
            uint16_t* f = (uint16_t*) (&data[offset]);
            rgba.r = f[0] / 65535.0f;
            rgba.g = f[0] / 65535.0f;
            rgba.b = f[0] / 65535.0f;
            rgba.a = f[1] / 65535.0f;
            break;
        }
        case imaging::PixelType::L_U32:
        {
            uint32_t* f = (uint32_t*) (&data[offset]);
            constexpr float max = static_cast<float>(
                std::numeric_limits<uint32_t>::max() );
            rgba.r = f[0] / max;
            rgba.g = f[0] / max;
            rgba.b = f[0] / max;
            break;
        }
        case imaging::PixelType::LA_U32:
        {
            uint32_t* f = (uint32_t*) (&data[offset]);
            constexpr float max = static_cast<float>(
                std::numeric_limits<uint32_t>::max() );
            rgba.r = f[0] / max;
            rgba.g = f[0] / max;
            rgba.b = f[0] / max;
            rgba.a = f[1] / max;
            break;
        }
        case imaging::PixelType::L_F16:
        {
            half* f = (half*) (&data[offset]);
            rgba.r = f[0];
            rgba.g = f[0];
            rgba.b = f[0];
            break;
        }
        case imaging::PixelType::LA_F16:
        {
            half* f = (half*) (&data[offset]);
            rgba.r = f[0];
            rgba.g = f[0];
            rgba.b = f[0];
            rgba.a = f[1];
            break;
        }
        case imaging::PixelType::RGB_U8:
            rgba.r = data[offset] / 255.0f;
            rgba.g = data[offset+1] / 255.0f;
            rgba.b = data[offset+2] / 255.0f;
            break;
        case imaging::PixelType::RGB_U10:
        {
            imaging::U10* f = (imaging::U10*) (&data[offset]);
            constexpr float max = static_cast<float>(
                std::numeric_limits<uint32_t>::max() );
            rgba.r = f->r / max;
            rgba.g = f->g / max;
            rgba.b = f->b / max;
            break;
        }
        case imaging::PixelType::RGBA_U8:
            rgba.r = data[offset] / 255.0f;
            rgba.g = data[offset+1] / 255.0f;
            rgba.b = data[offset+2] / 255.0f;
            rgba.a = data[offset+3] / 255.0f;
            break;
        case imaging::PixelType::RGB_U16:
        {
            uint16_t* f = (uint16_t*) (&data[offset]);
            rgba.r = f[0] / 65535.0f;
            rgba.g = f[1] / 65535.0f;
            rgba.b = f[2] / 65535.0f;
            break;
        }
        case imaging::PixelType::RGBA_U16:
        {
            uint16_t* f = (uint16_t*) (&data[offset]);
            rgba.r = f[0] / 65535.0f;
            rgba.g = f[1] / 65535.0f;
            rgba.b = f[2] / 65535.0f;
            rgba.a = f[3] / 65535.0f;
            break;
        }
        case imaging::PixelType::RGB_U32:
        {
            uint32_t* f = (uint32_t*) (&data[offset]);
            constexpr float max = static_cast<float>(
                std::numeric_limits<uint32_t>::max() );
            rgba.r = f[0] / max;
            rgba.g = f[1] / max;
            rgba.b = f[2] / max;
            break;
        }
        case imaging::PixelType::RGBA_U32:
        {
            uint32_t* f = (uint32_t*) (&data[offset]);
            constexpr float max = static_cast<float>(
                std::numeric_limits<uint32_t>::max() );
            rgba.r = f[0] / max;
            rgba.g = f[1] / max;
            rgba.b = f[2] / max;
            rgba.a = f[3] / max;
            break;
        }
        case imaging::PixelType::RGB_F16:
        {
            half* f = (half*) (&data[offset]);
            rgba.r = f[0];
            rgba.g = f[1];
            rgba.b = f[2];
            break;
        }
        case imaging::PixelType::RGBA_F16:
        {
            half* f = (half*) (&data[offset]);
            rgba.r = f[0];
            rgba.g = f[1];
            rgba.b = f[2];
            rgba.a = f[3];
            break;
        }
        case imaging::PixelType::RGB_F32:
        {
            float* f = (float*) (&data[offset]);
            rgba.r = f[0];
            rgba.g = f[1];
            rgba.b = f[2];
            break;
        }
        case imaging::PixelType::RGBA_F32:
        {
            float* f = (float*) (&data[offset]);
            rgba.r = f[0];
            rgba.g = f[1];
            rgba.b = f[2];
            rgba.a = f[3];
            break;
        }
        case imaging::PixelType::YUV_420P_U8:
        {
            size_t Ysize = size.w * size.h;
            size_t w2      = (size.w + 1) / 2;
            size_t h2      = (size.h + 1) / 2;
            size_t Usize   = w2 * h2;
            size_t offset2 = (Y/2) * w2 + X / 2;
            rgba.r = data[ offset ]                  / 255.0f;
            rgba.g = data[ Ysize + offset2 ]         / 255.0f;
            rgba.b = data[ Ysize + Usize + offset2 ] / 255.0f;
            color::checkLevels( rgba, videoLevels );
            rgba = color::YPbPr::to_rgb( rgba, yuvCoefficients );
            break;
        }
        case imaging::PixelType::YUV_422P_U8:
        {
            size_t Ysize = size.w * size.h;
            size_t w2      = (size.w + 1) / 2;
            size_t Usize   = w2 * size.h;
            size_t offset2 = Y * w2 + X / 2;
            rgba.r = data[ offset ]              / 255.0f;
            rgba.g = data[ Ysize + offset2 ]         / 255.0f;
            rgba.b = data[ Ysize + Usize + offset2 ] / 255.0f;
            color::checkLevels( rgba, videoLevels );
            rgba = color::YPbPr::to_rgb( rgba, yuvCoefficients );
            break;
        }
        case imaging::PixelType::YUV_444P_U8:
        {
            size_t Ysize = size.w * size.h;
            rgba.r = data[ offset ]             / 255.0f;
            rgba.g = data[ Ysize + offset ]     / 255.0f;
            rgba.b = data[ Ysize * 2 + offset ] / 255.0f;
            color::checkLevels( rgba, videoLevels );
            rgba = color::YPbPr::to_rgb( rgba, yuvCoefficients );
            break;
        }
        case imaging::PixelType::YUV_420P_U16:
        {
            size_t pos = Y * size.w / 4 + X / 2;
            size_t Ysize = size.w * size.h;
            size_t Usize = Ysize / 4;
            rgba.r = data[ offset ]              / 65535.0f;
            rgba.g = data[ Ysize + pos ]         / 65535.0f;
            rgba.b = data[ Ysize + Usize + pos ] / 65535.0f;
            color::checkLevels( rgba, videoLevels );
            rgba = color::YPbPr::to_rgb( rgba, yuvCoefficients );
            break;
        }
        case imaging::PixelType::YUV_422P_U16:
        {
            size_t Ysize = size.w * size.h * depth;
            size_t pos = Y * size.w + X;
            size_t Usize = size.w / 2 * size.h * depth;
            rgba.r = data[ offset ]              / 65535.0f;
            rgba.g = data[ Ysize + pos ]         / 65535.0f;
            rgba.b = data[ Ysize + Usize + pos ] / 65535.0f;
            color::checkLevels( rgba, videoLevels );
            rgba = color::YPbPr::to_rgb( rgba, yuvCoefficients );
            break;
        }
        case imaging::PixelType::YUV_444P_U16:
        {
            size_t Ysize = size.w * size.h * depth;
            rgba.r = data[ offset ]             / 65535.0f;
            rgba.g = data[ Ysize + offset ]     / 65535.0f;
            rgba.b = data[ Ysize * 2 + offset ] / 65535.0f;
            color::checkLevels( rgba, videoLevels );
            rgba = color::YPbPr::to_rgb( rgba, yuvCoefficients );
            break;
        }
        default:
            break;
        }

    }


    void
    TimelineViewport::_calculateColorAreaRawValues(
        area::Info& info ) const noexcept
    {
        TLRENDER_P();

        PixelToolBarClass* c = p.ui->uiPixelWindow;
        BrightnessType brightness_type =(BrightnessType) c->uiLType->value();
        int hsv_colorspace = c->uiBColorType->value() + 1;

        int maxX = info.box.max.x;
        int maxY = info.box.max.y;

        for ( int Y = info.box.y(); Y < maxY; ++Y )
        {
            for ( int X = info.box.x(); X < maxX; ++X )
            {
                imaging::Color4f rgba, hsv;
                rgba.r = rgba.g = rgba.b = rgba.a = 0.f;

                math::Vector2i pos( X, Y );
                for ( const auto& video : p.videoData )
                {
                    for ( const auto& layer : video.layers )
                    {
                        const auto& image = layer.image;
                        if ( ! image->isValid() ) continue;

                        imaging::Color4f pixel, pixelB;

                        _getPixelValue( pixel, image, pos );


                        const auto& imageB = layer.image;
                        if ( imageB->isValid() )
                        {
                            _getPixelValue( pixelB, imageB, pos );

                            if ( layer.transition ==
                                 timeline::Transition::Dissolve )
                            {
                                float f2 = layer.transitionValue;
                                float  f = 1.0 - f2;
                                pixel.r = pixel.r * f + pixelB.r * f2;
                                pixel.g = pixel.g * f + pixelB.g * f2;
                                pixel.b = pixel.b * f + pixelB.b * f2;
                                pixel.a = pixel.a * f + pixelB.a * f2;
                            }
                        }
                        rgba.r += pixel.r;
                        rgba.g += pixel.g;
                        rgba.b += pixel.b;
                        rgba.a += pixel.a;
                    }

                info.rgba.mean.r += rgba.r;
                info.rgba.mean.g += rgba.g;
                info.rgba.mean.b += rgba.b;
                info.rgba.mean.a += rgba.a;

                if ( rgba.r < info.rgba.min.r ) info.rgba.min.r = rgba.r;
                if ( rgba.g < info.rgba.min.g ) info.rgba.min.g = rgba.g;
                if ( rgba.b < info.rgba.min.b ) info.rgba.min.b = rgba.b;
                if ( rgba.a < info.rgba.min.a ) info.rgba.min.a = rgba.a;

                if ( rgba.r > info.rgba.max.r ) info.rgba.max.r = rgba.r;
                if ( rgba.g > info.rgba.max.g ) info.rgba.max.g = rgba.g;
                if ( rgba.b > info.rgba.max.b ) info.rgba.max.b = rgba.b;
                if ( rgba.a > info.rgba.max.a ) info.rgba.max.a = rgba.a;

                hsv = rgba_to_hsv( hsv_colorspace, rgba );
                hsv.a = calculate_brightness( rgba, brightness_type );
                hsv_to_info( hsv, info );
                }
            }
        }

        int num = info.box.w() * info.box.h();
        info.rgba.mean.r /= num;
        info.rgba.mean.g /= num;
        info.rgba.mean.b /= num;
        info.rgba.mean.a /= num;

        info.rgba.diff.r = info.rgba.max.r - info.rgba.min.r;
        info.rgba.diff.g = info.rgba.max.g - info.rgba.min.g;
        info.rgba.diff.b = info.rgba.max.b - info.rgba.min.b;
        info.rgba.diff.a = info.rgba.max.a - info.rgba.min.a;

        info.hsv.mean.r /= num;
        info.hsv.mean.g /= num;
        info.hsv.mean.b /= num;
        info.hsv.mean.a /= num;

        info.hsv.diff.r = info.hsv.max.r - info.hsv.min.r;
        info.hsv.diff.g = info.hsv.max.g - info.hsv.min.g;
        info.hsv.diff.b = info.hsv.max.b - info.hsv.min.b;
        info.hsv.diff.a = info.hsv.max.a - info.hsv.min.a;
    }


    void TimelineViewport::_mapBuffer() const noexcept
    {
        TLRENDER_P();

        p.rawImage = true;
        const imaging::Size& renderSize = getRenderSize();
        unsigned dataSize = renderSize.w * renderSize.h * 4 * sizeof(float);

        if ( dataSize != p.rawImageSize || !p.image )
        {
            free( p.image );
            p.image = (float*) malloc( dataSize );
            p.rawImageSize = dataSize;
        }
        if ( !p.image ) return;

        unsigned maxY = renderSize.h;
        unsigned maxX = renderSize.w;
        for ( int Y = 0; Y < maxY; ++Y )
        {
            for ( int X = 0; X < maxX; ++X )
            {
                imaging::Color4f& rgba = (imaging::Color4f&)
                                         p.image[ (X + maxX * Y) * 4];
                rgba.r = rgba.g = rgba.b = rgba.a = 0.f;

                math::Vector2i pos( X, Y );
                for ( const auto& video : p.videoData )
                {
                    for ( const auto& layer : video.layers )
                    {
                        const auto& image = layer.image;
                        if ( ! image->isValid() ) continue;

                        imaging::Color4f pixel, pixelB;

                        _getPixelValue( pixel, image, pos );


                        const auto& imageB = layer.image;
                        if ( imageB->isValid() )
                        {
                            _getPixelValue( pixelB, imageB, pos );

                            if ( layer.transition ==
                                 timeline::Transition::Dissolve )
                            {
                                float f2 = layer.transitionValue;
                                float  f = 1.0 - f2;
                                pixel.r = pixel.r * f + pixelB.r * f2;
                                pixel.g = pixel.g * f + pixelB.g * f2;
                                pixel.b = pixel.b * f + pixelB.b * f2;
                                pixel.a = pixel.a * f + pixelB.a * f2;
                            }
                        }
                        rgba.r += pixel.r;
                        rgba.g += pixel.g;
                        rgba.b += pixel.b;
                        rgba.a += pixel.a;
                    }
                    float tmp = rgba.r;
                    rgba.r = rgba.b;
                    rgba.b = tmp;
                }
            }
        }
    }

    void TimelineViewport::_unmapBuffer() const noexcept
    {
        TLRENDER_P();
        if ( p.rawImage )
        {
            free( p.image );
            p.image = nullptr;
            p.rawImage = true;
        }
    }

    const imaging::Color4f* TimelineViewport::image() const
    {
        return ( imaging::Color4f* ) ( _p->image );
    }

    //! Get the focal length of latiude longitude mapping
    double TimelineViewport::focalLength() const noexcept
    {
        if ( !environmentMapPanel ) return 7.0F;
        return environmentMapPanel->focalLength->value();
    }

    //! Set the focal length of latiude longitude mapping
    void TimelineViewport::setFocalLength(double x) noexcept
    {
        if ( !environmentMapPanel ) return;
        environmentMapPanel->focalLength->value(x);
        redrawWindows();
    }
}
