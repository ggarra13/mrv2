// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <memory>

#include <tlGL/Mesh.h>
#include <tlGL/OffscreenBuffer.h>
#include <tlGL/Render.h>
#include <tlGL/Shader.h>
#include <tlGL/Util.h>

#include <tlCore/Mesh.h>


#include <tlGlad/gl.h>

#include <FL/names.h>

#include <mrvCore/mrvUtil.h>
#include <mrvCore/mrvHotkey.h>
#include <mrvCore/mrvColorSpaces.h>

#include <mrvFl/mrvTimelinePlayer.h>
#include <mrvFl/mrvIO.h>

#include <mrvGL/mrvTimelineViewport.h>
#include <mrvGL/mrvTimelineViewportInline.h>

#include <mrViewer.h>

#include <glm/gtc/matrix_transform.hpp>


namespace {
    const char* kModule = "view";
    bool has_tools_grp = true, has_menu_bar = true,
        has_top_bar = true, has_bottom_bar = true,
        has_pixel_bar = true;
}


namespace mrv
{
    using namespace tl;

    TimelineViewport::TimelineViewport(
        int X, int Y, int W, int H, const char* L ) :
        Fl_Gl_Window( X, Y, W, H, L ),
        _p( new Private )
    {
        resizable(this);
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
        Fl_Gl_Window::resize( X, Y, W, H );
        if ( hasFrameView() )
        {
            frameView();
        }
    }

    void TimelineViewport::start()
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

    void TimelineViewport::end()
    {
        TLRENDER_P();
        for (const auto& i : p.timelinePlayers)
        {
            i->end();
        }
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
            return 1;
            break;
        case FL_LEAVE:
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
            _mouseMove();
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
            _mouseMove();
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
            if ( kFitScreen.match( rawkey ) )
            {
                frameView();
                return 1;
            }
            else if ( kPlayDirection.match( rawkey ) )
            {
                using timeline::Playback;
                Playback playback = p.timelinePlayers[0]->playback();

                for (const auto& i : p.timelinePlayers)
                {
                    i->setPlayback(
                        Playback::Stop == playback ?
                        Playback::Forward :
                        Playback::Stop );
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
                start();
                return 1;
            }
            else if ( kLastFrame.match( rawkey ) )
            {
                end();
                return 1;
            }
            else if ( kTogglePresentation.match( rawkey ) )
            {
                static int posX, posY, sizeX, sizeY;

                if ( p.ui->uiMain->fullscreen_active() )
                {

                    int W = p.ui->uiRegion->w();
                    int H = p.ui->uiRegion->h();

                    if ( has_menu_bar )    {
                        // Menubar MUST be 25 pixels-- for some reason
                        // it changes size
                        p.ui->uiMenuGroup->size( W, int(25) );
                        //fill_menu( p.ui->uiMenuBar );
                        p.ui->uiMenuGroup->show();
                        H -= p.ui->uiMenuGroup->h();
                    }


                    if ( has_top_bar )    {
                        // Topbar MUST be 28 pixels-- for some reason
                        // it changes size
                        p.ui->uiTopBar->size( W, int(28) );
                        p.ui->uiTopBar->show();
                    }

                    if ( has_bottom_bar)  {
                        p.ui->uiBottomBar->size( W, int(49) );
                        p.ui->uiBottomBar->show();
                    }
                    if ( has_pixel_bar )  {
                        p.ui->uiPixelBar->size( W, int(30) );
                        p.ui->uiPixelBar->show();
                    }
                    p.ui->uiToolsGroup->show();
                    p.ui->uiBottomBar->show();
                    p.ui->uiPixelBar->show();
                    p.ui->uiTopBar->show();
                    p.ui->uiMenuGroup->show();
                    Fl::check();

                    p.ui->uiRegion->init_sizes();
                    p.ui->uiRegion->layout();

                    p.ui->uiViewGroup->init_sizes();
                    p.ui->uiViewGroup->layout();

                    p.ui->uiMain->fullscreen_off();

                }
                else
                {
                    has_top_bar    = p.ui->uiTopBar->visible();
                    has_bottom_bar = p.ui->uiBottomBar->visible();
                    has_pixel_bar  = p.ui->uiPixelBar->visible();
                    has_tools_grp  = p.ui->uiToolsGroup ?
                                     p.ui->uiToolsGroup->visible() : false;

                    p.ui->uiToolsGroup->hide();
                    p.ui->uiBottomBar->hide();
                    p.ui->uiPixelBar->hide();
                    p.ui->uiTopBar->hide();
                    p.ui->uiMenuGroup->hide();
                    Fl::check();

                    p.ui->uiRegion->init_sizes();
                    p.ui->uiRegion->layout();

                    p.ui->uiViewGroup->init_sizes();
                    p.ui->uiViewGroup->layout();

                    p.ui->uiMain->fullscreen();
                }
            }
            else if ( kFullScreen.match( rawkey ) )
            {
                if ( p.ui->uiMain->fullscreen_active() )
                    p.ui->uiMain->fullscreen_off();
                else
                    p.ui->uiMain->fullscreen();
            }
            else if ( kToggleMenuBar.match( rawkey ) )
            {
                int H = p.ui->uiRegion->h();
                int W = p.ui->uiMenuGroup->w();
                if ( p.ui->uiMenuGroup->visible() ) {
                    p.ui->uiMenuGroup->hide();
                    H += p.ui->uiMenuGroup->h();
                }
                else
                {
                    //fill_menu( p.ui->uiMenuBar );
                    p.ui->uiMenuGroup->show();
                    H -= p.ui->uiMenuGroup->h();
                }
                p.ui->uiRegion->size( W, H );
                p.ui->uiRegion->layout();
                p.ui->uiRegion->redraw();
                _mouseMove();
                return 1;
            }
            else if ( kToggleTopBar.match( rawkey ) )
            {
                int H = p.ui->uiRegion->h();
                int W = p.ui->uiTopBar->w();
                // Topbar MUST be 28 pixels-- for some reason It changes size
                p.ui->uiTopBar->size( W, int(28) );
                if ( p.ui->uiTopBar->visible() )
                {
                    p.ui->uiTopBar->hide();
                    H += p.ui->uiTopBar->h();
                }
                else
                {
                    p.ui->uiTopBar->show();
                    H -= p.ui->uiTopBar->h();
                }
                p.ui->uiRegion->size( W, H );
                p.ui->uiRegion->init_sizes();
                p.ui->uiRegion->layout();
                p.ui->uiRegion->redraw();
                _mouseMove();
                return 1;
            }
            else if ( kTogglePixelBar.match( rawkey ) )
            {
                int W = p.ui->uiRegion->w();
                int H = p.ui->uiRegion->h();
                if ( p.ui->uiPixelBar->visible() )
                {
                    p.ui->uiPixelBar->hide();
                    H += p.ui->uiPixelBar->h();
                }
                else
                {
                    p.ui->uiPixelBar->show();
                    H -= p.ui->uiPixelBar->h();
                }
                p.ui->uiRegion->size( W, H );
                p.ui->uiRegion->init_sizes();
                p.ui->uiRegion->layout();
                p.ui->uiRegion->redraw();
                _mouseMove();
                return 1;
            }
            else if ( kToggleTimeline.match( rawkey ) )
            {
                int W = p.ui->uiRegion->w();
                int H = p.ui->uiRegion->h();
                if ( p.ui->uiBottomBar->visible() )
                {
                    p.ui->uiBottomBar->hide();
                    H += p.ui->uiBottomBar->h();
                }
                else
                {
                    p.ui->uiBottomBar->show();
                    H -= p.ui->uiBottomBar->h();
                }
                p.ui->uiRegion->size( W, H );
                p.ui->uiRegion->init_sizes();
                p.ui->uiRegion->layout();
                p.ui->uiRegion->redraw();
                _mouseMove();
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

        return Fl_Gl_Window::handle( event );
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
        for (const auto& i : p.timelinePlayers)
        {
            _p->videoData.push_back(i->video());
        }
        if (p.frameView)
        {
            _frameView();
        }
        redraw();
    }

    TimelinePlayer*
    TimelineViewport::getTimelinePlayer(const int index) const
    {
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
        redraw();
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
                                         const TimelinePlayer* sender )
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


    imaging::Size TimelineViewport::_getViewportSize() const
    {
        TimelineViewport* t =
            const_cast< TimelineViewport* >( this );
        return imaging::Size( t->pixel_w(), t->pixel_h() );
    }

    std::vector<imaging::Size> TimelineViewport::_getTimelineSizes() const
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

    imaging::Size TimelineViewport::_getRenderSize() const
    {
        return timeline::getRenderSize(_p->compareOptions.mode,
                                       _getTimelineSizes());
    }

    math::Vector2i TimelineViewport::_getViewportCenter() const
    {
        const auto viewportSize = _getViewportSize();
        return math::Vector2i(viewportSize.w / 2, viewportSize.h / 2);
    }

    void TimelineViewport::_frameView()
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
    }

    void TimelineViewport::resizeWindow()
    {
        TLRENDER_P();
        auto renderSize = _getRenderSize();
        if ( !renderSize.isValid() ) return;

        int W = renderSize.w;
        int H = renderSize.h;


        Fl_Double_Window* mw = p.ui->uiMain;
        int screen = mw->screen_num();
        float scale = Fl::screen_scale( screen );
        int minx, miny, maxW, maxH, posX, posY;
        Fl::screen_work_area( minx, miny, maxW, maxH, screen );

        PreferencesUI* uiPrefs = p.ui->uiPrefs;
        if ( uiPrefs->uiWindowFixedPosition->value() )
        {
            posX = (int) uiPrefs->uiWindowXPosition->value();
            posY = (int) uiPrefs->uiWindowYPosition->value();
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

        int maxX = posX + maxW;
        int maxY = posY + maxH;

        bool fit = false;

        if ( maxX > maxW ) {
            fit = true;
            W = maxW;
        }
        if ( maxY > maxH ) {
            fit = true;
            H = maxH;
        }


        if ( uiPrefs && uiPrefs->uiWindowFixedSize->value() )
        {
            W = (int) uiPrefs->uiWindowXSize->value();
            H = (int) uiPrefs->uiWindowYSize->value();
        }

        maxW = (int) (maxW / scale);
        if ( W < 690 )  W = 690;
        else if ( W > maxW )
        {
            W = maxW;
        }

        maxH =  (int) (maxH / scale);
        if ( H < 565 )  H =  565;
        else if ( H > maxH )
        {
            H = maxH;
        }

        mw->resize( posX, posY, W, H );
    }

    math::Vector2i
    TimelineViewport::_getFocus(int X, int Y ) const
    {
        TimelineViewport* self = const_cast< TimelineViewport* >( this );
        math::Vector2i pos;
        const float devicePixelRatio = self->pixels_per_unit();
        pos.x = X * devicePixelRatio;
        pos.y = h() * devicePixelRatio - 1 - Y * devicePixelRatio;
        return pos;
    }


    inline
    math::Vector2i
    TimelineViewport::_getFocus() const
    {
        return _getFocus( _p->event_x, _p->event_y );
    }

    void
    TimelineViewport::_updateCoords() const
    {
        TLRENDER_P();

        math::Vector2i pos;
        pos.x = ( p.mousePos.x - p.viewPos.x ) / p.viewZoom;
        pos.y = ( p.mousePos.y - p.viewPos.y ) / p.viewZoom;
        char buf[40];
        sprintf( buf, "%5d, %5d", pos.x, pos.y );
        p.ui->uiCoord->value( buf );
    }


    void TimelineViewport::_mouseMove()
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

        if ( inside && p.buffer )
        {
            timeline::Playback playback = p.timelinePlayers[0]->playback();
            if ( playback == timeline::Playback::Stop )
                glReadBuffer( GL_FRONT );
            else
                glReadBuffer( GL_BACK );

            glPixelStorei(GL_PACK_ALIGNMENT, 1);

            const GLenum format = GL_RGBA;
            const GLenum type = GL_FLOAT;

            glReadPixels( p.mousePos.x, p.mousePos.y, 1, 1,
                          format, type, &rgba );
        }

        switch( p.ui->uiAColorType->value() )
        {
        case kRGBA_Float:
            p.ui->uiPixelR->value( float_printf( rgba.r ).c_str() );
            p.ui->uiPixelG->value( float_printf( rgba.g ).c_str() );
            p.ui->uiPixelB->value( float_printf( rgba.b ).c_str() );
            p.ui->uiPixelA->value( float_printf( rgba.a ).c_str() );
            break;
        case kRGBA_Hex:
            p.ui->uiPixelR->value( hex_printf( rgba.r ).c_str() );
            p.ui->uiPixelG->value( hex_printf( rgba.g ).c_str() );
            p.ui->uiPixelB->value( hex_printf( rgba.b ).c_str() );
            p.ui->uiPixelA->value( hex_printf( rgba.a ).c_str() );
            break;
        case kRGBA_Decimal:
            p.ui->uiPixelR->value( dec_printf( rgba.r ).c_str() );
            p.ui->uiPixelG->value( dec_printf( rgba.g ).c_str() );
            p.ui->uiPixelB->value( dec_printf( rgba.b ).c_str() );
            p.ui->uiPixelA->value( dec_printf( rgba.a ).c_str() );
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

        p.ui->uiPixelH->value( float_printf( hsv.r ).c_str() );
        p.ui->uiPixelS->value( float_printf( hsv.g ).c_str() );
        p.ui->uiPixelV->value( float_printf( hsv.b ).c_str() );

        mrv::BrightnessType brightness_type = (mrv::BrightnessType)
                                              p.ui->uiLType->value();
        hsv.a = calculate_brightness( rgba, brightness_type );

        p.ui->uiPixelL->value( float_printf( hsv.a ).c_str() );

        _updateCoords();
    }

    void
    TimelineViewport::updateDisplayOptions( int idx )
    {
        TLRENDER_P();

        idx = 0;  // @todo: deal with all displayoptions when -1
        if ( p.displayOptions.empty() )
        {
            LOG_ERROR( "empty display options" );
            return;
        }
        timeline::DisplayOptions d;
        float gamma = p.ui->uiGamma->value();
        if ( gamma != d.levels.gamma )
        {
            d.levels.gamma = gamma;
            d.levelsEnabled = true;
            redraw();
        }
        float gain = p.ui->uiGain->value();
        float exposure = ( ::log(gain) / (2.0f) );
        if ( exposure != d.exposure.exposure )
        {
            d.exposure.exposure = exposure;
            d.exposureEnabled = true;
            redraw();
        }

        if ( idx < 0 )
        {
            for( auto& display : p.displayOptions )
            {
                display = d;
            }
        }
        else
        {
            p.displayOptions[idx] = d;
        }
    }
}
