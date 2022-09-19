// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <FL/gl.h>
#include <FL/glu.h>
#include <FL/names.h>


#include <memory>
#include <cmath>

#include <mrvCore/mrvUtil.h>
#include <mrvCore/mrvHotkey.h>
#include <mrvCore/mrvColorSpaces.h>

#include <mrvFl/mrvCallbacks.h>
#include <mrvFl/mrvTimelinePlayer.h>
#include <mrvFl/mrvIO.h>

#include <mrvGL/mrvTimelineViewport.h>
#include <mrvGL/mrvTimelineViewportPrivate.h>

#include <mrViewer.h>

#include <glm/gtc/matrix_transform.hpp>


namespace {
    const char* kModule = "view";
}

namespace mrv
{
    using namespace tl;

    TimelineViewport::TimelineViewport(
        int X, int Y, int W, int H, const char* L ) :
        Fl_SuperClass( X, Y, W, H, L ),
        _p( new Private )
    {
    }

    TimelineViewport::~TimelineViewport()
    {
    }

    void TimelineViewport::main( ViewerUI* m )
    {
        _p->ui = m;
    }

    void TimelineViewport::scrub()
    {
        TLRENDER_P();

        if (!p.timelinePlayers.empty())
        {
            const auto t = p.timelinePlayers[0]->currentTime();
            const int X = Fl::event_x() * pixels_per_unit();

            const float scale =
                p.ui->uiPrefs->uiPrefsScrubbingSensitivity->value();

            float dx = ( X - p.mousePress.x );
            dx /= scale;

            p.timelinePlayers[0]->seek(t + otime::RationalTime(dx, t.rate()));
            p.mousePress.x = X;
        }
    }

    void TimelineViewport::resize( int X, int Y, int W, int H )
    {
        TLRENDER_P();
        Fl_SuperClass::resize( X, Y, W, H );
        if ( hasFrameView() )
        {
            frameView();
        }
    }

    void TimelineViewport::startFrame()
    {
        TLRENDER_P();
        for (const auto& i : p.timelinePlayers)
        {
            i->start();
        }
    }

    void TimelineViewport::framePrev()
    {
        TLRENDER_P();
        for (const auto& i : p.timelinePlayers)
        {
            i->framePrev();
        }
    }

    void TimelineViewport::playBackwards()
    {
        TLRENDER_P();
        for (const auto& i : p.timelinePlayers)
        {
            i->setPlayback( timeline::Playback::Reverse );
        }
    }

    void TimelineViewport::stop()
    {
        TLRENDER_P();
        for (const auto& i : p.timelinePlayers)
        {
            i->setPlayback( timeline::Playback::Stop );
        }
    }

    void TimelineViewport::playForwards()
    {
        TLRENDER_P();
        for (const auto& i : p.timelinePlayers)
        {
            i->setPlayback( timeline::Playback::Forward );
        }
    }

    void TimelineViewport::frameNext()
    {
        TLRENDER_P();
        for (const auto& i : p.timelinePlayers)
        {
            i->frameNext();
        }
    }

    void TimelineViewport::endFrame()
    {
        TLRENDER_P();
        for (const auto& i : p.timelinePlayers)
        {
            i->end();
        }
    }

    void TimelineViewport::setHudDisplay( const HudDisplay hud )
    {
        TLRENDER_P();
        p.hud = hud;
        redraw();
    }

    HudDisplay TimelineViewport::getHudDisplay() const noexcept
    {
        return _p->hud;
    }

    int TimelineViewport::handle( int event )
    {
        TLRENDER_P();

        p.event_x = Fl::event_x();
        p.event_y = Fl::event_y();

        switch( event )
        {
        case FL_FOCUS:
            return 1;
            break;
        case FL_ENTER:
            window()->cursor( FL_CURSOR_CROSS );
            return 1;
            break;
        case FL_LEAVE:
            window()->cursor( FL_CURSOR_DEFAULT );
            return 1;
            break;
        case FL_PUSH:
        {
            if (!children()) take_focus();
            int button = Fl::event_button();
            p.mousePress = _getFocus();
            if ( button == FL_MIDDLE_MOUSE )
            {
                p.viewPosMousePress = p.viewPos;
            }
            return 1;
        }
        case FL_RELEASE:
        {
            return 1;
        }
        case FL_MOVE:
        {
            _updatePixelBar();
            _updateCoords();
            return 1;
        }
        case FL_DRAG:
        {
            int button = Fl::event_button();
            p.mousePos = _getFocus();
            if ( button == FL_MIDDLE_MOUSE )
            {
                p.viewPos.x = p.viewPosMousePress.x +
                              (p.mousePos.x - p.mousePress.x);
                p.viewPos.y = p.viewPosMousePress.y +
                              (p.mousePos.y - p.mousePress.y);
                p.frameView = false;
            }
            else if ( button == FL_LEFT_MOUSE )
            {
                scrub();
            }
            _updatePixelBar();
            _updateCoords();
            redraw();
            return 1;
        }
        case FL_MOUSEWHEEL:
        {
            float dy = Fl::event_dy();
            int idx = p.ui->uiPrefs->uiPrefsZoomSpeed->value();
            const float speedValues[] = { 0.1f, 0.25f, 0.5f };
            float speed = speedValues[idx];
            float change = 1.0f;
            if ( dy > 0 )
            {
                change += dy * speed;
                change = 1.0f / change;
            }
            else
            {
                change -= dy * speed;
            }
            setViewZoom( viewZoom() * change, _getFocus() );
            return 1;
        }
        case FL_KEYBOARD:
        {
            unsigned rawkey = Fl::event_key();
            if ( kResetChanges.match( rawkey ) )
            {
                p.ui->uiGamma->value( 1.0 );
                p.ui->uiGain->value( 1.0 );
                updateDisplayOptions();
                _refresh();
                _updatePixelBar();
                return 1;
            }
            else if ( kFitScreen.match( rawkey ) )
            {
                frameView();
                return 1;
            }
            else if ( kCenterImage.match( rawkey ) )
            {
                centerView();
                return 1;
            }
            else if ( kPlayDirection.match( rawkey ) )
            {
                using timeline::Playback;
                Playback playback = p.timelinePlayers[0]->playback();

                for (const auto& i : p.timelinePlayers)
                {
                    i->togglePlayback();
                }
                return 1;
            }
            else if ( kPlayFwd.match( rawkey ) )
            {
                playForwards();
                return 1;
            }
            else if ( kPlayBack.match( rawkey ) )
            {
                playBackwards();
                return 1;
            }
            else if ( kFrameStepFwd.match( rawkey ) )
            {
                frameNext();
                return 1;
            }
            else if ( kFrameStepBack.match( rawkey ) )
            {
                framePrev();
                return 1;
            }
            else if ( kFirstFrame.match( rawkey ) )
            {
                startFrame();
                return 1;
            }
            else if ( kLastFrame.match( rawkey ) )
            {
                endFrame();
                return 1;
            }
            else if ( kToggleToolBar.match( rawkey ) )
            {
                toggle_ui_bar( p.ui, p.ui->uiToolsGroup, 45, 433 );
                save_ui_state( p.ui );
                return 1;
            }
            else if ( kTogglePresentation.match( rawkey ) )
            {

                Fl_Window* w= p.ui->uiMain;

                if ( p.presentation )
                {
                    restore_ui_state( p.ui );
                    if ( w->fullscreen_active() ) w->fullscreen_off();
                    restore_ui_state( p.ui );
                    p.presentation = false;
                }
                else
                {
                    save_ui_state( p.ui );
#ifdef __linux__
                    // Not sure why we need this on linux, but we do
                    hide_ui_state( p.ui );
#endif
                    w->fullscreen();
                    hide_ui_state( p.ui );

                    p.presentation = true;
                }
                return 1;
            }
            else if ( kFullScreen.match( rawkey ) )
            {
                if ( p.fullScreen || p.presentation )
                {
                    p.ui->uiMain->fullscreen_off();
                    restore_ui_state( p.ui );
                    p.fullScreen = p.presentation = false;
                }
                else
                {
                    save_ui_state( p.ui );
                    p.ui->uiMain->fullscreen();
                    p.fullScreen = true;
                }
                return 1;
            }
            else if ( kToggleMenuBar.match( rawkey ) )
            {
                toggle_ui_bar( p.ui, p.ui->uiMenuGroup, 25 );
                if ( p.ui->uiMenuGroup->visible() )
                    p.ui->uiMain->fill_menu( p.ui->uiMenuBar );
                save_ui_state( p.ui );
                return 1;
            }
            else if ( kToggleTopBar.match( rawkey ) )
            {
                toggle_ui_bar( p.ui, p.ui->uiTopBar, 28 );
                save_ui_state( p.ui );
                return 1;
            }
            else if ( kTogglePixelBar.match( rawkey ) )
            {
                toggle_ui_bar( p.ui, p.ui->uiPixelBar, 30 );
                save_ui_state( p.ui );
                return 1;
            }
            else if ( kToggleTimeline.match( rawkey ) )
            {
                toggle_ui_bar( p.ui, p.ui->uiBottomBar, 49 );
                save_ui_state( p.ui );
                return 1;
            }
            else if ( rawkey >= kZoomMin.key && rawkey <= kZoomMax.key )
            {
                if ( rawkey == kZoomMin.key )
                {
                    viewZoom1To1();
                }
                else
                {
                    float z = (float) (rawkey - kZoomMin.key);
                    if ( Fl::event_state( FL_CTRL ) )
                        z = 1.0f / z;
                    setViewZoom( z, _getFocus() );
                }
                return 1;
            }
        }
        default:
            break;
        }

        return Fl_SuperClass::handle( event );
    }

    void TimelineViewport::setColorConfigOptions(
        const timeline::ColorConfigOptions& value)
    {
        TLRENDER_P();
        if (value == p.colorConfigOptions)
            return;
        p.colorConfigOptions = value;
        redraw();
    }

    void TimelineViewport::setLUTOptions(const timeline::LUTOptions& value)
    {
        TLRENDER_P();
        if (value == p.lutOptions)
            return;
        p.lutOptions = value;
        redraw();
    }

    void TimelineViewport::setImageOptions(
        const std::vector<timeline::ImageOptions>& value)
    {
        TLRENDER_P();
        if (value == p.imageOptions)
            return;
        p.imageOptions = value;
        redraw();
    }

    void TimelineViewport::setDisplayOptions(
        const std::vector<timeline::DisplayOptions>& value)
    {
        TLRENDER_P();
        if (value == p.displayOptions)
            return;
        p.displayOptions = value;
        redraw();
    }

    void TimelineViewport::setCompareOptions(const timeline::CompareOptions& value)
    {
        TLRENDER_P();
        if (value == p.compareOptions)
            return;
        p.compareOptions = value;
        redraw();
    }

    void TimelineViewport::setTimelinePlayers(
        const std::vector<TimelinePlayer*>& value)
    {
        TLRENDER_P();
        p.videoData.clear();
        p.timelinePlayers = value;
        updateVideoLayers();
        for (const auto& i : p.timelinePlayers)
        {
            _p->videoData.push_back(i->video());
        }
        if (p.frameView)
        {
            _frameView();
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

    TimelinePlayer*
    TimelineViewport::getTimelinePlayer(const int index) const
    {
        if ( index >= _p->timelinePlayers.size() ) return nullptr;
        return _p->timelinePlayers[index];
    }

    const math::Vector2i& TimelineViewport::viewPos() const
    {
        return _p->viewPos;
    }

    float TimelineViewport::viewZoom() const
    {
        return _p->viewZoom;
    }

    bool TimelineViewport::hasFrameView() const
    {
        return _p->frameView;
    }

    void TimelineViewport::setViewPosAndZoom(const math::Vector2i& pos,
                                             float zoom)
    {
        TLRENDER_P();
        if (pos == p.viewPos && zoom == p.viewZoom)
            return;
        p.viewPos = pos;
        p.viewZoom = zoom;
        char label[12];
        if ( zoom >= 1.0f )
            sprintf( label, N_("x%.2g"), zoom );
        else
            sprintf( label, N_("1/%.3g"), 1.0f/zoom );
        p.ui->uiZoom->copy_label( label );
        redraw();
    }

    void TimelineViewport::setViewZoom(float zoom,
                                       const math::Vector2i& focus)
    {
        TLRENDER_P();
        math::Vector2i pos;
        pos.x = focus.x + (p.viewPos.x - focus.x) * (zoom / p.viewZoom);
        pos.y = focus.y + (p.viewPos.y - focus.y) * (zoom / p.viewZoom);
        setViewPosAndZoom(pos, zoom);
    }

    void TimelineViewport::frameView()
    {
        TLRENDER_P();
        _frameView();
        _refresh();
        _updatePixelBar();
    }

    void TimelineViewport::viewZoom1To1()
    {
        TLRENDER_P();
        const auto viewportSize = _getViewportCenter();
        const auto renderSize = _getRenderSize();
        const math::Vector2i c(renderSize.w / 2, renderSize.h / 2);
        p.viewPos.x = viewportSize.x - c.x;
        p.viewPos.y = viewportSize.y - c.y;
        setViewPosAndZoom(p.viewPos, 1.F );
    }


    void TimelineViewport::videoCallback(const timeline::VideoData& value,
                                         const TimelinePlayer* sender ) noexcept
    {
        TLRENDER_P();
        const auto i = std::find(p.timelinePlayers.begin(),
                                 p.timelinePlayers.end(), sender);
        if (i != p.timelinePlayers.end())
        {
            const size_t index = i - p.timelinePlayers.begin();
            p.videoData[index] = value;
            p.ui->uiTimeline->redraw();
            p.ui->uiFrame->setTime( value.time );
            p.ui->uiFrame->redraw();
            redraw();
        }
    }


    imaging::Size TimelineViewport::_getViewportSize() const noexcept
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

    imaging::Size TimelineViewport::_getRenderSize() const noexcept
    {
        return timeline::getRenderSize(_p->compareOptions.mode,
                                       _getTimelineSizes());
    }

    math::Vector2i TimelineViewport::_getViewportCenter() const noexcept
    {
        const auto viewportSize = _getViewportSize();
        return math::Vector2i(viewportSize.w / 2, viewportSize.h / 2);
    }

    void TimelineViewport::centerView() noexcept
    {
        TLRENDER_P();
        const auto viewportSize = _getViewportSize();
        const auto renderSize = _getRenderSize();
        const math::Vector2i c(renderSize.w / 2, renderSize.h / 2);
        p.viewPos.x = viewportSize.w / 2.F - c.x * p.viewZoom;
        p.viewPos.y = viewportSize.h / 2.F - c.y * p.viewZoom;
        p.mousePos = _getFocus();
        _refresh();
        _updateCoords();
        _updatePixelBar();
    }
    void TimelineViewport::_frameView() noexcept
    {
        TLRENDER_P();
        const auto viewportSize = _getViewportSize();
        const auto renderSize = _getRenderSize();
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
        _updateCoords();
        _refresh();
        _updatePixelBar();
    }

    void TimelineViewport::resizeWindow() noexcept
    {
        TLRENDER_P();
        auto renderSize = _getRenderSize();
        if ( !renderSize.isValid() ) return;


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

        int dW = decW - mw->w();
        int dH = decH - mw->h();

        maxW -= dW;
        maxH -= dH;
        posX += dW / 2;
#ifdef _WIN32
        posY += dH - dW / 2;
#else
        posY += dH;
#endif

        // Take into account the different UI bars
        if ( p.ui->uiMenuGroup->visible() )
            H += p.ui->uiMenuGroup->h();

        if ( p.ui->uiTopBar->visible() )
            H += p.ui->uiTopBar->h();

        if ( p.ui->uiPixelBar->visible() )
            H += p.ui->uiPixelBar->h();

        if ( p.ui->uiBottomBar->visible() )
            H += p.ui->uiBottomBar->h();


        p.frameView = (bool)uiPrefs->uiPrefsAutoFitImage->value();


        if ( uiPrefs->uiWindowFixedSize->value() )
        {
            W = (int) uiPrefs->uiWindowXSize->value();
            H = (int) uiPrefs->uiWindowYSize->value();
        }

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
        if ( H < 565 ) {
            p.frameView = true;
            H =  565;
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


        mw->resize( posX, posY, W, H );

        if ( p.frameView )
        {
            frameView();
        }
    }

    math::Vector2i TimelineViewport::_getFocus(int X, int Y ) const noexcept
    {
        TimelineViewport* self = const_cast< TimelineViewport* >( this );
        math::Vector2i pos;
        const float devicePixelRatio = self->pixels_per_unit();
        pos.x = X * devicePixelRatio;
        pos.y = h() * devicePixelRatio - 1 - Y * devicePixelRatio;
        return pos;
    }


    math::Vector2i TimelineViewport::_getFocus() const noexcept
    {
        return _getFocus( _p->event_x, _p->event_y );
    }

    void
    TimelineViewport::_updateCoords() const noexcept
    {
        TLRENDER_P();

        math::Vector2i pos;
        pos.x = ( p.mousePos.x - p.viewPos.x ) / p.viewZoom;
        pos.y = ( p.mousePos.y - p.viewPos.y ) / p.viewZoom;
        char buf[40];
        sprintf( buf, "%5d, %5d", pos.x, pos.y );
        p.ui->uiCoord->value( buf );
    }


    void TimelineViewport::_updatePixelBar() noexcept
    {
        TLRENDER_P();

        if ( !p.ui->uiPixelBar->visible() ) return;

        const imaging::Size& r = _getRenderSize();

        p.mousePos = _getFocus();

        math::Vector2i posz;
        posz.x = ( p.mousePos.x - p.viewPos.x ) / p.viewZoom;
        posz.y = ( p.mousePos.y - p.viewPos.y ) / p.viewZoom;


        float NaN = std::numeric_limits<float>::quiet_NaN();
        imaging::Color4f rgba( NaN, NaN, NaN, NaN );
        bool inside = true;
        if ( posz.x < 0 || posz.x >= r.w || posz.y < 0 || posz.y >= r.h )
            inside = false;

        if ( inside )
        {
            _readPixel( rgba );
        }

        char buf[24];
        switch( p.ui->uiAColorType->value() )
        {
        case kRGBA_Float:
            p.ui->uiPixelR->value( float_printf( buf, rgba.r ) );
            p.ui->uiPixelG->value( float_printf( buf, rgba.g ) );
            p.ui->uiPixelB->value( float_printf( buf, rgba.b ) );
            p.ui->uiPixelA->value( float_printf( buf, rgba.a ) );
            break;
        case kRGBA_Hex:
            p.ui->uiPixelR->value( hex_printf( buf, rgba.r ) );
            p.ui->uiPixelG->value( hex_printf( buf, rgba.g ) );
            p.ui->uiPixelB->value( hex_printf( buf, rgba.b ) );
            p.ui->uiPixelA->value( hex_printf( buf, rgba.a ) );
            break;
        case kRGBA_Decimal:
            p.ui->uiPixelR->value( dec_printf( buf, rgba.r ) );
            p.ui->uiPixelG->value( dec_printf( buf, rgba.g ) );
            p.ui->uiPixelB->value( dec_printf( buf, rgba.b ) );
            p.ui->uiPixelA->value( dec_printf( buf, rgba.a ) );
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

        Fl_Color c( fl_rgb_color( col[0], col[1], col[2] ) );


        // @bug: in fltk color lookup? (0 != Fl_BLACK)
        if ( c == 0 )
            p.ui->uiPixelView->color( FL_BLACK );
        else
            p.ui->uiPixelView->color( c );
        p.ui->uiPixelView->redraw();

        imaging::Color4f hsv;

        int cspace = p.ui->uiBColorType->value() + 1;

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

        p.ui->uiPixelH->value( float_printf( buf, hsv.r ) );
        p.ui->uiPixelS->value( float_printf( buf, hsv.g ) );
        p.ui->uiPixelV->value( float_printf( buf, hsv.b ) );

        mrv::BrightnessType brightness_type = (mrv::BrightnessType)
                                              p.ui->uiLType->value();
        hsv.a = calculate_brightness( rgba, brightness_type );

        p.ui->uiPixelL->value( float_printf( buf, hsv.a ) );

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
        //o.videoLevels = FromFile;  // FromFile, FullRange, LegalRange
        //o.alphaBlend = Straight;   // Straight or Premultiplied
        const Fl_Menu_Item* item =
            p.ui->uiMenuBar->find_item(_("Render/Minify Filter/Linear") );
        timeline::ImageFilter min_filter = timeline::ImageFilter::Nearest;
        if ( item->value() ) min_filter = timeline::ImageFilter::Linear;

        item = p.ui->uiMenuBar->find_item(_("Render/Magnify Filter/Linear") );
        timeline::ImageFilter mag_filter = timeline::ImageFilter::Nearest;
        if ( item->value() ) mag_filter = timeline::ImageFilter::Linear;

        o.imageFilters.minify  = min_filter;
        //o.imageFilters.magnify = mag_filter;

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

        PopupMenu* menu = p.ui->gammaDefaults;

        p.colorConfigOptions.input = input;

        const Fl_Menu_Item* w = menu->mvalue();
        if ( ! w ) w = menu->child(0);
        const char* lbl = w->label();
        const Fl_Menu_Item* t = NULL;
        const Fl_Menu_Item* c = NULL;
        if ( menu->children() > 0 ) c = menu->child(0);
        for ( ; c != w; ++c )
        {
            if ( c->flags & FL_SUBMENU ) t = c;
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
        std::cerr << "p.colorConfigOptions.fileName= "
                  << p.colorConfigOptions.fileName << "." << std::endl
                  << "p.colorConfigOptions.input= "
                  << p.colorConfigOptions.input << "." << std::endl
                  << "p.colorConfigOptions.display= "
                  << p.colorConfigOptions.display << "." << std::endl
                  << "p.colorConfigOptions.view= "
                  << p.colorConfigOptions.view << "." << std::endl
                  << "p.colorConfigOptions.look= "
                  << p.colorConfigOptions.look << "." << std::endl;
#endif
        p.ui->uiTimeline->setColorConfigOptions( p.colorConfigOptions );
        p.ui->uiTimeline->redraw(); // to refresh filmstrip if we add it
        redraw();
    }

    void
    TimelineViewport::updateDisplayOptions( int idx ) noexcept
    {
        TLRENDER_P();

        timeline::DisplayOptions d;
        if ( idx < 1 ) d = p.displayOptions[0];
        else           d = p.displayOptions[idx];

        // Get these from the toggle menus
        d.mirror.x = false;
        d.mirror.y = false;

        // Get these from color window
        d.colorEnabled = false;
        d.color.add = math::Vector3f( 0.F, 0.F, 0.F );
        d.color.brightness = math::Vector3f( 1.F, 1.F, 1.F );
        d.color.contrast = math::Vector3f( 1.F, 1.F, 1.F );
        d.color.saturation = math::Vector3f( 1.F, 1.F, 1.F );
        d.color.tint       = 0.F;
        d.color.invert     = false;


        // We toggle R,G,B,A channels from hotkeys


        d.levelsEnabled = false;
        d.levels.inLow = 0.F;
        d.levels.inHigh = 1.F;
        d.levels.outLow = 0.F;
        d.levels.outHigh = 1.F;

        float gamma = p.ui->uiGamma->value();
        if ( gamma != d.levels.gamma )
        {
            d.levels.gamma = gamma;
            d.levelsEnabled = true;
            redraw();
        }

        d.exposureEnabled = false;
        d.exposure.defog = 0.F;
        d.exposure.kneeLow = 0.F;
        d.exposure.kneeHigh = 5.F;

        float gain = p.ui->uiGain->value();
        float exposure = ( ::log(gain) / (2.0f) );
        if ( exposure != d.exposure.exposure )
        {
            d.exposure.exposure = exposure;
            d.exposureEnabled = true;
            redraw();
        }

        d.softClipEnabled = false;
        d.softClip = 0.F;

        //  @tood.  bug?   ask darby why image filters are both in display
        //                 options and in imageoptions
        const Fl_Menu_Item* item =
            p.ui->uiMenuBar->find_item(_("Render/Minify Filter/Linear") );
        timeline::ImageFilter min_filter = timeline::ImageFilter::Nearest;
        if ( item->value() ) min_filter = timeline::ImageFilter::Linear;

        item = p.ui->uiMenuBar->find_item(_("Render/Magnify Filter/Linear") );
        timeline::ImageFilter mag_filter = timeline::ImageFilter::Nearest;
        if ( item->value() ) mag_filter = timeline::ImageFilter::Linear;

        d.imageFilters.minify  = min_filter;
        d.imageFilters.magnify = mag_filter;

        _updateDisplayOptions( idx, d );
    }

    void TimelineViewport::updateVideoLayers( int idx ) noexcept
    {
        TLRENDER_P();

        const TimelinePlayer* player = getTimelinePlayer(idx);
        if ( !player ) return;

        const auto& info   = player->timelinePlayer()->getIOInfo();

        const auto& videos = info.video;

        p.ui->uiColorChannel->clear();

        std::string name;
        for ( const auto& video : videos )
        {
            if ( video.name == "A,B,G,R" ) name = "Color";
            else name = video.name;
            p.ui->uiColorChannel->add( name.c_str() );
        }

        p.ui->uiColorChannel->menu_end();
    }

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
        if ( name == "A,B,G,R" ) name = "Color";

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

}
